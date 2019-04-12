/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/roster.h
 * Interface to roster management and manipulation routines
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

#ifndef CW_ROSTER_H
#define CW_ROSTER_H

typedef struct cw_player_struct {
  char *player_id, *last_name, *first_name;
  char bats, throws;
  struct cw_player_struct *prev, *next;
} CWPlayer;

/*
 * Initialize a CWPlayer object with the data provided.
 * Caller is responsible for memory management on created object
 */
CWPlayer *cw_player_create(char *player_id, 
			   char *last_name, char *first_name,
			   char bats, char throws);

/*
 * Cleans up memory allocated internally by 'player'.
 * Caller is responsible for free()ing 'player' itself
 */
void cw_player_cleanup(CWPlayer *player);

/*
 * Sets the first name of a player
 */
void cw_player_set_first_name(CWPlayer *player, char *name);

/*
 * Sets the last name of a player
 */
void cw_player_set_last_name(CWPlayer *player, char *name);


typedef struct cw_roster_struct {
  char *team_id, *city, *nickname, *league;
  int year;
  CWPlayer *first_player, *last_player;
  struct cw_roster_struct *prev, *next;
} CWRoster;

/*
 * Allocates and initializes a new CWRoster.  Roster initially has no
 * players.  Caller is responsible for memory management of returned pointer.
 */
CWRoster *cw_roster_create(char *team_id, int year, char *league,
			   char *city, char *nickname);

/*
 * Cleans up memory allocated internally to 'roster'.  Caller is
 * responsible for free()ing 'roster' itself.
 */
void cw_roster_cleanup(CWRoster *roster);


/*
 * Sets the city which the team represents
 */
void cw_roster_set_city(CWRoster *roster, char *city);

/*
 * Sets the nickname of the team
 */
void cw_roster_set_nickname(CWRoster *roster, char *nickname);

/*
 * Sets the league to which the team belongs
 */
void cw_roster_set_league(CWRoster *roster, char *league);

/*
 * Insert a new player to the roster.  This assumes that players are
 * sorted in ascending order by player ID, and maintains that sort
 * order.
 */
void cw_roster_player_insert(CWRoster *roster, CWPlayer *player);

/*
 * Append a new player to the roster.  The roster assumes responsibility
 * for the pointer 'player'.
 */
void cw_roster_player_append(CWRoster *roster, CWPlayer *player);

/*
 * Finds the record for the player with the given player_id.
 * Returns null if the player_id is not on the roster
 */
CWPlayer *cw_roster_player_find(CWRoster *roster, char *player_id);


/*
 * Returns the number of players on the roster 
 */
int cw_roster_player_count(CWRoster *roster);

/*
 * Read in a roster (in Retrosheet format) from file 'file'.
 * Returns nonzero on success, zero on failure.
 */
int cw_roster_read(CWRoster *roster, FILE *file);

/*
 * Write a roster (in Retrosheet format) to file 'file'.
 */
void cw_roster_write(CWRoster *roster, FILE *file);

/*
 * Get the batting hand of player with ID 'player_id'.  Returns '?' if
 * no player with that ID is on the roster.
 */
char cw_roster_batting_hand(CWRoster *roster, char *player_id);

/*
 * Get the throwing hand of player with ID 'player_id'.  Returns '?' if
 * no player with that ID is on the roster.
 */
char cw_roster_throwing_hand(CWRoster *roster, char *player_id);

#endif  /* CW_ROSTER_H */




