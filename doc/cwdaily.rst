.. _cwtools.cwdaily:

cwdaily: Player game-by-game generator
=======================================

:program:`cwdaily` produces one record for each individual player
appearance in a game. Each record contains identifying information and
the player's batting, pitching, and position-specific fielding totals
for that game. Team totals are not reported by :program:`cwdaily`;
they are available through the standard and extended fields of
:ref:`cwgame <cwtools.cwgame>`.

.. note::

   The name "daily" follows the traditional baseball usage of
   "dailies" or "day-by-day records" for a player's list of per-game
   records. It is a slight misnomer: each record represents one game
   rather than one calendar day, so a player can have multiple records
   on the date of a doubleheader or tripleheader.

:program:`cwdaily` has no DiamondWare equivalent. It outputs up to 154
fields, all of which are included by default. The fields generated can
be selected with the ``-f`` command-line option. The other common
command-line options are described in :ref:`the command-line options
table <cwtools.commandline>`.


Identification fields (0-10)
----------------------------

.. list-table:: cwdaily identification fields
   :header-rows: 1
   :widths: 5,25,15

   * - Field number
     - Description
     - Header
   * - 0
     - Game ID
     - ``GAME_ID``
   * - 1
     - Game date
     - ``GAME_DT``
   * - 2
     - Game number
     - ``GAME_CT``
   * - 3
     - Player appearance date
     - ``APPEAR_DT``
   * - 4
     - Team ID
     - ``TEAM_ID``
   * - 5
     - Player ID
     - ``PLAYER_ID``
   * - 6
     - Batting-order slot
     - ``SLOT_CT``
   * - 7
     - Sequence within batting-order slot
     - ``SEQ_CT``
   * - 8
     - Home-team flag
     - ``HOME_FL``
   * - 9
     - Opponent ID
     - ``OPPONENT_ID``
   * - 10
     - Park ID
     - ``PARK_ID``

``GAME_DT`` is derived from the game's ``info,date`` record and is
reported in ``YYYYMMDD`` form. ``APPEAR_DT`` is intended to identify
the date on which the player entered the game. For starters, it is set
to ``GAME_DT``.

While processing the game, Chadwick maintains a current appearance
date, initially set to ``GAME_DT``. When it encounters a comment whose
text begins with ``suspended,``, it replaces the current appearance
date with the value following that prefix. A substitute's
``APPEAR_DT`` is the current appearance date when the substitution is
processed. Thus a player who first enters after a properly coded
resumption receives the resumed date, while players already in the
game retain the dates on which they originally appeared.

This result depends on the suspension and resumption comments in the
source file. Chadwick does not infer an appearance date from schedules
or other games, and it does not validate or normalize the date taken
from a ``suspended,`` comment. If that coding is absent, misplaced, or
inconsistent, ``APPEAR_DT`` may not reflect the player's actual
appearance date. For boxscore event files, where player statistics are
supplied in extended records rather than reconstructed from plays,
``APPEAR_DT`` is set to ``GAME_DT`` for every player.

``SLOT_CT`` is 1 through 9 for a player in the batting order and 0 for
a pitcher who does not occupy a batting-order slot. ``SEQ_CT`` gives
the player's order among all players who occupied that slot.
``HOME_FL`` is 1 for the home team and 0 for the visiting team.


Batting fields (11-35)
----------------------

.. list-table:: cwdaily batting fields
   :header-rows: 1
   :widths: 5,25,15

   * - Field number
     - Description
     - Header
   * - 11
     - Games played as a batter
     - ``B_G``
   * - 12
     - Plate appearances
     - ``B_PA``
   * - 13
     - At bats
     - ``B_AB``
   * - 14
     - Runs
     - ``B_R``
   * - 15
     - Hits
     - ``B_H``
   * - 16
     - Total bases
     - ``B_TB``
   * - 17
     - Doubles
     - ``B_2B``
   * - 18
     - Triples
     - ``B_3B``
   * - 19
     - Home runs
     - ``B_HR``
   * - 20
     - Grand slams
     - ``B_HR4``
   * - 21
     - Runs batted in
     - ``B_RBI``
   * - 22
     - Game-winning RBI
     - ``B_GW``
   * - 23
     - Walks
     - ``B_BB``
   * - 24
     - Intentional walks
     - ``B_IBB``
   * - 25
     - Strikeouts
     - ``B_SO``
   * - 26
     - Grounded into double plays
     - ``B_GDP``
   * - 27
     - Hit by pitch
     - ``B_HP``
   * - 28
     - Sacrifice hits
     - ``B_SH``
   * - 29
     - Sacrifice flies
     - ``B_SF``
   * - 30
     - Stolen bases
     - ``B_SB``
   * - 31
     - Caught stealing
     - ``B_CS``
   * - 32
     - Reached on interference
     - ``B_XI``
   * - 33
     - Games as designated hitter
     - ``B_G_DH``
   * - 34
     - Games as pinch hitter
     - ``B_G_PH``
   * - 35
     - Games as pinch runner
     - ``B_G_PR``


