"""
ctx_file_lock.py
----------------
Context manager for atomic file locking.
Atomic file locking for ctx updates.

Usage:
    with file_lock("Data/foo.json"):
        # safe file read/write
"""
import fcntl

from contextlib import contextmanager

@contextmanager
def file_lock(path):
    with open(path, "a+") as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(f, fcntl.LOCK_UN)