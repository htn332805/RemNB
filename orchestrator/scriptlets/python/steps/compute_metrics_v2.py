"""
Adds percentiles while preserving v1 keys.
Outputs (additive):
 - metrics.summary_v1 (unchanged shape)
 - metrics.summary_v2 (extended)
"""
import json, argparse, sys, statistics
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

def percentile(values: List[float], p: float) -> float:
    if not values: return 0.0
    k = (len(values)-1) * p
    f = int(k); c = min(f+1, len(values)-1)
    if f == c: return values[f]
    d = k - f
    return values[f] + (values[c]-values[f]) * d

class ComputeMetricsV2(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.normalized_v1") is None:
            raise ValueError("numbers.normalized_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            vals = list(ctx.get("numbers.normalized_v1"))
            vals_sorted = sorted(vals)
            summary_v1 = {
                "count": len(vals),
                "mean": statistics.fmean(vals),
                "min": min(vals),
                "max": max(vals)
            }
            summary_v2 = dict(summary_v1)
            summary_v2.update({
                "p50": percentile(vals_sorted, 0.5),
                "p90": percentile(vals_sorted, 0.9),
                "p99": percentile(vals_sorted, 0.99)
            })
            ctx.set("metrics.summary_v1", summary_v1, who="compute_metrics_v2")
            ctx.set("metrics.summary_v2", summary_v2, who="compute_metrics_v2")
            print(json.dumps({"status":"ok","outputs":["metrics.summary_v1","metrics.summary_v2"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_metrics_v2"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeMetricsV2().run(Context(), p))
