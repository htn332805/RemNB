"""
ctx_restore.py
--------------
Restores ctx from a snapshot JSON file.

Usage:
    run(ctx, {"file": "Data/ctx_snapshot_20250825.json"})

Description:
    - Overwrites ctx with the loaded snapshot.
"""
import json

def run(ctx, params):
    file = params["file"]
    with open(file) as f:
        snapshot = json.load(f)
    ctx.clear()
    ctx.update(snapshot)
    return {"restored": file}