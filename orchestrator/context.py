"""Shared JSON-serializable context with history tracking.
"""
from __future__ import annotations
import json, time, threading
from typing import Any, Dict, List, Optional
from orchestrator.scriptlets.python.core.logging import get_logger

logger = get_logger(__name__)

class Context:
    def __init__(self):
        self._lock = threading.RLock()
        self._data: Dict[str, Any] = {}
        self._history: Dict[str, List[Dict[str, Any]]] = {}

    def set(self, key: str, value: Any, who: str) -> None:
        # ensure JSON serializable
        try:
            json.dumps(value)
        except TypeError as e:
            raise TypeError(f"Value for {key} not JSON-serializable: {e}")
        with self._lock:
            old = self._data.get(key)
            self._data[key] = value
            entry = {
                "ts": time.time(),
                "who": who,
                "old": old,
                "new": value,
            }
            self._history.setdefault(key, []).append(entry)
            logger.info(f"ctx.set key={key} who={who}")

    def get(self, key: str, default: Any=None) -> Any:
        return self._data.get(key, default)

    def get_history(self, key: str) -> List[Dict[str, Any]]:
        return self._history.get(key, [])

    def last_modified(self, key: str) -> Optional[float]:
        hist = self._history.get(key)
        if not hist:
            return None
        return hist[-1]["ts"]

    def to_dict(self) -> Dict[str, Any]:
        return dict(self._data)
