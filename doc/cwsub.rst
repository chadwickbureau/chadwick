.. _cwtools.cwsub:

cwsub: Player substitution descriptor
=====================================

:program:`cwsub` extracts information about
in-game player substitutions. It is designed to be used in conjunction
with :program:`cwevent` to mitigate a blind spot in the
existing tools. 

:program:`cwsub` outputs up to ten pieces of
information about each substitution. All are included by default; which
ones are generated is controlled by the -f switch.

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


