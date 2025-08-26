"""
ctx_map.py
----------
Applies a function to each row in a ctx key and stores the result.
Apply a function to each row in a ctx key.

Usage:
    run(ctx, {"key": "foo", "fn": my_fn, "out": "foo_mapped"})

Description:
    - my_fn should be a function that takes a row and returns a row.
"""
def run(ctx, params):
    key = params["key"]
    fn = params["fn"]
    out = params.get("out", f"{key}_mapped")
    ctx[out] = [fn(row) for row in ctx[key]]
    return {"mapped": out}