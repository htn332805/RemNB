"""
ctx_merge.py
------------
Merges two ctx keys (lists) into a new key.
Purpose: Merge two ctx keys (e.g., two lists of rows) into a new key.
Usage: Useful for combining results from parallel steps for dashboard display.

Usage:
    run(ctx, {"key1": "foo", "key2": "bar", "out": "merged"})
"""
def run(ctx, params):
    k1, k2, out = params["key1"], params["key2"], params["out"]
    ctx[out] = ctx.get(k1, []) + ctx.get(k2, [])
    return {"merged_len": len(ctx[out])}