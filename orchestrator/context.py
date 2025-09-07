### AUTOGEN(rebase_workspace.sh)
"""
Context store (minimal) – version-safe key/value JSON-friendly.
"""
from __future__ import annotations
import copy, threading, json
class Context:
    def __init__(self):
        self._lock = threading.RLock()
        self._data = {}
        self._history = []
    def set(self, key, value, who="unknown"):
        if not isinstance(key, str):
            raise TypeError("key must be str")
        if key.count("_v") == 0:
            # Non-fatal warning pattern (could elevate later)
            pass
        with self._lock:
            self._data[key] = value
            self._history.append({"key": key, "who": who})
    def get(self, key, default=None):
        with self._lock:
            return self._data.get(key, default)
    def to_dict(self):
        with self._lock:
            return copy.deepcopy(self._data)
    def history(self):
        with self._lock:
            return list(self._history)
    def __repr__(self):
        return f"Context(keys={len(self._data)})"
if __name__ == "__main__":
    c=Context(); c.set("demo.key_v1", {"ok":True}, who="selftest"); print(json.dumps(c.to_dict()))
