import json, argparse, sys
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class AggregateMetrics(BaseScriptlet):
    def validate(self, ctx, params):
        for k in ["metrics.mean_v1","metrics.median_v1","metrics.std_v1"]:
            if ctx.get(k) is None:
                raise ValueError(f"missing dependency key {k}")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            agg = {
                "mean": ctx.get("metrics.mean_v1"),
                "median": ctx.get("metrics.median_v1"),
                "std": ctx.get("metrics.std_v1")
            }
            ctx.set("metrics.aggregate_v1", agg, who="aggregate_metrics")
            print(json.dumps({"status":"ok","outputs":["metrics.aggregate_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"aggregate_metrics"}))
            return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(AggregateMetrics().run(Context(), p))
