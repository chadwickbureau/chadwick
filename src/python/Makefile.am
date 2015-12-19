# 
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Automake input file for Chadwick
#
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2002-2007, Ted Turocy (drarbiter@gmail.com)
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

EXTRA_DIST = \
	setup.py \
	libchadwick.py \
	libchadwick.i \
	libchadwick_python.c \
	Makefile.swig \
	Makefile.xc \
	lib2def.py \
	libchadwick.def

#all-local:
#	python setup.py build

#install-exec-local:
#	python setup.py install

#clean-local:
#	rm -rf build

python:	libchadwick.i
	swig -python -o libchadwick_python.c libchadwick.i
