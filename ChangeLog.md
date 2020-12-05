# [0.9.0]

## New features
- With the 2020 release, Retrosheet has introduced two new metadata
  fields representing the scheduled number of innings and the
  tiebreaker rule in effect (if any).  These currently appear only in
  the 2020 event files.  These are now extracted by two new extended
  fields in `cwgame`.
- `cwcomment` has been extended with new fields which record the
  contents of ejection and umpire change record, which in Retrosheet
  files are embedded in the comments.

## Behaviour changes
- Previously, `cwcomment` catenated all comments in a sequence into
  one record.  To support the addition of ejection and umpire change
  record extraction, this behaviour has changed.  Ejection and umpire
  change comments are not catenated to previous/subsequent comments.
  However, a series of comments which do not contain these special
  record type is still catenated as before.

## Bug fixes
- Fixed crash which would occur if running `cwcomment` on a boxscore
  file.


# [0.8.1] - 2020-07-17

## Behaviour changes
- The record type `radj` is now a synonym for the `cw:itb` extension
  for placing automatic runners on base in the tiebreaker.
- A new runner advancement code 7 has been introduced.  This marks
  the case in which an automatic runner placed on base via an radj
  record scores.  This run is always unearned.
  In the event an automatic runner is put out due to batter action,
  the subsequent runner becomes considered the automatic runner,
  just as with the rules for tracking pitcher responsibility for
  baserunners.

## Bug fixes
- Day of week was not being correctly reported by `cwgame` for games
  taking place prior to 1900; in some cases this could lead to
  core dumps due to accessing memory improperly.  This has been fixed
  and the new algorithm is valid for any Gregorian calendar date.
- `cwgame` was not reporting runs, hits, errors, LOB, or number of
  innings for games in the boxscore file format; these were all
  being reported as zero.  This has been fixed.

## Deprecations and removals
- The `cw:itb` record type is now considered deprecated in favour of
  `radj`.  Support for `cw:itb` will be removed at some point in future.


# [0.8.0] - 2020-06-09

## Behaviour changes
- cwevent now reports FLD_CD = 0 on generic outs coded as "99"
  (unknown fielding credits).  This is to match a change to the
  behaviour of BEVENT release in summer 2020.
- Fielding credits on any play containing "99" are now suppressed,
  including such constructions as `CS2(299)`.

## Bug fixes
- badj records are used to compute the batting hand of responsible
  batters who are removed with two strikes and then charged with the
  a strikeout in the plate appearance.
- padj records were not being respected when computing the batting
  hand of switch-hitters.
- Fixed bug in `cwbox` in which having more than 6 pitchers who were
  removed without retiring a batter in an inning would lead to a
  crash.
- Made `cwbox -X` and `cwbox -S` robust to the absence of the team
  file or a roster file by implementing sensible defaults.

## Enhancements
- Implemented `-n` switch for `cwcomment` to print column headers.

## Deprecations and removals
- Support for the `KE` (strikeout + error) primary event type was
  removed.  This was a very old Retrosheet code that has been
  deprecated long ago and has not appeared in any released files
  for years.


  

   

