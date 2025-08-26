"""
ctx_view.py
-----------
Creates a filtered/paginated view of a large ctx dataset.
Purpose: Serve filtered, paginated, or summarized "views" of large ctx datasets for Dash.
Pattern: Use in Dash callbacks to serve only the visible data slice.

Usage:
    run(ctx, {"key": "big_data", "filter": {"col": "val"}, "page": 0, "page_size": 100})

Description:
    - Returns a slice of data matching filter and page.
"""
def run(ctx, params):
    key = params["key"]
    data = ctx[key]
    filt = params.get("filter", {})
    page = params.get("page", 0)
    page_size = params.get("page_size", 100)
    # Simple filter
    filtered = [row for row in data if all(row.get(k) == v for k, v in filt.items())]
    start = page * page_size
    end = start + page_size
    return {"view": filtered[start:end], "total": len(filtered)}