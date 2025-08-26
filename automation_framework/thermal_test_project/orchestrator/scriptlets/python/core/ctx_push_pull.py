"""
ctx_push_pull.py
----------------
Pushes or pulls ctx to/from a remote HTTP endpoint.
Push/pull ctx to/from a remote server (HTTP/REST).

Usage:
    run(ctx, {"action": "push", "url": "http://host/ctx", "key": "foo"})
    run(ctx, {"action": "pull", "url": "http://host/ctx", "key": "foo"})

Description:
    - Uses requests for HTTP.
"""
import requests

def run(ctx, params):
    url = params["url"]
    key = params["key"]
    if params["action"] == "push":
        resp = requests.post(url, json={key: ctx[key]})
        return {"status": resp.status_code}
    elif params["action"] == "pull":
        resp = requests.get(url)
        ctx[key] = resp.json()[key]
        return {"pulled": key}
    else:
        return {"status": "noop"}