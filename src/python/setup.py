from distutils.core import setup, Extension

setup(name="_libchadwick",
      version="0.3.1",
      py_modules=['libchadwick'],
      description="A library for manipulating Retrosheet baseball data",
      author="Theodore Turocy",
      author_email="turocy@econmail.tamu.edu",
      url="http://chadwick.sourceforge.net",
      ext_modules=[Extension('_libchadwick',
                             ['libchadwick_python.c',
                              'book.c', 'file.c', 'game.c',
                              'gameiter.c', 'league.c', 'parse.c', 'roster.c'],
                             include_dirs=['../cwlib'] )])

