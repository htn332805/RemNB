"""
ctx_assert.py
-------------
Asserts a condition on ctx and logs or raises an error.

Usage:
    run(ctx, {"key": "foo", "expected": 42})

Description:
    - Checks if ctx["foo"] == 42, logs result.

Requirements:
    - Python 3.6+
"""
def run(ctx, params):
    key = params["key"]
    expected = params["expected"]
    actual = ctx.get(key)
    if actual != expected:
        msg = f"ASSERT FAIL: ctx[{key}]={actual} != {expected}"
        if "log" in ctx:
            ctx["log"].append({"timestamp": "ASSERT", "level": "ERROR", "msg": msg})
        raise AssertionError(msg)
    else:
        if "log" in ctx:
            ctx["log"].append({"timestamp": "ASSERT", "level": "INFO", "msg": f"ASSERT OK: ctx[{key}] == {expected}"})
    return {"asserted": True}