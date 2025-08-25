# orchestrator/scriptlets/python/dump_ctx_periodic.py
"""
Periodically dump a compact snapshot of the shared CTX every N seconds.
Stops cleanly on Ctrl+C (SIGINT).

Params:
- interval_sec (float, default 5.0): seconds between dumps
- max_items_per_collection (int, default 10): limit for previewing lists/sets/tuples/dicts
- max_string_len (int, default 200): truncate very long strings
- redact_keys (list[str], optional): keys to redact (exact matches at top level)
- stop_after (int, optional): number of dumps before auto-stop; if absent, run until Ctrl+C

Behavior:
- Prints a JSON snapshot (pretty) to stdout on each tick. Orchestrator already captures stdout to Logs/<step>.log.
- Attempts to be robust against non-serializable objects by representing them as "<type object at 0x...>" strings.
- Applies simple redaction for keys likely containing secrets.

Orchestrator signature:
- Preferred: run(ctx, params)
"""

import json
import signal
import sys
import time
from typing import Any, Dict

_RUNNING = True

def _sigint_handler(sig, frame):
    global _RUNNING
    print("\n[CTX-DUMP] Ctrl+C received. Stopping.", flush=True)
    _RUNNING = False

def _preview_value(v: Any, max_items: int, max_slen: int):
    """
    Return a preview of v suitable for logging:
    - dict: take first max_items items (sorted by key if possible)
    - list/tuple/set: first max_items items
    - str: truncate to max_slen
    - bytes: show length plus first bytes
    - other: try to JSON-serialize; if fails, return repr
    """
    try:
        # strings
        if isinstance(v, str):
            if len(v) <= max_slen:
                return v
            return v[:max_slen] + f"...(len={len(v)})"

        # bytes
        if isinstance(v, (bytes, bytearray)):
            preview = v[:min(max_slen, 32)]
            return f"<bytes len={len(v)} preview={preview!r}>"

        # mappings
        if isinstance(v, dict):
            items = list(v.items())
            try:
                items = sorted(items, key=lambda kv: str(kv[0]))
            except Exception:
                pass
            head = items[:max_items]
            return {str(k): _preview_value(val, max_items, max_slen) for k, val in head} | (
                {"...": f"+{len(items) - len(head)} more"} if len(items) > len(head) else {}
            )

        # sequences
        if isinstance(v, (list, tuple, set)):
            seq = list(v)
            head = seq[:max_items]
            pv = [_preview_value(x, max_items, max_slen) for x in head]
            if len(seq) > len(head):
                pv.append(f"...(+{len(seq) - len(head)} more)")
            return pv if not isinstance(v, tuple) else tuple(pv)

        # numbers, bools, None
        if isinstance(v, (int, float, bool)) or v is None:
            return v

        # fall-back: attempt JSON serialization directly
        json.dumps(v)
        return v
    except Exception:
        # final fallback: repr
        try:
            return repr(v)
        except Exception:
            return "<unrepresentable>"

def _redact(obj: Dict[str, Any], redact_keys):
    if not redact_keys:
        return obj
    redacted = {}
    for k, v in obj.items():
        if k in redact_keys:
            redacted[k] = "***REDACTED***"
        else:
            redacted[k] = v
    return redacted

def _auto_redact_keys(ctx_top_keys):
    # Heuristic list: add keys that look sensitive
    suspects = {"api_key", "apikey", "token", "password", "secret", "bearer", "auth"}
    return [k for k in ctx_top_keys if k.lower() in suspects]

def _build_snapshot(ctx: Dict[str, Any], params: Dict[str, Any]) -> Dict[str, Any]:
    interval = float(params.get("interval_sec", 5.0))
    max_items = int(params.get("max_items_per_collection", 10))
    max_slen = int(params.get("max_string_len", 200))
    redact_keys = params.get("redact_keys") or []
    # auto-extend redactions with heuristic matches
    redact_keys = list(set(redact_keys) | set(_auto_redact_keys(ctx.keys())))

    # apply top-level redaction first, then preview deeply
    safe_top = _redact(dict(ctx), redact_keys)

    preview = {}
    for k, v in safe_top.items():
        preview[k] = _preview_value(v, max_items=max_items, max_slen=max_slen)

    snapshot = {
        "info": {
            "interval_sec": interval,
            "max_items_per_collection": max_items,
            "max_string_len": max_slen,
            "redacted_keys": redact_keys,
        },
        "ctx_preview": preview
    }
    return snapshot

def run(ctx, params):
    global _RUNNING
    signal.signal(signal.SIGINT, _sigint_handler)
    # SIGTERM handling if available
    if hasattr(signal, "SIGTERM"):
        signal.signal(signal.SIGTERM, _sigint_handler)

    interval = float(params.get("interval_sec", 5.0))
    stop_after = params.get("stop_after")
    stop_after = int(stop_after) if stop_after is not None else None

    count = 0
    print(f"[CTX-DUMP] Starting periodic dump every {interval}s. Press Ctrl+C to stop.", flush=True)
    while _RUNNING:
        snap = _build_snapshot(ctx, params)
        try:
            print(json.dumps(snap, indent=2), flush=True)
        except Exception as e:
            # last resort: print minimal info
            print(json.dumps({"error": f"serialization failed: {e}"}, indent=2), flush=True)

        count += 1
        if stop_after is not None and count >= stop_after:
            print("[CTX-DUMP] Reached stop_after limit. Exiting.", flush=True)
            break

        # sleep with responsiveness to Ctrl+C
        end_t = time.time() + max(0.1, interval)
        while _RUNNING and time.time() < end_t:
            time.sleep(min(0.1, end_t - time.time()))

    return {"ok": True, "emitted": count}

"""
- idx: 50
  name: dump_ctx_periodic
  type: python
  module: "scriptlets.python.dump_ctx_periodic"
  function: "run"
  params:
    interval_sec: 5
    max_items_per_collection: 8
    max_string_len: 160
    # Optional: stop automatically after N dumps; omit to run until Ctrl+C
    # stop_after: 12

"""
