.. _documentation:

Command-line tools
##################

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

- :ref:`cwsub <cwtools.cwsub>`, which extracts information
  about in-game player substitutions. This program is unique to
  Chadwick.

- :program:`cwcomment`, which extracts comment
  fields from event files. This program is unique to Chadwick.


This documentation is intended to be read in conjunction with the 
materials provided by Retrosheet summarizing the contents of
play-by-play files and operation of the DiamondWare versions of these
tools.  The Chadwick documentation focuses on filling in gaps in the
Retrosheet documentation, and on detailing Chadwick-specific
extensions the toolset.

.. _cwtools.commandline:

Command-line options
====================

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

.. toctree::
    :maxdepth: 1

    cwevent
    cwgame
    cwsub

