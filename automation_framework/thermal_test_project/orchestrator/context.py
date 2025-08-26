"""
context.py
----------
Defines the Context class and helpers for in-memory data sharing between orchestrator steps.
Provides utilities to expose selected ctx values to shell environments.

Usage:
    from context import Context, expose_for_shell
    ctx = Context()
    ctx["foo"] = 123
    env = expose_for_shell(ctx, ["foo"])
    # env["foo"] == "123"
"""

import json
import time
from typing import Any, Dict, List

class Context(dict):
    """
    In-memory context for sharing data between orchestrator steps.
    Behaves like a dict, but can be extended for custom logic if needed.
    """
    def __init__(self):
        super().__init__()
        self._history = {}

    def __setitem__(self, key, value):
        try:
            json.dumps(value)
        except Exception as e:
            raise ValueError(f"ctx[{key!r}] value is not JSON serializable: {e}")
        self._history.setdefault(key, []).append({
            "timestamp": time.time(),
            "value": value,
            "who": "step_name"  # Pass this info from runner if needed
        })
        super().__setitem__(key, value)

    def update(self, *args, **kwargs):
        for k, v in dict(*args, **kwargs).items():
            self[k] = v

    def get_history(self, key):
        return self._history.get(key, [])

    def last_modified(self, key):
        h = self._history.get(key, [])
        return h[-1]["timestamp"] if h else None

def expose_for_shell(ctx: Dict[str, Any], keys: List[str]) -> Dict[str, str]:
    """
    Prepare a dict of environment variables from ctx for shell scriptlets.
    """
    env = {}
    for k in keys:
        v = ctx.get(k, "")
        if isinstance(v, (dict, list)):
            env[k] = json.dumps(v)
        else:
            env[k] = str(v)
    return env

if __name__ == "__main__":
    ctx = Context()
    ctx["foo"] = 42
    ctx["bar"] = [1, 2, 3]
    env = expose_for_shell(ctx, ["foo", "bar"])
    for k, v in env.items():
        print(f"{k}={v}")