.. _cwtools.cwcomment:

cwcomment: Comment extractor
============================

:program:`cwcomment` extracts comments from play-by-play event files.
It has no DiamondWare equivalent. The tool can be used alongside
:ref:`cwevent <cwtools.cwevent>` to retain information which appears
in ``com`` records rather than in play records.

:program:`cwcomment` outputs up to 10 fields. All fields are included
by default; the fields generated can be selected with the ``-f``
command-line option. The other common command-line options are
described in :ref:`the command-line options table
<cwtools.commandline>`.

.. list-table:: cwcomment field numbers
   :header-rows: 1
   :widths: 5,25,20

   * - Field number
     - Description
     - Header
   * - 0
     - Game ID
     - ``GAME_ID``
   * - 1
     - Event number
     - ``EVENT_ID``
   * - 2
     - Comment text
     - ``COMMENT_TX``
   * - 3
     - ID of person ejected
     - ``EJECT_PERSON_ID``
   * - 4
     - Role of person ejected
     - ``EJECT_PERSON_ROLE_CD``
   * - 5
     - ID of ejecting umpire
     - ``EJECT_UMPIRE_ID``
   * - 6
     - Reason for ejection
     - ``EJECT_REASON_TX``
   * - 7
     - Inning of umpire change
     - ``UMPCHANGE_INN_CT``
   * - 8
     - Position assumed by umpire
     - ``UMPCHANGE_POS_CD``
   * - 9
     - ID of umpire assuming position
     - ``UMPCHANGE_PERSON_ID``


Association with events
-----------------------

Comments which occur before the first play of a game have an
``EVENT_ID`` of 0. Other comments are associated with the play which
precedes them and normally have the same ``EVENT_ID`` as that play in
:program:`cwevent` output. A comment attached to an ``NP`` (no play)
record receives the number of the most recent event, because the
``NP`` record does not itself increment the event number.


Grouping of comments
--------------------

A consecutive series of ordinary comments attached at the same point
in the game is combined into one output record. Their text is joined
with spaces in ``COMMENT_TX``.

Ejection and umpire-change comments are each emitted as separate
records. They are not combined with adjacent comments. This preserves
a one-to-one relationship between each structured comment and its
parsed fields.


Structured comments
-------------------

Retrosheet represents ejections and umpire changes using specially
formatted comment text. :program:`cwcomment` recognizes these records
when ``COMMENT_TX`` begins with the corresponding prefix.

An ejection comment has the form::

   ej,person_id,person_role,umpire_id,reason

Its components are reported in ``EJECT_PERSON_ID``,
``EJECT_PERSON_ROLE_CD``, ``EJECT_UMPIRE_ID``, and
``EJECT_REASON_TX``.

An umpire-change comment has the form::

   umpchange,inning,position,person_id

Its components are reported in ``UMPCHANGE_INN_CT``,
``UMPCHANGE_POS_CD``, and ``UMPCHANGE_PERSON_ID``.

The structured fields which do not apply to a comment are blank.
``COMMENT_TX`` retains the complete comment text, including the
structured prefix and comma-separated components.
