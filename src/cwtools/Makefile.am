# 
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Automake input file for Chadwick command-line tools
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

bin_PROGRAMS = cwevent cwgame cwsub 

AM_CPPFLAGS = -I$(top_srcdir)/src


cwevent_SOURCES = cwevent.c cwtools.c

cwevent_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwgame_SOURCES = cwgame.c cwtools.c

cwgame_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwsub_SOURCES = cwsub.c cwtools.c

cwsub_LDADD = $(top_builddir)/src/cwlib/libchadwick.la

