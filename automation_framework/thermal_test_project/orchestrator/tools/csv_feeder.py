#!/usr/bin/env python3
"""
csv_feeder.py — append a CSV line every n seconds until Ctrl+C.

Features:
- Required: interval seconds (-n / --interval)
- Optional: output CSV path (-o / --output), default ./Data/live.csv
- If file exists: append rows; otherwise create and write headers first
- Optional: custom headers (-H / --headers), comma-separated (no spaces)
- Optional: min/max range for random numbers (-min / -max), defaults 0..100
- Timestamp format: MM/DD/YYYY HH:MM:SS (e.g., 08/23/2026 20:23:54)
- All non-Timestamp columns are random floats within [min, max], rounded to 3 decimals
Usage examples
Append every 2 seconds to default path ./Data/live.csv:

python csv_feeder.py -n 2

Append every 0.5 seconds with custom headers and range:

python csv_feeder.py -n 0.5 -H "Timestamp,Heatsink,Inlet,Outlet" --min 20 --max 95

Append to a specific file:

python csv_feeder.py -n 1 -o /tmp/thermal_stream.csv

Notes
If custom headers are provided and do not start with “Timestamp”, the tool automatically inserts “Timestamp” as the first column.

Existing file with a different header is left intact; a warning is printed and rows are appended to avoid data loss. Adjust to your policy if you prefer to fail instead.

Values are floats with 3 decimal places; change rounding as needed.

"""

import argparse
import csv
import os
import random
import signal
import sys
import time
from datetime import datetime

DEFAULT_OUTPUT = os.path.join(".", "Data", "live.csv")
DEFAULT_HEADERS = ["Timestamp", "SensorA", "SensorB", "Sensor1", "Sensor2"]
DEFAULT_MIN = 0.0
DEFAULT_MAX = 100.0
TIME_FMT = "%m/%d/%Y %H:%M:%S"

_running = True

def _handle_sigint(sig, frame):
    global _running
    print("\n[INFO] Ctrl+C received. Stopping...", flush=True)
    _running = False

def parse_args():
    p = argparse.ArgumentParser(description="Continuously append CSV rows every n seconds until Ctrl+C.")
    p.add_argument("-n", "--interval", type=float, required=True,
                   help="Interval in seconds between rows (e.g., 1, 0.5, 2). Required.")
    p.add_argument("-o", "--output", type=str, default=DEFAULT_OUTPUT,
                   help=f"Output CSV path (default: {DEFAULT_OUTPUT})")
    p.add_argument("-H", "--headers", type=str, default=None,
                   help="Comma-separated headers (excluding spaces). First header must be 'Timestamp' or it will be inserted automatically as the first column.")
    p.add_argument("--min", dest="min_val", type=float, default=DEFAULT_MIN,
                   help=f"Minimum random value inclusive (default: {DEFAULT_MIN})")
    p.add_argument("--max", dest="max_val", type=float, default=DEFAULT_MAX,
                   help=f"Maximum random value inclusive (default: {DEFAULT_MAX})")
    return p.parse_args()

def normalize_headers(headers_arg):
    if headers_arg is None:
        return DEFAULT_HEADERS[:]
    # split by comma, strip spaces around tokens
    toks = [t.strip() for t in headers_arg.split(",") if t.strip() != ""]
    if not toks:
        return DEFAULT_HEADERS[:]
    # Ensure Timestamp is first; if missing, insert it
    if toks[0].lower() != "timestamp":
        toks = ["Timestamp"] + toks
    return toks

def ensure_parent_dir(path):
    parent = os.path.dirname(os.path.abspath(path))
    if parent and not os.path.exists(parent):
        os.makedirs(parent, exist_ok=True)

def file_exists_nonempty(path):
    return os.path.exists(path) and os.path.getsize(path) > 0

def read_existing_header(path):
    try:
        with open(path, "r", newline="") as f:
            reader = csv.reader(f)
            row = next(reader, None)
            return row
    except Exception:
        return None

def headers_match(existing, desired):
    if existing is None:
        return False
    return [h.strip() for h in existing] == [h.strip() for h in desired]

def generate_row(headers, min_val, max_val):
    ts = datetime.now().strftime(TIME_FMT)
    row = [ts]
    # For each header after Timestamp, generate a random float
    for h in headers[1:]:
        val = random.uniform(min_val, max_val)
        row.append(f"{val:.3f}")
    return row

def main():
    args = parse_args()

    if args.min_val > args.max_val:
        print(f"[ERROR] --min must be <= --max (got {args.min_val} > {args.max_val})", file=sys.stderr)
        sys.exit(2)

    headers = normalize_headers(args.headers)

    ensure_parent_dir(args.output)

    # Handle Ctrl+C gracefully
    signal.signal(signal.SIGINT, _handle_sigint)
    if hasattr(signal, "SIGTERM"):
        signal.signal(signal.SIGTERM, _handle_sigint)

    # Prepare file: write headers if file absent or empty
    need_header = True
    if file_exists_nonempty(args.output):
        existing = read_existing_header(args.output)
        if existing is None:
            print(f"[WARN] Existing file unreadable header; rewriting with desired headers.", flush=True)
        else:
            if not headers_match(existing, headers):
                print(f"[WARN] Existing header {existing} differs from desired {headers}.", flush=True)
                print(f"[WARN] Appending rows with desired schema; downstream tools should handle header mismatch.", flush=True)
            need_header = False  # file already has a header line (even if different)
    else:
        # fresh file
        pass

    # Open in append mode
    with open(args.output, "a", newline="") as f:
        writer = csv.writer(f)
        if need_header:
            writer.writerow(headers)
            f.flush()
            os.fsync(f.fileno())
            print(f"[INFO] Wrote headers to {args.output}: {headers}", flush=True)

        print(f"[INFO] Appending a row every {args.interval} second(s) to {args.output}. Press Ctrl+C to stop.", flush=True)

        # Main loop
        while _running:
            row = generate_row(headers, args.min_val, args.max_val)
            writer.writerow(row)
            f.flush()
            try:
                os.fsync(f.fileno())
            except Exception:
                pass
            print(f"[ROW] {row}", flush=True)
            # Sleep
            # If interval is < 0.01, cap to avoid busy loop
            sleep_s = max(0.01, args.interval)
            # Sleep in small chunks so Ctrl+C is responsive
            end_time = time.time() + sleep_s
            while _running and time.time() < end_time:
                time.sleep(min(0.1, end_time - time.time()))

    print("[INFO] Exiting. Goodbye.", flush=True)

if __name__ == "__main__":
    main()
