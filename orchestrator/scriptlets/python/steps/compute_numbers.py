### AUTOGEN(rebase_workspace.sh)
"""
ComputeNumbers – reference numeric scriptlet
Outputs: numbers.stats_v1
"""
import json, sys, pathlib, statistics
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class ComputeNumbers(BaseScriptlet):
    def validate(self, ctx, params):
        if "src" not in params: raise ValueError("src required")
        if not pathlib.Path(params["src"]).is_file(): raise ValueError("file not found")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            nums = [float(l.strip()) for l in open(params["src"]) if l.strip() and not l.startswith("#")]
            if not nums:
                raise ValueError("no numeric data")
            stats = {
                "count": len(nums),
                "sum": sum(nums),
                "mean": sum(nums)/len(nums),
                "min": min(nums),
                "max": max(nums),
            }
            ctx.set("numbers.stats_v1", stats, who="compute_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.stats_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_numbers"}))
            return 1
if __name__ == "__main__":
    import argparse
    ap=argparse.ArgumentParser(); ap.add_argument("--params", required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeNumbers().run(Context(), p))
