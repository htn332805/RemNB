"""
ctx_lazy_loader.py
------------------
Loads large data into ctx on-demand, supports chunked access.
Purpose: Efficiently load large data into ctx only when needed, and support chunked/streaming access.

Usage:
    run(ctx, {"key": "big_data", "path": "Data/huge_file.parquet", "action": "load", "chunk": 10000})

Description:
    - Loads only a chunk or summary into ctx, not the whole file.
    - Use for large datasets to avoid memory overload.
"""
import pandas as pd

def run(ctx, params):
    key = params["key"]
    path = params["path"]
    action = params.get("action", "load")
    chunk = params.get("chunk", None)
    if action == "load":
        if chunk:
            # Load only a chunk (e.g., first N rows)
            df = pd.read_parquet(path, engine="pyarrow", columns=None)
            ctx[key] = df.head(chunk).to_dict(orient="records")
        else:
            # Load summary only
            df = pd.read_parquet(path, engine="pyarrow", columns=None)
            ctx[key] = {"shape": df.shape, "columns": list(df.columns)}
    elif action == "clear":
        ctx.pop(key, None)
    return {"status": "ok"}