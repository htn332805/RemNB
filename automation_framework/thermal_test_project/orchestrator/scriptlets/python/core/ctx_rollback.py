"""
ctx_rollback.py
---------------
Rolls back ctx to a previous snapshot file if an error occurs.

Usage:
    run(ctx, {"snapshot": "Data/ctx_snapshot.json", "errors_key": "errors"})

Description:
    - If ctx[errors_key] is non-empty, restores ctx from snapshot.
"""
import json

def run(ctx, params):
    snapshot = params["snapshot"]
    errors_key = params.get("errors_key", "errors")
    if ctx.get(errors_key):
        with open(snapshot) as f:
            snap = json.load(f)
        ctx.clear()
        ctx.update(snap)
        return {"rolled_back": True}
    return {"rolled_back": False}