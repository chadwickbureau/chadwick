#!/usr/bin/env python3
"""Compare Chadwick's cwevent/cwgame output with BEVENT/BGAME."""

from __future__ import annotations

import argparse
import csv
import io
import logging
import os
import re
import shlex
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


LOGGER = logging.getLogger("regress")


EVENT_FIELDS = (
    "GAME_ID", "AWAY_TEAM_ID", "INN_CT", "BAT_HOME_ID", "OUTS_CT",
    "BALLS_CT", "STRIKES_CT", "PITCH_SEQ_TX", "AWAY_SCORE_CT",
    "HOME_SCORE_CT", "BAT_ID", "BAT_HAND_CD", "RESP_BAT_ID",
    "RESP_BAT_HAND_CD", "PIT_ID", "PIT_HAND_CD", "RESP_PIT_ID",
    "RESP_PIT_HAND_CD", "POS2_FLD_ID", "POS3_FLD_ID", "POS4_FLD_ID",
    "POS5_FLD_ID", "POS6_FLD_ID", "POS7_FLD_ID", "POS8_FLD_ID",
    "POS9_FLD_ID", "BASE1_RUN_ID", "BASE2_RUN_ID", "BASE3_RUN_ID",
    "EVENT_TX", "LEADOFF_FL", "PH_FL", "BAT_FLD_CD", "BAT_LINEUP_ID",
    "EVENT_CD", "BAT_EVENT_FL", "AB_FL", "H_CD", "SH_FL", "SF_FL",
    "EVENT_OUTS_CT", "DP_FL", "TP_FL", "RBI_CT", "WP_FL", "PB_FL",
    "FLD_CD", "BATTEDBALL_CD", "BUNT_FL", "FOUL_FL",
    "BATTEDBALL_LOC_TX", "ERR_CT", "ERR1_FLD_CD", "ERR1_CD",
    "ERR2_FLD_CD", "ERR2_CD", "ERR3_FLD_CD", "ERR3_CD", "BAT_DEST_ID",
    "RUN1_DEST_ID", "RUN2_DEST_ID", "RUN3_DEST_ID", "BAT_PLAY_TX",
    "RUN1_PLAY_TX", "RUN2_PLAY_TX", "RUN3_PLAY_TX", "RUN1_SB_FL",
    "RUN2_SB_FL", "RUN3_SB_FL", "RUN1_CS_FL", "RUN2_CS_FL",
    "RUN3_CS_FL", "RUN1_PK_FL", "RUN2_PK_FL", "RUN3_PK_FL",
    "RUN1_RESP_PIT_ID", "RUN2_RESP_PIT_ID", "RUN3_RESP_PIT_ID",
    "GAME_NEW_FL", "GAME_END_FL", "PR_RUN1_FL", "PR_RUN2_FL",
    "PR_RUN3_FL", "REMOVED_FOR_PR_RUN1_ID", "REMOVED_FOR_PR_RUN2_ID",
    "REMOVED_FOR_PR_RUN3_ID", "REMOVED_FOR_PH_BAT_ID",
    "REMOVED_FOR_PH_BAT_FLD_CD", "PO1_FLD_CD", "PO2_FLD_CD",
    "PO3_FLD_CD", "ASS1_FLD_CD", "ASS2_FLD_CD", "ASS3_FLD_CD",
    "ASS4_FLD_CD", "ASS5_FLD_CD", "EVENT_ID",
)