Pitching fields (36-70)
-----------------------

.. list-table:: cwdaily pitching fields
   :header-rows: 1
   :widths: 5,25,15

   * - Field number
     - Description
     - Header
   * - 36
     - Games pitched
     - ``P_G``
   * - 37
     - Games started
     - ``P_GS``
   * - 38
     - Complete games
     - ``P_CG``
   * - 39
     - Shutouts
     - ``P_SHO``
   * - 40
     - Games finished
     - ``P_GF``
   * - 41
     - Wins
     - ``P_W``
   * - 42
     - Losses
     - ``P_L``
   * - 43
     - Saves
     - ``P_SV``
   * - 44
     - Outs recorded
     - ``P_OUT``
   * - 45
     - Batters faced
     - ``P_TBF``
   * - 46
     - At bats against
     - ``P_AB``
   * - 47
     - Runs allowed
     - ``P_R``
   * - 48
     - Earned runs allowed
     - ``P_ER``
   * - 49
     - Hits allowed
     - ``P_H``
   * - 50
     - Total bases allowed
     - ``P_TB``
   * - 51
     - Doubles allowed
     - ``P_2B``
   * - 52
     - Triples allowed
     - ``P_3B``
   * - 53
     - Home runs allowed
     - ``P_HR``
   * - 54
     - Grand slams allowed
     - ``P_HR4``
   * - 55
     - Walks allowed
     - ``P_BB``
   * - 56
     - Intentional walks allowed
     - ``P_IBB``
   * - 57
     - Strikeouts
     - ``P_SO``
   * - 58
     - Grounded into double plays
     - ``P_GDP``
   * - 59
     - Hit batsmen
     - ``P_HP``
   * - 60
     - Sacrifice hits against
     - ``P_SH``
   * - 61
     - Sacrifice flies against
     - ``P_SF``
   * - 62
     - Reached on interference
     - ``P_XI``
   * - 63
     - Wild pitches
     - ``P_WP``
   * - 64
     - Balks
     - ``P_BK``
   * - 65
     - Inherited runners
     - ``P_IR``
   * - 66
     - Inherited runners who scored
     - ``P_IRS``
   * - 67
     - Ground-ball outs
     - ``P_GO``
   * - 68
     - Air outs
     - ``P_AO``
   * - 69
     - Pitches
     - ``P_PITCH``
   * - 70
     - Strikes
     - ``P_STRIKE``

``P_OUT`` records outs rather than fractional innings; divide by
three to obtain innings pitched. If a pitcher has more than one
pitching stint in a game, the pitching fields combine all of those
stints.

``P_PITCH`` and ``P_STRIKE`` are reported only when the game's
``info,pitches`` value is ``pitches``. They are blank when complete
pitch data is not indicated.


Fielding fields (71-153)
------------------------

Fielding statistics are reported separately for each defensive
position. The table gives the exact field number and header in each
cell. The common suffixes have these meanings:

- ``G``: games at the position;
- ``GS``: games started at the position;
- ``OUT``: outs recorded while at the position;
- ``TC``: total chances;
- ``PO``: putouts;
- ``A``: assists;
- ``E``: errors;
- ``DP``: double plays; and
- ``TP``: triple plays.

As with ``P_OUT``, an ``OUT`` field records outs rather than
fractional innings; divide by three to obtain innings fielded.

