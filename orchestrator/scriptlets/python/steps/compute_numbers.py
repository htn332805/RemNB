"""
NAME: compute_numbers.py
DESCRIPTION: Reads a CSV of numbers, computes basic statistics, stores them in context.
USAGE:
  python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
PARAMETERS:
  - src: str path to CSV with a single column 'value'
RETURNS:
  {"status":"ok","outputs":["numbers.stats_v1"]}
LIMITATIONS:
  - CSV must contain a header named value and numeric rows.
EXAMPLES:
  python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
"""
from __future__ import annotations
import argparse
import csv
import json
import statistics
import sys
from typing import Dict, Any, List, TypedDict
class Stats(TypedDict):
    count: int
    mean: float
    min: float
    max: float

class SuccessOut(TypedDict):
    status: str
    outputs: List[str]

class ErrorOut(TypedDict):
    status: str
    reason: str
    exit_code: int
    step: str

from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources

logger = get_logger(__name__)


class ComputeNumbers(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if "src" not in params or not isinstance(params["src"], str):
            raise ValueError("'src' (str) required")

    @track_resources
    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            src = params["src"]
            values: List[float] = []
            with open(src, "r", newline="") as f:
                reader = csv.DictReader(f)
                fieldnames = reader.fieldnames or []
                if 'value' not in fieldnames:
                    raise ValueError("CSV must have 'value' column")
                for row in reader:
                    if row['value'].strip():
                        values.append(float(row['value']))
            if not values:
                raise ValueError("No numeric values found")
            stats: Stats = {
                "count": len(values),
                "mean": float(statistics.fmean(values)),
                "min": float(min(values)),
                "max": float(max(values)),
            }
            # ensure JSON serializable
            json.dumps(stats)
            ctx.set("numbers.stats_v1", stats, who="compute_numbers")
            out: SuccessOut = {"status": "ok", "outputs": ["numbers.stats_v1"]}
            print(json.dumps(out))
            return 0
        except Exception as e:  # pragma: no cover - error path
            err: ErrorOut = {"status": "error", "reason": str(e), "exit_code": 1, "step": "compute_numbers"}
            print(json.dumps(err))
            logger.exception("compute_numbers failed")
            return 1


def main():  # pragma: no cover - CLI wrapper
    parser = argparse.ArgumentParser()
    parser.add_argument("--params", required=True, help="JSON string of parameters")
    args = parser.parse_args()
    params = json.loads(args.params)
    ctx = Context()
    sys.exit(ComputeNumbers().run(ctx, params))


if __name__ == "__main__":  # pragma: no cover
    main()
