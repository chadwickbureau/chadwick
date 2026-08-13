# BEVENT/BGAME regression harness

`compare.py` runs a Chadwick tool and its DiamondWare counterpart over the
same Retrosheet event files, parses their CSV output, and compares records by
identity. It requires Python 3 and
[Rich](https://rich.readthedocs.io/) for logging and progress display.

For example, on a Unix host using Wine for the reference tools:

```sh
python3 contrib/regress/compare.py event 2023 \
  --data-dir /path/to/retrosheet \
  --candidate build/src/cwtools/cwevent \
  --reference /path/to/BEVENT.EXE --reference-prefix wine

python3 contrib/regress/compare.py game 2022 2023 \
  --data-dir /path/to/retrosheet \
  --candidate build/src/cwtools/cwgame \
  --reference /path/to/BGAME.EXE --reference-prefix wine
```

Executable paths containing a directory are resolved from the directory where
the harness is invoked; bare command names are resolved using `PATH`. The
harness exits 0 when output matches, 1 when differences are found, and 2 for
setup, execution, or malformed-output errors. Use `--max-differences N` to
limit diagnostics.

`--data-dir` is the root of a Retrosheet data repository. For a requested year
the harness reads event files from `seasons/<year>` beneath that root and runs
both tools with that season directory as their working directory.

Progress is written to standard error, while difference reports remain on
standard output. Pass `--verbose` to log the exact commands, input-file counts,
output sizes, and parsed record counts. Pass `--no-progress` for plain output
in environments where a progress display is not wanted.

Each tool is invoked once per season with all matching event files. The
progress display identifies whether the Chadwick tool, DiamondWare tool, or
comparison step is currently running.

The harness deliberately does not pass `-q` to BEVENT: in that program the
switch prompts interactively before processing each game. Chadwick's tools
still receive `-Q` to suppress their progress messages.

DiamondWare exit statuses under Wine are treated as advisory because the
programs may return a nonzero status after producing complete output. The
harness still parses and compares that output: malformed CSV is an execution
error, and truncated output appears as missing records. Known Wine and
MoltenVK startup diagnostics are suppressed; any remaining relevant details
are included when execution fails.

The event comparison retains the compatibility rules from the original
regression scripts:

- pinch-runner flags (`PR_RUN1_FL` through `PR_RUN3_FL`) are ignored;
- error type codes (`ERR1_CD` through `ERR3_CD`) are ignored;
- a runner's responsible pitcher is ignored when that base is empty;
- putouts and assists are compared as multisets, since their ordering is not
  significant.

Event differences are grouped by play. Each group identifies the inning half,
outs, count, score, occupied bases, batter, and original play text before
listing the fields that differ.

The harness compares BEVENT fields 0-96 and BGAME fields 0-83. Newer Chadwick
extensions are intentionally outside this compatibility check.
