"""
ctx_event_bus.py
----------------
Appends event dicts to ctx["events"] with timestamp and source.
Purpose: Append event dicts into ctx with timestamp and source step; Dash can display toast notifications and alerts when events change.

Usage:
    run(ctx, {"event": "data_loaded", "source": "step1", "payload": {...}})
"""
import time

def run(ctx, params):
    if "events" not in ctx:
        ctx["events"] = []
    event = {
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "event": params["event"],
        "source": params.get("source", "unknown"),
        "payload": params.get("payload", {})
    }
    ctx["events"].append(event)
    return {"events_len": len(ctx["events"])}