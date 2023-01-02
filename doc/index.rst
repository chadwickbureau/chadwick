=======================================================
 Chadwick: Software Tools for Game-Level Baseball Data
=======================================================

Introduction
============

Chadwick is a collection of command-line utility programs for extracting
information from baseball play-by-play and boxscore files in the
DiamondWare format, as used by Retrosheet (http://www.retrosheet.org).


Author
-------

Chadwick is written, maintained, and Copyright 2002-2023 by
Dr T. L. Turocy (ted.turocy <aht> gmail <daht> com) 
at Chadwick Baseball Bureau (http://www.chadwick-bureau.com).

License
-------

Chadwick is licensed under the terms of the GNU General Public License.
If the GPL doesn't meet your needs, contact the author for other licensing
possibilities.

Development
-----------

The Chadwick source code is managed using git, at
https://github.com/chadwickbureau/chadwick.

Bugs in Chadwick should be reported to the issue tracker on github at
https://github.com/chadwickbureau/chadwick/issues.
Please be as specific as possible in reporting a bug, including the
version of Chadwick you are using, the operating system(s) you're
using, and a detailed list of steps to reproduce the issue.

Community
---------

To get the latest news on the Chadwick tool suite, you can:

- Subscribe to the Chadwick Baseball Bureau's twitter feed
  (@chadwickbureau);
- Like the Chadwick Baseball Bureau on Facebook;
- Read the Chadwick Baseball Bureau's blog at
  (http://www.chadwick-bureau.com/blog/)

Acknowledgments
---------------

The author thanks `Sports Reference, LLC <http://www.sports-reference.com>`_,
the `Society for American Baseball Research <http://www.sabr.org>`_,
and `XMLTeam, Inc. <http://www.xmlteam.com>`_
for support in the development of portions of
Chadwick. The author also thanks David Smith of 
`Retrosheet <http://www.retrosheet.org>`_ for his
always-gracious assistance and guidance.


Command-line tools
==================

Chadwick provides the following command-line programs for extracting
information from Retrosheet play-by-play event files:

- :ref:`cwevent <cwtools.cwevent>`, an expanded event
  descriptor, which replaces and extends the DiamondWare program
  BEVENT.

- :ref:`cwgame <cwtools.cwgame>`, a game information
  extractor, which replaces and extends the DiamondWare program
  BGAME.

- :program:`cwbox`, a boxscore generator, which
  replaces and extends the DiamondWare program BOX.

- :program:`cwdaily`, which produces day-by-day player statistical
  totals.  This program is unique to Chadwick.
  
- :ref:`cwsub <cwtools.cwsub>`, which extracts information
  about in-game player substitutions. This program is unique to
  Chadwick.

- :program:`cwcomment`, which extracts comment
  fields from event files. This program is unique to Chadwick.

This documentation is intended to be read in conjunction with the 
materials provided by Retrosheet (see
https://www.retrosheet.org/game.htm)
summarizing the contents of
play-by-play files and operation of the DiamondWare versions of these
tools.  The Chadwick documentation focuses on filling in gaps in the
Retrosheet documentation, and on detailing Chadwick-specific
extensions the toolset.

.. _cwtools.commandline:

Command-line options
--------------------

Each of the command-line tools shares a common set of options
controlling their behavior. These are detailed in the following
table.

.. list-table:: Common command-line options and their effects
   :header-rows: 1
   :widths: 10,40

   * - Switch
     - Description
   * - ``-a``
     - Generate ASCII comma-delimited files (default)
   * - ``-d``
     - Print a list of the available fields and descriptions (for use with ``-f``)
   * - ``-e mmdd``
     - The latest date to process (inclusive)
   * - ``-f flist``
     - List of fields to output. The default list can be viewed with ``-h``; the list of available fields can be viewed with ``-d``
   * - ``-ft``
     - Generate FORTRAN format files.
   * - ``-h``
     - Prints description and usage information for the tool.
   * - ``-i *gameid*``
     - Only process the game with ID ``gameid``
   * - ``-n``
     - If in ASCII mode (the default), the first row of the output is a comma-separated list of column headers.
   * - ``-s mmdd``
     - The earliest date to process (inclusive)
   * - ``-y``
     - Specifies the year to use (four digits)


Documentation of individual tools
---------------------------------

.. toctree::
    :maxdepth: 1

    cwevent
    cwgame
    cwsub


.. Indices and tables
.. ------------------
.. 
.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`

