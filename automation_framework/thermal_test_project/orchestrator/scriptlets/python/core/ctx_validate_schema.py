"""
ctx_validate_schema.py
----------------------
Validates the schema of ctx data (keys, types, row shapes).
Purpose: Validate ctx structure (presence of keys, row shapes, types) for dashboards and downstream steps.

Usage:
    run(ctx, {"schema": {"foo": {"type": list, "len": 10}}})

Description:
    - Checks for required keys, types, and (optionally) row lengths.
"""
def run(ctx, params):
    schema = params["schema"]
    for key, rules in schema.items():
        if key not in ctx:
            raise ValueError(f"Missing ctx key: {key}")
        if "type" in rules and not isinstance(ctx[key], rules["type"]):
            raise ValueError(f"ctx[{key}] is not {rules['type']}")
        if "len" in rules and len(ctx[key]) != rules["len"]:
            raise ValueError(f"ctx[{key}] length != {rules['len']}")
    return {"validated": list(schema.keys())}