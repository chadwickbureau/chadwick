/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of event parser
 * 
 * This file is part of Chadwick, a library for baseball play-by-play and stats
 * Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parse.h"

/**************************************************************************
 * Data access on CWParsedEvent objects
 **************************************************************************/

int
cw_event_is_batter(CWParsedEvent *event)
{
  /* This takes advantage of the event numberings; logic would need changed
   * should those number assignments change
   */
  return (event->event_type == CW_EVENT_GENERICOUT ||
	  event->event_type == CW_EVENT_STRIKEOUT ||
	  (event->event_type >= CW_EVENT_WALK &&
	   event->event_type <= CW_EVENT_HOMERUN));
}

int
cw_event_is_official_ab(CWParsedEvent *event)
{
  if (!cw_event_is_batter(event)) {
    return 0;
  }

  if (event->sh_flag || event->sf_flag ||
      event->event_type == CW_EVENT_WALK ||
      event->event_type == CW_EVENT_INTENTIONALWALK ||
      event->event_type == CW_EVENT_HITBYPITCH ||
      event->event_type == CW_EVENT_INTERFERENCE) {
    return 0;
  }

  return 1;
}

int 
cw_event_runner_put_out(CWParsedEvent *event, int base)
{
  return ((event->play[base][0] != '\0' &&
	   !strstr(event->play[base], "E")) ? 1 : 0);
}

int
cw_event_outs_on_play(CWParsedEvent *event)
{
  return (cw_event_runner_put_out(event, 0) + 
	  cw_event_runner_put_out(event, 1) +
	  cw_event_runner_put_out(event, 2) + 
	  cw_event_runner_put_out(event, 3));
}

int
cw_event_runs_on_play(CWParsedEvent *event)
{
  return (((event->advance[0] >= 4) ? 1 : 0) +
	  ((event->advance[1] >= 4) ? 1 : 0) +
	  ((event->advance[2] >= 4) ? 1 : 0) +
	  ((event->advance[3] >= 4) ? 1 : 0));
}

int
cw_event_rbi_on_play(CWParsedEvent *event)
{
  return (((event->rbi_flag[0] > 0) ? 1 : 0) +
	  ((event->rbi_flag[1] > 0) ? 1 : 0) +
	  ((event->rbi_flag[2] > 0) ? 1 : 0) +
	  ((event->rbi_flag[3] > 0) ? 1 : 0));
}

/*
 * CWParserState holds the state of the parser as it proceeds through
 * the event string.
 * - 'sym' holds the last-read character from input_string;
 * - input_pos stores the index of the next character to be read
 * - 'token' contains the last-read token; see the various routines
 *   below for how this is used
 */
typedef struct {
  char sym;
  char *inputString;
  unsigned int inputPos;
  char token[20];
} CWParserState;

/*
 * Private auxiliary function to initialize parser state
 */
static void
cw_parse_initialize(CWParserState *state, char *input)
{
  unsigned int i;
  char *c;

  state->inputString = (char *) malloc(sizeof(char) * (strlen(input) + 1));
  strcpy(state->inputString, input);
  for (i = 0; i <= strlen(state->inputString); i++) {
    if (islower(state->inputString[i]))  {
      state->inputString[i] = toupper(state->inputString[i]);
    }
  }

  /* Preprocessing to turn SBH and CSH strings into SB4 and CS4 */
  if ((c = strstr(state->inputString, "SBH"))) {
    *(c+2) = '4';
  }
  if ((c = strstr(state->inputString, "CSH")) &&
      !strstr(state->inputString, "FCSH")) {
    *(c+2) = '4';
  }
     
  state->sym = state->inputString[0];
  state->inputPos = 1;
}  

/*
 * Private auxiliary function to cleanup parser
 */
static void 
cw_parse_cleanup(CWParserState *state)
{
  free(state->inputString);
  state->inputString = NULL;
}

/*
 * Advance parser to the next character
 */
static char 
cw_parse_nextsym(CWParserState *state)
{
  if (state->inputPos > strlen(state->inputString)) {
    state->sym = 0;
  }
  else {
    /* There are instances of the uncertain play character '#'
     * or the great play character '!' appearing inside play strings.
     * Since these are largely for human consumption (and the program
     * doesn't do anything with the data), we ignore them here.
     */
    do {
      state->sym = state->inputString[state->inputPos++];
    } while (state->sym == '#' || state->sym == '!');
  }
  return state->sym;
}

/*
 * Report a parse error; returns 0 (so as to be able to call it in
 * 'return cw_parse_error(state)' conveniently 
 */
static int
cw_parse_error(CWParserState *state)
{
  return 0;
}

/*
 * isfielder -- return 1 if parameter corresponds to a fielder, 0 if not
 *              This function accepts '?' as a fielder
 *              (this is a deprecated feature of the notation)
 */
static int isfielder(char fielder)
{ return ((fielder >= '1' && fielder <= '9') || fielder == '?'); }


/*
 * Private auxiliary function to initialize event fields
 */
static void
cw_parse_event_initialize(CWParsedEvent *event)
{
  int i;
  event->event_type = CW_EVENT_UNKNOWN;
  for (i = 0; i <= 3; i++) {
    event->advance[i] = 0;
    event->rbi_flag[i] = 0;
    event->fc_flag[i] = 0;
    strcpy(event->play[i], "");
  }
  for (i = 1; i <= 3; i++) {
    event->sb_flag[i] = 0;
    event->cs_flag[i] = 0;
    event->po_flag[i] = 0;
  }
  event->sh_flag = 0;
  event->sf_flag = 0;
  event->dp_flag = 0;
  event->gdp_flag = 0;
  event->tp_flag = 0;
  event->wp_flag = 0;
  event->pb_flag = 0;
  event->foul_flag = 0;
  event->bunt_flag = 0;
  event->fielded_by = 0;
  event->num_putouts = 0;
  event->num_assists = 0;
  event->num_errors = 0;
  for (i = 0; i < 3; i++) {
    event->putouts[i] = 0;
  }
  for (i = 0; i < 10; i++) {
    event->assists[i] = 0;
    event->errors[i] = 0;
    event->error_types[i] = 'F';
  }
  event->batted_ball_type = ' ';
  strcpy(event->hit_location, "");
}

/*
 * Read in a primary event (a string of consecutive letters)
 */
static void
cw_parse_primary_event(CWParserState *state)
{
  char *c = state->token;

  while (state->sym >= 'A' && state->sym <= 'Z') {
    *(c++) = state->sym;
    cw_parse_nextsym(state);
  }

  *c = '\0';
}

/*
 * parse_hit_fielder: read and process fielded by for base hit
 *
 * at entry: state->sym should be digit beginning fielder list
 * at exit:  state->sym points to first character after fielder list
 *
 * Notes:
 * - Using bevent convention, fielded by is set to first fielder listed
 * - Accepts question mark as a valid fielder, but sets fielded by to zero
 *   if question mark leads fielder list
 */

