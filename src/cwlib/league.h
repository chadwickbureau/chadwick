/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/league.h
 * Interface to module for storing and manipulating leagues
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

#ifndef CW_LEAGUE_H
#define CW_LEAGUE_H

#include "roster.h"

typedef struct cw_league_struct {
  CWRoster *first_roster, *last_roster;
} CWLeague;

/*
 * Allocates and initializes a new CWLeague structure.
 * Caller is responsible for memory management of the created pointer
 */
CWLeague *cw_league_create(void);

/*
 * Cleans up internal memory associated with 'league'.
 * Caller is responsible for free()ing the 'league' pointer.
 */
void cw_league_cleanup(CWLeague *league);

/*
 * Append a roster to the league.  The CWLeague takes responsibility
 * for the pointer 'roster'.
 */
void cw_league_roster_append(CWLeague *league, CWRoster *roster);

/*
 * Returns a pointer to the roster of the team with ID 'team'.
 * (The Retrosheet convention is for 'team' to be three characters long,
 * but longer or shorter IDs are accepted by the library
 */
CWRoster *cw_league_roster_find(CWLeague *league, char *team);

/*
 * Read a leaguefile (in Retrosheet convention, TEAMyyyy) from the
 * stream 'file'.  Rosters are created for each team, but are left
 * empty.  Returns nonzero on success, zero on failure.
 */
int cw_league_read(CWLeague *league, FILE *file);

/*
 * Write a leaguefile (in Retrosheet convention, TEAMyyyy) to the
 * stream 'file'.
 */
void cw_league_write(CWLeague *league, FILE *file);

#endif  /* CW_LEAGUE_H */
