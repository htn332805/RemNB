"""
ctx_profiler.py
---------------
Profiles execution time and memory usage of a step or callback.

Usage:
    with profile_ctx(ctx, "step_name"):
        # your code here

    from scriptlets.python.core.ctx_profiler import profile_ctx
        def run(ctx, params):
            with profile_ctx(ctx, "heavy_step"):
                # ... your heavy computation ...
                pass

Description:
    - Records start/end time, memory before/after, and logs to ctx["profile"].
"""
import time
import tracemalloc
from contextlib import contextmanager

@contextmanager
def profile_ctx(ctx, step_name):
    tracemalloc.start()
    t0 = time.time()
    mem0 = tracemalloc.get_traced_memory()[0]
    yield
    t1 = time.time()
    mem1 = tracemalloc.get_traced_memory()[0]
    tracemalloc.stop()
    entry = {
        "step": step_name,
        "start": t0,
        "end": t1,
        "duration": t1 - t0,
        "mem_start": mem0,
        "mem_end": mem1,
        "mem_delta": mem1 - mem0
    }
    if "profile" not in ctx:
        ctx["profile"] = []
    ctx["profile"].append(entry)