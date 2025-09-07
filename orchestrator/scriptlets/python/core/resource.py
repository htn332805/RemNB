"""Resource tracking decorator using psutil + tracemalloc.
"""
from __future__ import annotations
import functools, time, tracemalloc, json
from typing import Any, Callable, Dict, TypeVar, ParamSpec
try:
    import psutil  # type: ignore
except ImportError:  # lightweight fallback
    psutil = None  # type: ignore
from .logging import get_logger

logger = get_logger(__name__)

P = ParamSpec('P')
R = TypeVar('R')

def track_resources(func: Callable[P, R]) -> Callable[P, R]:
    @functools.wraps(func)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:  # type: ignore[misc]
        tracemalloc.start()
        start_time = time.time()
        process = psutil.Process() if psutil else None
        start_mem = process.memory_info().rss if process else 0
        try:
            result: R = func(*args, **kwargs)
            return result
        finally:
            end_time = time.time()
            current, peak = tracemalloc.get_traced_memory()
            tracemalloc.stop()
            end_mem = process.memory_info().rss if process else 0
            delta_rss = end_mem - start_mem
            logger.info(
                json.dumps({
                    "metric": "resource_usage",
                    "function": func.__name__,
                    "seconds": round(end_time - start_time, 6),
                    "rss_delta": delta_rss,
                    "heap_current": current,
                    "heap_peak": peak,
                })
            )
    return wrapper
