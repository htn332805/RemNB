"""
ctx_event_alert.py
------------------
Triggers notifications or alerts when specific ctx events occur.

Usage:
    run(ctx, {"event": "job_failed", "notify_fn": my_notify_fn, "payload": {...}})

Description:
    - Calls notify_fn if event is found in ctx["events"].
"""
def run(ctx, params):
    event = params["event"]
    notify_fn = params["notify_fn"]
    payload = params.get("payload", {})
    events = ctx.get("events", [])
    for e in events:
        if e.get("event") == event:
            notify_fn(payload)
            return {"alerted": event}
    return {"alerted": False}