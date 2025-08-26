"""
ctx_table_view.py
-----------------
Formats ctx data for display in a dashboard table (e.g., Dash DataTable).
Format ctx data for display in a dashboard table.

Usage:
    run(ctx, {"key": "foo", "out": "foo_table"})

Description:
    - Converts list of dicts or list of lists to columns/rows format.
"""
def run(ctx, params):
    key = params["key"]
    out = params.get("out", f"{key}_table")
    data = ctx[key]
    if isinstance(data, list) and data and isinstance(data[0], dict):
        columns = [{"name": k, "id": k} for k in data[0].keys()]
        rows = data
    elif isinstance(data, list) and data and isinstance(data[0], list):
        columns = [{"name": f"col{i}", "id": f"col{i}"} for i in range(len(data[0]))]
        rows = [dict(zip([c["id"] for c in columns], row)) for row in data]
    else:
        columns, rows = [], []
    ctx[out] = {"columns": columns, "rows": rows}
    return {"table": out}