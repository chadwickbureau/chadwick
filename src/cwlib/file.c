/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of convenience routines for reading/writing scorebook files
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "file.h"

/*
 * Tokenizes a line by commas; caller is responsible for deleting memory
 * The function first checks to see if quotes are balanced; if they are,
 * assumes that quotes escape possible commas in text strings.  If they
 * aren't, quotes are just ignored.  (This attemps to work around a
 * missing quote before Paul Sorrento's name in MIN199006240.)
 */
int cw_file_tokenize_line(char *line, char **tokens)
{
  unsigned int i;
  int quote = 0, numTokens = 0;

  strcpy(tokens[numTokens], "");
  for (i = 0; i < strlen(line); i++) {
    if (line[i] == '"') {
      quote++;
    }
  }

  if (quote % 2 == 0) {
    /* An even number of quotes appear; use them */
    quote = 0;
    for (i = 0; i < strlen(line); i++) {
      if (line[i] == '"') {
	quote = 1 - quote;
      }
      else if (line[i] == ',' && !quote) {
	strcpy(tokens[++numTokens], "");
      }
      else if (isprint(line[i])) {
	strncat(tokens[numTokens], &(line[i]), 1);
      }
    }

  }
  else {
    /* An odd number of quotes: assume some error, and ignore them */
    for (i = 0; i < strlen(line); i++) {
      if (line[i] == ',') {
	strcpy(tokens[++numTokens], "");
      }
      else if (line[i] != '"' && isprint(line[i])) {
	strncat(tokens[numTokens], &(line[i]), 1);
      }
    }
  }

  return (numTokens + 1);
}

/*
 * Find the position in the file of the specified gameID.
 * Returns nonzero on success.
 */
int cw_file_find_game(char *game_id, FILE *file)
{
  char buf[256];
  char **tokens;
  int numTokens, i, found = 0;
  fpos_t filepos;

  rewind(file);

  tokens = (char **) malloc(sizeof(char *) * CW_MAX_TOKENS);
  for (i = 0; i < CW_MAX_TOKENS; i++) {
    tokens[i] = (char *) malloc(sizeof(char) * CW_MAX_TOKEN_LENGTH);
  }

  while (!feof(file)) {
    fgetpos(file, &filepos);
    fgets(buf, 256, file);
    numTokens = cw_file_tokenize_line(buf, tokens);
    if (!strcmp(tokens[0], "id") && !strcmp(tokens[1], game_id)) {
      fsetpos(file, &filepos);
      found = 1;
      break;
    }
  }

  for (i = 0; i < CW_MAX_TOKENS; i++) {
    free(tokens[i]);
  }
  free(tokens);

  return found;
}

/*
 * Find the first game in the file (skipping over initial comments)
 */
int cw_file_find_first_game(FILE *file)
{
  char buf[256];
  char **tokens;
  int numTokens, i, found = 0;
  fpos_t filepos;

  rewind(file);

  tokens = (char **) malloc(sizeof(char *) * CW_MAX_TOKENS);
  for (i = 0; i < CW_MAX_TOKENS; i++) {
    tokens[i] = (char *) malloc(sizeof(char) * CW_MAX_TOKEN_LENGTH);
  }

  while (!feof(file)) {
    fgetpos(file, &filepos);
    fgets(buf, 256, file);
    numTokens = cw_file_tokenize_line(buf, tokens);
    if (!strcmp(tokens[0], "id")) {
      fsetpos(file, &filepos);
      found = 1;
      break;
    }
  }

  for (i = 0; i < CW_MAX_TOKENS; i++) {
    free(tokens[i]);
  }
  free(tokens);

  return found;
}