static void parse_hit_fielder(CWParserState *state, CWParsedEvent *event)
{
  if (isdigit(state->sym)) {
    event->fielded_by = state->sym - '0';
  }

  while (isfielder(state->sym)) {
    cw_parse_nextsym(state);
  }
}

/*
 * parse_fielding_credit: read a fielding credit, assign PO/A/E as appropriate
 *
 * at entry: state->sym should be digit beginning fielding credit
 * at exit:  state->sym points to first character after fielding credit
 *           returns 0 if batter out, nonzero if batter safe on muffed throw
 * Notes:
 * - Accepts question mark as a valid fielder, but does not issue putouts
 *   or assists when question mark is used.
 * - prev is the "previous" fielder, as in cases like '64(1)3/GDP';
 *   here, prev would equal '4' on the second call, to essentially turn
 *   the string into '64(1)43/GDP'
 * - Will handle strings starting in 'E'
 * - Question marks for missing fielders are saved in the token field;
 *   this is a difference from bevent's behavior
 */
static int parse_fielding_credit(CWParserState *state, CWParsedEvent *event,
				 char prev)
{
  char *play = state->token;
  char lastChar = state->sym;
  /* These track assists on this play; players can only receive one
   * assist per play */
  int assists[10], num_assists = 0;
  int i, j;

  if (state->sym == 'E') {
    cw_parse_nextsym(state);
    if (!isfielder(state->sym)) {
      return cw_parse_error(state);
    }
    if (isdigit(state->sym)) {
      event->errors[event->num_errors] = state->sym - '0';
      event->error_types[event->num_errors++] = 'F';
    }
    *(play++) = 'E';
    *(play++) = state->sym;
    *(play) = '\0';
    cw_parse_nextsym(state);
    return 1;
  }
      

  if (prev != ' ' && prev != state->sym) {
    assists[num_assists++] = prev - '0';
    *(play++) = prev;
  }

  *(play++) = state->sym;

  while (1) {
    cw_parse_nextsym(state);

    if ((state->sym >= '1' && state->sym <= '9') ||
	state->sym == '?') {
      if (isdigit(lastChar)) {
	assists[num_assists++] = lastChar - '0';
      }
      if (state->sym != '?') {
	*(play++) = state->sym;
      }
      lastChar = state->sym;
    }
    else if (state->sym == 'E') {
      if (isdigit(lastChar)) {
	assists[num_assists++] = lastChar - '0';
      }
      *(play++) = 'E';
      cw_parse_nextsym(state);
      if (!isdigit(state->sym)) {
	return cw_parse_error(state);
      }
      event->errors[event->num_errors] = state->sym - '0';
      event->error_types[event->num_errors++] = 'F';
      *(play++) = state->sym; 
      *(play) = '\0';
      cw_parse_nextsym(state);
      
      for (i = 0; i < num_assists; i++) {
	for (j = 0; j < i; j++) {
	  if (assists[j] == assists[i]) break;
	}

	if (j < i)  continue;

	event->assists[event->num_assists++] = assists[i];
      }

      return 1;
    }
    else {
      if (isdigit(lastChar)) {
	event->putouts[event->num_putouts++] = lastChar - '0';
      }
      *(play) = '\0';

      for (i = 0; i < num_assists; i++) {
	for (j = 0; j < i; j++) {
	  if (assists[j] == assists[i]) break;
	}

	if (j < i)  continue;
	
	event->assists[event->num_assists++] = assists[i];
      }
      return 0;
    }
  }
}

/*
 * parse_flag: read in a flag
 *
 * at entry: state->sym should be '/'
 * at exit:  state->sym points to first character after flag
 *           text of flag (without the slash) stored in state->token
 */
static void parse_flag(CWParserState *state)
{
  char *c = state->token;

  while (1) {
    cw_parse_nextsym(state);
    if (state->sym != '/' && state->sym != '.' && 
	state->sym != '(' && state->sym != ')' &&
	state->sym != '#' && state->sym != '!' && 
	state->sym != '+' && state->sym != '-' &&
	state->sym != 0) {
      (*c++) = state->sym;
    }
    else {
      break;
    }
  }
  
  *c = '\0';
}

static int parse_advance_modifier(CWParserState *state, CWParsedEvent *event,
				  int safe, int baseFrom, int baseTo)
{
  int i;

  if (isfielder(state->sym) || state->sym == 'E') {
    if (parse_fielding_credit(state, event, ' ')) {
      if (!safe) {
	safe = 1;
	if (event->advance[baseFrom] < 5) {
	  /* This guards against things like 3XH(UR)(5E2).  The (UR)
	   * already implies the runner must be safe, so the advancement
	   * will already be set.
	   */
	  event->advance[baseFrom] = baseTo;
	}
	if (baseFrom == 0 && event->event_type == CW_EVENT_STRIKEOUT) {
	  /* Special case: for K.BX1(2E3) and the like, need to remove
	   * the implied putout for the catcher (which is always
	   * listed first in the putout list 
	   */
	  event->putouts[0] = event->putouts[1];
	  event->putouts[1] = event->putouts[2];
	  event->putouts[2] = 0;
	  event->num_putouts--;
	}
	for (i = baseFrom; i >= 0; i--) {
	  event->rbi_flag[i] = -1;
	}
      }
    }
    if (state->token[0] != 'E') {
      strncpy(event->play[baseFrom], state->token, 20);
    }
    else {
      for (i = baseFrom; i >= 0; i--) {
	event->rbi_flag[i] = -1;
      }
    }

    if (state->sym == '/') {
      parse_flag(state);
      if (!strcmp(state->token, "TH") ||
	  !strcmp(state->token, "TH1") ||
	  !strcmp(state->token, "TH2") ||
	  !strcmp(state->token, "TH3") ||
	  !strcmp(state->token, "THH")) {
	event->error_types[event->num_errors - 1] = 'T';
      }
      else if (!strcmp(state->token, "INT")) {
	/* silently accept interference flag */
      }
      else if (!strcmp(state->token, "G")) {
	/* FC5.2X3(5/G) appears in 81TEX.EVA; accept silently */
      }
      else if (!strcmp(state->token, "AP")) {
	/* 1X3(15/AP) appears in 2005SFN.EVN; appeal play? */
      }	
      else {
	return cw_parse_error(state);
      }
    }

    if (state->sym == '(') {
      cw_parse_nextsym(state);
      if (!parse_advance_modifier(state, event, safe,
				  baseFrom, baseTo)) {
	return 0;
      }
    }

    /* This loop is here to tolerate weird things, like '2XH(9S)', which
     * appears in 1989 files */
    while (state->sym != ')' && state->sym != '\0') {
      cw_parse_nextsym(state);
    }
  }
  else {
    cw_parse_primary_event(state);

    /* (NORBI) is archaic; only appears in two places in 1983 files */
    if (!strcmp(state->token, "NR") || !strcmp(state->token, "NORBI")) {
      event->rbi_flag[baseFrom] = 0;
    }
    else if (!strcmp(state->token, "RBI") && 
	     event->advance[baseFrom] >= 4) {
      /* rbi_flag == 2 means (RBI) is actually present, overriding some
       * rare cases of ambiguity */
      event->rbi_flag[baseFrom] = 2;
    }
    else if (!strcmp(state->token, "UR")) {
      event->advance[baseFrom] = 5;
    }
    else if (!strcmp(state->token, "TUR")) {
      event->advance[baseFrom] = 6;
    }
    else if (!strcmp(state->token, "WP")) {
      event->wp_flag = 1;
    }
    else if (!strcmp(state->token, "PB")) {
      event->pb_flag = 1;
    }
    else if (!strcmp(state->token, "TH")) {
      if (state->sym >= '1' && state->sym <= '3') {
	cw_parse_nextsym(state);
      }
      /* just silently accept throw flags for now */
    }
    else if (!strcmp(state->token, "THH")) {
      /* just silently accept throw flags for now */
    }
    else if (!strcmp(state->token, "INT")) {
      /* silently accept interference flag */
    }
    else {
      return cw_parse_error(state);
    }
  }

  if (state->sym == ')') {
    cw_parse_nextsym(state);
    return 1;
  }
  else {
    return cw_parse_error(state);
  }
}

