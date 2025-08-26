"""
ctx_validate.py
---------------
Validates that required keys exist in ctx and optionally checks their types.

Usage:
    run(ctx, {"required": ["foo", "bar"], "types": {"foo": list, "bar": dict}})

Description:
    - Raises ValueError if required keys are missing or types do not match.

Limitations:
    - Only basic type checking.

Requirements:
    - Python 3.6+
"""
def run(ctx, params):
    required = params.get("required", [])
    types = params.get("types", {})
    for k in required:
        if k not in ctx:
            raise ValueError(f"Missing required ctx key: {k}")
    for k, t in types.items():
        if k in ctx and not isinstance(ctx[k], t):
            raise ValueError(f"ctx[{k}] is not of type {t.__name__}")
    return {"validated": required}