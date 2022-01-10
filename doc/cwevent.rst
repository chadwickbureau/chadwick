.. _cwtools.cwevent:

cwevent: Expanded event descriptor
==================================

:program:`cwevent` is a command-line tool which extracts detailed
information about individual events (plays) in the data file. These
are grouped into two categories. There are 97 fields which are
compatible with the Retrosheet BEVENT event descriptor tool.  These
are specified using the ``-f`` command-line flag. In addition, cwevent
offers a number "extended" fields which expand upon or give more
detailed information not easily accessed via the standard
fields. These are are specified using the ``-x`` command-line flag.

.. note::

   :program:`cwevent` guarantees that the standard field numbers will
   match those used by BEVENT.  Standard field numbers therefore can
   be treated as stable, and it should be safe to write scripts
   referring to them.  Extended fields are provisional, and extended
   fields may be added or withdrawn in future versions.  Extended
   fields are assigned numbers to maintain a logical cohesion, with
   related fields being grouped.  Therefore, extended field numbers
   are not promised to be stable.  It is recommended to use the field
   labels instead in writing scripts to process the output of
   :program:`cwevent` extended fields.


.. list-table:: cwevent standard field numbers
   :header-rows: 1
   :widths: 5,20,10

   * - Field number
     - Description
     - Header
   * - 0
     - Game ID
     - ``GAME_ID``
   * - 1
     - Visiting team
     - ``AWAY_TEAM_ID``
   * - 2
     - Inning
     - ``INN_CT``
   * - 3
     - Batting team
     - ``BAT_HOME_ID``
   * - 4
     - Outs
     - ``OUTS_CT``
   * - 5
     - Balls
     - ``BALLS_CT``
   * - 6
     - Strikes
     - ``STRIKES_CT``
   * - 7
     - Pitch sequence
     - ``PITCH_SEQ_TX``
   * - 8
     - Visitor score
     - ``AWAY_SCORE_CT``
   * - 9
     - Home score
     - ``HOME_SCORE_CT``
   * - 10
     - :ref:`Batter <cwtools.cwevent.resplayers>`
     - ``BAT_ID``
   * - 11
     - :ref:`Batter hand <cwtools.cwevent.resplayers>`
     - ``BAT_HAND_CD``
   * - 12
     - :ref:`Result batter <cwtools.cwevent.resplayers>`
     - ``RESP_BAT_ID``
   * - 13
     - :ref:`Result batter hand <cwtools.cwevent.resplayers>`
     - ``RESP_BAT_HAND_CD``
   * - 14
     - :ref:`Pitcher <cwtools.cwevent.resplayers>`
     - ``PIT_ID``
   * - 15
     - :ref:`Pitcher hand <cwtools.cwevent.resplayers>`
     - ``PIT_HAND_CD``
   * - 16
     - :ref:`Result pitcher <cwtools.cwevent.resplayers>`
     - ``RESP_PIT_ID``
   * - 17
     - :ref:`Result pitcher hand <cwtools.cwevent.resplayers>`
     - ``RESP_PIT_HAND_CD``
   * - 18
     - Catcher
     - ``POS2_FLD_ID``
   * - 19
     - First baseman
     - ``POS3_FLD_ID``
   * - 20
     - Second baseman
     - ``POS4_FLD_ID``
   * - 21
     - Third baseman
     - ``POS5_FLD_ID``
   * - 22
     - Shortstop
     - ``POS6_FLD_ID``
   * - 23
     - Left fielder  
     - ``POS7_FLD_ID``
   * - 24
     - Center fielder
     - ``POS8_FLD_ID``
   * - 25
     - Right fielder
     - ``POS9_FLD_ID`` 
   * - 26
     - Runner on first
     - ``BASE1_RUN_ID``
   * - 27
     - Runner on second
     - ``BASE2_RUN_ID``
   * - 28
     - Runner on third
     - ``BASE3_RUN_ID``
   * - 29
     - Event text
     - ``EVENT_TX``
   * - 30
     - Leadoff flag
     - ``LEADOFF_FL``
   * - 31
     - :ref:`Pinch-hit flag <cwtools.cwevent.phflag>`
     - ``PH_FL``
   * - 32
     - Defensive position
     - ``BAT_FLD_CD``
   * - 33
     - Lineup position
     - ``BAT_LINEUP_ID`` 
   * - 34
     - :ref:`Event type <cwtools.cwevent.eventtype>`
     - ``EVENT_CD`` 
   * - 35
     - Batter event flag
     - ``BAT_EVENT_FL``
   * - 36
     - :ref:`Official time at bat flag <cwtools.cwevent.sacflags>`
     - ``AB_FL``
   * - 37
     - Hit value
     - ``H_CD``
   * - 38
     - :ref:`Sacrifice hit flag <cwtools.cwevent.sacflags>`
     - ``SH_FL``
   * - 39
     - :ref:`Sacrifice fly flag <cwtools.cwevent.sacflags>`
     - ``SF_FL``
   * - 40
     - Outs on play
     - ``EVENT_OUTS_CT``
   * - 41
     - Double play flag
     - ``DP_FL``
   * - 42
     - Triple play flag
     - ``TP_FL``
   * - 43
     - RBI on play
     - ``RBI_CT``
   * - 44
     - Wild pitch flag
     - ``WP_FL``
   * - 45
     - Passed ball flag
     - ``PB_FL``
   * - 46
     - Fielded by
     - ``FLD_CD``
   * - 47
     - Batted ball type
     - ``BATTEDBALL_CD``
   * - 48
     - Bunt flag
     - ``BUNT_FL``
   * - 49
     - Foul flag
     - ``FOUL_FL``
   * - 50
     - Hit location
     - ``BATTEDBALL_LOC_TX``
   * - 51
     - :ref:`Number of errors <cwtools.cwevent.errors>`
     - ``ERR_CT``
   * - 52
     - :ref:`1st error player <cwtools.cwevent.errors>`
     - ``ERR1_FLD_CD``
   * - 53
     - :ref:`1st error type <cwtools.cwevent.errors>`
     - ``ERR1_CD``
   * - 54
     - :ref:`2nd error player <cwtools.cwevent.errors>`
     - ``ERR2_FLD_CD``
   * - 55
     - :ref:`2nd error type <cwtools.cwevent.errors>`
     - ``ERR2_CD``
   * - 56
     - :ref:`3rd error player <cwtools.cwevent.errors>`
     - ``ERR3_FLD_CD``
   * - 57
     - :ref:`3rd error type <cwtools.cwevent.errors>`
     - ``ERR3_CD``
   * - 58
     - :ref:`Batter destination <cwtools.cwevent.plays>`
     - ``BAT_DEST_ID``
   * - 59
     - :ref:`Runner on first destination <cwtools.cwevent.plays>`
     - ``RUN1_DEST_ID``
   * - 60
     - :ref:`Runner on second destination <cwtools.cwevent.plays>`
     - ``RUN2_DEST_ID``
   * - 61
     - :ref:`Runner on third destination <cwtools.cwevent.plays>`
     - ``RUN3_DEST_ID``
   * - 62
     - :ref:`Play on batter <cwtools.cwevent.plays>`
     - ``BAT_PLAY_TX``
   * - 63
     - :ref:`Play on runner on first <cwtools.cwevent.plays>`
     - ``RUN1_PLAY_TX``
   * - 64
     - :ref:`Play on runner on second <cwtools.cwevent.plays>`
     - ``RUN2_PLAY_TX``
   * - 65
     - :ref:`Play on runner on third <cwtools.cwevent.plays>`
     - ``RUN3_PLAY_TX``
   * - 66
     - Stolen base for runner on first
     - ``RUN1_SB_FL``
   * - 67
     - Stolen base for runner on second
     - ``RUN2_SB_FL``
   * - 68
     - Stolen base for runner on third
     - ``RUN3_SB_FL``
   * - 69
     - Caught stealing for runner on first
     - ``RUN1_CS_FL``
   * - 70
     - Caught stealing for runner on second
     - ``RUN2_CS_FL``
   * - 71
     - Caught stealing for runner on third
     - ``RUN3_CS_FL``
   * - 72
     - Pickoff of runner on first
     - ``RUN1_PK_FL``
   * - 73
     - Pickoff of runner on second
     - ``RUN2_PK_FL``
   * - 74
     - Pickoff of runner on third
     - ``RUN3_PK_FL``
   * - 75
     - :ref:`Pitcher charged with runner on first <cwtools.cwevent.responsibility>`
     - ``RUN1_RESP_PIT_ID``
   * - 76
     - :ref:`Pitcher charged with runner on second <cwtools.cwevent.responsibility>`
     - ``RUN2_RESP_PIT_ID``
   * - 77
     - :ref:`Pitcher charged with runner on third <cwtools.cwevent.responsibility>`
     - ``RUN3_RESP_PIT_ID``
   * - 78
     - New game flag
     - ``GAME_NEW_FL``
   * - 79
     - End game flag
     - ``GAME_END_FL``
   * - 80
     - Pinch-runner on first
     - ``PR_RUN1_FL``
   * - 81
     - Pinch-runner on second
     - ``PR_RUN2_FL``
   * - 82
     - Pinch-runner on third
     - ``PR_RUN3_FL``
   * - 83
     - Runner removed for pinch-runner on first
     - ``REMOVED_FOR_PR_RUN1_ID``
   * - 84
     - Runner removed for pinch-runner on second
     - ``REMOVED_FOR_PR_RUN2_ID``
   * - 85
     - Runner removed for pinch-runner on third
     - ``REMOVED_FOR_PR_RUN3_ID``
   * - 86
     - Batter removed for pinch-hitter
     - ``REMOVED_FOR_PH_BAT_ID``
   * - 87
     - Position of batter removed for pinch-hitter
     - ``REMOVED_FOR_PH_BAT_FLD_CD``
   * - 88
     - :ref:`Fielder with first putout <cwtools.cwevent.fielding>`
     - ``PO1_FLD_CD``
   * - 89
     - :ref:`Fielder with second putout <cwtools.cwevent.fielding>`
     - ``PO2_FLD_CD``
   * - 90
     - :ref:`Fielder with third putout <cwtools.cwevent.fielding>`
     - ``PO3_FLD_CD``
   * - 91
     - :ref:`Fielder with first assist <cwtools.cwevent.fielding>`
     - ``ASS1_FLD_CD``
   * - 92
     - :ref:`Fielder with second assist <cwtools.cwevent.fielding>`
     - ``ASS2_FLD_CD``
   * - 93
     - :ref:`Fielder with third assist <cwtools.cwevent.fielding>`
     - ``ASS3_FLD_CD``
   * - 94
     - :ref:`Fielder with fourth assist <cwtools.cwevent.fielding>`
     - ``ASS4_FLD_CD``
   * - 95
     - :ref:`Fielder with fifth assist <cwtools.cwevent.fielding>`
     - ``ASS5_FLD_CD``
   * - 96
     - Event number
     - ``EVENT_ID``