static char locations[][20] = {
  "1", "13", "15", "1S", "2", "2F", "23", "23F", "25", "25F",
  "3SF", "3F", "3DF", "3S", "3", "3D", "34S", "34", "34D",
  "4S", "4", "4D", "4MS", "4M", "4MD",
  "6MS", "6M", "6MD", "6S", "6", "6D",
  "56S", "56", "56D", "5S", "5", "5D", "5SF", "5F", "5DF",
  "7LSF", "7LS", "7S", "78S", "8S", "89S", "9S", "9LS", "9LSF",
  "7LF", "7L", "7", "78", "8", "89", "9", "9L", "9LF",
  "7LDF", "7LD", "7D", "78D", "8D", "89D", "9D", "9LD", "9LDF",
  "78XD", "8XD", "89XD",
  /* the following locations are nonstandard or archaic, but 
   * appear in existing Retrosheet data */
  "2LF", "3L", "46", "5L", 
  "7LDW", "7DW", "78XDW", "8XDW", "89XDW", "9DW", "9LDW",
  "2R", "7M",  /* where is 7M !?! */
  ""
};

static void parse_flags(CWParserState *state, CWParsedEvent *event)
{
  char flag[256];

  do {
    strcpy(flag, "/");

    do {
      cw_parse_nextsym(state);
      if (state->sym != '/' && state->sym != '.' && 
	  state->sym != '#' && state->sym != '!' && 
	  state->sym != '+' && state->sym != '-' && state->sym != 0) {
	strncat(flag, &(state->sym), 1); 
      }
    } while (state->sym != '/' && state->sym != '.' && 
	     state->sym != '#' && state->sym != '!' && 
	     state->sym != '+' && state->sym != '-' && state->sym != 0);

    if (!strcmp(flag, "/SH") || !strcmp(flag, "/SAC")) {
      event->sh_flag = 1;
      event->bunt_flag = 1;
    }
    else if (!strcmp(flag, "/SF")) {
      event->sf_flag = 1;
      if (event->batted_ball_type == ' ') {
	event->batted_ball_type = 'F';
      }
    }
    else if (!strcmp(flag, "/DP")) {
      event->dp_flag = 1;
    }
    else if (!strcmp(flag, "/GDP")) {
      event->dp_flag = 1;
      event->gdp_flag = 1;
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(flag, "/LDP")) {
      event->dp_flag = 1;
      event->batted_ball_type = 'L';
    }
    else if (!strcmp(flag, "/FDP")) {
      event->dp_flag = 1;
      event->batted_ball_type = 'F';
    }
    else if (!strcmp(flag, "/BGDP")) {
      event->bunt_flag = 1;
      event->dp_flag = 1;
      event->gdp_flag = 1;
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(flag, "/BPDP")) {
      event->bunt_flag = 1;
      event->dp_flag = 1;
      event->batted_ball_type = 'P';
    }
    else if (!strcmp(flag, "/TP")) {
      event->tp_flag = 1;
    }
    else if (!strcmp(flag, "/GTP")) {
      event->tp_flag = 1;
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(flag, "/LTP")) {
      event->tp_flag = 1;
      event->batted_ball_type = 'L';
    }
    else if (!strcmp(flag, "/FL")) {
      event->foul_flag = 1;
    }
    else if (!strcmp(flag, "/FO") && event->batted_ball_type == ' ') {
      event->batted_ball_type = 'G';
    }
    else if ((!strcmp(flag, "/TH") || !strcmp(flag, "/TH1") ||
	      !strcmp(flag, "/TH2") || !strcmp(flag, "/TH3") ||
	      !strcmp(flag, "/THH")) && 
	     (event->event_type == CW_EVENT_ERROR ||
	      event->event_type == CW_EVENT_PICKOFFERROR)) {
      event->error_types[0] = 'T';
    }
    else if (!strcmp(flag, "/B")) {
      event->bunt_flag = 1;
    }
    else if (!strcmp(flag, "/BG")) {
      event->bunt_flag = 1;
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(flag, "/BP")) {
      event->bunt_flag = 1;
      event->batted_ball_type = 'P';
    }
    else if (!strcmp(flag, "/BF")) {
      event->bunt_flag = 1;
      event->batted_ball_type = 'F';
    }
    else if (!strcmp(flag, "/BL")) {
      event->bunt_flag = 1;
      event->batted_ball_type = 'L';
    }
    else if (!strcmp(flag, "/P")) {
      event->batted_ball_type = 'P';
    }
    else if (!strcmp(flag, "/F")) {
      event->batted_ball_type = 'F';
    }
    else if (!strcmp(flag, "/G")) {
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(flag, "/L")) {
      event->batted_ball_type = 'L';
    }
    else if (strlen(flag) >= 3) {
      char traj = (flag[1] == 'B') ? flag[2] : flag[1];
      char *loc = (flag[1] == 'B') ? flag + 3 : flag + 2;
      if (traj == 'G' || traj == 'F' || traj == 'P' ||
	  traj == 'L') {
	int i = 0;
	for (i = 0; strcmp(locations[i], ""); i++) {
	  if (!strcmp(locations[i], loc)) {
	    event->batted_ball_type = traj;
	    strcpy(event->hit_location, locations[i]);
	    if (locations[i][strlen(locations[i]) - 1] == 'F') {
	      event->foul_flag = 1;
	    }
	    if (flag[1] == 'B') {
	      event->bunt_flag = 1;
	    }
	    break;
	  }
	}
      }
      else {
	char *loc = (flag[1] == 'B') ? flag + 2 : flag + 1;
	int i = 0;
	for (i = 0; strcmp(locations[i], ""); i++) {
	  if (!strcmp(locations[i], loc)) {
	    strcpy(event->hit_location, locations[i]);
	    if (locations[i][strlen(locations[i]) - 1] == 'F') {
	      event->foul_flag = 1;
	    }
	    if (flag[1] == 'B') {
	      event->bunt_flag = 1;
	    }
	    break;
	  }
	}
      }
    }
    else {
      int i = 0;
      for (i = 0; strcmp(locations[i], ""); i++) {
	if (!strcmp(locations[i], flag + 1)) {
	  strcpy(event->hit_location, locations[i]);
	  break;
	}
      }
    }
  } while (state->sym != '.' && state->sym != 0);
}

