"""
Module 1: Load numbers from a CSV into context.
Outputs: numbers.raw_v1 -> List[int]
"""
import json, argparse, sys, pathlib
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class LoadNumbers(BaseScriptlet):
    parallelizable = False
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        src = params.get("src")
        if not src or not pathlib.Path(src).is_file():
            raise ValueError("src file not found")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            src = params["src"]
            nums: List[int] = []
            with open(src) as f:
                for line in f:
                    line=line.strip()
                    if not line or line.startswith("#"): continue
                    nums.append(int(line.split(",")[0]))
            ctx.set("numbers.raw_v1", nums, who="load_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.raw_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"load_numbers"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(LoadNumbers().run(Context(), p))
