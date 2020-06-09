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


  

   

