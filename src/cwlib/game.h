/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Declaration of game storage classes, and interface to manipulation
 * functions.
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

#ifndef CW_GAME_H
#define CW_GAME_H

/*
 * Structures and functions for manipulating games
 */

/*
 * CWInfo represents an 'info' record in a game; these are
 * (label, data) pairs of strings.
 */
typedef struct cw_info_struct {
  char *label, *data;
  struct cw_info_struct *prev, *next;
} CWInfo;

/*
 * A CWAppearance represents an appearance by a player in a game.
 * This represents both 'start,' and 'sub,' records in the file
 */
typedef struct cw_appearance_struct {
  char *player_id, *name;
  int team, slot, pos;
  struct cw_appearance_struct *prev, *next;
} CWAppearance;

typedef struct cw_event_struct {
  int inning, half_inning;
  char *batter, *count, *pitches, *event_text;
  /* These are used for badj and padj; if spaces, use roster file */
  char batter_hand, pitcher_hand;
  CWAppearance *first_sub, *last_sub;
  struct cw_event_struct *prev, *next;
} CWEvent;

/*
 * CWData stores the data presented in a 'data,' record in files.
 * This is kept flexible to allow for future expansion (e.g., in
 * "boxscore" files
 */
typedef struct cw_data_struct {
  int num_data;
  char **data;
  struct cw_data_struct *prev, *next;
} CWData;

typedef struct cw_game_struct {
  char *game_id, *version;
  CWInfo *first_info, *last_info;
  CWAppearance *first_starter, *last_starter;
  CWEvent *first_event, *last_event;
  CWData *first_data, *last_data;
  struct cw_game_struct *prev, *next;
} CWGame;


/*
 * Allocates and initializes a new game with id 'game_id'.  Caller is
 * responsible for maintaining returned pointer
 */
CWGame *cw_game_create(char *game_id);

/*
 * Creates a game by reading from a file; returns a null pointer if
 * read is unsuccessful.  Generally used only by a CWScorebook.
 */
CWGame *cw_game_read(FILE *file);

/*
 * Cleans up internal memory allocation associated with 'game'.
 * Caller is responsible for free()ing the game itself
 */
void cw_game_cleanup(CWGame *game);

/*
 * Set the version number (for file format and game notation)
 */
void cw_game_set_version(CWGame *game, char *version);

/*
 * Add an info record to the game
 */
void cw_game_info_append(CWGame *game, char *label, char *data);

/*
 * Scans the info records for 'label' and returns the associated data.
 * The pointer returned is internal to the CWGame structure, so it
 * should not be deleted, nor its contents changed.
 * The list is scanned from the tail first, to return the last seen
 * record in the case of multiple records.
 */
char *cw_game_info_lookup(CWGame *game, char *label);

/*
 * Add a starter record to the game
 */
void cw_game_starter_append(CWGame *game, char *player_id, char *name,
			    int team, int slot, int pos);

/*
 * Add an event record to the game
 */
void cw_game_event_append(CWGame *p_game, int inning, int halfInning,
			  char *batter, char *count, char *pitches,
			  char *event_text);

/*
 * Add a substitute record to the game
 */
void cw_game_substitute_append(CWGame *game, char *playerID, char *name,
			       int team, int slot, int pos);

/*
 * Add a data record to the game
 */
void cw_game_data_append(CWGame *game, int num_data, char **data);

#endif  /* CW_GAME_H */




