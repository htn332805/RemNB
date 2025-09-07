"""Base scriptlet abstractions.

NAME: base.py
DESCRIPTION: Provides BaseScriptlet class defining interface for all Python scriptlets in the automation framework.
USAGE:
  - Inherit from BaseScriptlet and implement validate(ctx, params) and run(ctx, params).
  - Optionally override requires_process for CPU heavy work.
RETURNS: Implementations return a JSON-serializable dict with status and metadata.
LIMITATIONS:
  - Context object is expected to provide set/get/history methods.
  - No heavy logic here; keep minimal to preserve backward compatibility.
"""
from __future__ import annotations
from abc import ABC, abstractmethod
from typing import Any, Dict

class BaseScriptlet(ABC):
  requires_process: bool = False

  @abstractmethod
  def validate(self, ctx: Any, params: Dict[str, Any]) -> None:  # pragma: no cover - interface
    """Validate incoming params prior to run."""
    raise NotImplementedError

  @abstractmethod
  def run(self, ctx: Any, params: Dict[str, Any]) -> Dict[str, Any]:  # pragma: no cover - interface
    """Execute scriptlet logic and return JSON-serializable result."""
    raise NotImplementedError
