# orchestrator/scriptlets/python/stream_feeder_mem.py
import threading, time, random
from datetime import datetime

def init_meta(ctx, params):
    """
    Initialize shared stream config and buffer in CTX.
    params:
      headers: list[str] of column names (excluding 'Timestamp')
      interval_sec: float
      min_val, max_val: float
      buffer_max_rows: int
    """
    headers = params.get("headers") or ["AmbTemp","LiquidTemp","SystemTemp"]
    cfg = {
        "headers": headers,
        "interval_sec": float(params.get("interval_sec", 1.0)),
        "min_val": float(params.get("min_val", 20.0)),
        "max_val": float(params.get("max_val", 95.0)),
        "buffer_max_rows": int(params.get("buffer_max_rows", 20000)),
    }
    ctx["stream_config"] = cfg
    ctx["stream_rows"] = ctx.get("stream_rows") or []  # list of dict rows
    ctx["stream_stop_flag"] = False
    return {"ok": True, "config": cfg}

def _producer(ctx, override=None):
    cfg = dict(ctx.get("stream_config") or {})
    if override:
        cfg.update(override)
    headers = cfg["headers"]
    interval = cfg["interval_sec"]
    mn = cfg["min_val"]; mx = cfg["max_val"]
    cap = cfg["buffer_max_rows"]
    rows = ctx.get("stream_rows")

    while not ctx.get("stream_stop_flag", False):
        row = {"Timestamp": datetime.now().strftime("%m/%d/%Y %H:%M:%S")}
        for h in headers:
            row[h] = round(random.uniform(mn, mx), 3)
        rows.append(row)
        if len(rows) > cap:
            del rows[:len(rows)-cap]
        time.sleep(interval)

def start_background(ctx, params):
    """
    Start background producer thread. 
    params:
      override: dict to override interval_sec/min/max etc at run time
    """
    override = params.get("override") or {}
    if ctx.get("stream_thread") and ctx["stream_thread"].is_alive():
        return {"ok": True, "running": True}
    ctx["stream_stop_flag"] = False
    t = threading.Thread(target=_producer, args=(ctx, override), daemon=True)
    ctx["stream_thread"] = t
    t.start()
    return {"ok": True, "running": True}

def stop(ctx, params):
    ctx["stream_stop_flag"] = True
    t = ctx.get("stream_thread")
    if t:
        t.join(timeout=2.0)
    return {"ok": True, "stopped": True}
