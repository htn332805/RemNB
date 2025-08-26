"""
ctx_scheduler.py
----------------
Schedules steps or jobs to run at specific times or intervals.

Usage:
    run(ctx, {
        "jobs": [
            {"job_id": "job1", "fn": my_fn, "args": {...}, "interval": 60},  # every 60 seconds
            {"job_id": "job2", "fn": my_other_fn, "args": {...}, "at": "14:30"}
        ]
    })

Description:
    - Uses threading.Timer for interval jobs.
    - For "at" jobs, runs at the next occurrence of the specified time.
"""
import threading
import time
from datetime import datetime, timedelta

def _run_at(fn, args, at_time, job_id, ctx):
    now = datetime.now()
    target = datetime.strptime(at_time, "%H:%M").replace(year=now.year, month=now.month, day=now.day)
    if target < now:
        target += timedelta(days=1)
    delay = (target - now).total_seconds()
    def job():
        ctx["jobs"][job_id] = {"status": "running", "start": time.time()}
        try:
            result = fn(**args)
            ctx["jobs"][job_id].update({"status": "done", "result": result, "end": time.time()})
        except Exception as e:
            ctx["jobs"][job_id].update({"status": "error", "error": str(e), "end": time.time()})
    threading.Timer(delay, job).start()

def _run_interval(fn, args, interval, job_id, ctx):
    def job():
        ctx["jobs"][job_id] = {"status": "running", "start": time.time()}
        try:
            result = fn(**args)
            ctx["jobs"][job_id].update({"status": "done", "result": result, "end": time.time()})
        except Exception as e:
            ctx["jobs"][job_id].update({"status": "error", "error": str(e), "end": time.time()})
        threading.Timer(interval, job).start()
    threading.Timer(interval, job).start()

def run(ctx, params):
    if "jobs" not in ctx:
        ctx["jobs"] = {}
    for job in params["jobs"]:
        job_id = job["job_id"]
        fn = job["fn"]
        args = job.get("args", {})
        if "interval" in job:
            _run_interval(fn, args, job["interval"], job_id, ctx)
        elif "at" in job:
            _run_at(fn, args, job["at"], job_id, ctx)
    return {"scheduled": [job["job_id"] for job in params["jobs"]]}