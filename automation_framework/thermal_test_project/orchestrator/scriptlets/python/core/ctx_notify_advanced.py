"""
ctx_notify_advanced.py
----------------------
Sends notifications via Slack, email, or SMS when ctx events occur.

Usage:
    run(ctx, {"type": "slack", "webhook_url": "...", "message": "Job finished!"})
    run(ctx, {"type": "email", "to": "user@example.com", "subject": "Alert", "body": "Job failed."})

Description:
    - Supports Slack via webhook, email via SMTP, and can be extended for SMS.
"""
import requests
import smtplib
from email.mime.text import MIMEText

def run(ctx, params):
    if params["type"] == "slack":
        resp = requests.post(params["webhook_url"], json={"text": params["message"]})
        return {"status": resp.status_code}
    elif params["type"] == "email":
        msg = MIMEText(params["body"])
        msg["Subject"] = params["subject"]
        msg["From"] = params.get("from", "noreply@example.com")
        msg["To"] = params["to"]
        with smtplib.SMTP(params.get("smtp_server", "localhost")) as server:
            server.send_message(msg)
        return {"status": "sent"}
    # Add more notification types as needed (e.g., SMS)
    return {"status": "noop"}