.. list-table:: cwevent extended field numbers
   :header-rows: 1
   :widths: 5,20,10

   * - Field number
     - Description
     - Header
   * - 0
     - home team id
     - ``HOME_TEAM_ID``
   * - 1
     - batting team id
     - ``BAT_TEAM_ID``
   * - 2
     - fielding team id
     - ``FLD_TEAM_ID``
   * - 3
     - half inning (differs from batting team if home team bats first)
     - ``BAT_LAST_ID``
   * - 4
     - start of half inning flag
     - ``INN_NEW_FL``
   * - 5
     - end of half inning flag
     - ``INN_END_FL``
   * - 6
     - score for team on offense
     - ``START_BAT_SCORE_CT``
   * - 7
     - score for team on defense
     - ``START_FLD_SCORE_CT``
   * - 8
     - runs scored in this half inning
     - ``INN_RUNS_CT``
   * - 9
     - number of plate appearances in game for team on offense
     - ``GAME_PA_CT``
   * - 10
     - number of plate appearances in inning for team on offense
     - ``INN_PA_CT``
   * - 11
     - start of plate appearance flag
     - ``PA_NEW_FL``
   * - 12
     - truncated plate appearance flag
     - ``PA_TRUNC_FL``
   * - 13
     - base state at start of play
     - ``START_BASES_CD``
   * - 14
     - base state at end of play
     - ``END_BASES_CD``
   * - 15
     - batter is starter flag
     - ``BAT_START_FL``
   * - 16
     - result batter is starter flag
     - ``RESP_BAT_START_FL``
   * - 17
     - ID of the batter on deck
     - ``BAT_ON_DECK_ID``
   * - 18
     - ID of the batter in the hold
     - ``BAT_IN_HOLD_ID``
   * - 19
     - pitcher is starter flag
     - ``PIT_START_FL``
   * - 20
     - result pitcher is starter flag
     - ``RESP_PIT_START_FL``
   * - 21
     - defensive position of runner on first
     - ``RUN1_FLD_CD``
   * - 22
     - lineup position of runner on first
     - ``RUN1_LINEUP_CD``
   * - 23
     - event number on which runner on first reached base
     - ``RUN1_ORIGIN_EVENT_ID``
   * - 24
     - defensive position of runner on second
     - ``RUN2_FLD_CD``
   * - 25
     - lineup position of runner on second
     - ``RUN2_LINEUP_CD``
   * - 26
     - event number on which runner on second reached base
     - ``RUN2_ORIGIN_EVENT_ID``
   * - 27
     - defensive position of runner on third
     - ``RUN3_FLD_CD``
   * - 28
     - lineup position of runner on third
     - ``RUN3_LINEUP_CD``
   * - 29
     - event number on which runner on third reached base
     - ``RUN3_ORIGIN_EVENT_ID``
   * - 30
     - responsible catcher for runner on first
     - ``RUN1_RESP_CAT_ID``
   * - 31
     - responsible catcher for runner on second
     - ``RUN2_RESP_CAT_ID``
   * - 32
     - responsible catcher for runner on third
     - ``RUN3_RESP_CAT_ID``
   * - 33
     - number of balls in plate appearance
     - ``PA_BALL_CT``
   * - 34
     - number of called balls in plate appearance
     - ``PA_CALLED_BALL_CT``
   * - 35
     - number of intentional balls in plate appearance
     - ``PA_INTENT_BALL_CT``
   * - 36
     - number of pitchouts in plate appearance
     - ``PA_PITCHOUT_BALL_CT``
   * - 37
     - number of pitches hitting batter in plate appearance
     - ``PA_HITBATTER_BALL_CT``
   * - 38
     - number of other balls in plate appearance
     - ``PA_OTHER_BALL_CT``
   * - 39
     - number of strikes in plate appearance
     - ``PA_STRIKE_CT``
   * - 40
     - number of called strikes in plate appearance
     - ``PA_CALLED_STRIKE_CT``
   * - 41
     - number of swinging strikes in plate appearance
     - ``PA_SWINGMISS_STRIKE_CT``
   * - 42
     - number of foul balls in plate appearance
     - ``PA_FOUL_STRIKE_CT``
   * - 43
     - number of balls in play in plate appearance
     - ``PA_INPLAY_STRIKE_CT``
   * - 44
     - number of other strikes in plate appearance
     - ``PA_OTHER_STRIKE_CT``
   * - 45
     - number of runs on play
     - ``EVENT_RUNS_CT``
   * - 46
     - id of player fielding batted ball
     - ``FLD_ID``
   * - 47
     - force play at second flag
     - ``BASE2_FORCE_FL``
   * - 48
     - force play at third flag
     - ``BASE3_FORCE_FL``
   * - 49
     - force play at home flag
     - ``BASE4_FORCE_FL``
   * - 50
     - batter safe on error flag
     - ``BAT_SAFE_ERR_FL``
   * - 51
     - fate of batter (base ultimately advanced to)
     - ``BAT_FATE_ID``
   * - 52
     - fate of runner on first
     - ``RUN1_FATE_ID``
   * - 53
     - fate of runner on second
     - ``RUN2_FATE_ID``
   * - 54
     - fate of runner on third
     - ``RUN3_FATE_ID``
   * - 55
     - runs scored in half inning after this event
     - ``FATE_RUNS_CT``
   * - 56
     - fielder with sixth assist
     - ``ASS6_FLD_CD``
   * - 57
     - fielder with seventh assist
     - ``ASS7_FLD_CD``
   * - 58
     - fielder with eighth assist
     - ``ASS8_FLD_CD``
   * - 59
     - fielder with ninth assist
     - ``ASS9_FLD_CD``
   * - 60
     - fielder with tenth assist
     - ``ASS10_FLD_CD``
   * - 61
     - unknown fielding credit flag
     - ``UNKNOWN_OUT_EXC_FL``
   * - 62
     - uncertain play flag
     - ``UNCERTAIN_PLAY_EXC_FL``
   * - 63
     - :ref:`text of count as appears in event file <cwtools.cwevent.count>`
     - ``COUNT_TX``
    

