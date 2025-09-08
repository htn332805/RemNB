"""
Module 4: Normalize numbers (min-max 0..1) write file & ctx.
Outputs:
 - numbers.normalized_v1 (list[float])
 - numbers.stats_v1 (dict)
Artifact: orchestrator/Data/normalized.json
"""
import json, argparse, sys, statistics, pathlib
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

OUT_FILE = pathlib.Path("orchestrator/Data/normalized.json")

class NormalizeNumbers(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.raw_v1") is None:
            raise ValueError("numbers.raw_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            nums = ctx.get("numbers.raw_v1")
            mn, mx = min(nums), max(nums)
            rng = mx - mn if mx != mn else 1
            normalized = [(n - mn)/rng for n in nums]
            stats = {
                "count": len(nums),
                "mean": statistics.fmean(nums),
                "min": mn,
                "max": mx
            }
            OUT_FILE.write_text(json.dumps({"normalized": normalized, "stats": stats}, indent=2))
            ctx.set("numbers.normalized_v1", normalized, who="normalize_numbers")
            ctx.set("numbers.stats_v1", stats, who="normalize_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.normalized_v1","numbers.stats_v1"],"artifact":str(OUT_FILE)}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"normalize_numbers"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(NormalizeNumbers().run(Context(), p))
