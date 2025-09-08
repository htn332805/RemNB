import json, argparse, sys, statistics, math
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeStd(BaseScriptlet):
    parallelizable = True
    def validate(self, ctx, params):
        if ctx.get("numbers.normalized_v1") is None: raise ValueError("normalized missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            vals = ctx.get("numbers.normalized_v1")
            mean = statistics.fmean(vals)
            variance = sum((x-mean)**2 for x in vals)/len(vals)
            ctx.set("metrics.std_v1", math.sqrt(variance), who="compute_std")
            print(json.dumps({"status":"ok","outputs":["metrics.std_v1"]})); return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_std"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ComputeStd().run(Context(), p))
