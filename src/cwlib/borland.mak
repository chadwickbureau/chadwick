#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Borland C++ 5.5 makefile to build Chadwick library
#
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

.AUTODEPEND

# The following directories must be set or overriden for the target setup
BCCDIR = C:\BORLAND\BCC55
	
SOURCE_SUFFIX = .c
OBJECT_SUFFIX = .obj

SOURCES = book.c file.c game.c gameiter.c league.c parse.c roster.c
OBJECTS = $(SOURCES:.c=.obj)

CFG = borland.cfg

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }


LINKFLAGS = /c /aa /L$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od

CPPFLAGS = -I$(BCCDIR)\INCLUDE @$(CFG)

all: chadwick

chadwick: $(OBJECTS)
	erase chadwick.lib
  tlib chadwick @&&!
+$(OBJECTS:.obj =.obj +)
!

clean:
        -erase *.obj
	-erase *.lib
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




