.. _cwtools.cwbox:

cwbox: Boxscore generator
=========================

:program:`cwbox` generates a boxscore for each game in a play-by-play
or boxscore event file. It replaces the DiamondWare ``BOX`` program.
Unlike :ref:`cwevent <cwtools.cwevent>`, :ref:`cwgame
<cwtools.cwgame>`, and :ref:`cwdaily <cwtools.cwdaily>`, its default
output is a formatted, human-readable report rather than a
comma-delimited table.

The default report contains team batting and pitching lines, an
inning-by-inning line score, and lists of notable events. For
machine-readable tabular output, use :program:`cwgame` for team and
game totals or :program:`cwdaily` for individual player totals.


Command-line options
--------------------

The command has the following form::

   cwbox [options] eventfile...

:program:`cwbox` supports the common ``-h``, ``-i``, ``-y``, ``-s``,
``-e``, and ``-q`` options described in :ref:`the command-line options
table <cwtools.commandline>`. It does not provide selectable fields,
so ``-d``, ``-f``, and ``-n`` are not available.

Two options select alternate output formats:

.. list-table:: cwbox output-format options
   :header-rows: 1
   :widths: 10,40

   * - Switch
     - Description
   * - ``-X``
     - Produce Chadwick's XML boxscore format.
   * - ``-S``
     - Produce SportsML output.

The shared ``-a`` and ``-ft`` options are accepted but do not alter
:program:`cwbox` output. Without ``-X`` or ``-S``, the output is the
plain-text format described below.


Plain-text report
-----------------

Each boxscore begins with the game date, game number when applicable,
visiting and home teams, and the day or night designation when it is
known.

The batting section displays the two teams side by side. Each player
is listed in batting-order sequence with the defensive positions
played, at bats, runs, hits, and runs batted in. Substitutes follow
the player whom they replaced in the same batting-order slot. Team
totals appear after the player lines.

The line score reports runs by inning and the total runs for each
team. An ``x`` marks an unplayed home half-inning. A run total of 10 or
more in a single inning is enclosed in parentheses. When appropriate,
an additional line reports how many outs there were when the game
ended or when the winning run scored.

The pitching section lists each team's pitchers in order of
appearance. Its columns are innings pitched, hits, runs, earned runs,
walks, and strikeouts. Innings pitched are displayed conventionally:
the digit after the decimal point is the number of additional outs,
not a decimal fraction of an inning. Winning, losing, and saving
pitchers are marked ``(W)``, ``(L)``, and ``(S)``. A note identifies a
pitcher who faced one or more batters in an inning without recording
an out.

The remainder of the report is an apparatus of applicable team totals,
individual events, and game metadata. Entries which did not occur are
omitted. The possible labels are:

.. list-table:: Plain-text apparatus labels
   :header-rows: 1
   :widths: 10,40

   * - Label
     - Meaning
   * - ``E``
     - Errors
   * - ``DP``
     - Double plays by team
   * - ``TP``
     - Triple plays by team
   * - ``LOB``
     - Runners left on base by team
   * - ``2B``
     - Doubles
   * - ``3B``
     - Triples
   * - ``HR``
     - Home runs
   * - ``SB``
     - Stolen bases
   * - ``CS``
     - Caught stealing
   * - ``SH``
     - Sacrifice hits
   * - ``SF``
     - Sacrifice flies
   * - ``HBP``
     - Hit batsmen, with the responsible pitcher
   * - ``WP``
     - Wild pitches
   * - ``Balk``
     - Balks
   * - ``PB``
     - Passed balls
   * - ``T``
     - Time of game
   * - ``A``
     - Attendance

Time of game is converted from minutes to hours and minutes. Player
names are obtained from roster files when available. If a roster
cannot be found, :program:`cwbox` warns but continues, using names from
the game's appearance records or player IDs where possible.

Plain-text boxscores are separated by a form-feed character.


XML output
----------

The ``-S`` option instead emits SportsML. When more than one game is
processed, the individual ``sports-content`` elements are enclosed in
a ``sports-content-set`` document.

The ``-X`` option emits Chadwick's XML boxscore representation. It
includes game metadata, the line score, team and player batting and
fielding records, pitching records, and notable-event lists. This is
a Chadwick-specific format.  This is still maintained; however, since it
was created, other tools like :program:`cwsub`, :program:`cwcomment`,
:program:`cwdaily`, and extensions to :program:`cwgame` are preferred ways
to extract data in tabular format and cover the gaps in DiamondWare's
tooling that the XML format was intended to mitigate.
