/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Interface to scorebook (i.e. collections of games) module.
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

#ifndef CW_BOOK_H
#define CW_BOOK_H

/*
 * Structures and functions for manipulating lists of games ("scorebooks")
 */
typedef struct cw_scorebook_struct {
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
 * Reads the contents of the scorebook file in filename 'path' into
 * the scorebook 'scorebook'.  Returns the number of games successfully read,
 * or -1 if the file could not be opened.
 */ 
int cw_scorebook_read(CWScorebook *scorebook, char *path);

/*
 * Writes the contents of the scorebook to file 'file'.
 */
void cw_scorebook_write(CWScorebook *scorebook, FILE *file);


#endif  /* CW_BOOK_H */
