"""
Module 5: Basic metrics v1.
Outputs: metrics.summary_v1, metrics.columns_v1
"""
import json, argparse, sys, statistics
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeMetricsV1(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.normalized_v1") is None:
            raise ValueError("numbers.normalized_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            vals: List[float] = ctx.get("numbers.normalized_v1")
            summary = {
                "count": len(vals),
                "mean": statistics.fmean(vals),
                "min": min(vals),
                "max": max(vals)
            }
            ctx.set("metrics.summary_v1", summary, who="compute_metrics_v1")
            ctx.set("metrics.columns_v1", list(summary.keys()), who="compute_metrics_v1")
            print(json.dumps({"status":"ok","outputs":["metrics.summary_v1","metrics.columns_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_metrics_v1"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params", required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeMetricsV1().run(Context(), p))
