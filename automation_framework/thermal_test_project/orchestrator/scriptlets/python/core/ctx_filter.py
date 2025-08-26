"""
ctx_filter.py
-------------
Filters ctx keys by pattern or type.
Filter ctx keys by pattern or type.

Usage:
    run(ctx, {"pattern": "foo", "type": list, "out": "foo_keys"})

Description:
    - Stores matching keys in ctx[out].
"""
def run(ctx, params):
    pattern = params.get("pattern")
    typ = params.get("type")
    out = params.get("out", "filtered_keys")
    keys = [k for k in ctx if (not pattern or pattern in k) and (not typ or isinstance(ctx[k], typ))]
    ctx[out] = keys
    return {"filtered": out}