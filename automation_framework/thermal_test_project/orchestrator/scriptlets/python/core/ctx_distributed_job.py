"""
ctx_distributed_job.py
----------------------
Submits a job to a distributed job server and tracks status/results in ctx.
Submit jobs to a distributed job server (e.g., Celery, REST API) and track results in ctx.

Usage:
    run(ctx, {
        "job_id": "job1",
        "server_url": "http://localhost:5000/jobs",
        "payload": {"task": "process_data", "args": {...}}
    })

Description:
    - Submits job via HTTP POST, stores job_id and status in ctx["jobs"].
    - Polls for job completion and updates ctx["jobs"][job_id].
"""
import requests
import time

def run(ctx, params):
    job_id = params["job_id"]
    server_url = params["server_url"]
    payload = params["payload"]
    if "jobs" not in ctx:
        ctx["jobs"] = {}
    # Submit job
    resp = requests.post(server_url, json=payload)
    if resp.status_code != 200:
        ctx["jobs"][job_id] = {"status": "error", "error": resp.text}
        return {"error": resp.text}
    job_info = resp.json()
    ctx["jobs"][job_id] = {"status": "submitted", "server_id": job_info.get("id")}
    # Poll for completion
    poll_url = f"{server_url}/{job_info.get('id')}"
    while True:
        poll_resp = requests.get(poll_url)
        if poll_resp.status_code != 200:
            ctx["jobs"][job_id]["status"] = "error"
            ctx["jobs"][job_id]["error"] = poll_resp.text
            break
        status = poll_resp.json().get("status")
        ctx["jobs"][job_id]["status"] = status
        if status in ("done", "error"):
            ctx["jobs"][job_id]["result"] = poll_resp.json().get("result")
            break
        time.sleep(2)
    return {"job_id": job_id, "status": ctx["jobs"][job_id]["status"]}