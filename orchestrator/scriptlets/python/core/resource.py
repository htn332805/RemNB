from __future__ import annotations
import time
from functools import wraps
from typing import Callable, TypeVar, ParamSpec

P = ParamSpec("P")
R = TypeVar("R")


def track_resources(func: Callable[P, R]) -> Callable[P, R]:
    @wraps(func)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:
        start = time.time()
        try:
            return func(*args, **kwargs)
        finally:
            duration = time.time() - start
            import sys
            print(f"[resource] {func.__name__} duration_sec={duration:.4f}", file=sys.stderr)
    return wrapper
