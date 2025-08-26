"""
ctx_transform_timeseries.py
---------------------------
Transforms timeseries data in ctx (select columns, resample, smooth, normalize).
Purpose: Transform timeseries in ctx (select columns, resample, smooth, normalize) while keeping everything JSON-safe.

Usage:
    run(ctx, {"key": "ts", "out": "ts_norm", "select": [0,1], "normalize": True})

Description:
    - Selects columns, normalizes, and stores result in ctx[out].
"""
import numpy as np

def run(ctx, params):
    key = params["key"]
    out = params["out"]
    select = params.get("select", None)
    normalize = params.get("normalize", False)
    data = ctx[key]
    if select:
        data = [[row[i] for i in select] for row in data]
    if normalize:
        arr = np.array(data, dtype=float)
        arr = (arr - arr.mean(axis=0)) / (arr.std(axis=0) + 1e-8)
        data = arr.tolist()
    ctx[out] = data
    return {"transformed": out}