"""Shared Context management for scriptlets.

Minimal implementation: holds JSON-serializable data and tracks history.
"""
from __future__ import annotations
from typing import Any, Dict, List
import json
import time


class Context:
    def __init__(self) -> None:
        self._data: Dict[str, Any] = {}
        self._history: List[Dict[str, Any]] = []

    def set(self, key: str, value: Any, who: str) -> None:
        # Basic JSON-serializable validation
        try:
            json.dumps(value)
        except TypeError as e:
            raise TypeError(f"Value for key '{key}' not JSON-serializable: {e}")
        old = self._data.get(key)
        self._data[key] = value
        self._history.append({
            "ts": time.time(),
            "key": key,
            "who": who,
            "old": old,
            "new": value,
        })

    def get(self, key: str, default: Any = None) -> Any:
        return self._data.get(key, default)

    def to_dict(self) -> Dict[str, Any]:
        return dict(self._data)

    @property
    def history(self) -> List[Dict[str, Any]]:
        return list(self._history)
