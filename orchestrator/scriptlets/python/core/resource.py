### AUTOGEN(rebase_workspace.sh)
import time, functools, sys, json
def track_resources(fn):
    def wrapper(*a, **kw):
        start = time.time()
        try:
            return fn(*a, **kw)
        finally:
            dur = time.time() - start
            print(f"[resource] duration_sec={dur:.4f}", file=sys.stderr)
    return functools.wraps(fn)(wrapper)
