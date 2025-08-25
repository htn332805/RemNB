# Import necessary standard libraries
import csv, json, statistics, datetime, os
from datetime import datetime, UTC
def run(params):
    # Extract parameters for input and output paths
    input_csv = params["input_csv"]
    output_json = params["output_json"]

    # Open the CSV for reading; newline="" for correct CSV dialect handling
    with open(input_csv, "r", newline="") as f:
        # DictReader builds dicts keyed by header names
        reader = csv.DictReader(f)
        # Convert reader to a list to allow multiple passes
        rows = list(reader)
        # Save headers for potential use
        headers = reader.fieldnames or []

    # Prepare a list to hold metric rows in a uniform schema
    metrics_rows = []

    # Identify numeric columns to compute statistics, excluding the timestamp
    numeric_cols = [c for c in headers if c and c != "timestamp"]

    # Iterate through numeric columns to compute mean, max, min
    for col in numeric_cols:
        vals = []
        for r in rows:
            try:
                # Attempt to parse the value as float
                vals.append(float(r[col]))
            except Exception:
                # Skip non-numeric or missing entries
                pass
        if not vals:
            # No numeric data for this column, skip
            continue
        # Compute basic statistics
        mean_v = float(statistics.mean(vals))
        max_v = float(max(vals))
        min_v = float(min(vals))
        # Heuristic for units: if column ends with _C, assume Celsius
        unit = "C" if col.endswith("_C") else ("RPM" if "rpm" in col.lower() else "")

        # Use current UTC timestamp for metric records
        now = datetime.datetime.now(UTC).isoformat()

        # Append standardized metric rows
        metrics_rows.append({"timestamp": now, "name": f"{col}_mean", "value": mean_v, "unit": unit})
        metrics_rows.append({"timestamp": now, "name": f"{col}_max", "value": max_v, "unit": unit})
        metrics_rows.append({"timestamp": now, "name": f"{col}_min", "value": min_v, "unit": unit})

    # Bundle into a JSON object
    out = {"rows": metrics_rows}
    # Ensure output directory exists
    os.makedirs(os.path.dirname(output_json), exist_ok=True)
    # Write results with indentation for readability
    with open(output_json, "w") as f:
        json.dump(out, f, indent=2)

    # Return a structured result for the orchestrator to log
    return {"ok": True, "metrics_count": len(metrics_rows), "output": output_json}
