"""Base class for Python scriptlets.

Each scriptlet subclasses BaseScriptlet and implements validate + run.
"""
from __future__ import annotations
from typing import Any, Dict
from abc import ABC, abstractmethod


class BaseScriptlet(ABC):
    @abstractmethod
    def validate(self, ctx, params: Dict[str, Any]) -> None:  # pragma: no cover - interface
        ...

    @abstractmethod
    def run(self, ctx, params: Dict[str, Any]) -> int:  # pragma: no cover - interface
        ...
