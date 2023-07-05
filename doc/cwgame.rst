.. _cwtools.cwgame:

cwgame: Game information extractor
==================================

:program:`cwgame` is a command-line tool which extracts summary
information about each game in the data file.  
These fields 
are grouped into two categories. There are 83 fields which are
compatible with the Retrosheet BGAME game descriptor tool.  These
are specified using the ``-f`` command-line flag. In addition, cwgame
offers a number "extended" fields which expand upon or give more
detailed information not easily accessed via the standard
fields. These are are specified using the ``-x`` command-line flag.
Many of these extended fields are defined to match fields which appear
in the Retrosheet gamelogs.  

.. note::
   :program:`cwgame` guarantees that the standard field numbers will
   match those used by BGAME.  Standard field numbers therefore can
   be treated as stable, and it should be safe to write scripts
   referring to them.  Extended fields are provisional, and extended
   fields may be added or withdrawn in future versions.  Extended
   fields are assigned numbers to maintain a logical cohesion, with
   related fields being grouped.  Therefore, extended field numbers
   are not promised to be stable.  It is recommended to use the field
   labels instead in writing scripts to process the output of
   :program:`cwgame` extended fields.
 
The following table gives the contents of each of the 84 fields
:program:`cwgame` outputs.

.. list-table:: cwgame standard field numbers
   :header-rows: 1
   :widths: 5,20,10

   * - Field number
     - Description
     - Header
   * - 0
     - Game ID
     - ``GAME_ID``
   * - 1
     - Date
     - ``GAME_DT``
   * - 2
     - Game number
     - ``GAME_CT``
   * - 3
     - Day of week
     - ``GAME_DY``
   * - 4
     - Start time
     - ``START_GAME_TM``
   * - 5
     - DH used flag
     - ``DH_FL``
   * - 6
     - Day/night flag
     - ``DAYNIGHT_PARK_CD``
   * - 7
     - Visiting team
     - ``AWAY_TEAM_ID``
   * - 8
     - Home team
     - ``HOME_TEAM_ID``
   * - 9
     - Game site
     - ``PARK_ID``
   * - 10
     - Visitors starting pitcher
     - ``AWAY_START_PIT_ID``
   * - 11
     - Home starting pitcher
     - ``HOME_START_PIT_ID``
   * - 12
     - Home plate umpire
     - ``BASE4_UMP_ID``
   * - 13
     - First base umpire
     - ``BASE1_UMP_ID``
   * - 14
     - Second base umpire
     - ``BASE2_UMP_ID``
   * - 15
     - Third base umpire
     - ``BASE3_UMP_ID``
   * - 16
     - Left field umpire
     - ``LF_UMP_ID``
   * - 17
     - Right field umpire
     - ``RF_UMP_ID``
   * - 18
     - Attendance
     - ``ATTEND_PARK_CT``
   * - 19
     - PS scorer
     - ``SCORER_RECORD_ID``
   * - 20
     - Translator
     - ``TRANSLATOR_RECORD_ID``
   * - 21
     - Inputter
     - ``INPUTTER_RECORD_ID``
   * - 22
     - Input time
     - ``INPUT_RECORD_TS``
   * - 23
     - Edit time
     - ``EDIT_RECORD_TS``
   * - 24
     - :ref:`How scored <cwtools.cwgame.howscored>`
     - ``METHOD_RECORD_CD``
   * - 25
     - :ref:`Pitches entered <cwtools.cwgame.pitchesentered>`
     - ``PITCHES_RECORD_CD``
   * - 26
     - :ref:`Temperature <cwtools.cwgame.temperature>`
     - ``TEMP_PARK_CT``
   * - 27
     - :ref:`Wind direction <cwtools.cwgame.winddirection>`
     - ``WIND_DIRECTION_PARK_CD``
   * - 28
     - :ref:`Wind speed <cwtools.cwgame.windspeed>`
     - ``WIND_SPEED_PARK_CT``
   * - 29
     - :ref:`Field condition <cwtools.cwgame.fieldcondition>`
     - ``FIELD_PARK_CD``
   * - 30
     - :ref:`Precipitation <cwtools.cwgame.precipitation>`
     - ``PRECIP_PARK_CD``
   * - 31
     - :ref:`Sky <cwtools.cwgame.sky>`
     - ``SKY_PARK_CD``
   * - 32
     - Time of game
     - ``MINUTES_GAME_CT``
   * - 33
     - Number of innings
     - ``INN_CT``
   * - 34
     - Visitor final score
     - ``AWAY_SCORE_CT``
   * - 35
     - Home final score
     - ``HOME_SCORE_CT``
   * - 36
     - Visitor hits
     - ``AWAY_HITS_CT``
   * - 37
     - Home hits
     - ``HOME_HITS_CT``
   * - 38
     - Visitor errors
     - ``AWAY_ERR_CT``
   * - 39
     - Home errors
     - ``HOME_ERR_CT``
   * - 40
     - Visitor left on base
     - ``AWAY_LOB_CT``
   * - 41
     - Home left on base
     - ``HOME_LOB_CT``
   * - 42
     - Winning pitcher
     - ``WIN_PIT_ID``
   * - 43
     - Losing pitcher
     - ``LOSE_PIT_ID``
   * - 44
     - Save
     - ``SAVE_PIT_ID``
   * - 45
     - Game-winning RBI
     - ``GWRBI_BAT_ID``
   * - 46
     - Visitor batter 1
     - ``AWAY_LINEUP1_BAT_ID``
   * - 47
     - Visitor batter 1 position
     - ``AWAY_LINEUP1_FLD_CD``
   * - 48
     - Visitor batter 2
     - ``AWAY_LINEUP2_BAT_ID``
   * - 49
     - Visitor batter 2 position
     - ``AWAY_LINEUP2_FLD_CD``
   * - 50
     - Visitor batter 3
     - ``AWAY_LINEUP3_BAT_ID``
   * - 51
     - Visitor batter 3 position
     - ``AWAY_LINEUP3_FLD_CD``
   * - 52
     - Visitor batter 4
     - ``AWAY_LINEUP4_BAT_ID``
   * - 53
     - Visitor batter 4 position
     - ``AWAY_LINEUP4_FLD_CD``
   * - 54
     - Visitor batter 5
     - ``AWAY_LINEUP5_BAT_ID``
   * - 55
     - Visitor batter 5 position
     - ``AWAY_LINEUP5_FLD_CD``
   * - 56
     - Visitor batter 6
     - ``AWAY_LINEUP6_BAT_ID``
   * - 57
     - Visitor batter 6 position
     - ``AWAY_LINEUP6_FLD_CD``
   * - 58
     - Visitor batter 7
     - ``AWAY_LINEUP7_BAT_ID``
   * - 59
     - Visitor batter 7 position
     - ``AWAY_LINEUP7_FLD_CD``
   * - 60
     - Visitor batter 8
     - ``AWAY_LINEUP8_BAT_ID``
   * - 61
     - Visitor batter 8 position
     - ``AWAY_LINEUP8_FLD_CD``
   * - 62
     - Visitor batter 9
     - ``AWAY_LINEUP9_BAT_ID``
   * - 63
     - Visitor batter 9 position
     - ``AWAY_LINEUP9_FLD_CD``
   * - 64
     - Home batter 1
     - ``HOME_LINEUP1_BAT_ID``
   * - 65
     - Home batter 1 position
     - ``HOME_LINEUP1_FLD_CD``
   * - 66
     - Home batter 2
     - ``HOME_LINEUP2_BAT_ID``
   * - 67
     - Home batter 2 position
     - ``HOME_LINEUP2_FLD_CD``
   * - 68
     - Home batter 3
     - ``HOME_LINEUP3_BAT_ID``
   * - 69
     - Home batter 3 position
     - ``HOME_LINEUP3_FLD_CD``
   * - 70
     - Home batter 4
     - ``HOME_LINEUP4_BAT_ID``
   * - 71
     - Home batter 4 position
     - ``HOME_LINEUP4_FLD_CD``
   * - 72
     - Home batter 5
     - ``HOME_LINEUP5_BAT_ID``
   * - 73
     - Home batter 5 position
     - ``HOME_LINEUP5_FLD_CD``
   * - 74
     - Home batter 6
     - ``HOME_LINEUP6_BAT_ID``
   * - 75
     - Home batter 6 position
     - ``HOME_LINEUP6_FLD_CD``
   * - 76
     - Home batter 7
     - ``HOME_LINEUP7_BAT_ID``
   * - 77
     - Home batter 7 position
     - ``HOME_LINEUP7_FLD_CD``
   * - 78
     - Home batter 8
     - ``HOME_LINEUP8_BAT_ID``
   * - 79
     - Home batter 8 position
     - ``HOME_LINEUP8_FLD_CD``
   * - 80
     - Home batter 9
     - ``HOME_LINEUP9_BAT_ID``
   * - 81
     - Home batter 9 position
     - ``HOME_LINEUP9_FLD_CD``
   * - 82
     - Visitor finishing pitcher
     - ``AWAY_FINISH_PIT_ID``
   * - 83
     - Home finishing pitcher
     - ``HOME_FINISH_PIT_ID``
   * - 84
     - Game type
     - ``GAME_TYPE_TX``

