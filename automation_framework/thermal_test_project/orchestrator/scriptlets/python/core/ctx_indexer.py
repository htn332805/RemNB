"""
ctx_indexer.py
--------------
Builds and stores indexes for fast lookup in large ctx datasets.
Purpose: Build and maintain fast indexes (e.g., by ID, timestamp) for large datasets in ctx.
Pattern: Use for fast lookup in Dash callbacks or scriptlets.

Usage:
    run(ctx, {"key": "big_data", "index_by": "id"})

Description:
    - Creates ctx["big_data_index"] for O(1) lookup by id.
"""
def run(ctx, params):
    key = params["key"]
    index_by = params["index_by"]
    data = ctx[key]
    index = {row[index_by]: i for i, row in enumerate(data)}
    ctx[f"{key}_index"] = index
    return {"index_len": len(index)}