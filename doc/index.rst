===============================================================
 Chadwick: A Retrosheet and DiamondWare baseball data processor
===============================================================

Introduction
============

Chadwick is a suite of command-line tools for extracting statistics and
play-by-play data from Retrosheet's DiamondWare-format event and boxscore
files (https://www.retrosheet.org).

.. note::
   **Getting Chadwick.** Windows users can download ready-to-run
   binaries from the `latest GitHub release
   <https://github.com/chadwickbureau/chadwick/releases/latest>`_.
   macOS/Linux users, and anyone building from source, should see
   :doc:`installation`.


Author
-------

Chadwick is written, maintained, and Copyright 2002-2026 by
Dr T. L. Turocy (ted.turocy <aht> gmail <daht> com)
at Chadwick Baseball Bureau (https://www.chadwick-bureau.com).

License
-------

Chadwick is licensed under the terms of the GNU General Public License.
If the GPL doesn't meet your needs, contact the author for other licensing
possibilities.

Command-line tools
==================

Chadwick provides six command-line programs, each reading Retrosheet
play-by-play or boxscore event files and extracting a specific kind of
tabular data:

.. grid:: 1 2 2 3
   :gutter: 3

   .. grid-item-card:: ⚾ cwevent
      :link: cwtools.cwevent
      :link-type: ref

      Expanded event descriptor. Extracts detailed information about
      individual plays, replacing and extending DiamondWare's BEVENT.

   .. grid-item-card:: 🏟️ cwgame
      :link: cwtools.cwgame
      :link-type: ref

      Game information extractor. Extracts per-game summary and team
      totals, replacing and extending DiamondWare's BGAME.

   .. grid-item-card:: 📋 cwbox
      :link: cwtools.cwbox
      :link-type: ref

      Boxscore generator. Produces a human-readable boxscore report,
      replacing and extending DiamondWare's BOX.

   .. grid-item-card:: 📅 cwdaily
      :link: cwtools.cwdaily
      :link-type: ref

      Player game-by-game generator. Produces one record per player
      per game, with batting, pitching, and fielding totals. Unique
      to Chadwick.

   .. grid-item-card:: 🔄 cwsub
      :link: cwtools.cwsub
      :link-type: ref

      Player substitution descriptor. Extracts in-game substitutions,
      complementing cwevent. Unique to Chadwick.

   .. grid-item-card:: 💬 cwcomment
      :link: cwtools.cwcomment
      :link-type: ref

      Comment extractor. Extracts comment fields, including ejections
      and umpire changes. Unique to Chadwick.


Development
-----------

Chadwick development can be found at https://github.com/chadwickbureau/chadwick.

Bugs in Chadwick should be reported to the issue tracker on github at
https://github.com/chadwickbureau/chadwick/issues.
Please be as specific as possible in reporting a bug, including the
version of Chadwick you are using, the operating system(s) you're
using, and a detailed list of steps to reproduce the issue.

Acknowledgments
---------------

The author thanks `Sports Reference, LLC <https://www.sports-reference.com>`_,
the `Society for American Baseball Research <https://www.sabr.org>`_,
and `XMLTeam, Inc. <https://www.xmlteam.com>`_
for support in the development of portions of
Chadwick. The author also thanks David Smith of
`Retrosheet <https://www.retrosheet.org>`_ for his
always-gracious assistance and guidance.


.. toctree::
    :maxdepth: 1
    :hidden:

    installation
    commandline
    cwevent
    cwgame
    cwbox
    cwdaily
    cwsub
    cwcomment