GAME_FIELDS = (
    "GAME_ID", "GAME_DT", "GAME_CT", "GAME_DY", "START_GAME_TM", "DH_FL",
    "DAYNIGHT_PARK_CD", "AWAY_TEAM_ID", "HOME_TEAM_ID", "PARK_ID",
    "AWAY_START_PIT_ID", "HOME_START_PIT_ID", "BASE4_UMP_ID", "BASE1_UMP_ID",
    "BASE2_UMP_ID", "BASE3_UMP_ID", "LF_UMP_ID", "RF_UMP_ID",
    "ATTEND_PARK_CT", "SCORER_RECORD_ID", "TRANSLATOR_RECORD_ID",
    "INPUTTER_RECORD_ID", "INPUT_RECORD_TS", "EDIT_RECORD_TS",
    "METHOD_RECORD_CD", "PITCHES_RECORD_CD", "TEMP_PARK_CT",
    "WIND_DIRECTION_PARK_CD", "WIND_SPEED_PARK_CT", "FIELD_PARK_CD",
    "PRECIP_PARK_CD", "SKY_PARK_CD", "MINUTES_GAME_CT", "INN_CT",
    "AWAY_SCORE_CT", "HOME_SCORE_CT", "AWAY_HITS_CT", "HOME_HITS_CT",
    "AWAY_ERR_CT", "HOME_ERR_CT", "AWAY_LOB_CT", "HOME_LOB_CT",
    "WIN_PIT_ID", "LOSE_PIT_ID", "SAVE_PIT_ID", "GWRBI_BAT_ID",
    "AWAY_LINEUP1_BAT_ID", "AWAY_LINEUP1_FLD_CD", "AWAY_LINEUP2_BAT_ID",
    "AWAY_LINEUP2_FLD_CD", "AWAY_LINEUP3_BAT_ID", "AWAY_LINEUP3_FLD_CD",
    "AWAY_LINEUP4_BAT_ID", "AWAY_LINEUP4_FLD_CD", "AWAY_LINEUP5_BAT_ID",
    "AWAY_LINEUP5_FLD_CD", "AWAY_LINEUP6_BAT_ID", "AWAY_LINEUP6_FLD_CD",
    "AWAY_LINEUP7_BAT_ID", "AWAY_LINEUP7_FLD_CD", "AWAY_LINEUP8_BAT_ID",
    "AWAY_LINEUP8_FLD_CD", "AWAY_LINEUP9_BAT_ID", "AWAY_LINEUP9_FLD_CD",
    "HOME_LINEUP1_BAT_ID", "HOME_LINEUP1_FLD_CD", "HOME_LINEUP2_BAT_ID",
    "HOME_LINEUP2_FLD_CD", "HOME_LINEUP3_BAT_ID", "HOME_LINEUP3_FLD_CD",
    "HOME_LINEUP4_BAT_ID", "HOME_LINEUP4_FLD_CD", "HOME_LINEUP5_BAT_ID",
    "HOME_LINEUP5_FLD_CD", "HOME_LINEUP6_BAT_ID", "HOME_LINEUP6_FLD_CD",
    "HOME_LINEUP7_BAT_ID", "HOME_LINEUP7_FLD_CD", "HOME_LINEUP8_BAT_ID",
    "HOME_LINEUP8_FLD_CD", "HOME_LINEUP9_BAT_ID", "HOME_LINEUP9_FLD_CD",
    "AWAY_FINISH_PIT_ID", "HOME_FINISH_PIT_ID",
)

PUTOUT_FIELDS = ("PO1_FLD_CD", "PO2_FLD_CD", "PO3_FLD_CD")
ASSIST_FIELDS = (
    "ASS1_FLD_CD", "ASS2_FLD_CD", "ASS3_FLD_CD", "ASS4_FLD_CD",
    "ASS5_FLD_CD",
)
IGNORED_EVENT_FIELDS = frozenset(
    (
        "PR_RUN1_FL",
        "PR_RUN2_FL",
        "PR_RUN3_FL",
        "ERR1_CD",
        "ERR2_CD",
        "ERR3_CD",
    )
)
RESPONSIBILITY_FIELDS = {
    "RUN1_RESP_PIT_ID": "BASE1_RUN_ID",
    "RUN2_RESP_PIT_ID": "BASE2_RUN_ID",
    "RUN3_RESP_PIT_ID": "BASE3_RUN_ID",
}


class HarnessError(Exception):
    """A configuration, execution, or malformed-output error."""


@dataclass(frozen=True)
class ToolConfig:
    mode: str
    candidate_name: str
    reference_name: str
    fields: tuple[str, ...]
    field_range: str
    key_fields: tuple[str, ...]
    extra_args: tuple[str, ...] = ()


CONFIGS = {
    "event": ToolConfig(
        "event", "cwevent", "BEVENT", EVENT_FIELDS, "0-96", ("GAME_ID", "EVENT_ID")
    ),
    "game": ToolConfig(
        "game", "cwgame", "BGAME", GAME_FIELDS, "0-83", ("GAME_ID",),
        # cwgame defaults GAME_DT to a 4-digit year (-dnf); BGAME defaults to
        # a 2-digit year (-dnp). Force both to -dnf so GAME_DT lines up.
        extra_args=("-dnf",),
    ),
}


@dataclass(frozen=True)
class Difference:
    key: tuple[str, ...]
    field: str
    candidate: str
    reference: str
    context: EventContext | None = None


@dataclass(frozen=True)
class EventContext:
    inning: str
    half: str
    outs: str
    balls: str
    strikes: str
    away_score: str
    home_score: str
    batter: str
    runners: tuple[str, str, str]
    play: str