.. list-table:: cwgame extended field numbers
   :header-rows: 1
   :widths: 5,20,10

   * - Field number
     - Description
     - Header
   * - 0
     - visiting team league
     - ``AWAY_TEAM_LEAGUE_ID``
   * - 1
     - home team league
     - ``HOME_TEAM_LEAGUE_ID``
   * - 2
     - visiting team game number
     - ``AWAY_TEAM_GAME_CT``
   * - 3 
     - home team game number
     - ``HOME_TEAM_GAME_CT``
   * - 4
     - length of game in outs
     - ``OUTS_CT``
   * - 5
     - information on completion of game 
     - ``COMPLETION_TX``
   * - 6
     - information on forfeit of game
     - ``FORFEIT_TX``
   * - 7
     - information on protest of game
     - ``PROTEST_TX``
   * - 8 
     - visiting team linescore
     - ``AWAY_LINE_TX``
   * - 9
     - home team linescore
     - ``HOME_LINE_TX``
   * - 10
     - visiting team AB
     - ``AWAY_AB_CT``
   * - 11
     - visiting team 2B
     - ``AWAY_2B_CT``
   * - 12
     - visiting team 3B
     - ``AWAY_3B_CT``
   * - 13
     - visiting team HR
     - ``AWAY_HR_CT``
   * - 14 
     - visiting team RBI
     - ``AWAY_BI_CT``
   * - 15
     - visiting team SH
     - ``AWAY_SH_CT``
   * - 16
     - visiting team SF
     - ``AWAY_SF_CT``
   * - 17 
     - visiting team HP
     - ``AWAY_HP_CT``
   * - 18
     - visiting team BB
     - ``AWAY_BB_CT``
   * - 19
     - visiting team IBB
     - ``AWAY_IBB_CT``
   * - 20 
     - visiting team SO
     - ``AWAY_SO_CT``
   * - 21
     - visiting team SB
     - ``AWAY_SB_CT``
   * - 22
     - visiting team CS
     - ``AWAY_CS_CT``
   * - 23
     - visiting team GDP
     - ``AWAY_GDP_CT``
   * - 24
     - visiting team reach on interference
     - ``AWAY_XI_CT``
   * - 25
     - number of pitchers used by visiting team
     - ``AWAY_PITCHER_CT``
   * - 26
     - visiting team individual ER allowed
     - ``AWAY_ER_CT``
   * - 27
     - visiting team team ER allowed
     - ``AWAY_TER_CT``
   * - 28
     - visiting team WP
     - ``AWAY_WP_CT``
   * - 29 
     - visiting team BK
     - ``AWAY_BK_CT``
   * - 30
     - visiting team PO
     - ``AWAY_PO_CT``
   * - 31
     - visiting team A
     - ``AWAY_A_CT``
   * - 32
     - visiting team PB
     - ``AWAY_PB_CT``
   * - 33
     - visiting team DP
     - ``AWAY_DP_CT``
   * - 34
     - visiting team TP
     - ``AWAY_TP_CT``
   * - 35
     - home team AB
     - ``HOME_AB_CT``
   * - 36
     - home team 2B
     - ``HOME_2B_CT``
   * - 37
     - home team 3B
     - ``HOME_3B_CT``
   * - 38
     - home team HR
     - ``HOME_HR_CT``
   * - 39
     - home team RBI
     - ``HOME_RBI_CT``
   * - 40 
     - home team SH
     - ``HOME_SH_CT``
   * - 41
     - home team SF
     - ``HOME_SF_CT``
   * - 42
     - home team HP
     - ``HOME_HP_CT``
   * - 43
     - home team BB
     - ``HOME_BB_CT``
   * - 44
     - home team IBB
     - ``HOME_IBB_CT``
   * - 45
     - home team SO
     - ``HOME_SO_CT``
   * - 46
     - home team SB
     - ``HOME_SB_CT``
   * - 47
     - home team CS
     - ``HOME_CS_CT``
   * - 48
     - home team GDP
     - ``HOME_GDP_CT``
   * - 49
     - home team reach on interference
     - ``HOME_XI_CT``
   * - 50
     - number of pitchers used by home team
     - ``HOME_PITCHER_CT``
   * - 51
     - home team individual ER allowed
     - ``HOME_ER_CT``
   * - 52
     - home team team ER allowed
     - ``HOME_TER_CT``
   * - 53
     - home team WP
     - ``HOME_WP_CT``
   * - 54
     - home team BK
     - ``HOME_BK_CT``
   * - 55
     - home team PO
     - ``HOME_PO_CT``
   * - 56
     - home team A
     - ``HOME_A_CT``
   * - 57
     - home team PB
     - ``HOME_PB_CT``
   * - 58
     - home team DP
     - ``HOME_DP_CT``
   * - 59
     - home team TP
     - ``HOME_TP_CT``
   * - 60
     - home plate umpire name
     - ``UMP_HOME_NAME_TX``
   * - 61
     - first base umpire name
     - ``UMP_1B_NAME_TX``
   * - 62
     - second base umpire name
     - ``UMP_2B_NAME_TX``
   * - 63
     - third base umpire name
     - ``UMP_3B_NAME_TX``
   * - 64
     - left field umpire name
     - ``UMP_LF_NAME_TX``
   * - 65
     - right field umpire name
     - ``UMP_RF_NAME_TX``
   * - 66
     - visitors manager ID
     - ``AWAY_MANAGER_ID``
   * - 67
     - visitors manager name
     - ``AWAY_MANAGER_NAME_TX``
   * - 68
     - home manager ID
     - ``HOME_MANAGER_ID``
   * - 69
     - home manager name
     - ``HOME_MANAGER_NAME_TX``
   * - 70
     - winning pitcher name
     - ``WIN_PIT_NAME_TX``
   * - 71
     - losing pitcher name
     - ``LOSE_PIT_NAME_TX``
   * - 72
     - save pitcher name
     - ``SAVE_PIT_NAME_TX``
   * - 73
     - batter with goahead RBI ID
     - ``GOAHEAD_RBI_ID``
   * - 74
     - batter with goahead RBI
     - ``GOAHEAD_RBI_NAME_TX``
   * - 75
     - visitor batter 1 name
     - ``AWAY_LINEUP1_BAT_NAME_TX``
   * - 76
     - visitor batter 2 name
     - ``AWAY_LINEUP2_BAT_NAME_TX``
   * - 77
     - visitor batter 3 name
     - ``AWAY_LINEUP3_BAT_NAME_TX``
   * - 78
     - visitor batter 4 name
     - ``AWAY_LINEUP4_BAT_NAME_TX``
   * - 79
     - visitor batter 5 name
     - ``AWAY_LINEUP5_BAT_NAME_TX``
   * - 80
     - visitor batter 6 name
     - ``AWAY_LINEUP6_BAT_NAME_TX``
   * - 81
     - visitor batter 7 name
     - ``AWAY_LINEUP7_BAT_NAME_TX``
   * - 82
     - visitor batter 8 name
     - ``AWAY_LINEUP8_BAT_NAME_TX``
   * - 83
     - visitor batter 9 name
     - ``AWAY_LINEUP9_BAT_NAME_TX``
   * - 84
     - home batter 1 name
     - ``HOME_LINEUP1_BAT_NAME_TX``
   * - 85
     - home batter 2 name
     - ``HOME_LINEUP2_BAT_NAME_TX``
   * - 86
     - home batter 3 name
     - ``HOME_LINEUP3_BAT_NAME_TX``
   * - 87
     - home batter 4 name
     - ``HOME_LINEUP4_BAT_NAME_TX``
   * - 88
     - home batter 5 name
     - ``HOME_LINEUP5_BAT_NAME_TX``
   * - 89
     - home batter 6 name
     - ``HOME_LINEUP6_BAT_NAME_TX``
   * - 90
     - home batter 7 name
     - ``HOME_LINEUP7_BAT_NAME_TX``
   * - 91
     - home batter 8 name
     - ``HOME_LINEUP8_BAT_NAME_TX``
   * - 92
     - home batter 9 name
     - ``HOME_LINEUP9_BAT_NAME_TX``
   * - 93
     - additional information
     - ``ADD_INFO_TX``
   * - 94
     - acquisition information
     - ``ACQ_INFO_TX``
   * - 95
     - scheduled length of game in innings
     - ``SCHED_INN_CT``
   * - 96
     - :ref:`tiebreaker rule type in use <cwtools.cwgame.tiebreak>`
     - ``TIEBREAK_CD``

