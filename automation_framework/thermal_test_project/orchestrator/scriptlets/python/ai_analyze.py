# Minimal AI analysis placeholder module
import json, os

def _heuristic_flags(metrics):
    # Inspect metric rows and flag suspicious values
    flags = []
    for r in metrics.get("rows", []):
        name = r.get("name", "")
        val = r.get("value")
        # Flag if a mean temperature exceeds 80 C (demo threshold)
        if name.endswith("_mean") and isinstance(val, (int, float)) and val > 80:
            flags.append(f"High mean detected: {name} = {val}")
        # Flag if max RPM unrealistic (demo threshold)
        if name.endswith("_max") and "rpm" in name.lower() and isinstance(val, (int, float)) and val > 20000:
            flags.append(f"Unusual RPM: {name} = {val}")
    return flags

def run(params):
    # Read parameters for metrics input and markdown output
    metrics_json = params["metrics_json"]
    output_md = params["output_md"]

    # Load computed metrics from JSON file
    metrics = json.load(open(metrics_json, "r"))

    # Generate simple flags using heuristics; replace with real LLM call later
    flags = _heuristic_flags(metrics)

    # Compose a markdown summary
    lines = []
    lines.append("# AI Analysis Summary")
    lines.append("")
    if flags:
        lines.append("Key Findings:")
        for f in flags:
            lines.append(f"- {f}")
    else:
        lines.append("No significant anomalies detected in computed metrics.")
    lines.append("")
    lines.append("Recommendations:")
    lines.append("- Review fan curve and thermal paste application if sustained temps exceed targets.")
    lines.append("- Consider ambient control and re-run if variance seems high.")

    # Ensure output directory exists
    os.makedirs(os.path.dirname(output_md), exist_ok=True)
    # Write summary markdown
    with open(output_md, "w") as f:
        f.write("\n".join(lines))

    # Return structured info
    return {"ok": True, "flags": len(flags), "output": output_md}
