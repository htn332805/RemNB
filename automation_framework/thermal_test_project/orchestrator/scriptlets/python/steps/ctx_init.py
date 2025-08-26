"""
ctx_init.py
-----------
Initializes keys in the shared in-memory ctx.

Usage:
    python ctx_init.py --keys new_row

Arguments:
    --keys KEY1 [KEY2 ...]   List of ctx keys to initialize as empty lists.

Limitations:
    - Only initializes keys as empty lists.
    - Keys must be valid Python identifiers.
"""
import argparse

def run(ctx, params):
    keys = params.get("keys", [])
    for k in keys:
        ctx[k] = []
    return {"initialized": keys}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--keys', nargs='+', required=True)
    args = parser.parse_args()
    ctx = {}
    run(ctx, {"keys": args.keys})
    print(f"Initialized ctx keys: {args.keys}")