.. _cwtools.cwgame.howscored:

How scored (field 24)
---------------------

This field outputs a numeric code corresponding to the medium by
which the play-by-play account was recorded.

.. list-table:: Numeric codes for how scored field
   :header-rows: 1
   :widths: 5,20

   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - park
   * - 2
     - tv
   * - 3
     - radio



.. _cwtools.cwgame.pitchesentered:

Pitches entered (field 25)
--------------------------

This field outputs a numeric code corresponding to the level of
pitch detail in the file.

.. list-table:: Numeric codes for pitches entered field
   :header-rows: 1
   :widths: 5,20

   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - pitches
   * - 2
     - count
   * - 3
     - none

.. note:: This field reports the contents of the
   info,pitches field in the game file. Some games
   with partial pitch information will have this field set to
   none or count.

.. _cwtools.cwgame.temperature:

Temperature (field 26)
----------------------

The game-time temperature, in degrees Fahrenheit. The value
0 is used when the temperature is unknown.

.. _cwtools.cwgame.winddirection:

Wind direction (field 27)
-------------------------

This field outputs a numeric code corresponding to the direction
of the wind.

.. list-table:: Numeric codes for wind direction field
   :header-rows: 1
   :widths: 5,20

   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - tolf
   * - 2
     - tocf
   * - 3
     - torf
   * - 4
     - ltor
   * - 5
     - fromlf
   * - 6
     - fromcf
   * - 7 
     - fromrf
   * - 8 
     - rtol 


