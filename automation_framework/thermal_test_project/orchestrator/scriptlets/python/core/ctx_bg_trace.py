"""
ctx_bg_trace.py
---------------
Traces and logs background/parallel process execution and outputs.
Trace and log background/parallel process execution and outputs.

Usage:
    run(ctx, {"key": "result", "fn": my_fn, "args": {...}, "trace_key": "bg_trace"})

Description:
    - Logs start/end, duration, and exceptions for background tasks.
"""
import threading
import time

def run(ctx, params):
    trace_key = params.get("trace_key", "bg_trace")
    if trace_key not in ctx:
        ctx[trace_key] = []
    def worker():
        entry = {"key": params["key"], "start": time.time()}
        try:
            ctx[params["key"]] = params["fn"](**params.get("args", {}))
            entry["status"] = "success"
        except Exception as e:
            entry["status"] = "error"
            entry["error"] = str(e)
        entry["end"] = time.time()
        entry["duration"] = entry["end"] - entry["start"]
        ctx[trace_key].append(entry)
    t = threading.Thread(target=worker)
    t.start()
    return {"started": True}