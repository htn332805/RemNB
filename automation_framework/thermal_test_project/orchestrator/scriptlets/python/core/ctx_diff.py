"""
ctx_diff.py
-----------
Shows the difference between two ctx snapshots.

Usage:
    run(ctx, {"old": old_ctx})

Description:
    - Returns keys added, removed, and changed between old and current ctx.

Limitations:
    - Only compares top-level keys.

Requirements:
    - Python 3.6+
"""
def run(ctx, params):
    old = params["old"]
    added = [k for k in ctx if k not in old]
    removed = [k for k in old if k not in ctx]
    changed = [k for k in ctx if k in old and ctx[k] != old[k]]
    return {"added": added, "removed": removed, "changed": changed}