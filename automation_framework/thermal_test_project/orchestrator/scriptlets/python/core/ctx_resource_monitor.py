"""
ctx_resource_monitor.py
----------------------
Monitors and logs CPU/memory usage during workflow execution.
Monitor and log CPU/memory usage during workflow execution.

Usage:
    with resource_monitor(ctx, "step_name"):
        # step code

Description:
    - Appends resource usage info to ctx["resources"].
"""
import psutil
import time
from contextlib import contextmanager

@contextmanager
def resource_monitor(ctx, step_name):
    process = psutil.Process()
    cpu0 = process.cpu_percent(interval=None)
    mem0 = process.memory_info().rss
    t0 = time.time()
    yield
    t1 = time.time()
    cpu1 = process.cpu_percent(interval=None)
    mem1 = process.memory_info().rss
    entry = {
        "step": step_name,
        "start": t0,
        "end": t1,
        "duration": t1 - t0,
        "cpu_start": cpu0,
        "cpu_end": cpu1,
        "mem_start": mem0,
        "mem_end": mem1
    }
    if "resources" not in ctx:
        ctx["resources"] = []
    ctx["resources"].append(entry)