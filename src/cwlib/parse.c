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
  return (event->m_eventType == EVENT_GENERICOUT ||
	  event->m_eventType == EVENT_STRIKEOUT ||
	  (event->m_eventType >= EVENT_WALK &&
	   event->m_eventType <= EVENT_HOMERUN));
}

int
cw_event_is_official_ab(CWParsedEvent *event)
{
  if (!cw_event_is_batter(event)) {
    return 0;
  }

  if (event->m_shFlag || event->m_sfFlag ||
      event->m_eventType == EVENT_WALK ||
      event->m_eventType == EVENT_INTENTIONALWALK ||
      event->m_eventType == EVENT_HITBYPITCH ||
      event->m_eventType == EVENT_INTERFERENCE) {
    return 0;
  }

  return 1;
}

int 
cw_event_runner_put_out(CWParsedEvent *event, int base)
{
  return ((event->m_play[base][0] != '\0' &&
	   !strstr(event->m_play[base], "E")) ? 1 : 0);
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
  return (((event->m_advance[0] >= 4) ? 1 : 0) +
	  ((event->m_advance[1] >= 4) ? 1 : 0) +
	  ((event->m_advance[2] >= 4) ? 1 : 0) +
	  ((event->m_advance[3] >= 4) ? 1 : 0));
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
  if (c = strstr(state->inputString, "SBH")) {
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
    /* This was made a loop since there exists a string in 75SLN.EVN
     * of the form 3-H#(UR).  Having the hash in the middle is obnoxious,
     * but it implies it can appear anywhere in the text.  So we'll just
     * ignore it for now. */
    do {
      state->sym = state->inputString[state->inputPos++];
    } while (state->sym == '#');
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
  fprintf(stderr, "Error: unexpected character at %d in %s\n",
	  state->inputPos, state->inputString);
  exit(1);
  return 0;
}

/*
 * isfielder -- return 1 if parameter corresponds to a fielder, 0 if not
 *              This function accepts '?' as a fielder
 *              (this is a deprecated feature of the notation)
 */
static int isfielder(char p_fielder)
{ return ((p_fielder >= '1' && p_fielder <= '9') || p_fielder == '?'); }


/*
 * Private auxiliary function to initialize event fields
 */
static void
cw_parse_event_initialize(CWParsedEvent *event)
{
  int i;
  event->m_eventType = EVENT_UNKNOWN;
  for (i = 0; i <= 3; i++) {
    event->m_advance[i] = 0;
    event->m_rbiFlag[i] = 0;
    event->m_fcFlag[i] = 0;
    strcpy(event->m_play[i], "");
  }
  for (i = 1; i <= 3; i++) {
    event->m_sbFlag[i] = 0;
    event->m_csFlag[i] = 0;
    event->m_poFlag[i] = 0;
  }
  event->m_shFlag = 0;
  event->m_sfFlag = 0;
  event->m_dpFlag = 0;
  event->m_gdpFlag = 0;
  event->m_tpFlag = 0;
  event->m_wpFlag = 0;
  event->m_pbFlag = 0;
  event->m_foulFlag = 0;
  event->m_buntFlag = 0;
  event->m_fieldedBy = 0;
  event->m_numPutouts = 0;
  event->m_numAssists = 0;
  event->m_numErrors = 0;
  for (i = 0; i < 3; i++) {
    event->m_putouts[i] = 0;
  }
  for (i = 0; i < 10; i++) {
    event->m_assists[i] = 0;
    event->m_errors[i] = 0;
    event->m_errorTypes[i] = 'F';
  }
  event->m_battedBallType = ' ';
  strcpy(event->m_hitLocation, "");
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
 * at entry: p_state->sym should be digit beginning fielder list
 * at exit:  p_state->sym points to first character after fielder list
 *
 * Notes:
 * - Using bevent convention, fielded by is set to first fielder listed
 * - Accepts question mark as a valid fielder, but sets fielded by to zero
 *   if question mark leads fielder list
 */

static void parse_hit_fielder(CWParserState *p_state, CWParsedEvent *p_event)
{
  if (isdigit(p_state->sym)) {
    p_event->m_fieldedBy = p_state->sym - '0';
  }

  while (isfielder(p_state->sym)) {
    cw_parse_nextsym(p_state);
  }
}

/*
 * parse_fielding_credit: read a fielding credit, assign PO/A/E as appropriate
 *
 * at entry: p_state->sym should be digit beginning fielding credit
 * at exit:  p_state->sym points to first character after fielding credit
 *           returns 0 if batter out, nonzero if batter safe on muffed throw
 * Notes:
 * - Accepts question mark as a valid fielder, but does not issue putouts
 *   or assists when question mark is used.
 * - p_prev is the "previous" fielder, as in cases like '64(1)3/GDP';
 *   here, p_prev would equal '4' on the second call, to essentially turn
 *   the string into '64(1)43/GDP'
 * - Will handle strings starting in 'E'
 * - Question marks for missing fielders are saved in the token field;
 *   this is a difference from bevent's behavior
 */
static int parse_fielding_credit(CWParserState *p_state, CWParsedEvent *p_event,
				 char p_prev)
{
  char *play = p_state->token;
  char lastChar = p_state->sym;
  /* These track assists on this play; players can only receive one
   * assist per play */
  int assists[10], numAssists = 0;
  int i, j;

  if (p_state->sym == 'E') {
    cw_parse_nextsym(p_state);
    if (!isfielder(p_state->sym)) {
      cw_parse_error(p_state);
    }
    if (isdigit(p_state->sym)) {
      p_event->m_errors[p_event->m_numErrors] = p_state->sym - '0';
      p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
    }
    *(play++) = 'E';
    *(play++) = p_state->sym;
    *(play) = '\0';
    cw_parse_nextsym(p_state);
    return 1;
  }
      

  if (p_prev != ' ' && p_prev != p_state->sym) {
    assists[numAssists++] = p_prev - '0';
    *(play++) = p_prev;
  }

  *(play++) = p_state->sym;

  while (1) {
    cw_parse_nextsym(p_state);

    if ((p_state->sym >= '1' && p_state->sym <= '9') ||
	p_state->sym == '?') {
      if (isdigit(lastChar)) {
	assists[numAssists++] = lastChar - '0';
      }
      *(play++) = p_state->sym;
      lastChar = p_state->sym;
    }
    else if (p_state->sym == 'E') {
      if (isdigit(lastChar)) {
	assists[numAssists++] = lastChar - '0';
      }
      *(play++) = 'E';
      cw_parse_nextsym(p_state);
      if (!isdigit(p_state->sym)) {
	cw_parse_error(p_state);
      }
      p_event->m_errors[p_event->m_numErrors] = p_state->sym - '0';
      p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
      *(play++) = p_state->sym; 
      *(play) = '\0';
      cw_parse_nextsym(p_state);
      
      for (i = 0; i < numAssists; i++) {
	for (j = 0; j < i; j++) {
	  if (assists[j] == assists[i]) break;
	}

	if (j < i)  continue;

	p_event->m_assists[p_event->m_numAssists++] = assists[i];
      }

      return 1;
    }
    else {
      if (isdigit(lastChar)) {
	p_event->m_putouts[p_event->m_numPutouts++] = lastChar - '0';
      }
      *(play) = '\0';

      for (i = 0; i < numAssists; i++) {
	for (j = 0; j < i; j++) {
	  if (assists[j] == assists[i]) break;
	}

	if (j < i)  continue;
	
	p_event->m_assists[p_event->m_numAssists++] = assists[i];
      }
      return 0;
    }
  }
}

/*
 * parse_flag: read in a flag
 *
 * at entry: p_state->sym should be '/'
 * at exit:  p_state->sym points to first character after flag
 *           text of flag (without the slash) stored in p_state->token
 */
static void parse_flag(CWParserState *p_state)
{
  char *c = p_state->token;

  while (1) {
    cw_parse_nextsym(p_state);
    if (p_state->sym != '/' && p_state->sym != '.' && 
	p_state->sym != '(' && p_state->sym != ')' &&
	p_state->sym != '#' && p_state->sym != '!' && p_state->sym != 0) {
      (*c++) = p_state->sym;
    }
    else {
      break;
    }
  }
  
  *c = '\0';
}

static int parse_advance_modifier(CWParserState *p_state, CWParsedEvent *p_event,
				  int p_safe, int p_baseFrom, int p_baseTo)
{
  int i;

  if (isfielder(p_state->sym) || p_state->sym == 'E') {
    if (parse_fielding_credit(p_state, p_event, ' ')) {
      if (!p_safe) {
	p_safe = 1;
	p_event->m_advance[p_baseFrom] = p_baseTo;
	for (i = p_baseFrom; i >= 0; i--) {
	  p_event->m_rbiFlag[i] = -1;
	}
      }
    }
    if (p_state->token[0] != 'E') {
      strncpy(p_event->m_play[p_baseFrom], p_state->token, 20);
    }
    else {
      for (i = p_baseFrom; i >= 0; i--) {
	p_event->m_rbiFlag[i] = -1;
      }
    }

    if (p_state->sym == '/') {
      parse_flag(p_state);
      if (!strcmp(p_state->token, "TH") ||
	  !strcmp(p_state->token, "TH1") ||
	  !strcmp(p_state->token, "TH2") ||
	  !strcmp(p_state->token, "TH3") ||
	  !strcmp(p_state->token, "THH")) {
	p_event->m_errorTypes[p_event->m_numErrors - 1] = 'T';
      }
      else if (!strcmp(p_state->token, "INT")) {
	/* silently accept interference flag */
      }
      else if (!strcmp(p_state->token, "G")) {
	/* FC5.2X3(5/G) appears in 81TEX.EVA; accept silently */
      }
      else {
	cw_parse_error(p_state);
	return 0;
      }
    }

    if (p_state->sym == '(') {
      cw_parse_nextsym(p_state);
      if (!parse_advance_modifier(p_state, p_event, p_safe,
				  p_baseFrom, p_baseTo)) {
	return 0;
      }
    }

    /* This loop is here to tolerate weird things, like '2XH(9S)', which
     * appears in 1989 files */
    while (p_state->sym != ')') {
      cw_parse_nextsym(p_state);
    }
  }
  else {
    cw_parse_primary_event(p_state);

    /* (NORBI) is archaic; only appears in two places in 1983 files */
    if (!strcmp(p_state->token, "NR") || !strcmp(p_state->token, "NORBI")) {
      p_event->m_rbiFlag[p_baseFrom] = 0;
    }
    else if (!strcmp(p_state->token, "RBI") && 
	     p_event->m_advance[p_baseFrom] >= 4) {
      /* rbiFlag == 2 means (RBI) is actually present, overriding some
       * rare cases of ambiguity */
      p_event->m_rbiFlag[p_baseFrom] = 2;
    }
    else if (!strcmp(p_state->token, "UR") &&
	     p_event->m_advance[p_baseFrom] == 4) {
      p_event->m_advance[p_baseFrom] = 5;
    }
    else if (!strcmp(p_state->token, "TUR") &&
	     p_event->m_advance[p_baseFrom] == 4) {
      p_event->m_advance[p_baseFrom] = 6;
    }
    else if (!strcmp(p_state->token, "WP")) {
      p_event->m_wpFlag = 1;
    }
    else if (!strcmp(p_state->token, "PB")) {
      p_event->m_pbFlag = 1;
    }
    else if (!strcmp(p_state->token, "TH")) {
      if (p_state->sym >= '1' && p_state->sym <= '3') {
	cw_parse_nextsym(p_state);
      }
      /* just silently accept throw flags for now */
    }
    else if (!strcmp(p_state->token, "THH")) {
      /* just silently accept throw flags for now */
    }
    else if (!strcmp(p_state->token, "INT")) {
      /* silently accept interference flag */
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  if (p_state->sym == ')') {
    cw_parse_nextsym(p_state);
    return 1;
  }
  else {
    cw_parse_error(p_state);
    return 0;
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
  "3L", "46", "5L", "7LDW", "7DW", "78XDW", "8XDW", "89XDW", "9DW", "9LDW",
  "2R", "7M",  /* where is 7M !?! */
  ""
};

static void ParseFlags(CWParserState *p_state, CWParsedEvent *p_event)
{
  char flag[256];

  do {
    strcpy(flag, "/");

    do {
      cw_parse_nextsym(p_state);
      if (p_state->sym != '/' && p_state->sym != '.' && 
	  p_state->sym != '#' && p_state->sym != '!' && p_state->sym != 0) {
	strncat(flag, &(p_state->sym), 1); 
      }
    } while (p_state->sym != '/' && p_state->sym != '.' && 
	     p_state->sym != '#' && p_state->sym != '!' && p_state->sym != 0);

    if (!strcmp(flag, "/SH")) {
      p_event->m_shFlag = 1;
      p_event->m_buntFlag = 1;
    }
    else if (!strcmp(flag, "/SF")) {
      p_event->m_sfFlag = 1;
      if (p_event->m_battedBallType == ' ') {
	p_event->m_battedBallType = 'F';
      }
    }
    else if (!strcmp(flag, "/DP")) {
      p_event->m_dpFlag = 1;
    }
    else if (!strcmp(flag, "/GDP")) {
      p_event->m_dpFlag = 1;
      p_event->m_gdpFlag = 1;
      p_event->m_battedBallType = 'G';
    }
    else if (!strcmp(flag, "/LDP")) {
      p_event->m_dpFlag = 1;
      p_event->m_battedBallType = 'L';
    }
    else if (!strcmp(flag, "/FDP")) {
      p_event->m_dpFlag = 1;
      p_event->m_battedBallType = 'F';
    }
    else if (!strcmp(flag, "/BGDP")) {
      p_event->m_buntFlag = 1;
      p_event->m_dpFlag = 1;
      p_event->m_gdpFlag = 1;
      p_event->m_battedBallType = 'G';
    }
    else if (!strcmp(flag, "/BPDP")) {
      p_event->m_buntFlag = 1;
      p_event->m_dpFlag = 1;
      p_event->m_battedBallType = 'P';
    }
    else if (!strcmp(flag, "/TP")) {
      p_event->m_tpFlag = 1;
    }
    else if (!strcmp(flag, "/GTP")) {
      p_event->m_tpFlag = 1;
      p_event->m_battedBallType = 'G';
    }
    else if (!strcmp(flag, "/LTP")) {
      p_event->m_tpFlag = 1;
      p_event->m_battedBallType = 'L';
    }
    else if (!strcmp(flag, "/FL")) {
      p_event->m_foulFlag = 1;
    }
    else if (!strcmp(flag, "/FO") && p_event->m_battedBallType == ' ') {
      p_event->m_battedBallType = 'G';
    }
    else if ((!strcmp(flag, "/TH") || !strcmp(flag, "/TH1") ||
	      !strcmp(flag, "/TH2") || !strcmp(flag, "/TH3") ||
	      !strcmp(flag, "/THH")) && 
	     (p_event->m_eventType == EVENT_ERROR ||
	      p_event->m_eventType == EVENT_PICKOFFERROR)) {
      p_event->m_errorTypes[0] = 'T';
    }
    else if (!strcmp(flag, "/B")) {
      p_event->m_buntFlag = 1;
    }
    else if (!strcmp(flag, "/BG")) {
      p_event->m_buntFlag = 1;
      p_event->m_battedBallType = 'G';
    }
    else if (!strcmp(flag, "/BP")) {
      p_event->m_buntFlag = 1;
      p_event->m_battedBallType = 'P';
    }
    else if (!strcmp(flag, "/BF")) {
      p_event->m_buntFlag = 1;
      p_event->m_battedBallType = 'F';
    }
    else if (!strcmp(flag, "/BL")) {
      p_event->m_buntFlag = 1;
      p_event->m_battedBallType = 'L';
    }
    else if (!strcmp(flag, "/P")) {
      p_event->m_battedBallType = 'P';
    }
    else if (!strcmp(flag, "/F")) {
      p_event->m_battedBallType = 'F';
    }
    else if (!strcmp(flag, "/G")) {
      p_event->m_battedBallType = 'G';
    }
    else if (!strcmp(flag, "/L")) {
      p_event->m_battedBallType = 'L';
    }
    else if (strlen(flag) >= 3) {
      char traj = (flag[1] == 'B') ? flag[2] : flag[1];
      char *loc = (flag[1] == 'B') ? flag + 3 : flag + 2;
      if (traj == 'G' || traj == 'F' || traj == 'P' ||
	  traj == 'L') {
	int i = 0;
	for (i = 0; strcmp(locations[i], ""); i++) {
	  if (!strcmp(locations[i], loc)) {
	    p_event->m_battedBallType = traj;
	    strcpy(p_event->m_hitLocation, locations[i]);
	    if (locations[i][strlen(locations[i]) - 1] == 'F') {
	      p_event->m_foulFlag = 1;
	    }
	    if (flag[1] == 'B') {
	      p_event->m_buntFlag = 1;
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
	    strcpy(p_event->m_hitLocation, locations[i]);
	    if (locations[i][strlen(locations[i]) - 1] == 'F') {
	      p_event->m_foulFlag = 1;
	    }
	    if (flag[1] == 'B') {
	      p_event->m_buntFlag = 1;
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
	  strcpy(p_event->m_hitLocation, locations[i]);
	  break;
	}
      }
    }
  } while (p_state->sym != '.' && p_state->sym != 0);
}

/*
 * parse_balk: process a balk event
 *
 * at entry: p_state->sym should be character after 'BK' token
 * at exit:  p_state->sym points to first character after 'BK' token
 *
 * Notes:
 * - Nothing to do here really; balks shouldn't take flags, etc.
 */
static int parse_balk(CWParserState *p_state, CWParsedEvent *p_event, int p_flags)
{
  return 1;
}

static int parse_stolen_base(CWParserState *p_state, CWParsedEvent *p_event, 
			     int p_flags)
{
  if (p_state->sym == '2') {
    p_event->m_sbFlag[1] = 1;
    p_event->m_advance[1] = 2;
    cw_parse_nextsym(p_state);
  }
  else if (p_state->sym == '3') {
    p_event->m_sbFlag[2] = 1;
    p_event->m_advance[2] = 3;
    cw_parse_nextsym(p_state);
  }
  else if (p_state->sym == '4') {
    /* SBH is converted to SB4 in initialization */
    p_event->m_sbFlag[3] = 1;
    p_event->m_advance[3] = 4;
    cw_parse_nextsym(p_state);
    /* special case: accept archaic SBH(UR) or SBH(TUR) */
    if (p_state->sym == '(') {
      p_event->m_advance[3] = 5;
      cw_parse_nextsym(p_state);
      if (p_state->sym == 'T') {
	p_event->m_advance[3] = 6;
	cw_parse_nextsym(p_state);
      }
      if (p_state->sym != 'U') {
	cw_parse_error(p_state);
      }
      cw_parse_nextsym(p_state);
      if (p_state->sym != 'R') {
	cw_parse_error(p_state);
      }
      cw_parse_nextsym(p_state);
      if (p_state->sym != ')') {
	cw_parse_error(p_state);
      }
      cw_parse_nextsym(p_state);
    }
  }
  else {
    cw_parse_error(p_state);
  }

  if (p_state->sym == ';') {
    cw_parse_nextsym(p_state);
    cw_parse_primary_event(p_state);

    if (!strcmp(p_state->token, "SB")) {
      parse_stolen_base(p_state, p_event, 0);
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  while (p_flags && p_state->sym == '/') {
    parse_flag(p_state);

    if (!strcmp(p_state->token, "INT")) {
      /* accept /INT flag silently */
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  return 1;
}

static int parse_caught_stealing(CWParserState *p_state, CWParsedEvent *p_event,
				 int p_flags)
{
  int runner;

  if (p_state->sym >= '2' && p_state->sym <= '4') {
    /* CSH is converted to CS4 in initialization */
    p_event->m_csFlag[runner = p_state->sym - '1'] = 1;
  }
  else {
    cw_parse_error(p_state);
    return 0;
  }
    
  while (cw_parse_nextsym(p_state) == '(') {
    cw_parse_nextsym(p_state);
    if (isfielder(p_state->sym) || p_state->sym == 'E') {
      if (parse_fielding_credit(p_state, p_event, ' ')) {
	p_event->m_advance[runner] = runner + 1;
      }
      strcpy(p_event->m_play[runner], p_state->token);
    }
    else if (isalpha(p_state->sym)) {
      cw_parse_primary_event(p_state);

      if (!strcmp(p_state->token, "UR") && p_event->m_advance[runner] == 4) {
	p_event->m_advance[runner] = 5;
      }
      else if (!strcmp(p_state->token, "TUR") &&
	       p_event->m_advance[runner] == 4) {
	p_event->m_advance[runner] = 6;
      }
      else {
	cw_parse_error(p_state);
	return 0;
      }

      if (p_state->sym != ')') {
	cw_parse_error(p_state);
	return 0;
      }
    }
  }

  if (p_state->sym == ';') {
    /* Two caught stealings can happen, though they're rare */
    cw_parse_nextsym(p_state);
    cw_parse_primary_event(p_state);

    if (!strcmp(p_state->token, "CS")) {
      parse_caught_stealing(p_state, p_event, 0);
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  while (p_flags && p_state->sym == '/') {
    parse_flag(p_state);

    if (!strcmp(p_state->token, "DP")) {
      p_event->m_dpFlag = 1;
    }
    else if (!strcmp(p_state->token, "TH") ||
	     !strcmp(p_state->token, "TH1") ||
	     !strcmp(p_state->token, "TH2") ||
	     !strcmp(p_state->token, "TH3") ||
	     !strcmp(p_state->token, "THH")) {
      /* silently accept a throw flag; there is an instance of
       * a POCSn(nnEn)/TH1 in 1968 */
    }
    else if (!strcmp(p_state->token, "INT")) {
      /* silently accept interference flag */
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  return 1;
}

/*
 * parse_safe_on_error: process an error event
 *
 * at entry: p_state->sym should be character after 'E' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Does not accept question mark for fielder
 * - Generates a fielded by credit (since that's what bevent does)
 * - Assumes infielders make errors on grounders, outfielders on flies
 *   (this is currently a bug in bevent, which puts a batted ball type
 *   of 'G' for all errors)
 * - Implied advancement of batter is to first base
 */
static int parse_safe_on_error(CWParserState *p_state,
			       CWParsedEvent *p_event, int p_flags)
{
  p_event->m_advance[0] = 1;
  
  if (p_state->sym < '1' || p_state->sym > '9') {
    cw_parse_error(p_state);
    return 0;
  }

  p_event->m_errors[p_event->m_numErrors] = (p_state->sym - '0');
  p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
  p_event->m_fieldedBy = (p_state->sym - '0');
  p_event->m_battedBallType = (p_state->sym <= '6') ? 'G' : 'F';
  cw_parse_nextsym(p_state);

  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }
  return 1;
}

/*
 * parse_fielders_choice: process a fielder's choice event
 *
 * at entry: p_state->sym should be character after 'FC' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Fielder's choice plays are assumed to be grounders unless specified
 * - Implied advance for batters is first base 
 */
static int parse_fielders_choice(CWParserState *p_state, CWParsedEvent *p_event,
				 int p_flags)
{
  p_event->m_advance[0] = 1;
  p_event->m_battedBallType = 'G';

  if (p_state->sym >= '1' && p_state->sym <= '9') {
    p_event->m_fieldedBy = (p_state->sym - '0');
    cw_parse_nextsym(p_state);
  }
  else if (p_state->sym == '?') {
    cw_parse_nextsym(p_state);
  }

  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }

  return 1;
}

/*
 * parse_sac_fielders_choice: process a sac fielder's choice event (archaic)
 *
 * at entry: p_state->sym should be character after 'FCSH' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Archaic; this function sets the bunt and sac flags, then hands off
 *   to parse_fielders_choice() to do the actual work
 */
static int parse_sac_fielders_choice(CWParserState *p_state, CWParsedEvent *p_event,
				     int p_flags)
{
  p_event->m_buntFlag = 1;
  p_event->m_shFlag = 1;
  return parse_fielders_choice(p_state, p_event, p_flags);
}

/*
 * parse_foul_error: process a foul ball error event
 *
 * at entry: p_state->sym should be character after 'FLE' token
 * at exit:  p_state->sym points to end of string
 *
 * Notes:
 * - Foul flag is not set, since bevent does not set it (?)
 * - Ball fielded is generated, since bevent generates it
 */
static int parse_foul_error(CWParserState *p_state, CWParsedEvent *p_event,
			    int p_flags)
{
  if (p_state->sym >= '1' && p_state->sym <= '9') {
    p_event->m_errors[p_event->m_numErrors] = (p_state->sym - '0');
    p_event->m_errorTypes[p_event->m_numErrors++] = 'F'; 
    p_event->m_fieldedBy = (p_state->sym - '0');
    cw_parse_nextsym(p_state);
  }
  else {
    cw_parse_error(p_state);
    return 0;
  }

  if (p_flags && p_state->sym == '/') {
    /* Most likely a trajectory code */
    ParseFlags(p_state, p_event);
  }

  return 1;
}

static int parse_out_base(CWParserState *p_state)
{
  int base;

  cw_parse_nextsym(p_state);
  if (p_state->sym!= '1' && p_state->sym!= '2' && p_state->sym!= '3' && p_state->sym!= 'B') {
    cw_parse_error(p_state);
  }
  base = (p_state->sym== 'B') ? 0 : (p_state->sym- '0');
  
  cw_parse_nextsym(p_state);
  if (p_state->sym != ')') {
    cw_parse_error(p_state);
  }
  cw_parse_nextsym(p_state);

  return base;
}

static int parse_generic_out(CWParserState *p_state, CWParsedEvent *p_event,
			     int p_flags)
{
  /* lastFielder keeps track of the fielder who made the previous putout,
     so as to generate correct credit on plays like 54(1)3/GDP */
  char lastFielder = ' ';
  int safe;

  if (p_state->sym != '?') {
    p_event->m_fieldedBy = (p_state->sym - '0');
  }
  p_event->m_advance[0] = 1;
 
  while (isfielder(p_state->sym)) {
    safe = parse_fielding_credit(p_state, p_event, lastFielder);

    if (p_state->sym == '(') {
      int base = parse_out_base(p_state);
      p_event->m_advance[base] = (safe) ? base + 1 : 0;
      p_event->m_fcFlag[base] = 1;
      if (strlen(p_state->token) > 1 || base > 0) {
	/* Assumption: more than one fielder implies ground ball,
	   unless overriden later by a flag; also, getting the first
	   out on a non-batter implies a bounce */
	p_event->m_battedBallType = 'G';
      }
      else if (strlen(p_state->token) == 1 && base == 0) {
	p_event->m_battedBallType = 'F';
      }
	 
      strncpy(p_event->m_play[base], p_state->token, 19);
      lastFielder = p_state->token[strlen(p_state->token) - 1];
    }
    else {
      if (strlen(p_state->token) > 1 || lastFielder != ' ') {
	/* Assumption: more than one fielder implies ground ball,
	   unless overriden later by a flag */
	p_event->m_battedBallType = 'G';
      }
      else {
	p_event->m_battedBallType = 'F';
      }

      strcpy(p_event->m_play[0], p_state->token);
      p_event->m_advance[0] = (safe) ? 1 : 0;
      break;
    }
  }

  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }

  return 1;
}

/*
 * parse_hit_by_pitch: process a hit-by-pitch event
 *
 * at entry: p_state->sym should be character after 'HP'/'HBP' token
 * at exit:  p_state->sym points to first character after 'HP'/'HBP' token
 *
 * Notes:
 * - Sets advancement of batter to 1, which is implied by primary event
 */
static int parse_hit_by_pitch(CWParserState *p_state, CWParsedEvent *p_event,
			      int p_flags)
{
  p_event->m_advance[0] = 1;
  return 1;
}

/*
 * parse_interference: process a catcher's interference event
 *
 * at entry: p_state->sym should be first character after 'C' token
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
static int parse_interference(CWParserState *p_state, CWParsedEvent *p_event,
			      int p_flags)
{
  p_event->m_advance[0] = 1;

  while (p_state->sym == '/') {
    parse_flag(p_state);

    if (p_state->token[0] == 'E' && p_event->m_numErrors > 0) {
      cw_parse_error(p_state);
      return 0;
    }

    if (!strcmp(p_state->token, "E2")) {
      p_event->m_errors[p_event->m_numErrors++] = 2;
    }
    else if (!strcmp(p_state->token, "E1")) {
      p_event->m_errors[p_event->m_numErrors++] = 1;
    }
    else if (!strcmp(p_state->token, "E3")) {
      p_event->m_errors[p_event->m_numErrors++] = 3;
    }
    else if (!strcmp(p_state->token, "INT")) {
      /* silently accept redundant /INT flag */
    }
    else {
      cw_parse_error(p_state);
      return 0;
    }
  }

  if (p_event->m_numErrors == 0) {
    p_event->m_errors[p_event->m_numErrors++] = 2;
  }
  p_event->m_errorTypes[0] = 'F';

  return 1;
}

/*
 * parse_indifference: process a defensive indifference event
 *
 * at entry: p_state->sym should be character after 'DI' token
 * at exit:  p_state->sym points to first character after 'DI' token
 *
 * Notes:
 * - Nothing to do here; DI doesn't take flags
 */
static int parse_indifference(CWParserState *p_state, CWParsedEvent *p_event,
			      int p_flags)
{
  return 1;
}

/*
 * parse_other_advance: process an other advance/out advancing event
 *
 * at entry: p_state->sym should be character after 'OA'/'OBA' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - Flags are unusual with this event; /INT, /DP, and /TP are accepted,
 *   though only /INT has occurred so far in Retrosheet data
 */
static int parse_other_advance(CWParserState *p_state, CWParsedEvent *p_event,
			       int p_flags)
{
  while (p_flags && p_state->sym == '/') {
    parse_flag(p_state);

    if (!strcmp(p_state->token, "INT")) {
      /* silently accept interference flag */
    }
    else if (!strcmp(p_state->token, "DP")) {
      p_event->m_dpFlag = 1;
    }
    else if (!strcmp(p_state->token, "TP")) {
      p_event->m_tpFlag = 1;
    }
    else {
      return cw_parse_error(p_state);
    }
  }

  return 1;
}

/*
 * parse_passed_ball: process a passed ball event
 *
 * at entry: p_state->sym should be character after 'PB' token
 * at exit:  p_state->sym points to first character after 'PB' token
 *
 * Notes:
 * - Nothing to do here; PB doesn't take flags
 */
static int parse_passed_ball(CWParserState *p_state, CWParsedEvent *p_event,
			     int p_flags)
{
  p_event->m_pbFlag = 1;
  return 1;
}

/*
 * parse_pickoff_error: process an old-style pickoff error event
 *
 * at entry: p_state->sym should be character after 'POE' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - This is a deprecated code; preferred now is 'POn(En)'
 */
static int parse_pickoff_error(CWParserState *p_state, CWParsedEvent *p_event,
			       int p_flags)
{
  if (p_state->sym >= '1' && p_state->sym <= '9') {
    p_event->m_errors[p_event->m_numErrors] = (p_state->sym- '0');
    p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
  }
  else {
    return cw_parse_error(p_state);
  }

  cw_parse_nextsym(p_state);

  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }

  return 1;
}

/*
 * parse_pickoff_stolen_base: process a pickoff-stolen base event
 *
 * at entry: p_state->sym should be character after 'POSB' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 */
static int parse_pickoff_stolen_base(CWParserState *p_state, CWParsedEvent *p_event,
				     int p_flags)
{
  return parse_stolen_base(p_state, p_event, p_flags);
}

/*
 * parse_pickoff_caught_stealing: process a pickoff-caught stealing event
 * 
 * at entry: p_state->sym should be character after 'POCS' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 */
static int parse_pickoff_caught_stealing(CWParserState *p_state,
					 CWParsedEvent *p_event, 
					 int p_flags)
{
  int runner;

  if (!parse_caught_stealing(p_state, p_event, p_flags)) {
    return 0;
  }

  for (runner = 1; runner <= 3; runner++) {
    if (p_event->m_csFlag[runner]) {
      p_event->m_poFlag[runner] = 1;
    }
  }

  return 1;
}

static int parse_pickoff(CWParserState *p_state, CWParsedEvent *p_event,
			 int p_flags)
{
  int runner;

  if (p_state->sym >= '1' && p_state->sym <= '3') {
    runner = p_state->sym - '0';
  }
  else {
    return cw_parse_error(p_state);
  }
  p_event->m_poFlag[runner] = 1;
    
  if (cw_parse_nextsym(p_state) != '(') {
    return cw_parse_error(p_state);
  }
  cw_parse_nextsym(p_state);
  if (isfielder(p_state->sym)) {
    parse_fielding_credit(p_state, p_event, ' ');
    strncpy(p_event->m_play[runner], p_state->token, 20);
  }
  else if (p_state->sym == 'E') {
    /* going to leave this commented to match hevent output for now */
    /*    p_event->m_eventType = EVENT_PICKOFFERROR;  */
    parse_fielding_credit(p_state, p_event, ' ' );
    strncpy(p_event->m_play[runner], p_state->token, 20);

    if (p_state->sym == '/') {
      parse_flag(p_state);
      if (!strcmp(p_state->token, "TH") ||
	  !strcmp(p_state->token, "TH1") ||
	  !strcmp(p_state->token, "TH2") ||
	  !strcmp(p_state->token, "TH3") ||
	  !strcmp(p_state->token, "THH")) {
	p_event->m_errorTypes[p_event->m_numErrors - 1] = 'T';
      }
      else {
	return cw_parse_error(p_state);
      }
    }
  }
  else {
    return cw_parse_error(p_state);
  }

  if (p_state->sym == ')') {
    cw_parse_nextsym(p_state);
  }
  else {
    return cw_parse_error(p_state);
  }

  if (p_flags && p_state->sym == '/') {
    /* Most likely flag is /DP */
    ParseFlags(p_state, p_event);
  }

  return 1;
}

/*
 * parse_sac_hit_error: process a sac hit error event (archaic)
 *
 * at entry: p_state->sym should be character after 'SHE' token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 * - This is an archaic primary event; this function sets the bunt and
 *   sac hit flags, then hands off to parse_error(), since the syntax is
 *   identical after this point.
 */
static int parse_sac_hit_error(CWParserState *p_state, CWParsedEvent *p_event,
			       int p_flags)
{
  p_event->m_buntFlag = 1;
  p_event->m_shFlag = 1;
  return parse_safe_on_error(p_state, p_event, p_flags);
}

/*
 * parse_base_hit: process a base hit event ('S', 'D', 'T', 'H', 'HR')
 *
 * at entry: p_state->sym should be character after hit token
 * at exit:  p_state->sym points to '.' or end of string, as appropriate
 *
 * Notes:
 */
static int parse_base_hit(CWParserState *p_state, CWParsedEvent *p_event,
			  int p_flags)
{
  if (isfielder(p_state->sym)) {
    parse_hit_fielder(p_state, p_event);
  }

  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }

  return 1;
}

static int parse_ground_rule_double(CWParserState *p_state, CWParsedEvent *p_event,
				    int p_flags)
{
  if (p_flags && p_state->sym == '/') {
    ParseFlags(p_state, p_event);
  }
  
  return 1;
}

static int parse_strikeout(CWParserState *p_state, CWParsedEvent *p_event,
			   int p_flags)
{
  if (p_state->sym >= '1' && p_state->sym <= '9') {
    p_event->m_advance[0] = ((parse_fielding_credit(p_state, p_event, ' ')) ?
			     1 : 0);
    strcpy(p_event->m_play[0], p_state->token);
  } 
  else {
    /* just a bare strikeout */
    strcpy(p_event->m_play[0], "2");
    p_event->m_putouts[p_event->m_numPutouts++] = 2;
  }

  if (p_state->sym == '+') {
    cw_parse_nextsym(p_state);
    cw_parse_primary_event(p_state);

    if (!strcmp(p_state->token, "WP")) {
      p_event->m_wpFlag = 1;
    }
    else if (!strcmp(p_state->token, "PB")) {
      p_event->m_pbFlag = 1;
    }
    else if (!strcmp(p_state->token, "PO")) {
      parse_pickoff(p_state, p_event, 0);
    }
    else if (!strcmp(p_state->token, "POCS")) {
      parse_pickoff_caught_stealing(p_state, p_event, 0);
    }
    else if (!strcmp(p_state->token, "POSB")) {
      parse_pickoff_stolen_base(p_state, p_event, 0);
    }
    else if (!strcmp(p_state->token, "SB")) {
      parse_stolen_base(p_state, p_event, 0);
    }
    else if (!strcmp(p_state->token, "CS")) {
      parse_caught_stealing(p_state, p_event, 0);
    }
    else if (!strcmp(p_state->token, "OA") ||
	     !strcmp(p_state->token, "OBA")) {
      /* we don't need to do anything special for K+OA */
    }
    else if (!strcmp(p_state->token, "E")) {
      if (p_state->sym < '1' || p_state->sym > '9') {
	return cw_parse_error(p_state);
      }
      p_event->m_errors[p_event->m_numErrors] = p_state->sym - '0';
      p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
      cw_parse_nextsym(p_state);
    }
    else if (!strcmp(p_state->token, "")) {
      /* 'K+/23' appears in 1988 files; just accept this silently */
    }
    else {
      return cw_parse_error(p_state);
    }
  }

  while (p_flags && p_state->sym == '/') {
    parse_flag(p_state);

    if ((!strcmp(p_state->token, "TH") ||
	 !strcmp(p_state->token, "TH1") ||
	 !strcmp(p_state->token, "TH2") ||
	 !strcmp(p_state->token, "TH3") ||
	 !strcmp(p_state->token, "THH")) && p_event->m_numErrors > 0) {
      p_event->m_errorTypes[0] = 'T';
    }
    else if (!strcmp(p_state->token, "DP")) {
      p_event->m_dpFlag = 1;
    }
    else if (!strcmp(p_state->token, "TP")) {
      p_event->m_tpFlag = 1;
    }
    else if (!strcmp(p_state->token, "2") || !strcmp(p_state->token, "23")) {
      /* 'K/2' appears in 1988, 'K/23' in 1969 */
    }
    else if (!strcmp(p_state->token, "C") ||
	     !strcmp(p_state->token, "B") ||
	     !strcmp(p_state->token, "BF") ||
	     !strcmp(p_state->token, "BP") ||
	     !strcmp(p_state->token, "BG") ||
	     !strcmp(p_state->token, "FL") ||
	     !strcmp(p_state->token, "L") ||
	     !strcmp(p_state->token, "F") ||
	     !strcmp(p_state->token, "FO") ||
	     !strcmp(p_state->token, "INT") ||
	     !strcmp(p_state->token, "TH")) {
      /* accept these flags silently; though 'K/L' is a bit weird... */
    }
    else if (!strcmp(p_state->token, "")) {
      /* a 'K/' event appears in 80SFN.EVN */
    }
    else {
      return cw_parse_error(p_state);
    }
  }

  return 1;
}

static int parse_strikeout_error(CWParserState *p_state, CWParsedEvent *p_event,
				 int p_flags)
{
  if (p_state->sym < '1' || p_state->sym > '9') {
    return cw_parse_error(p_state);
  }

  p_event->m_errors[p_event->m_numErrors] = p_state->sym - '0';
  p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
  p_event->m_play[0][0] = 'E';
  p_event->m_play[0][1] = p_state->sym;
  p_event->m_play[0][2] = '\0';
  cw_parse_nextsym(p_state);
  return 1;
}

static int parse_walk(CWParserState *p_state, CWParsedEvent *p_event, int p_flags)
{
  p_event->m_advance[0] = 1;

  if (p_state->sym == '+') {
    cw_parse_nextsym(p_state);
    cw_parse_primary_event(p_state);

    if (!strcmp(p_state->token, "WP")) {
      p_event->m_wpFlag = 1;
    }
    else if (!strcmp(p_state->token, "PB")) {
      p_event->m_pbFlag = 1;
    }
    else if (!strcmp(p_state->token, "PO")) {
      if (!parse_pickoff(p_state, p_event, 0))  return 0;
    }
    else if (!strcmp(p_state->token, "POSB")) {
      if (!parse_pickoff_stolen_base(p_state, p_event, 0)) return 0;
    }
    else if (!strcmp(p_state->token, "POCS")) {
      if (!parse_pickoff_caught_stealing(p_state, p_event, 0)) return 0;
    }
    else if (!strcmp(p_state->token, "SB")) {
      if (!parse_stolen_base(p_state, p_event, 0)) return 0;
    }
    else if (!strcmp(p_state->token, "CS")) {
      if (!parse_caught_stealing(p_state, p_event, 0)) return 0;
    }
    else if (!strcmp(p_state->token, "OA") ||
	     !strcmp(p_state->token, "OBA")) {
      /* we don't need to do anything special for W+OA */
    }
    else if (!strcmp(p_state->token, "E")) {
      if (p_state->sym < '1' || p_state->sym > '9') {
	return cw_parse_error(p_state);
      }
      p_event->m_errors[p_event->m_numErrors] = p_state->sym - '0';
      p_event->m_errorTypes[p_event->m_numErrors++] = 'F';
      cw_parse_nextsym(p_state);
    }
  }

  while (p_flags && p_state->sym == '/') {
    parse_flag(p_state);

    if ((!strcmp(p_state->token, "TH") ||
	 !strcmp(p_state->token, "TH1") ||
	 !strcmp(p_state->token, "TH2") ||
	 !strcmp(p_state->token, "TH3") ||
	 !strcmp(p_state->token, "THH")) && p_event->m_numErrors > 0) {
      p_event->m_errorTypes[0] = 'T';
    }
    else if (!strcmp(p_state->token, "DP")) {
      p_event->m_dpFlag = 1;
    }
    else {
      return cw_parse_error(p_state);
    }
  }

  return 1;
}

/*
 * parse_wild_pitch: process a wild pitch event
 *
 * at entry: p_state->sym should be character after 'WP' token
 * at exit:  p_state->sym points to first character after 'WP' token
 *
 * Notes:
 * - Nothing to do here; WP doesn't take flags
 */
static int parse_wild_pitch(CWParserState *p_state, CWParsedEvent *p_event,
			    int p_flags)
{
  p_event->m_wpFlag = 1;
  return 1;
} 

static int parse_runner_advance(CWParserState *p_state, CWParsedEvent *p_event)
{
  int baseFrom = 0, baseTo = 0, safe = 0;

  if (p_state->sym < '1' && p_state->sym > '3' && p_state->sym != 'B') {
    return cw_parse_error(p_state);
  }
  baseFrom = (p_state->sym == 'B') ? 0 : (p_state->sym - '0');

  cw_parse_nextsym(p_state);
  if (p_state->sym != '-' && p_state->sym != 'X') {
    return cw_parse_error(p_state);
  }
  safe = (p_state->sym == '-') ? 1 : 0;

  cw_parse_nextsym(p_state);
  if (p_state->sym < '1' && p_state->sym > '3' && p_state->sym != 'H') {
    return cw_parse_error(p_state);
  }
  baseTo = (p_state->sym == 'H') ? 4 : (p_state->sym - '0');

  if (safe) {
    /* This logic takes care of possibility of plays like
     * CSH(1E2)(UR).3-H, where advancement is already implied, and
     * marked as unearned */
    if (baseTo < 4 || p_event->m_advance[baseFrom] < 4) {
      p_event->m_advance[baseFrom] = baseTo;
    }
    if (baseTo == 4 && cw_event_is_batter(p_event) && !p_event->m_gdpFlag &&
	(p_event->m_eventType != EVENT_ERROR || baseFrom == 3) &&
	p_event->m_eventType != EVENT_STRIKEOUT &&
	p_event->m_rbiFlag[baseFrom] != -1) {
      p_event->m_rbiFlag[baseFrom] = 1;
    }
  }
  else {
    p_event->m_advance[baseFrom] = 0;
    if (p_event->m_eventType == EVENT_FIELDERSCHOICE) {
      p_event->m_fcFlag[baseFrom] = 1;
    }
  }
  
  cw_parse_nextsym(p_state);
  while (p_state->sym == '(') {
    cw_parse_nextsym(p_state);
    if (!parse_advance_modifier(p_state, p_event, safe, baseFrom, baseTo)) {
      return 0;
    }
  }
  return 1;
}

static int parse_advancement(CWParserState *p_state, CWParsedEvent *p_event)
{
  do {
    cw_parse_nextsym(p_state);
    parse_runner_advance(p_state, p_event);
  } while (p_state->sym == ';');

  return (p_state->sym == '\0');
}

/*
 * Cleanup and so forth
 */
void SanityCheck(CWParsedEvent *p_event)
{
  int base;

  if (p_event->m_eventType == EVENT_SINGLE &&
      p_event->m_advance[0] == 0 && 
      !strcmp(p_event->m_play[0], "")) {
    p_event->m_advance[0] = 1;
  }
  else if (p_event->m_eventType == EVENT_DOUBLE &&
	   p_event->m_advance[0] == 0 && 
	   !strcmp(p_event->m_play[0], "")) {
    p_event->m_advance[0] = 2;
  }
  if (p_event->m_eventType == EVENT_TRIPLE &&
      p_event->m_advance[0] == 0 && 
      !strcmp(p_event->m_play[0], "")) {
    p_event->m_advance[0] = 3;
  }
  if (p_event->m_eventType == EVENT_HOMERUN &&
      p_event->m_advance[0] == 0 && 
      !strcmp(p_event->m_play[0], "")) {
    p_event->m_advance[0] = 4;
    p_event->m_rbiFlag[0] = 1;
  }

  if (p_event->m_eventType == EVENT_STRIKEOUT)  {
    p_event->m_battedBallType = ' ';
    if (!strcmp(p_event->m_play[0], "2") && 
	p_event->m_advance[0] > 0) {
      strcpy(p_event->m_play[0], "");
      p_event->m_putouts[0] = p_event->m_putouts[1];
      p_event->m_putouts[1] = p_event->m_putouts[2];
      p_event->m_numPutouts--;
    }
  }

  if (p_event->m_eventType == EVENT_WALK) {
    p_event->m_rbiFlag[0] = 0;
    p_event->m_rbiFlag[1] = 0;
    p_event->m_rbiFlag[2] = 0;
  }

  for (base = 0; base <= 3; base++) {
    if (p_event->m_rbiFlag[base] == -1) {
      p_event->m_rbiFlag[base] = 0;
    }

    if (strcmp(p_event->m_play[base], "") &&
	!strstr(p_event->m_play[base], "E")) {
      /* This patches up instances like BXH(832)(E8) */
      p_event->m_advance[base] = 0;
    }
  }

  /* The following implements rules for default batted ball types based
   * upon defensive fielding credit */
  if (p_event->m_eventType == EVENT_GENERICOUT) {
    if (strlen(p_event->m_play[0]) == 1 &&
	!p_event->m_dpFlag && !p_event->m_tpFlag && !p_event->m_buntFlag &&
	p_event->m_battedBallType == ' ') {
      p_event->m_battedBallType = 'F';
    }
    else if (strlen(p_event->m_play[0]) >= 1 && 
	     p_event->m_battedBallType == ' ') {
      p_event->m_battedBallType = 'G';
    }
  }

  if (p_event->m_eventType == EVENT_SINGLE && p_event->m_buntFlag &&
      p_event->m_battedBallType == ' ') {
    p_event->m_battedBallType = 'G';
  }

  if (p_event->m_shFlag) {
    p_event->m_battedBallType = 'G';
  }
}

typedef struct {
  int event_code;
  char event_string[5];
  int (*parse_func)(CWParserState *, CWParsedEvent *, int);
} cw_parse_table_entry;

int cw_parse_event(char *p_text, CWParsedEvent *p_event)
{
  static cw_parse_table_entry primary_table[] = {
    { EVENT_BALK, "BK", parse_balk },
    { EVENT_INTERFERENCE, "C", parse_interference },
    { EVENT_CAUGHTSTEALING, "CS", parse_caught_stealing },
    { EVENT_DOUBLE, "D", parse_base_hit },
    { EVENT_DOUBLE, "DGR", parse_ground_rule_double },
    { EVENT_INDIFFERENCE, "DI", parse_indifference },
    { EVENT_ERROR, "E", parse_safe_on_error },
    { EVENT_FIELDERSCHOICE, "FC", parse_fielders_choice },
    { EVENT_FIELDERSCHOICE, "FCSH", parse_sac_fielders_choice },  /* archaic */
    { EVENT_FOULERROR, "FLE", parse_foul_error },
    { EVENT_HOMERUN, "H", parse_base_hit },
    { EVENT_HITBYPITCH, "HBP", parse_hit_by_pitch },
    { EVENT_HITBYPITCH, "HP", parse_hit_by_pitch },
    { EVENT_HOMERUN, "HR", parse_base_hit },
    { EVENT_INTENTIONALWALK, "I", parse_walk },
    { EVENT_INTENTIONALWALK, "IW", parse_walk },
    { EVENT_STRIKEOUT, "K", parse_strikeout },
    { EVENT_STRIKEOUT, "KE", parse_strikeout_error },  /* archaic */
    { EVENT_OTHERADVANCE, "OA", parse_other_advance },
    { EVENT_OTHERADVANCE, "OBA", parse_other_advance },
    { EVENT_PASSEDBALL, "PB", parse_passed_ball },
    { EVENT_PICKOFF, "PO", parse_pickoff },
    { EVENT_PICKOFF, "POCS", parse_pickoff_caught_stealing },
    { EVENT_PICKOFFERROR, "POE", parse_pickoff_error },
    { EVENT_STOLENBASE, "POSB", parse_pickoff_stolen_base },
    { EVENT_SINGLE, "S", parse_base_hit },
    { EVENT_STOLENBASE, "SB", parse_stolen_base },
    { EVENT_ERROR, "SHE", parse_sac_hit_error },   /* archaic */
    { EVENT_TRIPLE, "T", parse_base_hit },
    { EVENT_WALK, "W", parse_walk },
    { EVENT_WILDPITCH, "WP", parse_wild_pitch },
    { EVENT_GENERICOUT, "", parse_generic_out },
  };
    
  int i;
  CWParserState state;

  cw_parse_initialize(&state, p_text);
  cw_parse_event_initialize(p_event);
  cw_parse_primary_event(&state);

  if (state.token[0] == '\0') {
    p_event->m_eventType = EVENT_GENERICOUT;
    if (!parse_generic_out(&state, p_event, 1)) {
      cw_parse_cleanup(&state);
      return 0;
    }
  }
  else {
    for (i = 0; strcmp(primary_table[i].event_string, ""); i++) {
      if (!strcmp(primary_table[i].event_string, state.token)) {
	p_event->m_eventType = primary_table[i].event_code;
	(*primary_table[i].parse_func)(&state, p_event, 1);
	break;
      }

      if (!strcmp(primary_table[i].event_string, "")) {
	cw_parse_cleanup(&state);
	return 0;
      }
    }
  }

  if (state.sym == '.') {
    if (!parse_advancement(&state, p_event)) {
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


  SanityCheck(p_event);
  cw_parse_cleanup(&state);
  return 1;
}

