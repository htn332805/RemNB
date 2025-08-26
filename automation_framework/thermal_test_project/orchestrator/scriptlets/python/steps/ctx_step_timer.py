"""
ctx_step_timer.py
-----------------
Times and logs the duration of each step in a recipe.
Time and log the duration of each step in a recipe.


Usage:
    with step_timer(ctx, "step_name"):
        # step code

Description:
    - Appends timing info to ctx["step_times"].
"""
import time
from contextlib import contextmanager

@contextmanager
def step_timer(ctx, step_name):
    t0 = time.time()
    yield
    t1 = time.time()
    entry = {"step": step_name, "start": t0, "end": t1, "duration": t1 - t0}
    if "step_times" not in ctx:
        ctx["step_times"] = []
    ctx["step_times"].append(entry)