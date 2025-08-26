"""
ctx_job_queue.py
----------------
Queues multiple jobs and runs them in background threads.
Simple job queue for orchestrating multiple background jobs.

Usage:
    run(ctx, {
        "jobs": [
            {"job_id": "job1", "fn": fn1, "args": {...}},
            {"job_id": "job2", "fn": fn2, "args": {...}}
        ]
    })

Description:
    - Each job runs in its own thread, tracked in ctx["jobs"].
"""
import threading
import time

def run(ctx, params):
    if "jobs" not in ctx:
        ctx["jobs"] = {}
    for job in params["jobs"]:
        job_id = job["job_id"]
        fn = job["fn"]
        args = job.get("args", {})
        ctx["jobs"][job_id] = {"status": "running", "start": time.time()}
        def worker(job_id=job_id, fn=fn, args=args):
            try:
                result = fn(**args)
                ctx["jobs"][job_id].update({"status": "done", "result": result, "end": time.time()})
            except Exception as e:
                ctx["jobs"][job_id].update({"status": "error", "error": str(e), "end": time.time()})
        t = threading.Thread(target=worker)
        t.start()
    return {"queued": [job["job_id"] for job in params["jobs"]]}