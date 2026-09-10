.. _installation:

Installation
============

Windows
-------

Pre-built binaries for Windows are attached to each release on GitHub:
https://github.com/chadwickbureau/chadwick/releases/latest.  Download
the ``chadwick-x.y.z-bin.zip`` archive and unzip the programs into the
directory where you would like to run them from.  No further
installation is required.

macOS and Linux: building from a release tarball
-------------------------------------------------

If you have downloaded a release source tarball
(``chadwick-x.y.z.tar.gz``) from
https://github.com/chadwickbureau/chadwick/releases, the tarball
already contains a generated ``configure`` script, so building is the
standard three steps::

    ./configure
    make
    make install

``make install`` will typically require ``sudo`` unless you pass
``--prefix`` to ``configure`` to install somewhere in your own home
directory, e.g. ``./configure --prefix=$HOME/.local``.

Building from a git checkout
-----------------------------

The git repository does not include the generated ``configure``
script, so you will need Autoconf, Automake, and Libtool installed
first:

- On macOS (with `Homebrew <https://brew.sh>`_):
  ``brew install autoconf automake libtool``
- On Debian/Ubuntu:
  ``sudo apt install build-essential autoconf automake libtool``

Then, from the top of the checkout, generate the build scripts before
following the usual configure/make steps::

    autoreconf -fi
    ./configure
    make
    make install

You can run the test suite with ``make check`` before installing.

Chadwick has no dependencies beyond a C compiler and the standard
autotools; it is regularly built on both macOS and Linux.