.. _cwtools.cwevent.resplayers:

Result batters and pitchers (fields 10-17)
------------------------------------------

In most cases, the pitcher and batter charged or credited with
the event (when a batter event) are the ones in the game when the
event occurs. However, Rules 10.17 and 10.18 have special clauses
discussing how to charge walks to pitchers when a relief pitcher
enters in the middle of the plate appearance, and how to charge
strikeouts to batters when a pinch hitter enters in the middle of the
plate appearance. The batter and
pitcher fields always give the identity of the
batter and pitcher in the game at the time of the event; the
result batter and result pitcher
give the batter and pitcher credited or charged with the event.

There is one known bug in the Retrosheet-provided tools
regarding the result pitcher. When a relief pitcher enters the game,
and then the next batter is retired on a fielder's choice, the pitcher
responsible for the runner put out is shown in the result
pitcher field. While it is correct that the batter reaching
base in this case would be charged to the former pitcher should he
score, the purpose of the result pitcher field is
to indicate the pitcher charged with the outcome of this particular
event. In this case, for example, the relief pitcher is awarded
one-third of an inning pitched; therefore, he should be the
result pitcher, and then the previous pitcher
should be (and is) listed in the responsible
pitcher field for the batter in subsequent events.

In the case of switch-hitters, the batter
hand and result batter hand fields are
set to L or R, as appropriate,
based upon the hand with which the pitcher throws. If the pitcher's
throwing hand is unknown, or if the batter's batting hand is unknown,
a question mark appears in these fields.

