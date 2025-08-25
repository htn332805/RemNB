"""
Context object for in-memory sharing between scriptlets.

- Context is a dict-like store for small-to-medium sized Python objects.
- Includes helper methods for namespacing and safe updates.
- Not persisted to disk (by design in Option A).
"""

from typing import Any, Dict, Optional

class Context(dict):
    """
    A thin wrapper over dict so we can add helpers and type hints.
    Use like a normal dict: ctx["key"] = value
    """
    def ns(self, name: str) -> Dict[str, Any]:
        """
        Return a namespaced sub-dict, creating it if necessary.
        Example: ctx.ns("metrics") returns ctx["metrics"] (dict).
        """
        if name not in self or not isinstance(self[name], dict):
            self[name] = {}
        return self[name]

    def get_ns(self, name: str) -> Dict[str, Any]:
        """
        Get an existing namespace (dict), or empty dict if missing (without creating).
        """
        val = self.get(name)
        return val if isinstance(val, dict) else {}

    def set_if_absent(self, key: str, value: Any) -> None:
        """
        Set a key if it doesn't already exist.
        """
        if key not in self:
            self[key] = value

def expose_for_shell(ctx: "Context", keys: Optional[list] = None) -> Dict[str, str]:
    """
    Prepare environment variables for shell steps from selected ctx keys.
    - Only simple scalar types (str, int, float, bool) are directly stringified.
    - For complex objects, consider using stdin streaming instead.
    Returns a dict of env vars like {"CTX_TEST_ID": "T-001"}.
    """
    env = {}
    if not keys:
        return env
    for k in keys:
        if k in ctx:
            v = ctx[k]
            if isinstance(v, (str, int, float, bool)):
                env[f"CTX_{k.upper()}"] = str(v)
            else:
                # Skip complex values here to avoid giant env; use stdin bridge instead.
                pass
    return env