/*
 * parse_balk: process a balk event
 *
 * at entry: state->sym should be character after 'BK' token
 * at exit:  state->sym points to first character after 'BK' token
 *
 * Notes:
 * - Nothing to do here really; balks shouldn't take flags, etc.
 */
static int parse_balk(CWParserState *state, CWParsedEvent *event, int flags)
{
  return 1;
}

static int parse_stolen_base(CWParserState *state, CWParsedEvent *event, 
			     int flags)
{
  if (state->sym == '2') {
    event->sb_flag[1] = 1;
    event->advance[1] = 2;
    cw_parse_nextsym(state);
  }
  else if (state->sym == '3') {
    event->sb_flag[2] = 1;
    event->advance[2] = 3;
    cw_parse_nextsym(state);
  }
  else if (state->sym == '4') {
    /* SBH is converted to SB4 in initialization */
    event->sb_flag[3] = 1;
    event->advance[3] = 4;
    cw_parse_nextsym(state);
    /* special case: accept archaic SBH(UR) or SBH(TUR) */
    if (state->sym == '(') {
      event->advance[3] = 5;
      cw_parse_nextsym(state);
      if (state->sym == 'T') {
	event->advance[3] = 6;
	cw_parse_nextsym(state);
      }
      if (state->sym != 'U') {
	return cw_parse_error(state);
      }
      cw_parse_nextsym(state);
      if (state->sym != 'R') {
	return cw_parse_error(state);
      }
      cw_parse_nextsym(state);
      if (state->sym != ')') {
	return cw_parse_error(state);
      }
      cw_parse_nextsym(state);
    }
  }
  else {
    return cw_parse_error(state);
  }

  if (state->sym == ';') {
    cw_parse_nextsym(state);
    cw_parse_primary_event(state);

    if (!strcmp(state->token, "SB")) {
      parse_stolen_base(state, event, 0);
    }
    else {
      return cw_parse_error(state);
    }
  }

  while (flags && state->sym == '/') {
    parse_flag(state);

    if (!strcmp(state->token, "INT")) {
      /* accept /INT flag silently */
    }
    else {
      return cw_parse_error(state);
    }
  }

  return 1;
}

static int parse_caught_stealing(CWParserState *state, CWParsedEvent *event,
				 int flags)
{
  int runner;

  if (state->sym >= '2' && state->sym <= '4') {
    /* CSH is converted to CS4 in initialization */
    event->cs_flag[runner = state->sym - '1'] = 1;
  }
  else {
    return cw_parse_error(state);
  }
    
  while (cw_parse_nextsym(state) == '(') {
    cw_parse_nextsym(state);
    if (isfielder(state->sym) || state->sym == 'E') {
      if (parse_fielding_credit(state, event, ' ')) {
	event->advance[runner] = runner + 1;
      }
      strcpy(event->play[runner], state->token);
    }
    else if (isalpha(state->sym)) {
      cw_parse_primary_event(state);

      if (!strcmp(state->token, "UR") && event->advance[runner] == 4) {
	event->advance[runner] = 5;
      }
      else if (!strcmp(state->token, "TUR") &&
	       event->advance[runner] == 4) {
	event->advance[runner] = 6;
      }
      else {
	return cw_parse_error(state);
      }

      if (state->sym != ')') {
	return cw_parse_error(state);
      }
    }
  }

  if (state->sym == ';') {
    /* Two caught stealings can happen, though they're rare */
    cw_parse_nextsym(state);
    cw_parse_primary_event(state);

    if (!strcmp(state->token, "CS")) {
      parse_caught_stealing(state, event, 0);
    }
    else {
      return cw_parse_error(state);
    }
  }

  while (flags && state->sym == '/') {
    parse_flag(state);

    if (!strcmp(state->token, "DP")) {
      event->dp_flag = 1;
    }
    else if (!strcmp(state->token, "TH") ||
	     !strcmp(state->token, "TH1") ||
	     !strcmp(state->token, "TH2") ||
	     !strcmp(state->token, "TH3") ||
	     !strcmp(state->token, "THH")) {
      /* silently accept a throw flag; there is an instance of
       * a POCSn(nnEn)/TH1 in 1968 */
    }
    else if (!strcmp(state->token, "INT")) {
      /* silently accept interference flag */
    }
    else {
      return cw_parse_error(state);
    }
  }

  return 1;
}

/*
 * parse_safe_on_error: process an error event
 *
 * at entry: state->sym should be character after 'E' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Does not accept question mark for fielder
 * - Generates a fielded by credit (since that's what bevent does)
 * - Assumes infielders make errors on grounders, outfielders on flies
 *   (this is currently a bug in bevent, which puts a batted ball type
 *   of 'G' for all errors)
 * - Implied advancement of batter is to first base
 */
static int parse_safe_on_error(CWParserState *state,
			       CWParsedEvent *event, int flags)
{
  event->advance[0] = 1;
  
  if (state->sym < '1' || state->sym > '9') {
    return cw_parse_error(state);
  }

  event->errors[event->num_errors] = (state->sym - '0');
  event->error_types[event->num_errors++] = 'F';
  event->fielded_by = (state->sym - '0');
  event->batted_ball_type = (state->sym <= '6') ? 'G' : 'F';
  cw_parse_nextsym(state);

  /* Special case: writing En? for really bad play */
  if (state->sym == '?') {
    cw_parse_nextsym(state);
  }

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }
  return 1;
}

/*
 * parse_fielders_choice: process a fielder's choice event
 *
 * at entry: state->sym should be character after 'FC' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Fielder's choice plays are assumed to be grounders unless specified
 * - Implied advance for batters is first base 
 */
static int parse_fielders_choice(CWParserState *state, CWParsedEvent *event,
				 int flags)
{
  event->advance[0] = 1;
  event->batted_ball_type = 'G';

  if (state->sym >= '1' && state->sym <= '9') {
    event->fielded_by = (state->sym - '0');
    cw_parse_nextsym(state);
  }
  else if (state->sym == '?') {
    cw_parse_nextsym(state);
  }

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }

  return 1;
}

