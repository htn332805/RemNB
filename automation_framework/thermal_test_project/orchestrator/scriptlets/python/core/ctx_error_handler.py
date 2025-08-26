"""
ctx_error_handler.py
--------------------
Centralized error handling and logging for steps.

Usage:
    with error_handler(ctx, "step_name"):
        # step code

Description:
    - Catches exceptions, logs error info to ctx["errors"], and re-raises if needed.
"""
import traceback
from contextlib import contextmanager
import time

@contextmanager
def error_handler(ctx, step_name, re_raise=True):
    try:
        yield
    except Exception as e:
        entry = {
            "step": step_name,
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "error": str(e),
            "traceback": traceback.format_exc()
        }
        if "errors" not in ctx:
            ctx["errors"] = []
        ctx["errors"].append(entry)
        if re_raise:
            raise