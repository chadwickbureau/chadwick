.. _cwtools.cwsub:

cwsub: Player substitution descriptor
=====================================

:program:`cwsub` extracts information about
in-game player substitutions. It is designed to be used in conjunction
with :program:`cwevent` to mitigate a blind spot in the
existing tools.

:program:`cwsub` outputs up to 26 pieces of
information about each substitution. All are included by default; which
ones are generated is controlled by the -f switch.

The fields have the same interpretation those with the same name in
:program:`cwevent`.  That is to say, you can think of :program:`cwsub` as
extending :program:`cwevent` by treating substitutions as events in
their own right.  In particular, the count and pitch counts are cumulative
counts for the plate appearance up to the point of the substitution.

As with :program:`cwevent`, fields 10 and 11, ``BALLS_CT`` and
``STRIKES_CT``, report zero whenever the count at the point of the
substitution is unknown, to maintain compatibility with the Retrosheet
tools. Field 25, ``COUNT_TX``, reports the count verbatim, including
question marks, and so can be used to disambiguate a genuine 0-0 count
from an unknown one; see :ref:`Reporting of counts
<cwtools.cwevent.count>` in the :program:`cwevent` documentation for
further detail.

.. list-table:: cwsub field numbers
   :header-rows: 1
   :widths: 5,20,10

   * - Field number
     - Description
     - Header
   * - 0
     - Game ID
     - ``GAME_ID``
   * - 1
     - Inning
     - ``INN_CT``
   * - 2
     - Batting team
     - ``BAT_HOME_ID``
   * - 3
     - Substitute
     - ``SUB_ID``
   * - 4
     - Team
     - ``SUB_HOME_ID``
   * - 5
     - Lineup position
     - ``SUB_LINEUP_ID``
   * - 6
     - Fielding position
     - ``SUB_FLD_CD``
   * - 7
     - Removed player
     - ``REMOVED_ID``
   * - 8
     - Position of removed player
     - ``REMOVED_FLD_CD``
   * - 9
     - Event number
     - ``EVENT_ID``
   * - 10
     - :ref:`Balls <cwtools.cwevent.count>`
     - ``BALLS_CT``
   * - 11
     - :ref:`Strikes <cwtools.cwevent.count>`
     - ``STRIKES_CT``
   * - 12
     - :ref:`Pitch sequence <cwtools.cwevent.count>`
     - ``PITCH_SEQ_TX``
   * - 13
     - number of balls in plate appearance
     - ``PA_BALL_CT``
   * - 14
     - number of called balls in plate appearance
     - ``PA_CALLED_BALL_CT``
   * - 15
     - number of intentional balls in plate appearance
     - ``PA_INTENT_BALL_CT``
   * - 16
     - number of pitchouts in plate appearance
     - ``PA_PITCHOUT_BALL_CT``
   * - 17
     - number of pitches hitting batter in plate appearance
     - ``PA_HITBATTER_BALL_CT``
   * - 18
     - number of other balls in plate appearance
     - ``PA_OTHER_BALL_CT``
   * - 19
     - number of strikes in plate appearance
     - ``PA_STRIKE_CT``
   * - 20
     - number of called strikes in plate appearance
     - ``PA_CALLED_STRIKE_CT``
   * - 21
     - number of swinging strikes in plate appearance
     - ``PA_SWINGMISS_STRIKE_CT``
   * - 22
     - number of foul balls in plate appearance
     - ``PA_FOUL_STRIKE_CT``
   * - 23
     - number of balls in play in plate appearance
     - ``PA_INPLAY_STRIKE_CT``
   * - 24
     - number of other strikes in plate appearance
     - ``PA_OTHER_STRIKE_CT``
   * - 25
     - :ref:`text of count as appears in event file <cwtools.cwevent.count>`
     - ``COUNT_TX``
