#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Borland C++ 5.5 makefile for Chadwick
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

CHADWICK_TOP = c:\chadwick

all:  cwlib cwtools

cwlib:
	cd $(CHADWICK_TOP)\src\cwlib
	make -f borland

cwtools:
	cd $(CHADWICK_TOP)\src\cwtools
	make -f borland


clean:
	cd $(CHADWICK_TOP)\src\cwlib
	make -f borland clean
	cd $(CHADWICK_TOP)\src\cwtools
	make -f borland clean



