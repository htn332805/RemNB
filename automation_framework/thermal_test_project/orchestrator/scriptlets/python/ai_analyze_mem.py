"""
Read ctx["metrics"], apply simple rules, and store a summary back into ctx.

Expected params:
- threshold (optional): float, default 85.0. Flags any mean above threshold.

Side effects:
- ctx["ai_summary"] = { "findings": [...], "recommendations": [...] }
"""

def run(ctx, params):
    thr = float(params.get("threshold", 85.0))

    metrics = ctx.get("metrics", {})
    findings = []
    for col, agg in metrics.items():
        mean_val = agg.get("mean", 0.0)
        if mean_val > thr:
            findings.append(f"High mean on {col}: {mean_val:.2f}")

    recommendations = []
    if findings:
        recommendations.append("Inspect cooling (fan curves, TIM re-seat) and re-test at varied ambient.")
    else:
        recommendations.append("No anomalies; proceed to next test step.")

    ctx["ai_summary"] = {
        "findings": findings,
        "recommendations": recommendations
    }

    return {"ok": True, "flags": len(findings)}
