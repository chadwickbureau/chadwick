/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2023, Dr T L Turocy (ted.turocy@gmail.com)
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
 * Record reader: Encapsulates reading files one line at a time, normalising
 * line endings.
 */
typedef struct cw_record_reader {
  FILE *fp;
  char *line;
  size_t capacity;
} CWRecordReader;

/*
 * Initialise an existing record reader struct to read from `fp`
 */
void cw_record_reader_init(CWRecordReader *r, FILE *fp);

/*
 * Deallocate resources.
 */
void cw_record_reader_cleanup(CWRecordReader *r);

/*
 * Read the next record.  Returns 1 on success, 0 on EOF, -1 on error.
 */
int cw_record_reader_next(CWRecordReader *r);

/*
 * Access the current line (NULL-terminated)
 */
const char *cw_record_reader_line(const CWRecordReader *r);


/*
 * A slightly nicer tokenizer API, which removes the first-next call asymmetry.
 * As with standard C `strtok`, mutates the provided buffer.
 */
typedef struct cw_tokenizer {
  char *current;
} CWTokenizer;

/*
 * Initialise the tokenizer for a line
 */
void cw_tokenizer_init(CWTokenizer *t, char *line);

/*
 * Return the next token, or NULL if none remain.
 * This handles DiamondWare's pseudo-CSV quoted fields correctly.
 */
char *cw_tokenizer_next(CWTokenizer *t);

/*
 * A replacement for C atoi(), which does validity checking and returns
 * -1 as the "null" value for invalid inputs.
 * If 'msg' is specified and not null, it is used as the format string
 * to print a warning message.
 */
int cw_atoi(char *s, char *msg);

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
