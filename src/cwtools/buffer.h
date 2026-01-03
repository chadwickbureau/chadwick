/*
* This file is part of Chadwick
 * Copyright (c) 2002-2023, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (https://www.chadwick-bureau.com)
 *
 * FILE: src/cwtools/buffer.h
 * Bounds-safe buffer formatter writer
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


#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct cw_buffer {
  char *current;
  char *end;
  int truncated;
  int need_sep;
  int use_delimiter;
  char delimiter;
} CWBuffer;

static inline void
cw_buffer_init(CWBuffer *buf, char *storage, size_t size, int use_delimiter, char delimiter)
{
  buf->current = storage;
  buf->end = storage + size;
  buf->truncated = 0;
  buf->need_sep = 0;
  buf->use_delimiter = use_delimiter;
  buf->delimiter = delimiter;
  if (size > 0) {
    storage[0] = '\0';
  }
}

static inline int cw_buffer_emit(CWBuffer *buf, const char *fmt, ...)
{
  if (buf->current >= buf->end) {
    buf->truncated = 1;
    return 0;
  }

  /* Add delimiter if required */
  if (buf->use_delimiter && buf->need_sep) {
    if (buf->current < buf->end) {
      *(buf->current++) = buf->delimiter;
    }
    else {
      buf->truncated = 1;
      return 0;
    }
  }
  buf->need_sep = 1;

  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf->current, buf->end - buf->current, fmt, ap);
  va_end(ap);
  if (n < 0) {
    buf->truncated = 1;
    return 0;
  }
  if (buf->current + n >= buf->end) {
    buf->current = buf->end;
    buf->truncated = 1;
    return (int)(buf->end - buf->current - 1);
  }
  buf->current += n;
  return n;
}

static inline int
cw_buffer_emit_string(CWBuffer *buf, const char *s, int width)
{
  if (buf->use_delimiter) {
    return cw_buffer_emit(buf, "\"%s\"", s ? s : "");
  }
  else {
    return cw_buffer_emit(buf, "%-*s", width, s ? s : "");
  }
}

static inline int
cw_buffer_emit_optional_string(CWBuffer *buf, const char *s, int width)
{
  return cw_buffer_emit_string(buf, s ? s : "", width);
}

static inline int
cw_buffer_emit_char(CWBuffer *buf, char c)
{
  if (buf->use_delimiter) {
    return cw_buffer_emit(buf, "\"%c\"", c);
  }
  else {
    return cw_buffer_emit(buf, "%c", c);
  }
}

static inline int
cw_buffer_emit_char_unquoted(CWBuffer *buf, char c)
{
  return cw_buffer_emit(buf, "%c", c);
}

static inline int
cw_buffer_emit_char_rjust(CWBuffer *buf, char c, int width)
{
  if (buf->use_delimiter) {
    return cw_buffer_emit(buf, "\"%c\"", c);
  }
  else {
    return cw_buffer_emit(buf, "%*c", width, c);
  }
}

static inline int
cw_buffer_emit_int(CWBuffer *buf, int value, int width)
{
  if (buf->use_delimiter || width == 0) {
    return cw_buffer_emit(buf, "%d", value);
  }
  else {
    return cw_buffer_emit(buf, "%*d", width, value);
  }
}

static inline int
cw_buffer_emit_flag(CWBuffer *buf, int flag)
{
  return cw_buffer_emit_char(buf, flag ? 'T' : 'F');
}

static inline int
cw_buffer_emit_flag_rjust(CWBuffer *buf, int flag, int width)
{
  return cw_buffer_emit_char_rjust(buf, flag ? 'T' : 'F', width);
}

#endif  // BUFFER_H