/*
 * parse_sac_fielders_choice: process a sac fielder's choice event (archaic)
 *
 * at entry: state->sym should be character after 'FCSH' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Archaic; this function sets the bunt and sac flags, then hands off
 *   to parse_fielders_choice() to do the actual work
 */
static int parse_sac_fielders_choice(CWParserState *state, CWParsedEvent *event,
				     int flags)
{
  event->bunt_flag = 1;
  event->sh_flag = 1;
  return parse_fielders_choice(state, event, flags);
}

/*
 * parse_foul_error: process a foul ball error event
 *
 * at entry: state->sym should be character after 'FLE' token
 * at exit:  state->sym points to end of string
 *
 * Notes:
 * - Foul flag is not set, since bevent does not set it (?)
 * - Ball fielded is generated, since bevent generates it
 */
static int parse_foul_error(CWParserState *state, CWParsedEvent *event,
			    int flags)
{
  if (state->sym >= '1' && state->sym <= '9') {
    event->errors[event->num_errors] = (state->sym - '0');
    event->error_types[event->num_errors++] = 'F'; 
    event->fielded_by = (state->sym - '0');
    cw_parse_nextsym(state);
  }
  else {
    return cw_parse_error(state);
  }

  if (flags && state->sym == '/') {
    /* Most likely a trajectory code */
    parse_flags(state, event);
  }

  return 1;
}

/*
 * Parse the parenthesis notation for a force play.
 * Returns the base parsed (zero for batter), or a negative
 * number if an error occurs
 */
static int parse_out_base(CWParserState *state)
{
  int base;

  cw_parse_nextsym(state);
  if (state->sym!= '1' && state->sym!= '2' && state->sym!= '3' && state->sym!= 'B') {
    cw_parse_error(state);
    return -1;
  }
  base = (state->sym== 'B') ? 0 : (state->sym- '0');
  
  cw_parse_nextsym(state);
  if (state->sym != ')') {
    cw_parse_error(state);
    return -1;
  }
  cw_parse_nextsym(state);

  return base;
}

static int parse_generic_out(CWParserState *state, CWParsedEvent *event,
			     int flags)
{
  /* lastFielder keeps track of the fielder who made the previous putout,
     so as to generate correct credit on plays like 54(1)3/GDP */
  char lastFielder = ' ';
  int safe;

  if (state->sym != '?') {
    event->fielded_by = (state->sym - '0');
  }
  event->advance[0] = 1;
 
  while (isfielder(state->sym)) {
    safe = parse_fielding_credit(state, event, lastFielder);

    if (state->sym == '(') {
      int base = parse_out_base(state);
      if (base < 0)  return 0;
      event->advance[base] = (safe) ? base + 1 : 0;
      event->fc_flag[base] = 1;
      if (event->batted_ball_type == ' ') {
	if (strlen(state->token) > 1 || base > 0) {
	  /* Assumption: more than one fielder implies ground ball,
	     unless overriden later by a flag; also, getting the first
	     out on a non-batter implies a bounce */
	  event->batted_ball_type = 'G';
	}
	else if (strlen(state->token) == 1 && base == 0) {
	  event->batted_ball_type = 'F';
	}
      }
	 
      strncpy(event->play[base], state->token, 19);
      lastFielder = state->token[strlen(state->token) - 1];
    }
    else {
      if (strlen(state->token) > 1 || lastFielder != ' ') {
	/* Assumption: more than one fielder implies ground ball,
	   unless overriden later by a flag */
	event->batted_ball_type = 'G';
      }
      else {
	event->batted_ball_type = 'F';
      }

      strcpy(event->play[0], state->token);
      event->advance[0] = (safe) ? 1 : 0;
      break;
    }
  }

  if (state->sym == '+' || state->sym == '-') {
    /* Ignore hard/soft-hit ball modifiers */
    cw_parse_nextsym(state);
  }

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }

  return 1;
}

/*
 * parse_hit_by_pitch: process a hit-by-pitch event
 *
 * at entry: state->sym should be character after 'HP'/'HBP' token
 * at exit:  state->sym points to first character after 'HP'/'HBP' token
 *
 * Notes:
 * - Sets advancement of batter to 1, which is implied by primary event
 */
static int parse_hit_by_pitch(CWParserState *state, CWParsedEvent *event,
			      int flags)
{
  event->advance[0] = 1;
  return 1;
}

/*
 * parse_interference: process a catcher's interference event
 *
 * at entry: state->sym should be first character after 'C' token
 * at exit:  p-state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - 'C/E1' and 'C/E3' are considered legal strings (per David W. Smith);
 *   these are for cases where batter was awarded first due to interference
 *   by the pitcher or first baseman, respectively, event though these
 *   are not truly "catcher's" interference.  DWS says these are the only
 *   legal exceptions currently in the DiamondWare engine, so we will
 *   follow along with that convention here.
 */
static int parse_interference(CWParserState *state, CWParsedEvent *event,
			      int flags)
{
  event->advance[0] = 1;

  while (state->sym == '/') {
    parse_flag(state);

    if (state->token[0] == 'E' && event->num_errors > 0) {
      return cw_parse_error(state);
    }

    if (!strcmp(state->token, "E2")) {
      event->errors[event->num_errors++] = 2;
    }
    else if (!strcmp(state->token, "E1")) {
      event->errors[event->num_errors++] = 1;
    }
    else if (!strcmp(state->token, "E3")) {
      event->errors[event->num_errors++] = 3;
    }
    else if (!strcmp(state->token, "INT")) {
      /* silently accept redundant /INT flag */
    }
    else {
      /* silently accept other flags and do nothing */
      /* there exists a C/E1/INT/G4 in Retrosheet 1991 datafiles */
    }
  }

  if (event->num_errors == 0) {
    event->errors[event->num_errors++] = 2;
  }
  event->error_types[0] = 'F';

  return 1;
}

/*
 * parse_indifference: process a defensive indifference event
 *
 * at entry: state->sym should be character after 'DI' token
 * at exit:  state->sym points to first character after 'DI' token
 *
 * Notes:
 * - Nothing to do here; DI doesn't take flags
 */
static int parse_indifference(CWParserState *state, CWParsedEvent *event,
			      int flags)
{
  return 1;
}

/*
 * parse_other_advance: process an other advance/out advancing event
 *
 * at entry: state->sym should be character after 'OA'/'OBA' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - _flags are unusual with this event; /INT, /DP, and /TP are accepted,
 *   though only /INT has occurred so far in Retrosheet data
 */
static int parse_other_advance(CWParserState *state, CWParsedEvent *event,
			       int flags)
{
  while (flags && state->sym == '/') {
    parse_flag(state);

    if (!strcmp(state->token, "INT")) {
      /* silently accept interference flag */
    }
    else if (!strcmp(state->token, "DP")) {
      event->dp_flag = 1;
    }
    else if (!strcmp(state->token, "TP")) {
      event->tp_flag = 1;
    }
    else {
      return cw_parse_error(state);
    }
  }

  return 1;
}

