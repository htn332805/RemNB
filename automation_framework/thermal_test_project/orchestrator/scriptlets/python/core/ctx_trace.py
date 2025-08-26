"""
ctx_trace.py
------------
Appends a trace entry to ctx["trace"].

Usage:
    run(ctx, {"step": "my_step", "info": "Started"})

Description:
    - Each entry: {"timestamp": ..., "step": ..., "info": ...}

Requirements:
    - Python 3.6+
"""
import time

def run(ctx, params):
    if "trace" not in ctx:
        ctx["trace"] = []
    entry = {
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "step": params.get("step", "unknown"),
        "info": params.get("info", "")
    }
    ctx["trace"].append(entry)
    return {"trace_len": len(ctx["trace"])}