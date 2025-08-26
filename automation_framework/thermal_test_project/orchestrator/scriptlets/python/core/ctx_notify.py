"""
ctx_notify.py
-------------
Sends notifications (email, Slack, etc.) when ctx events occur.
Send notifications (email, Slack, etc.) when ctx events occur.
Usage:
    run(ctx, {"type": "slack", "webhook_url": "...", "message": "Job finished!"})

Description:
    - Supports Slack via webhook, can be extended for email/SMS.
"""
import requests

def run(ctx, params):
    if params["type"] == "slack":
        resp = requests.post(params["webhook_url"], json={"text": params["message"]})
        return {"status": resp.status_code}
    # Add more notification types as needed
    return {"status": "noop"}