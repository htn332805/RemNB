"""
ctx_logger.py
-------------
Appends log messages to ctx["log"] with timestamp and level.

Usage:
    run(ctx, {"msg": "Step started", "level": "INFO"})

Description:
    - Adds a log entry to ctx["log"] (list of dicts).
    - Each entry: {"timestamp": ..., "level": ..., "msg": ...}

Requirements:
    - Python 3.6+
"""
import time

def run(ctx, params):
    msg = params["msg"]
    level = params.get("level", "INFO")
    if "log" not in ctx:
        ctx["log"] = []
    ctx["log"].append({
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "level": level,
        "msg": msg
    })
    return {"log_len": len(ctx["log"])}