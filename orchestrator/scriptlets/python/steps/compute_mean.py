import json, argparse, sys, statistics
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeMean(BaseScriptlet):
    parallelizable = True
    def validate(self, ctx, params): 
        if ctx.get("numbers.normalized_v1") is None: raise ValueError("normalized missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            data = ctx.get("numbers.normalized_v1")
            ctx.set("metrics.mean_v1", float(statistics.fmean(data)), who="compute_mean")
            print(json.dumps({"status":"ok","outputs":["metrics.mean_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_mean"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ComputeMean().run(Context(), p))
