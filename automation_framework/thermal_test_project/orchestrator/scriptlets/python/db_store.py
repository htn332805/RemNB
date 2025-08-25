# SQLite storage module for tests and metrics
import os, json, sqlite3
from datetime import datetime, UTC

def _ensure_schema(conn):
    # Create required tables if they do not exist
    cur = conn.cursor()
    cur.execute("""CREATE TABLE IF NOT EXISTS tests(
        test_id TEXT PRIMARY KEY,
        tester TEXT,
        created_at TEXT,
        description TEXT,
        summary TEXT,
        issues TEXT,
        notes TEXT
    )""")
    cur.execute("""CREATE TABLE IF NOT EXISTS measurements(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        test_id TEXT,
        timestamp TEXT,
        metric_name TEXT,
        metric_value REAL,
        unit TEXT
    )""")
    cur.execute("""CREATE TABLE IF NOT EXISTS artifacts(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        test_id TEXT,
        kind TEXT,
        path TEXT,
        created_at TEXT
    )""")
    conn.commit()

def run(params):
    # Extract parameters
    db_path = params["db_path"]
    metrics_json = params["metrics_json"]
    ai_summary_md = params.get("ai_summary_md")
    meta_sidecar = params.get("meta_sidecar")

    # Load meta sidecar JSON written by orchestrator (contains test_meta)
    meta = {}
    if meta_sidecar and os.path.exists(meta_sidecar):
        meta = json.load(open(meta_sidecar, "r"))
    test_id = meta.get("test_id", "UNKNOWN_TEST")
    tester = meta.get("tester", "unknown")
    description = meta.get("description", "")

    # Ensure DB directory exists
    os.makedirs(os.path.dirname(db_path), exist_ok=True)
    # Connect to SQLite database
    conn = sqlite3.connect(db_path)
    try:
        _ensure_schema(conn)
        cur = conn.cursor()
        # Insert the test row if not present
        cur.execute("""INSERT OR IGNORE INTO tests(test_id, tester, created_at, description, summary, issues, notes)
                       VALUES(?,?,?,?,?,?,?)""",
                    (test_id, tester, datetime.now(UTC).isoformat(), description, None, None, None))
        # Load metrics from JSON
        metrics = json.load(open(metrics_json, "r"))
        # Insert each metric into the measurements table
        for r in metrics.get("rows", []):
            cur.execute("""INSERT INTO measurements(test_id, timestamp, metric_name, metric_value, unit)
                           VALUES(?,?,?,?,?)""",
                        (test_id, r.get("timestamp"), r.get("name"), r.get("value"), r.get("unit")))
        # If AI summary exists, store it in tests.summary
        if ai_summary_md and os.path.exists(ai_summary_md):
            summary_text = open(ai_summary_md, "r").read()
            cur.execute("UPDATE tests SET summary=? WHERE test_id=?", (summary_text, test_id))
        conn.commit()
    finally:
        conn.close()

    # Return counts and IDs for logging
    return {"ok": True, "test_id": test_id, "inserted": len(metrics.get("rows", []))}
