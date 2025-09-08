"""
Convert aggregated parallel metrics to summary format for reporting.
Takes metrics.aggregate_v1 and numbers.stats_v1 to create metrics.summary_v1
"""
import json, argparse, sys
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ConvertToSummary(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("metrics.aggregate_v1") is None:
            raise ValueError("metrics.aggregate_v1 missing")
        if ctx.get("numbers.stats_v1") is None:
            raise ValueError("numbers.stats_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            
            # Get the aggregated metrics and original stats
            agg = ctx.get("metrics.aggregate_v1")
            stats = ctx.get("numbers.stats_v1")
            
            # Create summary in the format expected by generate_report
            summary_v1 = {
                "count": stats["count"],
                "mean": agg["mean"],
                "min": stats["min"],
                "max": stats["max"]
            }
            
            ctx.set("metrics.summary_v1", summary_v1, who="convert_to_summary")
            print(json.dumps({"status":"ok","outputs":["metrics.summary_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"convert_to_summary"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(ConvertToSummary().run(Context(), p))