"""
ctx_snapshot.py
---------------
Takes a full snapshot of ctx and saves to Data/ctx_snapshot_TIMESTAMP.json.
Take a full snapshot of ctx and save to a timestamped JSON file.

Usage:
    run(ctx, {"out": "Data/ctx_snapshot_20250825.json"})

Description:
    - Useful for debugging, auditing, and resuming flows.
"""
import json
import time

def run(ctx, params):
    out = params.get("out") or f"Data/ctx_snapshot_{int(time.time())}.json"
    with open(out, "w") as f:
        json.dump(dict(ctx), f, indent=2)
    return {"snapshot": out}