#
# This file is part of Chadwick
# Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
#                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
#
# FILE: src/python/setup.py
# Build script for Python extension
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

from setuptools import setup, Extension

setup(name="chadwick",
      description="A library for manipulating baseball game-level and play-level data",
      version="0.9.0",
      author="Dr T L Turocy",
      author_email="ted.turocy@gmail.com",
      url="http://chadwick.sourceforge.net",
      packages=['chadwick'],
      ext_modules=[Extension('chadwick._libchadwick',
                             ['chadwick/libchadwick.i',
                              '../cwlib/book.c',
                              '../cwlib/box.c',
                              '../cwlib/file.c',
                              '../cwlib/game.c',
                              '../cwlib/gameiter.c',
                              '../cwlib/league.c',
                              '../cwlib/lint.c',
                              '../cwlib/parse.c',
                              '../cwlib/roster.c'],
                             include_dirs=['../cwlib'],
                             swig_opts=['-I../cwlib'] )]
      )