.. list-table:: cwdaily fielding fields
   :header-rows: 1
   :widths: 8,10,10,10,10,10,10,10,10,10

   * - Position
     - G
     - GS
     - OUT
     - TC
     - PO
     - A
     - E
     - DP
     - TP
   * - Pitcher
     - 71 ``F_P_G``
     - 72 ``F_P_GS``
     - 73 ``F_P_OUT``
     - 74 ``F_P_TC``
     - 75 ``F_P_PO``
     - 76 ``F_P_A``
     - 77 ``F_P_E``
     - 78 ``F_P_DP``
     - 79 ``F_P_TP``
   * - Catcher
     - 80 ``F_C_G``
     - 81 ``F_C_GS``
     - 82 ``F_C_OUT``
     - 83 ``F_C_TC``
     - 84 ``F_C_PO``
     - 85 ``F_C_A``
     - 86 ``F_C_E``
     - 87 ``F_C_DP``
     - 88 ``F_C_TP``
   * - First base
     - 91 ``F_1B_G``
     - 92 ``F_1B_GS``
     - 93 ``F_1B_OUT``
     - 94 ``F_1B_TC``
     - 95 ``F_1B_PO``
     - 96 ``F_1B_A``
     - 97 ``F_1B_E``
     - 98 ``F_1B_DP``
     - 99 ``F_1B_TP``
   * - Second base
     - 100 ``F_2B_G``
     - 101 ``F_2B_GS``
     - 102 ``F_2B_OUT``
     - 103 ``F_2B_TC``
     - 104 ``F_2B_PO``
     - 105 ``F_2B_A``
     - 106 ``F_2B_E``
     - 107 ``F_2B_DP``
     - 108 ``F_2B_TP``
   * - Third base
     - 109 ``F_3B_G``
     - 110 ``F_3B_GS``
     - 111 ``F_3B_OUT``
     - 112 ``F_3B_TC``
     - 113 ``F_3B_PO``
     - 114 ``F_3B_A``
     - 115 ``F_3B_E``
     - 116 ``F_3B_DP``
     - 117 ``F_3B_TP``
   * - Shortstop
     - 118 ``F_SS_G``
     - 119 ``F_SS_GS``
     - 120 ``F_SS_OUT``
     - 121 ``F_SS_TC``
     - 122 ``F_SS_PO``
     - 123 ``F_SS_A``
     - 124 ``F_SS_E``
     - 125 ``F_SS_DP``
     - 126 ``F_SS_TP``
   * - Left field
     - 127 ``F_LF_G``
     - 128 ``F_LF_GS``
     - 129 ``F_LF_OUT``
     - 130 ``F_LF_TC``
     - 131 ``F_LF_PO``
     - 132 ``F_LF_A``
     - 133 ``F_LF_E``
     - 134 ``F_LF_DP``
     - 135 ``F_LF_TP``
   * - Center field
     - 136 ``F_CF_G``
     - 137 ``F_CF_GS``
     - 138 ``F_CF_OUT``
     - 139 ``F_CF_TC``
     - 140 ``F_CF_PO``
     - 141 ``F_CF_A``
     - 142 ``F_CF_E``
     - 143 ``F_CF_DP``
     - 144 ``F_CF_TP``
   * - Right field
     - 145 ``F_RF_G``
     - 146 ``F_RF_GS``
     - 147 ``F_RF_OUT``
     - 148 ``F_RF_TC``
     - 149 ``F_RF_PO``
     - 150 ``F_RF_A``
     - 151 ``F_RF_E``
     - 152 ``F_RF_DP``
     - 153 ``F_RF_TP``

The catcher has two additional fields:

.. list-table:: Additional catcher fielding fields
   :header-rows: 1
   :widths: 5,25,15

   * - Field number
     - Description
     - Header
   * - 89
     - Passed balls
     - ``F_C_PB``
   * - 90
     - Catcher's interference
     - ``F_C_XI``

``TC`` is calculated as putouts plus assists plus errors. It is blank
if any of those three component statistics is unavailable.


Missing and inapplicable statistics
-----------------------------------

An unavailable statistic, whether because it is not reported or not able to
be computed from the data, is emitted as a blank field. A statistic
which is computable from the data, or which does not apply because
the player did not appear in that role or position, is reported as zero.
In particular, Retrosheet boxscore files use "-1" to indicate null values;
these are correctly rendered as null (blank) values.
