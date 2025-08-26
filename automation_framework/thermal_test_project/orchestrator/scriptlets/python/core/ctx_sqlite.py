"""
ctx_sqlite.py
-------------
Runs SQL queries on a SQLite DB, optionally using ctx data.
Create/query/modify SQLite tables from ctx data.
Usage:
    run(ctx, {"db": "Data/my.db", "query": "SELECT * FROM mytable"})
    run(ctx, {"db": "Data/my.db", "table": "foo", "key": "foo_data", "action": "insert"})

Description:
    - Can insert ctx[key] (list of dicts) into table, or run queries.
"""
import sqlite3

def run(ctx, params):
    db = params["db"]
    conn = sqlite3.connect(db)
    cur = conn.cursor()
    if params.get("action") == "insert":
        table = params["table"]
        data = ctx[params["key"]]
        if not data:
            return {"inserted": 0}
        cols = data[0].keys()
        cur.execute(f"CREATE TABLE IF NOT EXISTS {table} ({', '.join([c+' TEXT' for c in cols])})")
        for row in data:
            cur.execute(f"INSERT INTO {table} ({', '.join(cols)}) VALUES ({', '.join(['?']*len(cols))})", tuple(row.values()))
        conn.commit()
        return {"inserted": len(data)}
    elif "query" in params:
        cur.execute(params["query"])
        rows = cur.fetchall()
        ctx[params.get("out", "sql_result")] = rows
        return {"rows": len(rows)}
    else:
        return {"status": "noop"}