"""
ctx_step_audit.py
-----------------
Logs all step executions, inputs, outputs, and status to ctx["audit"].

Usage:
    with step_audit(ctx, "step_name", params):
        # step code

Description:
    - Appends audit info to ctx["audit"].
"""
import time
from contextlib import contextmanager

@contextmanager
def step_audit(ctx, step_name, params):
    entry = {
        "step": step_name,
        "params": params,
        "start": time.time()
    }
    try:
        yield
        entry["status"] = "success"
    except Exception as e:
        entry["status"] = "error"
        entry["error"] = str(e)
        raise
    finally:
        entry["end"] = time.time()
        entry["duration"] = entry["end"] - entry["start"]
        if "audit" not in ctx:
            ctx["audit"] = []
        ctx["audit"].append(entry)