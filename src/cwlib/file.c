/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2023, Dr T L Turocy (ted.turocy@gmail.com)
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
 * Internal helper: A portable version of `getline`.
 */
static ssize_t cw_getline(char **line, size_t *cap, FILE *fp)
{
  if (*line == NULL || *cap == 0) {
    *cap = 256;
    *line = malloc(*cap);
    if (!*line) {
      return -1;
    }
  }

  size_t len = 0;
  int c;

  while ((c = fgetc(fp)) != EOF) {
    if (len + 1 >= *cap) {
      size_t newcap = *cap * 2;
      char *tmp = realloc(*line, newcap);
      if (!tmp) {
        return -1;
      }
      *line = tmp;
      *cap = newcap;
    }

    if (c == '\n') {
      break;
    }
    if (c != '\r') {
      (*line)[len++] = (char) c;
    }
  }

  if (c == EOF && len == 0) {
    return -1;
  }

  (*line)[len] = '\0';
  return (ssize_t) len;
}

void cw_record_reader_init(CWRecordReader *r, FILE *fp)
{
  r->fp = fp;
  r->line = NULL;
  r->capacity = 0;
}

void cw_record_reader_cleanup(CWRecordReader *r)
{
  free(r->line);
  r->line = NULL;
  r->capacity = 0;
}

int cw_record_reader_next(CWRecordReader *r)
{
  ssize_t n = cw_getline(&r->line, &r->capacity, r->fp);
  if (n < 0) {
    return feof(r->fp) ? 0 : -1;
  }
  return 1;
}

const char *cw_record_reader_line(const CWRecordReader *r)
{
  return r->line;
}


void cw_tokenizer_init(CWTokenizer *t, char *line)
{
  t->current = line;
};

char *cw_tokenizer_next(CWTokenizer *t)
{
  char *s = t->current;
  char *start;

  if (!s || *s == '\0') {
    return NULL;
  }

  /* Skip leading whitespace */
  while (*s == ' ' || *s == '\t') {
    s++;
  }
  if (*s == '\0') {
    t->current = s;
    return NULL;
  }

  /* Quoted field */
  if (*s == '"') {
    s++;  /* skip opening quote */
    start = s;
    while (*s && *s != '"') {
      s++;
    }
    if (*s == '"') {
      *s = '\0';
      s++;
    }
    if (*s == ',') {
      s++;
    }
    t->current = s;
    return start;
  }

  /* Unquoted field */
  start = s;
  while (*s && *s != ',') {
    s++;
  }
  if (*s == ',') {
    *s = '\0';
    s++;
  }
  t->current = s;
  return start;
}


/*
 * This replacement for atoi() does validity checking on the input,
 * and returns -1 (which is used by Retrosheet as the null value)
 * for invalid values.
 */
int
cw_atoi(char *s, char *msg)
{
  char *end = NULL;
  long temp = strtol(s, &end, 10);
  if (end != s && errno != ERANGE && temp >= INT_MIN && temp <= INT_MAX) {
    return (int) temp;
  }
  if (msg != NULL) {
    fprintf(stderr, msg, s);
  }
  else {
    fprintf(stderr, "WARNING: Invalid integer value '%s'\n", s);
  }
  return -1;
}


/*
 * Find the position in the file of the specified gameID.
 * Returns nonzero on success.
 */
int cw_file_find_game(char *game_id, FILE *file)
{
  CWRecordReader r;
  CWTokenizer tok;
  fpos_t filepos;

  rewind(file);
  cw_record_reader_init(&r, file);

  while (1) {
    fgetpos(file, &filepos);
    if (cw_record_reader_next(&r) != 1) {
      break;
    }
    char *line = (char *) cw_record_reader_line(&r);
    char *tag, *game;

    cw_tokenizer_init(&tok, line);
    tag = cw_tokenizer_next(&tok);
    game = cw_tokenizer_next(&tok);
    if (tag && !strcmp(tag, "id") && game && !strcmp(game, game_id)) {
      fsetpos(file, &filepos);
      cw_record_reader_cleanup(&r);
      return 1;
    }
  }
  cw_record_reader_cleanup(&r);
  return 0;
}

/*
 * Find the first game in the file (skipping over initial comments)
 */
int cw_file_find_first_game(FILE *file)
{
  CWRecordReader r;
  CWTokenizer tok;
  fpos_t filepos;

  rewind(file);
  cw_record_reader_init(&r, file);

  while (1) {
    fgetpos(file, &filepos);
    if (cw_record_reader_next(&r) != 1) {
      break;
    }
    char *line = (char *) cw_record_reader_line(&r);
    char *tag;

    cw_tokenizer_init(&tok, line);
    tag = cw_tokenizer_next(&tok);

    if (tag && !strcmp(tag, "id")) {
      fsetpos(file, &filepos);
      cw_record_reader_cleanup(&r);
      return 1;
    }
  }
  cw_record_reader_cleanup(&r);
  return 0;
}


