import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

import compare


def row(fields, **values):
    result = dict.fromkeys(fields, "")
    result.update(values)
    return result


class ParseCsvTest(unittest.TestCase):
    def test_parses_quoted_commas(self):
        self.assertEqual(
            compare.parse_csv('"game","single, line"\n', ("id", "play"), "tool"),
            [{"id": "game", "play": "single, line"}],
        )

    def test_rejects_wrong_column_count(self):
        with self.assertRaisesRegex(compare.HarnessError, "row 1 has 1 columns"):
            compare.parse_csv("only-one\n", ("one", "two"), "tool")


class EventRulesTest(unittest.TestCase):
    def setUp(self):
        self.left = row(compare.EVENT_FIELDS, GAME_ID="G", EVENT_ID="1")
        self.right = dict(self.left)

    def test_ignores_pinch_runner_flags(self):
        self.left["PR_RUN1_FL"] = "F"
        self.right["PR_RUN1_FL"] = "T"
        self.assertFalse(compare.event_fields_differ("PR_RUN1_FL", self.left, self.right))

    def test_ignores_error_type_codes(self):
        for field in ("ERR1_CD", "ERR2_CD", "ERR3_CD"):
            self.left[field] = "F"
            self.right[field] = "T"
            self.assertFalse(compare.event_fields_differ(field, self.left, self.right))

    def test_ignores_responsibility_for_empty_base(self):
        self.left["RUN1_RESP_PIT_ID"] = "pitcher-a"
        self.right["RUN1_RESP_PIT_ID"] = "pitcher-b"
        self.assertFalse(
            compare.event_fields_differ("RUN1_RESP_PIT_ID", self.left, self.right)
        )

    def test_compares_responsibility_for_occupied_base(self):
        self.left["BASE1_RUN_ID"] = self.right["BASE1_RUN_ID"] = "runner"
        self.left["RUN1_RESP_PIT_ID"] = "pitcher-a"
        self.right["RUN1_RESP_PIT_ID"] = "pitcher-b"
        self.assertTrue(
            compare.event_fields_differ("RUN1_RESP_PIT_ID", self.left, self.right)
        )

    def test_accepts_reordered_putouts_and_assists(self):
        self.left.update(PO1_FLD_CD="2", PO2_FLD_CD="6", PO3_FLD_CD="3")
        self.right.update(PO1_FLD_CD="6", PO2_FLD_CD="3", PO3_FLD_CD="2")
        self.left.update(ASS1_FLD_CD="4", ASS2_FLD_CD="6")
        self.right.update(ASS1_FLD_CD="6", ASS2_FLD_CD="4")
        for field in compare.PUTOUT_FIELDS + compare.ASSIST_FIELDS:
            self.assertFalse(compare.event_fields_differ(field, self.left, self.right))

    def test_detects_different_fielding_multiset(self):
        self.left.update(PO1_FLD_CD="2", PO2_FLD_CD="6", PO3_FLD_CD="3")
        self.right.update(PO1_FLD_CD="2", PO2_FLD_CD="6", PO3_FLD_CD="6")
        self.assertTrue(compare.event_fields_differ("PO1_FLD_CD", self.left, self.right))


class ComparisonTest(unittest.TestCase):
    def test_aligns_records_by_key_and_finds_missing_records(self):
        first = row(compare.GAME_FIELDS, GAME_ID="A", HOME_SCORE_CT="1")
        second = row(compare.GAME_FIELDS, GAME_ID="B")
        reference = row(compare.GAME_FIELDS, GAME_ID="A", HOME_SCORE_CT="2")
        differences, candidate_only, reference_only = compare.compare_rows(
            [second, first], [reference], compare.CONFIGS["game"]
        )
        self.assertEqual([item.field for item in differences], ["HOME_SCORE_CT"])
        self.assertEqual(candidate_only, [("B",)])
        self.assertEqual(reference_only, [])
        self.assertIsNone(differences[0].context)

    def test_event_difference_includes_play_context(self):
        candidate = row(
            compare.EVENT_FIELDS,
            GAME_ID="GAME",
            EVENT_ID="12",
            INN_CT="4",
            BAT_HOME_ID="1",
            OUTS_CT="1",
            BALLS_CT="2",
            STRIKES_CT="1",
            AWAY_SCORE_CT="3",
            HOME_SCORE_CT="2",
            BASE1_RUN_ID="runner",
            BAT_ID="batter",
            EVENT_TX="S7/G",
            H_CD="1",
        )
        reference = dict(candidate)
        reference["H_CD"] = "0"
        differences, _, _ = compare.compare_rows(
            [candidate], [reference], compare.CONFIGS["event"]
        )
        context = differences[0].context
        self.assertIsNotNone(context)
        self.assertEqual(compare.format_event_situation(context), "bottom 4 | 1 out | count 2-1 | score away 3, home 2 | 1B:runner | batter batter")
        self.assertEqual(context.play, "S7/G")

    def test_rejects_duplicate_keys(self):
        duplicate = row(compare.GAME_FIELDS, GAME_ID="A")
        with self.assertRaisesRegex(compare.HarnessError, "duplicate record key A"):
            compare.compare_rows([duplicate, duplicate], [], compare.CONFIGS["game"])


class ToolCommandTest(unittest.TestCase):
    def test_quiet_switch_is_optional(self):
        path = Path("2023ABC.EVA")
        quiet = compare.tool_command(
            "cwevent", "", "2023", compare.CONFIGS["event"], (path,),
            quiet=True, quiet_flag="-Q",
        )
        interactive = compare.tool_command(
            "BEVENT", "wine", "2023", compare.CONFIGS["event"], (path,), quiet=False
        )
        self.assertIn("-Q", quiet)
        self.assertNotIn("-q", quiet)
        self.assertNotIn("-q", interactive)
        self.assertNotIn("-Q", interactive)
        self.assertEqual(interactive[0:2], ["wine", "BEVENT"])


class DataLayoutTest(unittest.TestCase):
    def test_finds_files_in_season_repository_layout(self):
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            directory = root / "seasons" / "2025"
            directory.mkdir(parents=True)
            expected = directory / "2025ANA.EVA"
            expected.touch()
            (directory / "2024OLD.EVA").touch()
            self.assertEqual(compare.season_directory(root, "2025"), directory)
            self.assertEqual(compare.event_files(root, "2025"), [expected])

    def test_rejects_missing_season_directory(self):
        with TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(
                compare.HarnessError, "season directory does not exist"
            ):
                compare.event_files(Path(temporary), "2025")


class DiagnosticsTest(unittest.TestCase):
    def test_filters_wine_moltenvk_and_processing_chatter(self):
        diagnostics = """[mvk-info] MoltenVK version 1.3.0
\tVK_KHR_surface v25
0074:fixme:keyboard:NtUserActivateKeyboardLayout unsupported
[Processing file 2025ANA.EVA.]
BEVENT-specific failure
"""
        self.assertEqual(
            compare.filtered_diagnostics(diagnostics), "BEVENT-specific failure"
        )


if __name__ == "__main__":
    unittest.main()