/*
 * parse_passed_ball: process a passed ball event
 *
 * at entry: state->sym should be character after 'PB' token
 * at exit:  state->sym points to first character after 'PB' token
 *
 * Notes:
 * - Nothing to do here; PB doesn't take flags
 */
static int parse_passed_ball(CWParserState *state, CWParsedEvent *event,
			     int flags)
{
  event->pb_flag = 1;
  return 1;
}

/*
 * parse_pickoff_error: process an old-style pickoff error event
 *
 * at entry: state->sym should be character after 'POE' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - This is a deprecated code; preferred now is 'POn(En)'
 */
static int parse_pickoff_error(CWParserState *state, CWParsedEvent *event,
			       int flags)
{
  if (state->sym >= '1' && state->sym <= '9') {
    event->errors[event->num_errors] = (state->sym- '0');
    event->error_types[event->num_errors++] = 'F';
  }
  else {
    return cw_parse_error(state);
  }

  cw_parse_nextsym(state);

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }

  return 1;
}

/*
 * parse_pickoff_stolen_base: process a pickoff-stolen base event
 *
 * at entry: state->sym should be character after 'POSB' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 */
static int parse_pickoff_stolen_base(CWParserState *state, CWParsedEvent *event,
				     int flags)
{
  return parse_stolen_base(state, event, flags);
}

/*
 * parse_pickoff_caught_stealing: process a pickoff-caught stealing event
 * 
 * at entry: state->sym should be character after 'POCS' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 */
static int parse_pickoff_caught_stealing(CWParserState *state,
					 CWParsedEvent *event, 
					 int flags)
{
  int runner;

  if (!parse_caught_stealing(state, event, flags)) {
    return 0;
  }

  for (runner = 1; runner <= 3; runner++) {
    if (event->cs_flag[runner]) {
      event->po_flag[runner] = 1;
    }
  }

  return 1;
}

static int parse_pickoff(CWParserState *state, CWParsedEvent *event,
			 int flags)
{
  int runner;

  if (state->sym >= '1' && state->sym <= '3') {
    runner = state->sym - '0';
  }
  else {
    return cw_parse_error(state);
  }
  event->po_flag[runner] = 1;
    
  if (cw_parse_nextsym(state) != '(') {
    return cw_parse_error(state);
  }
  cw_parse_nextsym(state);
  if (isfielder(state->sym)) {
    parse_fielding_credit(state, event, ' ');
    strncpy(event->play[runner], state->token, 20);
  }
  else if (state->sym == 'E') {
    /* going to leave this commented to match hevent output for now */
    /*    event->event_type = CW_EVENT_PICKOFFERROR;  */
    parse_fielding_credit(state, event, ' ' );
    strncpy(event->play[runner], state->token, 20);

    if (state->sym == '/') {
      parse_flag(state);
      if (!strcmp(state->token, "TH") ||
	  !strcmp(state->token, "TH1") ||
	  !strcmp(state->token, "TH2") ||
	  !strcmp(state->token, "TH3") ||
	  !strcmp(state->token, "THH")) {
	event->error_types[event->num_errors - 1] = 'T';
      }
      else {
	return cw_parse_error(state);
      }
    }
  }
  else {
    return cw_parse_error(state);
  }

  if (state->sym == ')') {
    cw_parse_nextsym(state);
  }
  else {
    return cw_parse_error(state);
  }

  if (flags && state->sym == '/') {
    /* Most likely flag is /DP */
    parse_flags(state, event);
  }

  return 1;
}

/*
 * parse_sac_hit_error: process a sac hit error event (archaic)
 *
 * at entry: state->sym should be character after 'SHE' token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - This is an archaic primary event; this function sets the bunt and
 *   sac hit flags, then hands off to parse_error(), since the syntax is
 *   identical after this point.
 */
static int parse_sac_hit_error(CWParserState *state, CWParsedEvent *event,
			       int flags)
{
  event->bunt_flag = 1;
  event->sh_flag = 1;
  return parse_safe_on_error(state, event, flags);
}

/*
 * parse_base_hit: process a base hit event ('S', 'D', 'T', 'H', 'HR')
 *
 * at entry: state->sym should be character after hit token
 * at exit:  state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 */
static int parse_base_hit(CWParserState *state, CWParsedEvent *event,
			  int flags)
{
  if (isfielder(state->sym)) {
    parse_hit_fielder(state, event);
  }

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }

  return 1;
}

static int parse_ground_rule_double(CWParserState *state, CWParsedEvent *event,
				    int flags)
{
  if (state->sym >= '1' && state->sym <= '9') {
    /* Some newer event files have fielders after the DGR, which seems
     * like it ought not be possible -- but oh well!
     * bevent does not give a 'fielded by' credit for this case.
     */
    cw_parse_nextsym(state);
  }

  if (flags && state->sym == '/') {
    parse_flags(state, event);
  }
  
  return 1;
}

static int parse_strikeout(CWParserState *state, CWParsedEvent *event,
			   int flags)
{
  if (state->sym >= '1' && state->sym <= '9') {
    event->advance[0] = ((parse_fielding_credit(state, event, ' ')) ?
			     1 : 0);
    strcpy(event->play[0], state->token);
  } 
  else {
    /* just a bare strikeout */
    strcpy(event->play[0], "2");
    event->putouts[event->num_putouts++] = 2;
  }

  if (state->sym == '+') {
    cw_parse_nextsym(state);
    cw_parse_primary_event(state);

    if (!strcmp(state->token, "WP")) {
      event->wp_flag = 1;
    }
    else if (!strcmp(state->token, "PB")) {
      event->pb_flag = 1;
    }
    else if (!strcmp(state->token, "PO")) {
      parse_pickoff(state, event, 0);
    }
    else if (!strcmp(state->token, "POCS")) {
      parse_pickoff_caught_stealing(state, event, 0);
    }
    else if (!strcmp(state->token, "POSB")) {
      parse_pickoff_stolen_base(state, event, 0);
    }
    else if (!strcmp(state->token, "SB")) {
      parse_stolen_base(state, event, 0);
    }
    else if (!strcmp(state->token, "CS")) {
      parse_caught_stealing(state, event, 0);
    }
    else if (!strcmp(state->token, "DI")) {
      parse_indifference(state, event, 0);
    }
    else if (!strcmp(state->token, "OA") ||
	     !strcmp(state->token, "OBA")) {
      /* we don't need to do anything special for K+OA */
    }
    else if (!strcmp(state->token, "E")) {
      if (state->sym < '1' || state->sym > '9') {
	return cw_parse_error(state);
      }
      event->errors[event->num_errors] = state->sym - '0';
      event->error_types[event->num_errors++] = 'F';
      cw_parse_nextsym(state);
    }
    else if (!strcmp(state->token, "")) {
      /* 'K+/23' appears in 1988 files; just accept this silently */
    }
    else {
      return cw_parse_error(state);
    }
  }

  while (flags && state->sym == '/') {
    parse_flag(state);

    if ((!strcmp(state->token, "TH") ||
	 !strcmp(state->token, "TH1") ||
	 !strcmp(state->token, "TH2") ||
	 !strcmp(state->token, "TH3") ||
	 !strcmp(state->token, "THH")) && event->num_errors > 0) {
      event->error_types[0] = 'T';
    }
    else if (!strcmp(state->token, "DP")) {
      event->dp_flag = 1;
    }
    else if (!strcmp(state->token, "TP")) {
      event->tp_flag = 1;
    }
    else if (!strcmp(state->token, "B")) {
      event->bunt_flag = 1;
      /* This is to match bevent's output.  Apparently, bevent believes
       * that K/B is reserved for foul bunt strikeouts, and not strikeouts
       * on missed bunt attempts. */
      event->batted_ball_type = 'G';
    }
    else if (!strcmp(state->token, "FL")) {
      event->foul_flag = 1;
    }
    else {
      /* Do nothing.  In theory, there shouldn't be any other flags other
       * than the list above.  In practice, there are in the Retrosheet
       * files; some are just inconsistencies in scoring, others are just
       * plain weird (K/L?).  Rather than enumerate them, best practice
       * at the moment simply seems to be to accept them silently.
       */
    }
  }

  return 1;
}

