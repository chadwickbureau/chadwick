/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/file.h
 * Declaration of convenience routines for reading/writing scorebook files
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

#ifndef CW_FILE_H
#define CW_FILE_H

/*
 * A replacement for C strtok(), using commas as the token separator,
 * and respecting quoted fields
 */
char *cw_strtok(char *strToken);

/*
 * A replacement for C atoi(), which does validity checking and returns
 * -1 as the "null" value for invalid inputs.
 */
int cw_atoi(char *s);

/*
 * Searches for the game 'game_id' in 'file'; sets the file pointer to
 * the first record of the game, if present.
 * Returns nonzero if the game is found.
 */
int cw_file_find_game(char *game_id, FILE *file);

/*
 * Finds the first game record in 'file'.
 */
int cw_file_find_first_game(FILE *file);

#endif  /* CW_FILE_H */
