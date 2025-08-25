# orchestrator/scriptlets/python/ctx_dump_loop.py
import json
import time
from datetime import datetime

def _slice_rows(ctx, n):
    """
    Return the last n rows from ctx['stream_rows'] (if present), otherwise [].
    """
    rows = ctx.get("stream_rows") or []
    if not rows:
        return []
    try:
        n = int(n)
    except Exception:
        n = 0
    return rows[-n:] if n and n > 0 else rows

def run(ctx, params):
    """
    Periodically print a compact JSON snapshot of the shared CTX to stdout.

    params:
      period_sec: int seconds between dumps (default 10)
      title: optional label included in the dump
      keys_only: bool, if True only prints ctx keys (no row sample)
      sample_rows: int, how many last rows to sample from ctx['stream_rows'] (default 3)

    The function runs in a loop until interrupted (Ctrl+C) and is intended as a blocking step.
    """
    period = int(params.get("period_sec", 10))
    title = params.get("title", "CTX dump")
    keys_only = bool(params.get("keys_only", False))
    sample_rows = int(params.get("sample_rows", 3))

    print(f"[CTX DUMPER] Started ({period}s interval) — {title}", flush=True)
    try:
        while True:
            payload = {
                "at": f"{datetime.utcnow().isoformat()}Z",
                "title": title,
                "ctx_keys": list(ctx.keys())
            }
            if not keys_only:
                rows = ctx.get("stream_rows") or []
                payload["stream_row_count"] = len(rows)
                # Only include the last of the sampled rows to keep the log compact
                last_sample = _slice_rows(ctx, sample_rows)
                payload["rows_last_sample"] = last_sample[-1] if last_sample else None
            print(json.dumps(payload, indent=2), flush=True)
            time.sleep(period)
    except KeyboardInterrupt:
        print("[CTX DUMPER] Stopped by user.", flush=True)
        return {"ok": True, "stopped": True}
