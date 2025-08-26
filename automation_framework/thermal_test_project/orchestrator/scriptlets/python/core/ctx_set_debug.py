"""
ctx_set_debug.py
----------------
Sets ctx["debug"] to True or False.

Usage:
    run(ctx, {"debug": True})

Description:
    - Use ctx["debug"] in other scriptlets to control verbosity.

Requirements:
    - Python 3.6+
"""
def run(ctx, params):
    ctx["debug"] = bool(params.get("debug", True))
    return {"debug": ctx["debug"]}