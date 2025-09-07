"""Standalone runner to keep shared context server alive without launching Dash UI."""
import time

started_backend = None
try:
    import data.shared_context  # starts threads (expected real implementation)
    started_backend = "data.shared_context"
except Exception as e:
    print(f"[run_context_server] WARN: failed to import data.shared_context ({e}); starting minimal in-process key/value server.")
    import threading, json, socket

    _STORE = {}

    def _handle(conn, addr):
        with conn:
            raw = conn.recv(8192).decode().strip()
            parts = raw.split(" ", 2)
            resp = {}
            try:
                op = parts[0] if parts else ""
                if op == "full":
                    resp = {"status": "ok", "data": _STORE}
                elif op == "get":
                    key = parts[1]
                    resp = {"status": "ok", "value": _STORE.get(key)}
                elif op == "set":
                    key = parts[1]; val_raw = parts[2]
                    try:
                        val = json.loads(val_raw)
                    except Exception:
                        val = val_raw
                    _STORE[key] = val
                    resp = {"status": "ok"}
                elif op == "append":
                    key = parts[1]; val_raw = parts[2]
                    try:
                        val = json.loads(val_raw)
                    except Exception:
                        val = val_raw
                    lst = _STORE.setdefault(key, [])
                    if not isinstance(lst, list):
                        resp = {"status": "error", "reason": "target not list"}
                    else:
                        lst.append(val)
                        resp = {"status": "ok"}
                else:
                    resp = {"status": "error", "reason": f"unknown op '{op}'", "raw": raw}
            except Exception as ex:
                resp = {"status": "error", "reason": str(ex)}
            try:
                conn.sendall((json.dumps(resp) + "\n").encode())
            except Exception:
                pass

    def _serve():
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind(("127.0.0.1", 8765))
            s.listen(32)
            while True:
                c, a = s.accept()
                threading.Thread(target=_handle, args=(c, a), daemon=True).start()

    import threading
    threading.Thread(target=_serve, daemon=True).start()
    started_backend = "fallback_minimal_server"

print(f"Shared context server backend: {started_backend}. Listening on 127.0.0.1:8765. Press Ctrl+C to stop.")
try:
    while True:
        time.sleep(3600)
except KeyboardInterrupt:
    print("Exiting context server runner.")
