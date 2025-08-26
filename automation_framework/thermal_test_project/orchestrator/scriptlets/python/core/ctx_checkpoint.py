"""
ctx_checkpoint.py
-----------------
Creates and restores named checkpoints of ctx.
Create and restore named checkpoints of ctx for safe workflow resumption.

Usage:
    run(ctx, {"action": "save", "name": "checkpoint1"})
    run(ctx, {"action": "restore", "name": "checkpoint1"})

Description:
    - Stores/restores a deep copy of ctx under ctx["_checkpoints"][name].
"""
import copy

def run(ctx, params):
    action = params["action"]
    name = params["name"]
    if "_checkpoints" not in ctx:
        ctx["_checkpoints"] = {}
    if action == "save":
        ctx["_checkpoints"][name] = copy.deepcopy(dict(ctx))
        return {"checkpoint_saved": name}
    elif action == "restore":
        if name in ctx["_checkpoints"]:
            old = ctx["_checkpoints"][name]
            ctx.clear()
            ctx.update(old)
            return {"checkpoint_restored": name}
        else:
            return {"error": "No such checkpoint"}
    else:
        return {"error": "Unknown action"}