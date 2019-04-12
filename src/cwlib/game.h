/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/game.h
 * Declaration of game storage structures, and interface to manipulation
 * functions.
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

/*
 * CWComment represents a comment in the game file.
 * Comments are attached to the event preceding them
 */

typedef struct cw_comment_struct {
  char *text;
  struct cw_comment_struct *prev, *next;
} CWComment;

typedef struct cw_event_struct {
  int inning, batting_team;
  char *batter, *count, *pitches, *event_text;
  /* These are used for badj and padj; if spaces, use roster file */
  char batter_hand, pitcher_hand, *pitcher_hand_id;
  /* These are used for ladj; ladj_slot = 0 means no adjustment */
  int ladj_align, ladj_slot;
  /* These are used for international tiebreaker (Chadwick extension) */
  int itb_base;
  char *itb_runner_id;
  CWAppearance *first_sub, *last_sub;
  CWComment *first_comment, *last_comment;
  struct cw_event_struct *prev, *next;
} CWEvent;

/*
 * CWData stores the data presented in a 'data,' or 'stat,' record in files.
 * This is kept flexible to allow for future expansion.
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
  CWData *first_data, *last_data, *first_stat, *last_stat;
  CWData *first_line, *last_line;  /* linescores, for boxscore event files */
  CWData *first_evdata, *last_evdata;  /* event detail, for boxscore event files */
  CWComment *first_comment, *last_comment; /* for comments before first evt */
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
 * Writes 'game' to 'file'.
 */
void cw_game_write(CWGame *game, FILE *file);

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
 * Modify an info record
 */
void cw_game_info_set(CWGame *game, char *label, char *data);

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
 * Find the starter record for the 'slot' in the batting order for 'team'
 */
CWAppearance *cw_game_starter_find(CWGame *game, int team, int slot);

/*
 * Find the starter record for the player at position 'pos'
 */
CWAppearance *cw_game_starter_find_by_position(CWGame *game, 
					       int team, int pos);

/*
 * Add an event record to the game
 */
void cw_game_event_append(CWGame *game, int inning, int batting_team,
			  char *batter, char *count, char *pitches,
			  char *event_text);

/*
 * Truncate the game before an event
 */
void cw_game_truncate(CWGame *game, CWEvent *event); 

/*
 * Add a substitute record to the game
 */
void cw_game_substitute_append(CWGame *game, char *playerID, char *name,
			       int team, int slot, int pos);

/*
 * Add a data record to the game
 */
void cw_game_data_append(CWGame *game, int num_data, char **data);

/*
 * Set pitcher's earned run data record
 */
void cw_game_data_set_er(CWGame *game, char *playerID, int er);

/*
 * Add a stat record to the game
 */
void cw_game_stat_append(CWGame *game, int num_data, char **data);

/*
 * Add a event detail record to the game
 */
void cw_game_evdata_append(CWGame *game, int num_data, char **data);

/*
 * Add a comment to the game
 */
void cw_game_comment_append(CWGame *game, char *comment);

/*
 * Replace all instances of player ID 'key_old' with 'key_new' in the game.
 */
void cw_game_replace_player(CWGame *game, char *key_old, char *key_new);

/*
 * Safely extract an integer value from a CWData object.  Returns -1 (interpreted as
 * a null value) if the requested index is invalid
 */
int cw_data_get_item_int(CWData *data, unsigned int index);

/*
 * Examine game for internal (in)consistency.  Returns zero if a problem found.
 */
int cw_game_lint(CWGame *game);

/*
 * Add a comment to the event
 */
void cw_event_comment_append(CWEvent *event, char *comment);

#endif  /* CW_GAME_H */




