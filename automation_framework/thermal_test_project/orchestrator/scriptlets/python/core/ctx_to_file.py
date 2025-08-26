"""
ctx_to_file.py
--------------
Persists a ctx key to a file (JSON or CSV) in a custom folder.
Purpose: Persist a ctx key as JSON or CSV to a custom folder for cross-process sharing, auditing, and resume flows.
vulnerability: path-injection Unvalidated input in path value creation risks unintended file/directory access

Usage:
    run(ctx, {"key": "foo", "path": "Data/foo.json", "format": "json"})
    run(ctx, {"key": "bar", "path": "Data/bar.csv", "format": "csv"})

Description:
    - Supports JSON and CSV formats.
    - Enables cross-process sharing and audit trails.
"""
import json
import csv

def run(ctx, params):
    key = params["key"]
    path = params["path"]
    fmt = params.get("format", "json")
    data = ctx[key]
    if fmt == "json":
        with open(path, "w") as f:
            json.dump(data, f, indent=2)
    elif fmt == "csv":
        with open(path, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerows(data)
    else:
        raise ValueError("Unsupported format")
    return {"written": path}