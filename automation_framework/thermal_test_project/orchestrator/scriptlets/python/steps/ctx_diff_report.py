"""
ctx_diff_report.py
------------------
Compares two ctx keys and stores a human-readable diff.
Purpose: Compare two ctx keys (e.g., before/after transform) and store a human-readable diff in ctx["diff_report"] and Data/diff.md for audit.
Usage:
    run(ctx, {"key1": "before", "key2": "after", "out": "diff_report", "file": "Data/diff.md"})
"""
import difflib

def run(ctx, params):
    k1, k2 = params["key1"], params["key2"]
    out = params["out"]
    file = params["file"]
    lines1 = [str(row) for row in ctx[k1]]
    lines2 = [str(row) for row in ctx[k2]]
    diff = list(difflib.unified_diff(lines1, lines2, fromfile=k1, tofile=k2))
    ctx[out] = diff
    with open(file, "w") as f:
        f.write("\n".join(diff))
    return {"diff_lines": len(diff)}