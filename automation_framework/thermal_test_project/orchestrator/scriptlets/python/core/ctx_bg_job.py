"""
ctx_bg_job.py
-------------
Runs a function as a background job, tracks status and logs output/errors in ctx.
Background job orchestration: run, monitor, and log jobs.

Usage:
    run(ctx, {"job_id": "job1", "fn": my_fn, "args": {...}})

Description:
    - Starts a thread, logs status, output, and errors in ctx["jobs"].
    - Can be extended for process pools or distributed jobs.
"""
import threading
import time

def run(ctx, params):
    job_id = params["job_id"]
    fn = params["fn"]
    args = params.get("args", {})
    if "jobs" not in ctx:
        ctx["jobs"] = {}
    ctx["jobs"][job_id] = {"status": "running", "start": time.time()}
    def worker():
        try:
            result = fn(**args)
            ctx["jobs"][job_id].update({"status": "done", "result": result, "end": time.time()})
        except Exception as e:
            ctx["jobs"][job_id].update({"status": "error", "error": str(e), "end": time.time()})
    t = threading.Thread(target=worker)
    t.start()
    return {"started": job_id}