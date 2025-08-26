"""
ctx_step_retry.py
-----------------
Retries a step on failure, with exponential backoff.

Usage:
    run(ctx, {"fn": my_fn, "args": {...}, "retries": 3, "delay": 2})

Description:
    - Calls fn(**args), retries up to N times on exception.
"""
import time

def run(ctx, params):
    fn = params["fn"]
    args = params.get("args", {})
    retries = params.get("retries", 3)
    delay = params.get("delay", 2)
    for attempt in range(retries):
        try:
            return {"result": fn(**args), "attempt": attempt+1}
        except Exception as e:
            if attempt == retries - 1:
                raise
            time.sleep(delay * (2 ** attempt))
    return {"error": "unreachable"}