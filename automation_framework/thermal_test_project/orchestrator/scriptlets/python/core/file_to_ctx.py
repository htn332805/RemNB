"""
file_to_ctx.py
--------------
Loads a JSON or CSV file into ctx under a specified key.
Purpose: Load JSON or CSV from a file into ctx under a key (for resume, external preprocessing, or shell integration).

Usage:
    run(ctx, {"path": "Data/foo.json", "key": "foo", "format": "json"})
    run(ctx, {"path": "Data/bar.csv", "key": "bar", "format": "csv"})

Description:
    - Supports JSON and CSV formats.
    - Useful for resuming flows or integrating external data.
"""
import json
import csv

def run(ctx, params):
    path = params["path"]
    key = params["key"]
    fmt = params.get("format", "json")
    if fmt == "json":
        with open(path) as f:
            ctx[key] = json.load(f)
    elif fmt == "csv":
        with open(path) as f:
            reader = csv.reader(f)
            ctx[key] = [row for row in reader]
    else:
        raise ValueError("Unsupported format")
    return {"loaded": key}