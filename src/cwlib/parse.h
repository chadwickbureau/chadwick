/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Interface to event parser
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

#ifndef CW_PARSE_H
#define CW_PARSE_H

#define EVENT_UNKNOWN 0
#define EVENT_NONE 1
#define EVENT_GENERICOUT 2
#define EVENT_STRIKEOUT 3
#define EVENT_STOLENBASE 4
#define EVENT_INDIFFERENCE 5
#define EVENT_CAUGHTSTEALING 6
#define EVENT_PICKOFFERROR 7
#define EVENT_PICKOFF 8
#define EVENT_WILDPITCH 9
#define EVENT_PASSEDBALL 10
#define EVENT_BALK 11
#define EVENT_OTHERADVANCE 12
#define EVENT_FOULERROR 13
#define EVENT_WALK 14
#define EVENT_INTENTIONALWALK 15
#define EVENT_HITBYPITCH 16
#define EVENT_INTERFERENCE 17
#define EVENT_ERROR 18
#define EVENT_FIELDERSCHOICE 19
#define EVENT_SINGLE 20
#define EVENT_DOUBLE 21
#define EVENT_TRIPLE 22
#define EVENT_HOMERUN 23
#define EVENT_MISSINGPLAY 24

typedef struct cw_parsed_event_struct {
  int event_type;
  /* rbiFlag: 2 == (RBI) actually in play text */
  int advance[4], rbi_flag[4], fc_flag[4];
  char play[4][20];
  int sh_flag, sf_flag, dp_flag, gdp_flag, tp_flag;
  int wp_flag, pb_flag, foul_flag, bunt_flag;
  int sb_flag[4], cs_flag[4], po_flag[4];
  int fielded_by;
  int num_putouts, num_assists, num_errors;
  int putouts[3], assists[10], errors[10];
  char error_types[10];
  char batted_ball_type;
  char hit_location[20];
} CWParsedEvent;

int cw_parse_event(char *text, CWParsedEvent *event);

int cw_event_is_batter(CWParsedEvent *event);
int cw_event_is_official_ab(CWParsedEvent *event);
int cw_event_runner_put_out(CWParsedEvent *event, int runner);
int cw_event_outs_on_play(CWParsedEvent *event);
int cw_event_runs_on_play(CWParsedEvent *event);

#endif  /* CW_PARSE_H */







