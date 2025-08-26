"""
ctx_cache.py
------------
Caches results of expensive computations in ctx.
Purpose: Cache expensive computations or data slices in ctx for fast repeated access.
Pattern: Use for caching filtered/aggregated data for dashboard widgets.
Usage:
    run(ctx, {"key": "result", "compute_fn": my_fn, "args": {...}})

Description:
    - If ctx[key] exists, returns it; else computes, stores, and returns.
"""
def run(ctx, params):
    key = params["key"]
    if key in ctx:
        return {"cached": True, "value": ctx[key]}
    value = params["compute_fn"](**params.get("args", {}))
    ctx[key] = value
    return {"cached": False, "value": value}