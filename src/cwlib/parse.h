/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/parse.h
 * Interface to event parser
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

#ifndef CW_PARSE_H
#define CW_PARSE_H

/*
 * This enumerates the possible types of events.
 * Events 0 through 24 are set identical to the event codes used by
 * programs available from the Retrosheet website.
 * The pitch event codes are arbitrary, for use with pitch-by-pitch
 * iteration features in Chadwick.
 */
typedef enum {
  CW_EVENT_UNKNOWN = 0,
  CW_EVENT_NONE = 1,
  CW_EVENT_GENERICOUT = 2,
  CW_EVENT_STRIKEOUT = 3,
  CW_EVENT_STOLENBASE = 4,
  CW_EVENT_INDIFFERENCE = 5,
  CW_EVENT_CAUGHTSTEALING = 6,
  CW_EVENT_PICKOFFERROR = 7,
  CW_EVENT_PICKOFF = 8,
  CW_EVENT_WILDPITCH = 9,
  CW_EVENT_PASSEDBALL = 10,
  CW_EVENT_BALK = 11,
  CW_EVENT_OTHERADVANCE = 12,
  CW_EVENT_FOULERROR = 13,
  CW_EVENT_WALK = 14,
  CW_EVENT_INTENTIONALWALK = 15,
  CW_EVENT_HITBYPITCH = 16,
  CW_EVENT_INTERFERENCE = 17,
  CW_EVENT_ERROR = 18,
  CW_EVENT_FIELDERSCHOICE = 19,
  CW_EVENT_SINGLE = 20,
  CW_EVENT_DOUBLE = 21,
  CW_EVENT_TRIPLE = 22,
  CW_EVENT_HOMERUN = 23,
  CW_EVENT_MISSINGPLAY = 24,
  CW_EVENT_PITCH_BALL = 100,
  CW_EVENT_PITCH_BALL_INTENTIONAL = 101,
  CW_EVENT_PITCH_BALL_PITCHOUT = 102,
  CW_EVENT_PITCH_BALL_HITBYPITCH = 103,
  CW_EVENT_PITCH_BALL_PENALTYBALL = 104,
  CW_EVENT_PITCH_INPLAY = 105,
  CW_EVENT_PITCH_STRIKE_CALLED = 106,
  CW_EVENT_PITCH_STRIKE_SWINGING = 107,
  CW_EVENT_PITCH_STRIKE_UNKNOWN = 108,
  CW_EVENT_PITCH_STRIKE_FOUL = 109,
  CW_EVENT_PITCH_STRIKE_BUNT_MISSED = 110,
  CW_EVENT_PITCH_STRIKE_BUNT_FOUL = 111,
  CW_EVENT_PITCH_STRIKE_PITCHOUT = 112,
  CW_EVENT_PITCH_STRIKE_PITCHOUT_FOUL = 113,
  CW_EVENT_PITCH_STRIKE_FOULTIP = 114,
  CW_EVENT_PITCH_STRIKE_BUNT_FOULTIP = 115,
  CW_EVENT_PITCH_NOPITCH = 116,
  CW_EVENT_PITCH_UNKNOWN = 117,
  CW_EVENT_PITCH_PICKOFF_PITCHER_FIRST = 118,
  CW_EVENT_PITCH_PICKOFF_PITCHER_SECOND = 119,
  CW_EVENT_PITCH_PICKOFF_PITCHER_THIRD = 120,
  CW_EVENT_PITCH_PICKOFF_CATCHER_FIRST = 121,
  CW_EVENT_PITCH_PICKOFF_CATCHER_SECOND = 122,
  CW_EVENT_PITCH_PICKOFF_CATCHER_THIRD = 123
} CWEventType;


typedef struct cw_parsed_event_struct {
  CWEventType event_type;
  /* rbi_flag: 2 == (RBI) actually in play text */
  /* muff_flag: nonzero if runner is safe on a play like 1X2(6E4) */
  int advance[4], rbi_flag[4], fc_flag[4], muff_flag[4];
  char play[4][20];
  int sh_flag, sf_flag, dp_flag, gdp_flag, tp_flag;
  int wp_flag, pb_flag, foul_flag, bunt_flag, force_flag;
  int sb_flag[4], cs_flag[4], po_flag[4];
  int fielded_by;
  /* touches lists players in the order fielding credits are assigned;
   * intended for listing players involved in DP or TP */
  int num_putouts, num_assists, num_errors, num_touches;
  int putouts[3], assists[10], errors[10], touches[20];
  /* Error types are 'N' for none, 'T' for throwing, 
   * 'F' for fumbled, and 'D' for dropped throw. */
  char error_types[10];
  char batted_ball_type;
  char hit_location[20];
} CWEventData;

void cw_event_data_copy(CWEventData *dest, CWEventData *src);

int cw_parse_event(char *text, CWEventData *event);

int cw_event_is_batter(CWEventData *event);
int cw_event_is_official_ab(CWEventData *event);
int cw_event_runner_put_out(CWEventData *event, int runner);
int cw_event_outs_on_play(CWEventData *event);
int cw_event_runs_on_play(CWEventData *event);
int cw_event_rbi_on_play(CWEventData *event);

#endif  /* CW_PARSE_H */







