"""
ctx_history.py
--------------
Tracks and exposes the history of changes to ctx keys.
Purpose: Track and expose the history of changes to any ctx key.
Usage: Useful for dashboards to show how data evolved over time.

Usage:
    from scriptlets.python.core.ctx_history import run
    run(ctx, {"key": "foo", "action": "get"})
    run(ctx, {"key": "foo", "action": "append", "value": 123})

Limitations:
    - Only tracks keys explicitly managed via this scriptlet.
"""
import time

def run(ctx, params):
    key = params["key"]
    action = params.get("action", "get")
    if "_history" not in ctx:
        ctx["_history"] = {}
    if key not in ctx["_history"]:
        ctx["_history"][key] = []
    if action == "append":
        value = params["value"]
        ctx["_history"][key].append({"timestamp": time.time(), "value": value})
        ctx[key] = value
    elif action == "get":
        return ctx["_history"][key]
    return {"history": ctx["_history"][key]}