static int parse_strikeout_error(CWParserState *state, CWParsedEvent *event,
				 int flags)
{
  if (state->sym < '1' || state->sym > '9') {
    return cw_parse_error(state);
  }

  event->errors[event->num_errors] = state->sym - '0';
  event->error_types[event->num_errors++] = 'F';
  event->play[0][0] = 'E';
  event->play[0][1] = state->sym;
  event->play[0][2] = '\0';
  cw_parse_nextsym(state);
  return 1;
}

static int parse_walk(CWParserState *state, CWParsedEvent *event, int flags)
{
  event->advance[0] = 1;

  if (state->sym == '+') {
    cw_parse_nextsym(state);
    cw_parse_primary_event(state);

    if (!strcmp(state->token, "WP")) {
      event->wp_flag = 1;
    }
    else if (!strcmp(state->token, "PB")) {
      event->pb_flag = 1;
    }
    else if (!strcmp(state->token, "PO")) {
      if (!parse_pickoff(state, event, 0))  return 0;
    }
    else if (!strcmp(state->token, "POSB")) {
      if (!parse_pickoff_stolen_base(state, event, 0)) return 0;
    }
    else if (!strcmp(state->token, "POCS")) {
      if (!parse_pickoff_caught_stealing(state, event, 0)) return 0;
    }
    else if (!strcmp(state->token, "SB")) {
      if (!parse_stolen_base(state, event, 0)) return 0;
    }
    else if (!strcmp(state->token, "CS")) {
      if (!parse_caught_stealing(state, event, 0)) return 0;
    }
    else if (!strcmp(state->token, "DI")) {
      if (!parse_indifference(state, event, 0)) return 0;
    }
    else if (!strcmp(state->token, "OA") ||
	     !strcmp(state->token, "OBA")) {
      /* we don't need to do anything special for W+OA */
    }
    else if (!strcmp(state->token, "E")) {
      if (state->sym < '1' || state->sym > '9') {
	return cw_parse_error(state);
      }
      event->errors[event->num_errors] = state->sym - '0';
      event->error_types[event->num_errors++] = 'F';
      cw_parse_nextsym(state);
    }
  }

  while (flags && state->sym == '/') {
    parse_flag(state);

    if ((!strcmp(state->token, "TH") ||
	 !strcmp(state->token, "TH1") ||
	 !strcmp(state->token, "TH2") ||
	 !strcmp(state->token, "TH3") ||
	 !strcmp(state->token, "THH")) && event->num_errors > 0) {
      event->error_types[0] = 'T';
    }
    else if (!strcmp(state->token, "DP")) {
      event->dp_flag = 1;
    }
    else {
      return cw_parse_error(state);
    }
  }

  return 1;
}

/*
 * parse_wild_pitch: process a wild pitch event
 *
 * at entry: state->sym should be character after 'WP' token
 * at exit:  state->sym points to first character after 'WP' token
 *
 * Notes:
 * - Nothing to do here; WP doesn't take flags
 */
static int parse_wild_pitch(CWParserState *state, CWParsedEvent *event,
			    int flags)
{
  event->wp_flag = 1;
  return 1;
} 

static int parse_runner_advance(CWParserState *state, CWParsedEvent *event)
{
  int baseFrom = 0, baseTo = 0, safe = 0;

  if ((state->sym < '1' || state->sym > '3') && state->sym != 'B') {
    return cw_parse_error(state);
  }
  baseFrom = (state->sym == 'B') ? 0 : (state->sym - '0');

  cw_parse_nextsym(state);
  if (state->sym != '-' && state->sym != 'X') {
    return cw_parse_error(state);
  }
  safe = (state->sym == '-') ? 1 : 0;

  cw_parse_nextsym(state);
  if ((state->sym < '1' || state->sym > '3') && state->sym != 'H') {
    return cw_parse_error(state);
  }
  baseTo = (state->sym == 'H') ? 4 : (state->sym - '0');

  if (safe) {
    /* This logic takes care of possibility of plays like
     * CSH(1E2)(UR).3-H, where advancement is already implied, and
     * marked as unearned */
    if (baseTo < 4 || event->advance[baseFrom] < 4) {
      event->advance[baseFrom] = baseTo;
    }
    if (baseTo == 4 && cw_event_is_batter(event) && !event->gdp_flag &&
	(event->event_type != CW_EVENT_ERROR || baseFrom == 3) &&
	event->event_type != CW_EVENT_STRIKEOUT &&
	event->rbi_flag[baseFrom] != -1) {
      event->rbi_flag[baseFrom] = 1;
    }
  }
  else {
    event->advance[baseFrom] = 0;
    if (event->event_type == CW_EVENT_FIELDERSCHOICE) {
      event->fc_flag[baseFrom] = 1;
    }
  }
  
  cw_parse_nextsym(state);
  while (state->sym == '(') {
    cw_parse_nextsym(state);
    if (!parse_advance_modifier(state, event, safe, baseFrom, baseTo)) {
      return 0;
    }
  }
  return 1;
}

static int parse_advancement(CWParserState *state, CWParsedEvent *event)
{
  do {
    cw_parse_nextsym(state);
    if (!parse_runner_advance(state, event))  return 0;
  } while (state->sym == ';');

  return (state->sym == '\0');
}

/*
 * Cleanup and so forth
 */
