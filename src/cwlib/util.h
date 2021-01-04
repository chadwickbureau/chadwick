/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/gameiter.c
 * Implementation of routines to store game state
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

/* This macro is a convenient shorthand for free()ing and NULLing a pointer
 * if it's not currently NULL. */
#define XFREE(var)    if (var) { free((var)); (var) = NULL; }

/* This macro is a convenient shortland for malloc()ing and copying a
 * pointer to a string, if it's not currently NULL */
#define XCOPY(dest, src) \
  if (src) {  \
    dest = (char *) malloc(sizeof(char) * (strlen(src) + 1)); \
    strcpy(dest, src); \
  } \
  else { \
    dest = NULL; \
  }