.. _cwtools.cwevent.phflag:

Pinch-hit flag (field 31)
-------------------------

This field is T if the batter is a
pinch-hitter, and F if he is not. If a
player enters the game as a pinch-hitter, and then bats again in the
same inning because his team bats around, this field will be
F for the player's second plate appearance. To
identify the cases where this occurs, consult the defensive
position field (field 32), which will continue to be equal
to 11 (or 12 for a pinch-runner) until that player assumes a defensive
position.

.. _cwtools.cwevent.eventtype:

Event type code (field 34)
--------------------------

All plays are categorized by their primary event type. Here is a
list of all types and the corresponding codes used in this
field.  Codes marked "obsolete" are no longer used, or no longer appear
in Retrosheet-produced play-by-play files.

.. list-table:: cwevent extended field numbers
   :header-rows: 1
   :widths: 5,20

   * - Code
     - Primary event
   * - 0
     - Unknown (obsolete)
   * - 1
     - None (obsolete)
   * - 2
     - Generic out
   * - 3
     - Strikeout
   * - 4
     - Stolen base
   * - 5 
     - Defensive indifference
   * - 6
     - Caught stealing
   * - 7
     - Pickoff error (obsolete)
   * - 8 
     - Pickoff
   * - 9 
     - Wild pitch
   * - 10
     - Passed ball
   * - 11
     - Balk
   * - 12
     - Other advance/out advancing
   * - 13
     - Foul error
   * - 14 
     - Walk
   * - 15
     - Intentional walk
   * - 16
     - Hit by pitch
   * - 17
     - Interference
   * - 18
     - Error
   * - 19
     - Fielder's choice
   * - 20
     - Single
   * - 21
     - Double
   * - 22
     - Triple
   * - 23
     - Home run
   * - 24
     - Missing play (obsolete)


