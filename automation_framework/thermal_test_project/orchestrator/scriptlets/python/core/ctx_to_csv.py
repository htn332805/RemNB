"""
ctx_to_csv.py
-------------
Exports a ctx key (list of lists) to a CSV file.
Purpose: Export any ctx key (list of lists) to a CSV file for sharing with other tools.
Usage: Enables shell steps or external tools to consume ctx data.
Usage:
    python ctx_to_csv.py --key new_row --out Data/new_row.csv

Limitations:
    - Only works for ctx[key] as list of lists.
"""
import csv
import argparse

def run(ctx, params):
    key = params["key"]
    out = params["out"]
    data = ctx[key]
    with open(out, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(data)
    return {"csv_written": out}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--key", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    ctx = {args.key: [["timestamp", "val1", "val2"], ["2024-01-01T00:00:00", 1, 2]]}
    run(ctx, {"key": args.key, "out": args.out})
    print(f"Wrote {args.key} to {args.out}")