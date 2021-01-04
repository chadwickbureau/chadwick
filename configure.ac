#
# This file is part of Chadwick
# Copyright (c) 2002-2020, Dr T L Turocy (ted.turocy@gmail.com)
#                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
#
# FILE: configure.ac
# Top-level build configuration script
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


AC_INIT([chadwick], [0.9.0])
AC_CONFIG_SRCDIR([src/cwlib/chadwick.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER([dir.h],
                [AC_DEFINE(HAVE_DIR_H)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_MEMBER([struct tm.tm_gmtoff],
	        [AC_DEFINE(HAVE_STRUCT_TM_TM_GMTOFF)],,
                [#include <time.h>])

AC_CHECK_DECL([_timezone],
              [AC_DEFINE(HAVE_UNDERSCORE_TIMEZONE)],,
              [#include <time.h>])

dnl Checks for library functions.

AC_CONFIG_FILES([src/cwlib/Makefile \
          src/cwtools/Makefile \
          src/Makefile \
  	  Makefile])
AC_OUTPUT
