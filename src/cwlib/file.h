/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Declaration of convenience routines for reading/writing scorebook files
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

#ifndef CW_FILE_H
#define CW_FILE_H

/*
 * These constants are used for creating token arrays
 */
#define CW_MAX_TOKENS        20
#define CW_MAX_TOKEN_LENGTH 100

/*
 * Tokenize 'line' into tokens, which are separated by commas.
 * If an even number of quotation marks appear in the line, quotes
 * are used to 'escape' commas.
 */
int cw_file_tokenize_line(char *line, char **tokens);

/*
 * Searches for the game 'game_id' in 'file'; sets the file pointer to
 * the first record of the game, if present.
 * Returns nonzero if the game is found.
 */
int cw_find_game(char *game_id, FILE *file);

/*
 * Finds the first game record in 'file'.
 */
int cw_find_first_game(FILE *file);

#endif  /* CW_FILE_H */