.. _cwtools.cwevent.sacflags:

Sacrifice flags and eras (fields 36, 38, 39)
--------------------------------------------

Chadwick in all cases applies the modern rules concerning the
awarding of sacrifice hits, sacrifice flies, and official times at
bat, regardless of the year indicated with the -y flag.

.. _cwtools.cwevent.plays:

Plays on runners (fields 58-65)
-------------------------------

Fields 58 through 65 give the destination of all runners,
including the batter, as well as the fielding play made on them, if
any. For the purposes of the destination fields, a code of 5 indicates
the runner scored, and is charged as unearned, and a code of 6
indicates the runner scored, and is charged as unearned to the team,
but earned to the pitcher. These codes only appear when the
(NR) or (TUR) modifiers are
explicitly used on the advancement code. There is no internal logic in
Chadwick to ferret out which runs should be earned or unearned, as in
many cases there is insufficient information, or the situation
requires the judgment of the official scorer. Runners which are put
out are reported as having an advancement of 0.

*New in 0.8.1*: With the introduction of the automatic runner
placement in extra innings as a tiebreaker for the 2020 season in
MLB (and earlier in the minor leagues and other leagues), a new
code 7 is used for runner advancement in the case in which an
automatic runner scores.  The scoring rule for automatic runners is
that they are charged to the pitcher who starts the inning, as an
unearned run.  For the purposes of determining whether a run
scored is due to the automatic runner, Chadwick follows the same
convention as is used for assigning responsibility for runners to
pitchers: in the event that an automatic runner is put out by
batter action, then the subsequent runner becomes marked as an
automatic runner.

