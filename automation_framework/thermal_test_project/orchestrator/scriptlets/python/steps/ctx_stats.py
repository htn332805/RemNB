"""
ctx_stats.py
------------
Computes basic statistics (mean, std, min, max) for numeric columns in ctx[key].
Purpose: Compute basic stats over numeric columns in a timeseries stored in ctx.

Usage:
    run(ctx, {"key": "ts", "out": "ts_stats"})

Description:
    - Stores stats in ctx[out].
"""
import numpy as np

def run(ctx, params):
    key = params["key"]
    out = params["out"]
    data = np.array(ctx[key], dtype=float)
    stats = {
        "mean": data.mean(axis=0).tolist(),
        "std": data.std(axis=0).tolist(),
        "min": data.min(axis=0).tolist(),
        "max": data.max(axis=0).tolist()
    }
    ctx[out] = stats
    return {"stats": out}