/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/league.c
 * Implementation of module for storing and manipulating leagues
 * (i.e., collections of rosters)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

int
cw_league_read(CWLeague *rosterList, FILE *file)
{
  char buf[256], *team_id, *league, *city, *nickname;

  rewind(file);

  while (!feof(file)) {
    strcpy(buf, "");
    if (fgets(buf, 256, file) == NULL) {
      return 0;
    }
    team_id = cw_strtok(buf);
    league = cw_strtok(NULL);
    city = cw_strtok(NULL);
    nickname = cw_strtok(NULL);
    if (!team_id || !league || !city || !nickname) {
      continue;
    }

    cw_league_roster_append(rosterList, 
			    cw_roster_create(team_id, 0, league,
					     city, nickname));
  }
  return 1;
}

void
cw_league_write(CWLeague *league, FILE *file)
{
  CWRoster *roster = league->first_roster;

  while (roster != NULL) {
    fprintf(file, "%s,%s,%s,%s\n", 
	    roster->team_id, roster->league, roster->city, roster->nickname);
    roster = roster->next;
  }
}


