"""
Compute simple statistics from an input CSV and store results in ctx.
No disk outputs; downstream steps read ctx["metrics"].

Expected params:
- input_csv: path to CSV with 'timestamp' column and numeric sensor columns.

Side effects:
- ctx["metrics"] = { column: {"mean":..., "min":..., "max":...}, ... }
- ctx["metrics_columns"] = [list of processed columns]
"""

import csv
import statistics

def run(ctx, params):
    input_csv = params["input_csv"]

    # Read CSV into memory
    with open(input_csv, "r", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        headers = reader.fieldnames or []

    # Identify numeric columns (skip a header named exactly 'timestamp' or 'Timestamp')
    numeric_cols = [c for c in headers if c.lower() != "timestamp"]

    metrics = {}
    for col in numeric_cols:
        vals = []
        for r in rows:
            try:
                vals.append(float(r[col]))
            except Exception:
                # Ignore non-parsable values
                pass
        if not vals:
            continue
        metrics[col] = {
            "mean": float(statistics.mean(vals)),
            "min": float(min(vals)),
            "max": float(max(vals))
        }

    # Write into shared context
    ctx["metrics"] = metrics
    ctx["metrics_columns"] = list(metrics.keys())

    # Return a small status dict for logging
    return {"ok": True, "columns": list(metrics.keys())}
