/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/book.c
 * Interface to scorebook (i.e. collections of games) module.
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

#ifndef CW_BOOK_H
#define CW_BOOK_H

/*
 * Structures and functions for manipulating lists of games ("scorebooks")
 */
typedef struct cw_scorebook_struct {
  CWComment *first_comment, *last_comment;
  CWGame *first_game, *last_game;
} CWScorebook;

/*
 * Allocates and initializes a new empty scorebook.  Caller is responsible
 * for maintaining returned pointer.
 */
CWScorebook *cw_scorebook_create(void);

/*
 * Cleans up internal memory allocation associated with 'scorebook'.
 * Caller is responsible for free()ing the scorebook itself.
 */
void cw_scorebook_cleanup(CWScorebook *scorebook);

/*
 * Appends 'game' to 'scorebook'.  Returns 1 if successful, 0 if not.
 * (Failure occurs, for example, if 'game' is null).
 * After appending, scorebook is responsible for the memory associated
 * with 'game'.
 */
int cw_scorebook_append_game(CWScorebook *scorebook, CWGame *game);

/*
 * Inserts 'game' to 'scorebook'.  Inserts the game in chronological
 * order.  Returns 1 if successful, 0 if not.
 */
int cw_scorebook_insert_game(CWScorebook *scorebook, CWGame *game);

/*
 * Removes the game with game ID 'game_id' from scorebook.  Returns the
 * game object; the caller becomes responsible for the memory allocated
 * to the game.
 * If 'game_id' is not present, NULL is returned and there is no effect
 * on the scorebook.
 */
CWGame *cw_scorebook_remove_game(CWScorebook *scorebook, char *game_id);

/*
 * Reads the contents of the scorebook file 'file' into
 * the scorebook 'scorebook'.  Returns the number of games successfully read,
 * or -1 if the file could not be opened.
 */ 
int cw_scorebook_read(CWScorebook *scorebook, FILE *file);

/*
 * Writes the contents of the scorebook to file 'file'.
 */
void cw_scorebook_write(CWScorebook *scorebook, FILE *file);


typedef struct cw_scorebook_iter_struct {
  CWGame *current;
  int (*f)(CWGame *);
} CWScorebookIterator;

/*
 * Returns an iterator object for 'scorebook'.
 * 'f' points to a filter function that returns nonzero for games
 * the iterator should return, and zero for games the iterator should
 * skip.  Passing NULL for 'f' gives an iterator that will return all
 * games in the scorebook.
 */
CWScorebookIterator *cw_scorebook_iterate(CWScorebook *scorebook,
					  int (*f)(CWGame *));

/*
 * Cleans up internal memory allocation associated with 'iterator'.
 * Caller is responsible for free()ing the scorebook itself.
 */
void cw_scorebook_iterator_cleanup(CWScorebookIterator *iterator);

/*
 * Returns the next game in the scorebook associated with 'iterator'.
 * Returns NULL when the end of the scorebook has been reached
 */
CWGame *cw_scorebook_iterator_next(CWScorebookIterator *iterator);


#endif  /* CW_BOOK_H */
