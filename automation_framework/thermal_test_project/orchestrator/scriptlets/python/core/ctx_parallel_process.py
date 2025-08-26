"""
ctx_parallel_process.py
-----------------------
Runs multiple functions in parallel using multiprocessing and collects results in ctx.
Run multiple functions in parallel using multiprocessing (for CPU-bound tasks).

Usage:
    run(ctx, {"tasks": [
        {"key": "a", "fn": fn_a, "args": {...}},
        {"key": "b", "fn": fn_b, "args": {...}}
    ]})

Description:
    - Each task runs in its own process.
    - Results are stored in ctx[task["key"]].
"""
import multiprocessing

def _worker(fn, args, queue, key):
    try:
        result = fn(**args)
        queue.put((key, result, None))
    except Exception as e:
        queue.put((key, None, str(e)))

def run(ctx, params):
    queue = multiprocessing.Queue()
    processes = []
    for task in params["tasks"]:
        p = multiprocessing.Process(target=_worker, args=(task["fn"], task.get("args", {}), queue, task["key"]))
        p.start()
        processes.append(p)
    for _ in processes:
        key, result, error = queue.get()
        if error:
            ctx[key] = {"error": error}
        else:
            ctx[key] = result
    for p in processes:
        p.join()
    return {"completed": [task["key"] for task in params["tasks"]]}