.. _cwtools.cwgame.windspeed:

Wind speed (field 28)
---------------------

The game time wind speed, in miles per hour. The value
-1 is used when the wind speed is unknown.

.. _cwtools.cwgame.fieldcondition:

Field condition (field 29)
--------------------------

This field outputs a numeric code corresponding to the condition
of the field.

.. list-table:: Numeric codes for field condition field
   :header-rows: 1
   :widths: 5,20

   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - soaked
   * - 2
     - wet
   * - 3
     - damp
   * - 4
     - dry

.. _cwtools.cwgame.precipitation:

Precipitation (field 30)
------------------------

This field outputs a numeric code corresponding to the
precipitation level.

.. list-table:: Numeric codes for precipitation field
   :header-rows: 1
   :widths: 5,20
  
   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - none
   * - 2
     - drizzle
   * - 3
     - showers
   * - 4
     - rain
   * - 5
     - snow

.. _cwtools.cwgame.sky:

Sky (field 31)
--------------

This field outputs a numeric code corresponding to the sky
conditions.

Numeric codes for sky field.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table:: Numeric codes for sky field
   :header-rows: 1
   :widths: 5,20
  
   * - Code
     - Description
   * - 0
     - unknown
   * - 1
     - sunny
   * - 2 
     - cloudy
   * - 3
     - overcast
   * - 4
     - night
   * - 5
     - dome


.. _cwtools.cwgame.tiebreak:

Tiebreaker rule type in use (extended field 96)
-----------------------------------------------

This field indicates games in which an extra-innings tiebreaker
rule was in use.  The only valid value for this field currently
is `2`, indicating that extra innings began with a runner on
second base.

