/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/file.c
 * Implementation of convenience routines for reading/writing scorebook files
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "file.h"

/*
 * This adaptation of strtok() respects the quoted string fields in
 * Retrosheet files.  The function assumes that quotes appear at the
 * very beginning and end of a field, i.e., comma-quote-data-quote-comma.
 *
 * This function operates similarly to strtok() in that it maintains
 * static data.
 *
 * The implementation is based on ConsoleStrTok() by Chris Cookson,
 * <cjcookson@hotmail.com>, posted at
 * http://www.flipcode.com/cgi-bin/fcarticles.cgi?show=64037
 */
char *cw_strtok(char *strToken)
{
  /* Where to start searching next */
  static char *pNext;
  /* Start of next token */
  char *pStart;

  /* If NULL is passed in, continue searching */
  if (strToken == NULL) {
    if (pNext != NULL) {
      strToken = pNext;
    } 
    else {
      /* Reached end of original string */
      return NULL;
    }
  }

  /* Zero length string, so no more tokens to be found */
  if (*strToken == 0) {
    pNext = NULL;
    return NULL;
  }

  /* Skip leading whitespace before next token */
  while ((*strToken != 0) &&
	 ((*strToken == ' ') || (*strToken == '\t') || (*strToken == '\n'))) {
    ++strToken;
  }

  if (*strToken == 0) {
    pNext = NULL;
    return NULL;
  }

  /* It's a quoted literal - skip the first quote char */
  if (*strToken == '\"') {
    ++strToken;

    pStart = strToken;

    /* Find ending quote or end of string */
    while ((*strToken != '\"') && (*strToken != 0) && 
	   (*strToken != '\n') && (*strToken != '\r')) {
      ++strToken;
    }

    if (*strToken == 0) {
      /* Reached end of original string */
      pNext = NULL;
    } 
    else {
      /* More to find, note where to continue searching */
      *strToken = 0;
      pNext = strToken + 1;
      /* A comma immediately following a quote should be skipped past */
      if (*pNext == ',') {
	pNext++;
      }
    }
    /* Return ptr to start of token */
    return pStart;
  } 
  else {
    /* Unquoted token */
    pStart = strToken;

    /* Find next comma or end of string */
    while ((*strToken != 0) && (*strToken != ',') && 
	   (*strToken != '\n') && (*strToken != '\r')) {
      ++strToken;
    }

    /* Reached end of original string? */
    if (*strToken == 0) {
      pNext = NULL;
    } 
    else {
      *strToken = 0;
      pNext = strToken + 1;
    }
    /* Return ptr to start of token */
    return pStart;
  }
}

/*
 * This replacement for atoi() does validity checking on the input,
 * and returns -1 (which is used by Retrosheet as the null value)
 * for invalid values.
 */
int
cw_atoi(char *s)
{
  char *end = NULL;
  long temp = strtol(s, &end, 10);
  if (end != s && errno != ERANGE && temp >= INT_MIN && temp <= INT_MAX) {
    return (int) temp;
  }
  fprintf(stderr, "Warning: Invalid integer value '%s'\n", s);
  return -1;
}


/*
 * Find the position in the file of the specified gameID.
 * Returns nonzero on success.
 */
int cw_file_find_game(char *game_id, FILE *file)
{
  char buf[1024], *tok, *game;
  fpos_t filepos;

  rewind(file);

  while (!feof(file)) {
    fgetpos(file, &filepos);
    if (fgets(buf, 1023, file) == NULL) {
      return 0;
    }
    tok = cw_strtok(buf);
    game = cw_strtok(NULL);
    if (tok && !strcmp(tok, "id") && game && !strcmp(game, game_id)) {
      fsetpos(file, &filepos);
      return 1;
    }
  }

  return 0;
}

/*
 * Find the first game in the file (skipping over initial comments)
 */
int cw_file_find_first_game(FILE *file)
{
  char buf[256], *tok;
  fpos_t filepos;

  rewind(file);

  while (!feof(file)) {
    fgetpos(file, &filepos);
    if (fgets(buf, 256, file) == NULL) {
      return 0;
    }
    tok = cw_strtok(buf);
    if (tok && !strcmp(tok, "id")) {
      fsetpos(file, &filepos);
      return 1;
    }
  }

  return 0;
}


