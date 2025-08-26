"""
ctx_step_rollback_point.py
--------------------------
Marks rollback points and restores ctx to the last rollback point.

Usage:
    run(ctx, {"action": "mark", "out": "rollback_point"})
    run(ctx, {"action": "restore", "in": "rollback_point"})

Description:
    - Stores/restores a deep copy of ctx at a rollback point.
"""
import copy

def run(ctx, params):
    action = params["action"]
    key = params.get("out") or params.get("in") or "rollback_point"
    if action == "mark":
        ctx[key] = copy.deepcopy(dict(ctx))
        return {"marked": key}
    elif action == "restore":
        if key in ctx:
            old = ctx[key]
            ctx.clear()
            ctx.update(old)
            return {"restored": key}
        else:
            return {"error": "No rollback point"}
    else:
        return {"error": "Unknown action"}