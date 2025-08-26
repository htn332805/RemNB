"""
ctx_print.py
------------
Pretty-prints the current ctx for debugging/logging.

Usage:
    run(ctx, {})

Description:
    - Prints ctx as formatted JSON.

Limitations:
    - For debugging only.

Requirements:
    - Python 3.6+
"""
import json

def run(ctx, params):
    print(json.dumps(dict(ctx), indent=2, sort_keys=True))
    return {"printed": True}