In most cases, the play on a runner indicates the fielding
credits involved in putting him out. Chadwick also reports a fielding
play on a runner when the runner is safe on a dropped throw, such as
3E1 or FC6.1X2(6E4).

.. _cwtools.cwevent.errors:

Fielding errors (fields 51-57)
------------------------------

Up to three errors can be indicated in
:program:`cwevent` output. Supported error types are
F for generic fielding errors, and
T for throwing errors; no distinction is made
between bobbled batted balls, muffed throws or fly balls, and so
forth.

.. _cwtools.cwevent.responsibility:

Pitcher responsibility for runs (fields 75-77)
----------------------------------------------

The Official Rules for charging runs to pitchers stipulate that if a
pitcher is relieved in the middle of an inning with runners left on
base, he is charged with runs if those runners (or the ones who
replace them in the event of fielder's choices) subsequently score in
the inning.  The current rule is Rule 10.16(g), the comment on which
in the rules states:

   It is the intent of Rule 10.16(g) to charge each pitcher with the
   number of runners he put on base, rather than with the individual
   runners. When a pitcher puts runners on base and is relieved, such
   pitcher  shall be charged with all runs subsequently scored up to
   and including the number of runners such pitcher left on base when
   such pitcher left the game, unless such runners are put 
   out without action by the batter.

Chadwick implements this by assigning "responsibility"
for runners, and shifting those runners after fielder's choices as
appropriate to implement the rule.  Fields 75 through 77 report the
pitcher currently "charged" with runners on base using this method.

There is one special case to note in reporting these fields.  As
noted, a fielder's choice does not absolve a departed pitcher for
responsibility for a potential run. Ordinarily it is good enough to
report the shift in responsibility at the start of the next play.
However, consider the following scenario: The bases are loaded, with
the runner on third (R3) the responsibility of Pitcher A and runners
on second and first (R2 and R1) the responsibility of Pitcher B. The
batter hits a ground ball and R3 is forced at home. Then, the catcher
throws wildly trying to complete the double play, and as a result R2
scores.  In this case, the run scored by R2 is charged to Pitcher A,
not Pitcher B, i.e., the responsibility shifts in the middle of the
play. In order to facilitate calculation of runs and earned runs
allowed correctly from cwevent output, in this case, the record for
the play will report R2 as being the responsibility of Pitcher A,
i.e., it will report the responsibility after the mid-play shift. 

This convention will not affect most applications.  Indeed, the
Official Rules technically do not have a concept of assigning
responsibility to particular runners, and the contents of fields 75-77
only have meaning on plays in which the corresponding runners
score. This convention may confuse certain calculations, however,
including those which try to track what happens to inherited runners,
if one does not take appropriate care to handle this very unusual case.


.. _cwtools.cwevent.fielding:

Fielding credits (fields 88-95)
-------------------------------

The order in which Chadwick and the Retrosheet-provided tools
list putouts and assists may vary. The number of plays on which this
occurs is quite few, and generally in cases where there is a putout
in the primary event as well as one in the baserunning modifiers.
The words "first", "second" and so on do not necessarily indicate
chronological order of the credits, though in most cases they
do.


.. _cwtools.cwevent.count:

Reporting of counts
-------------------

The DiamondWare data model originally assumed that pitch-level data
for a game was one of all pitches, count only, or no pitches
(see the ``info,pitches`` metadata field).  However, many Retrosheet
files contain count data for selected plate appearances, where known.
In event files, a question mark is used when either the count of balls
or strikes (or both) is unknown.  ``BEVENT``` renders nulls in the
number of balls or strikes as zero in fields 5 and 6
(``BALLS_CT`` and ``STRIKES_CT``), making it impossible to discern
whether a play is marked as occurring on a count of 0-0 (even if ``info,pitches``
is set to ``none``), or on an unknown count.

Extended field 63, ``COUNT_TX``, remedies this by reporting the count
string as it appears in the file, as a parallel facility to ``PITCH_SEQ_TX``
and ``EVENT_TX``.  With ``COUNT_TX``, all three of the main elements
of the `play`  record are accessible in :program:`cwevent` output.


