## 
## $Source$
## $Date$
## $Revision$
##
## DESCRIPTION:
## Makefile for cross-compiling the Chadwick Python extension
##
## This file is part of Chadwick, a library for baseball play-by-play and stats
## Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##


#
# Instructions:
#
# (1) Copy all the *.c and *.h files from src/cwlib
# (2) Modify PYVER and PYDIR to point to the Python installation
#     to cross-compile to
# (3) Issue a "make -f Makefile.xc"
#
# Note: Make sure that there's no libchadwick_python.o sitting around,
# since one may be there from a native build.
#
# You should wind up with a _chadwick.pyd suitable for use under Windows
#

CC=i586-mingw32msvc-gcc
NM=i586-mingw32msvc-nm
DLLWRAP=i586-mingw32msvc-dllwrap
DLLTOOL=i586-mingw32msvc-dlltool
PYVER=23
PYDIR=/home/arbiter/.wine/fake_windows/Python$(PYVER)
INCLUDES=-I$(PYDIR)/include -I/usr/local/include
LDFLAGS=-L$(PYDIR)/libs
LIBS=-lpython$(PYVER)
CFLAGS=-fno-strict-aliasing -DNDEBUG -g -O3 -Wall -Wstrict-prototypes $(INCLUDES)
LIB2DEF=./lib2def.py

LIBPY_DEF=$(PYDIR)/libs/libpython$(PYVER).def
LIBPY_A=$(PYDIR)/libs/libpython$(PYVER).a

MODULE=libchadwick
INPUT=book.o box.o file.o game.o gameiter.o league.o parse.o roster.o libchadwick_python.o
#OUTPUT=$(PYDIR)/Lib/site-packages/$(MODULE).pyd
OUTPUT=$(MODULE).pyd
MODULE_DEF=$(MODULE).def

all: $(OUTPUT)

$(LIBPY_DEF): $(PYDIR)/libs/python$(PYVER).lib
	NM=$(NM) PYVER=$(PYVER) $(LIB2DEF) $^ $@

$(LIBPY_A): $(LIBPY_DEF)
	$(DLLTOOL) --dllname python$(PYVER).dll --def $^ --output-lib $@

$(OUTPUT): $(INPUT) $(LIBPY_A) $(MODULE_DEF)
	$(DLLWRAP) --dllname $(MODULE).pyd --driver-name $(CC) --def $(MODULE_DEF) -o $@ $(INPUT) -s --entry _DllMain@12 --target=i586-mingw32 $(LDFLAGS) $(LIBS)
	mv libchadwick.pyd _libchadwick.pyd
