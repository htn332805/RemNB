"""
ctx_async.py
------------
Runs a function in a background thread and updates ctx when done.
Purpose: Offload heavy ctx operations to background threads/processes to keep Dash UI responsive.
Pattern: Use for long-running data loads or computations.


Usage:
    run(ctx, {"key": "result", "fn": my_fn, "args": {...}})

Description:
    - Starts a thread, returns immediately; updates ctx[key] when done.
"""
import threading

def run(ctx, params):
    def worker():
        ctx[params["key"]] = params["fn"](**params.get("args", {}))
    t = threading.Thread(target=worker)
    t.start()
    return {"started": True}