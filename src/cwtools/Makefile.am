#
# This file is part of Chadwick
# Copyright (c) 2002-2023, Dr T L Turocy (ted.turocy@gmail.com)
#                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
#
# FILE: src/cwtools/Makefile.am
# Automake input script for cwtools subdirectory
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

bin_PROGRAMS = cwbox cwcomment cwevent cwgame cwsub cwdaily

AM_CPPFLAGS = -I$(top_srcdir)/src


cwbox_SOURCES = cwbox.c cwtools.c cwboxxml.c cwboxsml.c xmlwrite.c xmlwrite.h

cwbox_LDADD = $(top_builddir)/src/cwlib/libchadwick.la 


cwcomment_SOURCES = cwcomment.c cwtools.c

cwcomment_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwevent_SOURCES = cwevent.c cwtools.c

cwevent_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwgame_SOURCES = cwgame.c cwtools.c

cwgame_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwsub_SOURCES = cwsub.c cwtools.c

cwsub_LDADD = $(top_builddir)/src/cwlib/libchadwick.la


cwdaily_SOURCES = cwdaily.c cwtools.c

cwdaily_LDADD = $(top_builddir)/src/cwlib/libchadwick.la
