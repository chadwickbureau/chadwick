/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of module for storing and manipulating leagues
 * (i.e., collections of rosters)
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

#include "file.h"    /* for file-reading and tokenizing */
#include "league.h"

CWLeague *
cw_league_create(void)
{
  CWLeague *rosterList = (CWLeague *) malloc(sizeof(CWLeague));
  rosterList->first_roster = NULL;
  rosterList->last_roster = NULL;

  return rosterList;
}

void
cw_league_cleanup(CWLeague *rosterList)
{
  CWRoster *roster = rosterList->first_roster;

  while (roster) {
    CWRoster *nextRoster = roster->next;
    cw_roster_cleanup(roster);
    free(roster);
    roster = nextRoster;
  }

  rosterList->first_roster = NULL;
  rosterList->last_roster = NULL;
}

void
cw_league_roster_append(CWLeague *rosterList, CWRoster *roster)
{
  roster->prev = rosterList->last_roster;
 
  if (rosterList->first_roster == NULL) {
    rosterList->first_roster = roster;
  }
  else {
    rosterList->last_roster->next = roster;
  }

  rosterList->last_roster = roster;
}

CWRoster *
cw_league_roster_find(CWLeague *league, char *team)
{
  CWRoster *roster = league->first_roster;
  while (roster && strcmp(roster->team_id, team)) {
    roster = roster->next;
  }
  return roster;
}

void
cw_league_read(CWLeague *rosterList, FILE *file)
{
  char buf[256];
  char **tokens;
  int numTokens, i;

  rewind(file);

  tokens = (char **) malloc(sizeof(char *) * CW_MAX_TOKENS);
  for (i = 0; i < CW_MAX_TOKENS; i++) {
    tokens[i] = (char *) malloc(sizeof(char) * CW_MAX_TOKEN_LENGTH);
  }

  while (!feof(file)) {
    fgets(buf, 256, file);
    numTokens = cw_file_tokenize_line(buf, tokens);

    cw_league_roster_append(rosterList, 
			    cw_roster_create(tokens[0], 0, tokens[1][0],
					     tokens[2], tokens[3]));
  }

  for (i = 0; i < CW_MAX_TOKENS; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void
cw_league_write(CWLeague *league, FILE *file)
{
  CWRoster *roster = league->first_roster;

  while (roster != NULL) {
    fprintf(file, "%s,%c,%s,%s\n", 
	    roster->team_id, roster->league, roster->city, roster->nickname);
    roster = roster->next;
  }
}


