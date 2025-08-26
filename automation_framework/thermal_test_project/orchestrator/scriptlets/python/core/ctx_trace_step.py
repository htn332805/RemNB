"""
ctx_trace_step.py
-----------------
Decorator/context manager to trace step execution time and memory.
Trace step execution with timing and resource usage.
Usage:
    with trace_step(ctx, "step_name"):
        # your code

    @trace_step_decorator(ctx, "step_name")
    def my_step(ctx, params):
        ...
"""
import time
import tracemalloc
from contextlib import contextmanager

@contextmanager
def trace_step(ctx, step_name):
    tracemalloc.start()
    t0 = time.time()
    yield
    t1 = time.time()
    mem = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    entry = {
        "step": step_name,
        "start": t0,
        "end": t1,
        "duration": t1 - t0,
        "mem_start": mem[0],
        "mem_end": mem[1],
        "mem_peak": mem[1]
    }
    if "trace" not in ctx:
        ctx["trace"] = []
    ctx["trace"].append(entry)

def trace_step_decorator(ctx, step_name):
    def decorator(fn):
        def wrapper(*args, **kwargs):
            with trace_step(ctx, step_name):
                return fn(*args, **kwargs)
        return wrapper
    return decorator