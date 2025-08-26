"""
ctx_pubsub.py
-------------
Implements a simple pub/sub mechanism for ctx.
Purpose: Enable reactive, event-driven updates between Dash and ctx (pub/sub pattern).
Pattern: Use to trigger updates in Dash when ctx changes, or to notify other scriptlets.


Usage:
    run(ctx, {"event": "data_updated", "payload": {...}})  # publish
    run(ctx, {"subscribe": "data_updated"})                # subscribe

Description:
    - Dash callbacks can publish events to ctx["events"].
    - Other steps/callbacks can check for new events.
"""
def run(ctx, params):
    if "events" not in ctx:
        ctx["events"] = []
    if "event" in params:
        ctx["events"].append({"event": params["event"], "payload": params.get("payload", {})})
        return {"published": params["event"]}
    elif "subscribe" in params:
        # Return and clear all events of this type
        ev = [e for e in ctx["events"] if e["event"] == params["subscribe"]]
        ctx["events"] = [e for e in ctx["events"] if e["event"] != params["subscribe"]]
        return {"events": ev}
    return {"status": "noop"}