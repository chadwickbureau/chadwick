#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for SWIG interface wrapper for Chadwick library
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005, Ted Turocy (turocy@econ.tamu.edu)
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

all: python

#
# The -I flag assumes Chadwick is installed in /usr/local;
# modify as appropriate to your installation.
#
libchadwick_python.c:	libchadwick.i
	swig -python -I/usr/local/include -o libchadwick_python.c libchadwick.i

#
# Modify the -I flag to point to your Python installation, if
# different.
#
libchadwick_python.o:	libchadwick_python.c
	gcc -c -I/usr/include/python2.3 libchadwick_python.c

#
# The -L flag assumes Chadwick is installed in /usr/local;
# modify as appropriate to your installation.
#
python:	libchadwick_python.o
	gcc -shared libchadwick_python.o -L/usr/local/lib -lchadwick -o _libchadwick.so


clean:
	rm -f libchadwick_python.c 
	rm -f libchadwick_python.o 
	rm -f libchadwick.py _libchadwick.so