@dataclass(frozen=True)
class ToolResult:
    stdout: str
    stderr: str
    returncode: int


def parse_csv(output: str, fields: Sequence[str], tool_name: str) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line_number, values in enumerate(csv.reader(io.StringIO(output)), 1):
        if len(values) != len(fields):
            raise HarnessError(
                f"{tool_name} row {line_number} has {len(values)} columns; "
                f"expected {len(fields)}"
            )
        rows.append(dict(zip(fields, values)))
    return rows


def index_rows(
    rows: Iterable[dict[str, str]], key_fields: Sequence[str], tool_name: str
) -> dict[tuple[str, ...], dict[str, str]]:
    indexed: dict[tuple[str, ...], dict[str, str]] = {}
    for row in rows:
        key = tuple(row[field] for field in key_fields)
        if key in indexed:
            raise HarnessError(f"{tool_name} emitted duplicate record key {format_key(key)}")
        indexed[key] = row
    return indexed


def event_fields_differ(
    field: str, candidate: dict[str, str], reference: dict[str, str]
) -> bool:
    if field in IGNORED_EVENT_FIELDS:
        return False
    runner_field = RESPONSIBILITY_FIELDS.get(field)
    if runner_field is not None and candidate[runner_field] == "":
        return False
    if field in PUTOUT_FIELDS:
        return Counter(candidate[name] for name in PUTOUT_FIELDS) != Counter(
            reference[name] for name in PUTOUT_FIELDS
        )
    if field in ASSIST_FIELDS:
        return Counter(candidate[name] for name in ASSIST_FIELDS) != Counter(
            reference[name] for name in ASSIST_FIELDS
        )
    return candidate[field] != reference[field]


def event_context(row: dict[str, str]) -> EventContext:
    return EventContext(
        inning=row["INN_CT"],
        half="bottom" if row["BAT_HOME_ID"] == "1" else "top",
        outs=row["OUTS_CT"],
        balls=row["BALLS_CT"],
        strikes=row["STRIKES_CT"],
        away_score=row["AWAY_SCORE_CT"],
        home_score=row["HOME_SCORE_CT"],
        batter=row["BAT_ID"],
        runners=(row["BASE1_RUN_ID"], row["BASE2_RUN_ID"], row["BASE3_RUN_ID"]),
        play=row["EVENT_TX"],
    )


def compare_rows(
    candidate_rows: Iterable[dict[str, str]],
    reference_rows: Iterable[dict[str, str]],
    config: ToolConfig,
) -> tuple[list[Difference], list[tuple[str, ...]], list[tuple[str, ...]]]:
    candidate = index_rows(candidate_rows, config.key_fields, config.candidate_name)
    reference = index_rows(reference_rows, config.key_fields, config.reference_name)
    candidate_only = sorted(candidate.keys() - reference.keys())
    reference_only = sorted(reference.keys() - candidate.keys())
    differences: list[Difference] = []
    for key in sorted(candidate.keys() & reference.keys()):
        left, right = candidate[key], reference[key]
        for field in config.fields:
            differs = (
                event_fields_differ(field, left, right)
                if config.mode == "event"
                else left[field] != right[field]
            )
            if differs:
                context = event_context(left) if config.mode == "event" else None
                differences.append(
                    Difference(key, field, left[field], right[field], context)
                )
    return differences, candidate_only, reference_only


def filtered_diagnostics(stderr: str) -> str:
    """Remove known Wine/MoltenVK and normal DiamondWare chatter."""
    kept: list[str] = []
    in_moltenvk_block = False
    for line in stderr.splitlines():
        stripped = line.strip()
        if line.startswith("[mvk-info]"):
            in_moltenvk_block = True
            continue
        if in_moltenvk_block and (line.startswith("\t") or line.startswith(" ")):
            continue
        in_moltenvk_block = False
        if re.match(r"^[0-9a-fA-F]{4}:(?:fixme|warn|err):", line):
            continue
        if stripped.startswith("[Processing file "):
            continue
        if stripped.startswith("Expanded event descriptor, version "):
            continue
        if stripped.startswith("Expanded game descriptor, version "):
            continue
        if stripped.startswith("Type '") or stripped.startswith("Copyright (c)"):
            continue
        if stripped in {"", "DiamondWare. All rights reserved."}:
            continue
        kept.append(line)
    return "\n".join(kept).strip()


