"""
ctx_parallel.py
---------------
Runs multiple functions in parallel and collects results in ctx.
Run multiple functions in parallel and collect results in ctx.

Usage:
    run(ctx, {"tasks": [
        {"key": "a", "fn": fn_a, "args": {...}},
        {"key": "b", "fn": fn_b, "args": {...}}
    ]})

Description:
    - Each task runs in its own thread.
    - Results are stored in ctx[task["key"]].
"""
import threading

def run(ctx, params):
    threads = []
    for task in params["tasks"]:
        def worker(task=task):
            ctx[task["key"]] = task["fn"](**task.get("args", {}))
        t = threading.Thread(target=worker)
        t.start()
        threads.append(t)
    for t in threads:
        t.join()
    return {"completed": [task["key"] for task in params["tasks"]]}