#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Borland C++ 5.5 makefile to build Chadwick command line tools
#
# This file is part of Chadwick, tools for baseball play-by-play and stats
# Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
#
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or (at 
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
# for more details.
#
# You should have received a copy of the GNU General Public License along 
# with this program; if not, write to the Free Software Foundation, Inc., 
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# 

.AUTODEPEND

# The following directories must be set or overriden for the target setup
BCCDIR = C:\BORLAND\BCC55
	
SOURCE_SUFFIX = .c
OBJECT_SUFFIX = .obj

CWEVENT_SOURCES = cwevent.c cwtools.c
CWEVENT_OBJECTS = $(CWEVENT_SOURCES:.c=.obj)

CWGAME_SOURCES = cwgame.c cwtools.c
CWGAME_OBJECTS = $(CWGAME_SOURCES:.c=.obj)

CWSUB_SOURCES = cwsub.c cwtools.c
CWSUB_OBJECTS = $(CWSUB_SOURCES:.c=.obj)

CFG = borland.cfg

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }


OPT = -Od

CPPFLAGS = -DMSDOS -I$(BCCDIR)\INCLUDE -I.. @$(CFG)

all: cwevent cwgame cwsub

cwevent: $(CWEVENT_OBJECTS)
  bcc32 /L$(BCCDIR)\lib $(CWEVENT_OBJECTS) ..\cwlib\chadwick.lib

cwgame:  $(CWGAME_OBJECTS)
  bcc32 /L$(BCCDIR)\lib $(CWGAME_OBJECTS) ..\cwlib\chadwick.lib

cwsub:   $(CWSUB_OBJECTS)
  bcc32 /L$(BCCDIR)\lib $(CWSUB_OBJECTS) ..\cwlib\chadwick.lib

clean:
        -erase *.obj
	-erase *.lib
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




