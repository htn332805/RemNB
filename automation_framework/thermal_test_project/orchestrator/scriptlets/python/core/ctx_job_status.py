"""
ctx_job_status.py
-----------------
Queries the status or result of a background job.
Query status/results of background jobs.

Usage:
    run(ctx, {"job_id": "job1"})

Description:
    - Returns status/result/error for the given job_id.
"""
def run(ctx, params):
    job_id = params["job_id"]
    job = ctx.get("jobs", {}).get(job_id)
    if not job:
        return {"error": "No such job"}
    return {"status": job.get("status"), "result": job.get("result"), "error": job.get("error")}