void sanity_check(CWParsedEvent *event)
{
  int base;

  if (event->event_type == CW_EVENT_SINGLE &&
      event->advance[0] == 0 && 
      !strcmp(event->play[0], "")) {
    event->advance[0] = 1;
  }
  else if (event->event_type == CW_EVENT_DOUBLE &&
	   event->advance[0] == 0 && 
	   !strcmp(event->play[0], "")) {
    event->advance[0] = 2;
  }
  if (event->event_type == CW_EVENT_TRIPLE &&
      event->advance[0] == 0 && 
      !strcmp(event->play[0], "")) {
    event->advance[0] = 3;
  }
  if (event->event_type == CW_EVENT_HOMERUN &&
      event->advance[0] == 0 && 
      !strcmp(event->play[0], "")) {
    event->advance[0] = 4;
    event->rbi_flag[0] = 1;
  }

  if (event->event_type == CW_EVENT_STRIKEOUT)  {
    if (!strcmp(event->play[0], "2") && 
	event->advance[0] > 0) {
      strcpy(event->play[0], "");
      event->putouts[0] = event->putouts[1];
      event->putouts[1] = event->putouts[2];
      event->putouts[2] = 0;
      event->num_putouts--;
    }
  }

  if (event->event_type == CW_EVENT_WALK) {
    event->rbi_flag[0] = 0;
    event->rbi_flag[1] = 0;
    event->rbi_flag[2] = 0;
  }

  if (event->event_type == CW_EVENT_FOULERROR) {
    event->foul_flag = 1;
  }


  for (base = 0; base <= 3; base++) {
    if (event->rbi_flag[base] == -1) {
      event->rbi_flag[base] = 0;
    }

    if (strcmp(event->play[base], "") &&
	!strstr(event->play[base], "E")) {
      /* This patches up instances like BXH(832)(E8) */
      event->advance[base] = 0;
    }
  }

  /* The following implements rules for default batted ball types based
   * upon defensive fielding credit */
  if (event->event_type == CW_EVENT_GENERICOUT) {
    if (strlen(event->play[0]) == 1 &&
	!event->dp_flag && !event->tp_flag && !event->bunt_flag &&
	event->batted_ball_type == ' ') {
      event->batted_ball_type = 'F';
    }
    else if (strlen(event->play[0]) >= 1 && 
	     event->batted_ball_type == ' ') {
      event->batted_ball_type = 'G';
    }
  }

  if (event->event_type == CW_EVENT_SINGLE && event->bunt_flag &&
      event->batted_ball_type == ' ') {
    event->batted_ball_type = 'G';
  }

  if (event->sh_flag) {
    event->batted_ball_type = 'G';
  }
}

typedef struct {
  int event_code;
  char event_string[5];
  int (*parse_func)(CWParserState *, CWParsedEvent *, int);
} cw_parse_table_entry;

int cw_parse_event(char *text, CWParsedEvent *event)
{
  static cw_parse_table_entry primary_table[] = {
    { CW_EVENT_BALK, "BK", parse_balk },
    { CW_EVENT_INTERFERENCE, "C", parse_interference },
    { CW_EVENT_CAUGHTSTEALING, "CS", parse_caught_stealing },
    { CW_EVENT_DOUBLE, "D", parse_base_hit },
    { CW_EVENT_DOUBLE, "DGR", parse_ground_rule_double },
    { CW_EVENT_INDIFFERENCE, "DI", parse_indifference },
    { CW_EVENT_ERROR, "E", parse_safe_on_error },
    { CW_EVENT_FIELDERSCHOICE, "FC", parse_fielders_choice },
    { CW_EVENT_FIELDERSCHOICE, "FCSH", parse_sac_fielders_choice },  /* archaic */
    { CW_EVENT_FOULERROR, "FLE", parse_foul_error },
    { CW_EVENT_HOMERUN, "H", parse_base_hit },
    { CW_EVENT_HITBYPITCH, "HBP", parse_hit_by_pitch },
    { CW_EVENT_HITBYPITCH, "HP", parse_hit_by_pitch },
    { CW_EVENT_HOMERUN, "HR", parse_base_hit },
    { CW_EVENT_INTENTIONALWALK, "I", parse_walk },
    { CW_EVENT_INTENTIONALWALK, "IW", parse_walk },
    { CW_EVENT_STRIKEOUT, "K", parse_strikeout },
    { CW_EVENT_STRIKEOUT, "KE", parse_strikeout_error },  /* archaic */
    { CW_EVENT_OTHERADVANCE, "OA", parse_other_advance },
    { CW_EVENT_OTHERADVANCE, "OBA", parse_other_advance },
    { CW_EVENT_PASSEDBALL, "PB", parse_passed_ball },
    { CW_EVENT_PICKOFF, "PO", parse_pickoff },
    { CW_EVENT_PICKOFF, "POCS", parse_pickoff_caught_stealing },
    { CW_EVENT_PICKOFFERROR, "POE", parse_pickoff_error },
    { CW_EVENT_STOLENBASE, "POSB", parse_pickoff_stolen_base },
    { CW_EVENT_SINGLE, "S", parse_base_hit },
    { CW_EVENT_STOLENBASE, "SB", parse_stolen_base },
    { CW_EVENT_ERROR, "SHE", parse_sac_hit_error },   /* archaic */
    { CW_EVENT_TRIPLE, "T", parse_base_hit },
    { CW_EVENT_WALK, "W", parse_walk },
    { CW_EVENT_WILDPITCH, "WP", parse_wild_pitch },
    { CW_EVENT_GENERICOUT, "", parse_generic_out },
  };
    
  int i;
  CWParserState state;

  cw_parse_initialize(&state, text);
  cw_parse_event_initialize(event);
  cw_parse_primary_event(&state);

  if (state.token[0] == '\0') {
    event->event_type = CW_EVENT_GENERICOUT;
    if (!parse_generic_out(&state, event, 1)) {
      cw_parse_cleanup(&state);
      return 0;
    }
  }
  else {
    for (i = 0; strcmp(primary_table[i].event_string, ""); i++) {
      if (!strcmp(primary_table[i].event_string, state.token)) {
	event->event_type = primary_table[i].event_code;
	(*primary_table[i].parse_func)(&state, event, 1);
	break;
      }
    }

    if (!strcmp(primary_table[i].event_string, "")) {
      cw_parse_cleanup(&state);
      return 0;
    }
  }

  if (state.sym == '.') {
    if (!parse_advancement(&state, event)) {
      cw_parse_cleanup(&state);
      return 0;
    }
  }

  if (state.sym == '+' || state.sym == '-' || state.sym == '#') {
    cw_parse_nextsym(&state);
  }

  if (state.sym != '\0') {
    cw_parse_error(&state);
    cw_parse_cleanup(&state);
    return 0;
  }


  sanity_check(event);
  cw_parse_cleanup(&state);
  return 1;
}