def run_tool(
    command: Sequence[str], cwd: Path, name: str, *, strict_status: bool = True
) -> ToolResult:
    LOGGER.debug("Running %s: %s", name, shlex.join(command))
    environment = os.environ.copy()
    if any(Path(part).name.lower() in {"wine", "wine64"} for part in command):
        environment.setdefault("WINEDEBUG", "-all")
        environment.setdefault("MVK_CONFIG_LOG_LEVEL", "0")
    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=environment,
            check=False,
        )
    except OSError as exc:
        raise HarnessError(f"could not run {name}: {exc}") from exc
    stdout = result.stdout.decode("utf-8", errors="surrogateescape")
    stderr = result.stderr.decode("utf-8", errors="replace")
    if result.returncode != 0 and strict_status:
        diagnostics = filtered_diagnostics(stderr)
        detail = f"\n{diagnostics}" if diagnostics else ""
        raise HarnessError(f"{name} exited with status {result.returncode}{detail}")
    if result.returncode != 0:
        LOGGER.debug(
            "%s returned status %d; validating and comparing its output",
            name,
            result.returncode,
        )
    LOGGER.debug("%s produced %d bytes", name, len(result.stdout))
    return ToolResult(stdout, stderr, result.returncode)


def tool_command(
    executable: str,
    prefix: str,
    year: str,
    config: ToolConfig,
    files: Sequence[Path],
    *,
    quiet: bool,
    quiet_flag: str = "-q",
) -> list[str]:
    return [
        *shlex.split(prefix),
        executable,
        *([quiet_flag] if quiet else []),
        "-y",
        year,
        "-f",
        config.field_range,
        *config.extra_args,
        *(path.name for path in files),
    ]


def season_directory(data_root: Path, year: str) -> Path:
    directory = data_root / "seasons" / year
    if not directory.is_dir():
        raise HarnessError(f"season directory does not exist: {directory}")
    return directory


def event_files(data_root: Path, year: str) -> list[Path]:
    directory = season_directory(data_root, year)
    files = sorted(
        path for path in directory.glob(f"{year}*.EV?") if path.is_file()
    )
    if not files:
        raise HarnessError(f"no event files matching {year}*.EV? in {directory}")
    return files


def format_key(key: Sequence[str]) -> str:
    return "/".join(value if value else "<empty>" for value in key)


def format_event_situation(context: EventContext) -> str:
    outs = "out" if context.outs == "1" else "outs"
    occupied = [
        f"{base}B:{runner}"
        for base, runner in enumerate(context.runners, 1)
        if runner
    ]
    bases = ", ".join(occupied) if occupied else "bases empty"
    return (
        f"{context.half} {context.inning} | {context.outs} {outs} | "
        f"count {context.balls}-{context.strikes} | "
        f"score away {context.away_score}, home {context.home_score} | "
        f"{bases} | batter {context.batter or '<empty>'}"
    )


def report(
    config: ToolConfig,
    differences: Sequence[Difference],
    candidate_only: Sequence[tuple[str, ...]],
    reference_only: Sequence[tuple[str, ...]],
    max_differences: int,
) -> None:
    shown = 0
    for key in candidate_only:
        if shown == max_differences:
            break
        print(f"only in {config.candidate_name}: {format_key(key)}")
        shown += 1
    for key in reference_only:
        if shown == max_differences:
            break
        print(f"only in {config.reference_name}: {format_key(key)}")
        shown += 1
    previous_key: tuple[str, ...] | None = None
    for difference in differences:
        if shown == max_differences:
            break
        if difference.context is not None and difference.key != previous_key:
            print(f"{format_key(difference.key)} — {format_event_situation(difference.context)}")
            print(f"  play: {difference.context.play}")
        prefix = "  " if difference.context is not None else f"{format_key(difference.key)} "
        print(
            f"{prefix}{difference.field}: {config.candidate_name}={difference.candidate!r} "
            f"{config.reference_name}={difference.reference!r}"
        )
        previous_key = difference.key
        shown += 1
    total = len(candidate_only) + len(reference_only) + len(differences)
    if total > shown:
        print(f"... {total - shown} more differences omitted")


def resolve_executable(value: str) -> str:
    """Keep PATH commands as-is and anchor path-like commands before chdir."""
    if "/" not in value and "\\" not in value:
        return value
    return str(Path(value).expanduser().resolve())


