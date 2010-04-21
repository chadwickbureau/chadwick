/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2013, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/libchadwick.i
 * SWIG interface wrapper for Chadwick Python library
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

%module libchadwick

%rename(Scorebook) CWScorebook;


%include file.i

%{
#include <chadwick.h>

int is_valid_play(char *play)
{
  CWEventData data;
  return cw_parse_event(play, &data); 
}

%}

%include <book.h>
%include <file.h>
%include <gameiter.h>
%include <league.h>
%include <parse.h>
%include <roster.h>
%include <box.h>

%include "game.h"

%pythoncode %{
import time
import datetime
import tempfile
%}

//==========================================================================
//                            Renaming constants
//==========================================================================

// Using %rename doesn't work on #defines, so we instead simply define
// the appropriate variables

%constant int EVENT_UNKNOWN         = CW_EVENT_UNKNOWN;
%constant int EVENT_NONE            = CW_EVENT_NONE;
%constant int EVENT_GENERICOUT      = CW_EVENT_GENERICOUT;
%constant int EVENT_STRIKEOUT       = CW_EVENT_STRIKEOUT;
%constant int EVENT_STOLENBASE      = CW_EVENT_STOLENBASE;
%constant int EVENT_INDIFFERENCE    = CW_EVENT_INDIFFERENCE;
%constant int EVENT_CAUGHTSTEALING  = CW_EVENT_CAUGHTSTEALING;
%constant int EVENT_PICKOFFERROR    = CW_EVENT_PICKOFFERROR;
%constant int EVENT_PICKOFF         = CW_EVENT_PICKOFF;
%constant int EVENT_WILDPITCH       = CW_EVENT_WILDPITCH;
%constant int EVENT_PASSEDBALL      = CW_EVENT_PASSEDBALL;
%constant int EVENT_BALK            = CW_EVENT_BALK;
%constant int EVENT_OTHERADVANCE    = CW_EVENT_OTHERADVANCE;
%constant int EVENT_FOULERROR       = CW_EVENT_FOULERROR;
%constant int EVENT_WALK            = CW_EVENT_WALK;
%constant int EVENT_INTENTIONALWALK = CW_EVENT_INTENTIONALWALK;
%constant int EVENT_HITBYPITCH      = CW_EVENT_HITBYPITCH;
%constant int EVENT_INTERFERENCE    = CW_EVENT_INTERFERENCE;
%constant int EVENT_ERROR           = CW_EVENT_ERROR;
%constant int EVENT_FIELDERSCHOICE  = CW_EVENT_FIELDERSCHOICE;
%constant int EVENT_SINGLE          = CW_EVENT_SINGLE;
%constant int EVENT_DOUBLE          = CW_EVENT_DOUBLE;
%constant int EVENT_TRIPLE          = CW_EVENT_TRIPLE;
%constant int EVENT_HOMERUN         = CW_EVENT_HOMERUN;
%constant int EVENT_MISSINGPLAY     = CW_EVENT_MISSINGPLAY;

%include roster.i
%include scorebook.i
%include boxscore.i
%include state.i
%include game.i

