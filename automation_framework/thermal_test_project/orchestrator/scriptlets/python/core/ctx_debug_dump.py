"""
ctx_debug_dump.py
-----------------
Dumps the current ctx to a JSON file.

Usage:
    run(ctx, {"out": "Logs/ctx_dump.json"})

Description:
    - Serializes ctx to a file for offline debugging.

Requirements:
    - Python 3.6+
"""
import json

def run(ctx, params):
    out = params["out"]
    with open(out, "w") as f:
        json.dump(dict(ctx), f, indent=2, sort_keys=True)
    return {"dumped_to": out}