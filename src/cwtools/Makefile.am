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

bin_PROGRAMS = cwbox cwevent cwgame cwlog cwsub 

AM_CPPFLAGS = -I$(top_srcdir)/src


cwbox_SOURCES = cwbox.c cwtools.c cwboxxml.c cwboxsml.c xmlwrite.c xmlwrite.h

cwbox_LDADD = $(top_builddir)/src/cwlib/libchadwick.la 


cwevent_SOURCES = cwevent.c cwtools.c

cwevent_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwgame_SOURCES = cwgame.c cwtools.c

cwgame_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwlog_SOURCES = cwlog.c cwtools.c

cwlog_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwsub_SOURCES = cwsub.c cwtools.c

cwsub_LDADD = $(top_builddir)/src/cwlib/libchadwick.la