def configure_logging(verbose: bool):
    try:
        from rich.console import Console
        from rich.logging import RichHandler
    except ImportError as exc:
        raise HarnessError(
            "Rich is required to run the harness; install it with 'python -m pip install rich'"
        ) from exc

    console = Console(stderr=True)
    handler = RichHandler(
        console=console,
        show_path=False,
        show_time=verbose,
        markup=True,
    )
    logging.basicConfig(
        level=logging.DEBUG if verbose else logging.INFO,
        format="%(message)s",
        handlers=[handler],
        force=True,
    )
    return console


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=CONFIGS, help="outputs to compare")
    parser.add_argument("year", nargs="+", help="season year(s) to process")
    parser.add_argument(
        "--data-dir", type=Path, required=True, help="Retrosheet repository root"
    )
    parser.add_argument("--candidate", help="path to cwevent or cwgame")
    parser.add_argument("--reference", required=True, help="path to BEVENT or BGAME")
    parser.add_argument(
        "--candidate-prefix", default="", help="command prefix, parsed without a shell"
    )
    parser.add_argument(
        "--reference-prefix", default="", help="e.g. 'wine', parsed without a shell"
    )
    parser.add_argument("--max-differences", type=int, default=100, metavar="N")
    parser.add_argument("-v", "--verbose", action="store_true", help="log commands and record counts")
    parser.add_argument("--no-progress", action="store_true", help="disable progress display")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    console = configure_logging(args.verbose)
    config = CONFIGS[args.mode]
    candidate_executable = resolve_executable(args.candidate or config.candidate_name)
    reference_executable = resolve_executable(args.reference)
    if args.max_differences < 1:
        raise HarnessError("--max-differences must be at least 1")
    data_root = args.data_dir.resolve()
    if not data_root.is_dir():
        raise HarnessError(f"data directory does not exist: {data_root}")

    from rich.progress import (
        Progress,
        SpinnerColumn,
        TextColumn,
        TimeElapsedColumn,
    )

    LOGGER.info(
        "Comparing [bold]%s[/bold] with [bold]%s[/bold] for %d season(s)",
        config.candidate_name,
        config.reference_name,
        len(args.year),
    )

    failed = False
    with Progress(
        SpinnerColumn(),
        TextColumn("{task.description}"),
        TimeElapsedColumn(),
        console=console,
        disable=args.no_progress,
        transient=True,
    ) as progress:
        for year in args.year:
            directory = season_directory(data_root, year)
            files = event_files(data_root, year)
            LOGGER.debug("%s: found %d event files in %s", year, len(files), directory)
            task = progress.add_task(f"[cyan]{year}[/cyan]: running {config.candidate_name}")
            candidate_output = run_tool(
                tool_command(
                    candidate_executable,
                    args.candidate_prefix,
                    year,
                    config,
                    files,
                    quiet=True,
                    quiet_flag="-Q",
                ),
                directory,
                config.candidate_name,
            )
            progress.update(task, description=f"[cyan]{year}[/cyan]: running {config.reference_name}")
            reference_output = run_tool(
                tool_command(
                    reference_executable,
                    args.reference_prefix,
                    year,
                    config,
                    files,
                    quiet=False,
                ),
                directory,
                config.reference_name,
                strict_status=False,
            )
            progress.update(task, description=f"[cyan]{year}[/cyan]: comparing output")
            candidate_rows = parse_csv(
                candidate_output.stdout, config.fields, config.candidate_name
            )
            try:
                reference_rows = parse_csv(
                    reference_output.stdout, config.fields, config.reference_name
                )
            except HarnessError as exc:
                diagnostics = filtered_diagnostics(reference_output.stderr)
                detail = f"\n{diagnostics}" if diagnostics else ""
                raise HarnessError(
                    f"{exc}; {config.reference_name} returned status "
                    f"{reference_output.returncode}{detail}"
                ) from exc
            LOGGER.debug(
                "%s: parsed %d %s records and %d %s records",
                year,
                len(candidate_rows),
                config.candidate_name,
                len(reference_rows),
                config.reference_name,
            )
            differences, candidate_only, reference_only = compare_rows(
                candidate_rows, reference_rows, config
            )
            candidate_records = len(candidate_rows)
            count = len(differences) + len(candidate_only) + len(reference_only)
            progress.update(
                task,
                description=(
                    f"[cyan]{year}[/cyan]: "
                    + (
                        f"[red]FAIL[/red] ({count} differences)"
                        if count
                        else f"[green]PASS[/green] ({candidate_records} records)"
                    )
                ),
                completed=1,
                total=1,
            )
            if count:
                failed = True
                print(f"{year}: FAIL ({count} differences)")
                report(
                    config,
                    differences,
                    candidate_only,
                    reference_only,
                    args.max_differences,
                )
            else:
                print(f"{year}: PASS ({candidate_records} records)")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except HarnessError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(2)
