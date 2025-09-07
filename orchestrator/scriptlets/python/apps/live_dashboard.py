"""
Module 9: Dash(-like) live dashboard stub.
Polls either:
 - Context server via host/port (JSON API assumed)
 - Fallback: normalized JSON file.

Non-blocking launch expected (runner_v2 spawns).
"""
import os, json, time, threading, http.client
from pathlib import Path

NORMALIZED = Path("orchestrator/Data/normalized.json")

def fetch_ctx(host, port):
    try:
        conn = http.client.HTTPConnection(host, port, timeout=1)
        conn.request("GET", "/full")
        resp = conn.getresponse()
        if resp.status == 200:
            return json.loads(resp.read().decode())
    except Exception:
        return {}
    return {}

def loop(host, port, interval=2.0):
    while True:
        data = fetch_ctx(host, port)
        if not data and NORMALIZED.is_file():
            try:
                data = json.loads(NORMALIZED.read_text())
            except Exception:
                data = {}
        print("[dashboard] snapshot keys:", list(data.keys())[:10])
        time.sleep(interval)

def main():
    host = os.environ.get("DASH_HOST", "127.0.0.1")
    port = int(os.environ.get("DASH_PORT", "8765"))
    t = threading.Thread(target=loop, args=(host, port), daemon=True)
    t.start()
    # Keep alive minimal
    try:
        while True:
            time.sleep(60)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()
