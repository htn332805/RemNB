"""
Dump a few keys from CTX to the step log for inspection.
"""

def run(ctx, params):
    return {
        "ctx_keys": list(ctx.keys()),
        "metric_col_count": ctx.get("metric_col_count"),
        "metrics_columns_sample": (ctx.get("metrics_columns") or [])[:3],
        "ai_summary": ctx.get("ai_summary", {})
    }
