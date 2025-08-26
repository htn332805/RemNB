"""
ctx_row_updater.py
------------------
Continuously appends rows [timestamp, random1, ..., randomn] to ctx[key] in a background thread.

Usage:
    python ctx_row_updater.py --key new_row --n 3 --interval_sec 1

Arguments:
    --key KEY         ctx key to update
    --n N             number of random columns
    --interval_sec S  interval in seconds

Limitations:
    - Only works with JSON-serializable ctx.
    - Intended for use in orchestrator or as CLI demo.
"""
import threading
import time
import datetime
import random
import argparse

def run(ctx, params):
    key = params["key"]
    n = params.get("n", 3)
    interval = params.get("interval_sec", 1)
    def updater():
        while True:
            now = datetime.datetime.now().isoformat()
            row = [now] + [random.random() for _ in range(n)]
            ctx[key].append(row)
            time.sleep(interval)
    t = threading.Thread(target=updater, daemon=True)
    t.start()
    return {"thread": "started"}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--key', required=True)
    parser.add_argument('--n', type=int, default=3)
    parser.add_argument('--interval_sec', type=int, default=1)
    args = parser.parse_args()
    ctx = {args.key: []}
    run(ctx, {"key": args.key, "n": args.n, "interval_sec": args.interval_sec})
    print(f"Started background updater for ctx['{args.key}']")
    while True:
        time.sleep(10)