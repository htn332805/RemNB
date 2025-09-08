"""
Module 3: Validate numbers constraints.
Errors produce structured JSON.
"""
import json, argparse, sys, statistics
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from datetime import datetime, timezone

class ValidateNumbers(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if "min_count" not in params: raise ValueError("min_count required")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            nums = ctx.get("numbers.raw_v1") or []
            min_count = params["min_count"]
            min_val = params.get("min_value", -10**9)
            max_val = params.get("max_value", 10**9)
            if len(nums) < min_count:
                raise ValueError(f"Too few numbers (min={min_count})")
            for n in nums:
                if n < min_val or n > max_val:
                    raise ValueError(f"Number {n} out of range [{min_val},{max_val}]")
            ctx.set("numbers.validated_v1", True, who="validate_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.validated_v1"]}))
            return 0
        except Exception as e:
            err = {
                "status":"error",
                "reason":str(e),
                "details":{"count": len(ctx.get('numbers.raw_v1') or [])},
                "exit_code":1,
                "step":"validate_numbers",
                "ts": datetime.now(timezone.utc).isoformat()
            }
            print(json.dumps(err))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(ValidateNumbers().run(Context(), p))
