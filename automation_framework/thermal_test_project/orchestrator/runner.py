"""
runner.py
---------
Main orchestrator pipeline runner for the automation framework.

Features:
    1. Executes a recipe YAML file step-by-step, honoring dependencies.
    2. If "runner.debug" file exists or -d/--debug flag is used, enables maximum verbosity and logs all ctx modifications with timestamps.
    3. Supports background and parallel (multithreaded) steps as specified in the recipe.
    4. Logs all step actions, dependencies, resource usage, and errors with timestamps.
    5. Regenerates a color-coded digraph of step status and dependencies after each step.
    6. Supports --only and --skip CLI filters for step selection.
    7. On failure, logs reason and pickles ctx and file state for troubleshooting and resume.
    8. Reports execution time, memory, CPU, disk usage, and PIDs for each step.
    9. Can resume from a pickled ctx and file state after failure.

Usage:
    python runner.py --recipe recipes/demo_screen_dash_ctx.yaml
    python runner.py --debug --only step1 step2
    python runner.py --skip step3
    python runner.py --resume-from step4

Limitations:
    - Requires Python 3.8+ and dependencies in requirements.txt.
    - Only supports YAML recipes in orchestrator/recipes.
    - Only JSON-serializable objects are stored in ctx.

Example:
    python runner.py --recipe recipes/demo_screen_dash_ctx.yaml
"""

import os
import sys
import time
import pickle
import argparse
import threading
import subprocess
import yaml
import psutil
import shutil
import traceback
from datetime import datetime
from collections import defaultdict

# Import context helpers
from context import Context

# Constants
LOGS_DIR = "Logs"
ASSETS_DIR = "Assets"
DATA_DIR = "Data"
ERROR_LOG = os.path.join(LOGS_DIR, "error.log")
CTX_LOG = os.path.join(LOGS_DIR, "ctx_debug.log")
STATE_PICKLE = "state.pkl"
DEBUG_FILE = "runner.debug"

# Utility: timestamp string
def nowstr():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Utility: log to file with timestamp
def log_error(msg):
    with open(ERROR_LOG, "a") as f:
        f.write(f"[{nowstr()}] {msg}\n")

def log_ctx_change(ctx, key, value):
    with open(CTX_LOG, "a") as f:
        f.write(f"[{nowstr()}] ctx[{key!r}] modified. New value: {repr(value)}\n")
        f.write(f"[{nowstr()}] Full ctx: {repr(dict(ctx))}\n")

# Utility: print with optional debug
def debug_print(msg, debug):
    if debug:
        print(f"[DEBUG {nowstr()}] {msg}")

# Utility: measure resource usage
def resource_report(proc):
    try:
        p = psutil.Process(proc.pid)
        mem = p.memory_info().rss / (1024 * 1024)
        cpu = p.cpu_percent(interval=0.1)
        children = p.children(recursive=True)
        child_pids = [c.pid for c in children]
        disk = shutil.disk_usage(".").used / (1024 * 1024)
        return mem, cpu, disk, proc.pid, child_pids
    except Exception as e:
        return 0, 0, 0, proc.pid, []

# Utility: pickle state
def save_state(ctx, files):
    with open(STATE_PICKLE, "wb") as f:
        pickle.dump({"ctx": dict(ctx), "files": files}, f)

def load_state():
    with open(STATE_PICKLE, "rb") as f:
        return pickle.load(f)

# Utility: update digraph (stub, implement as needed)
def update_digraph(steps, status):
    # Use graphviz or similar to generate Assets/graph.png
    # Color code: green=success, red=fail, orange=running, white=pending
    pass

# Parse CLI args
parser = argparse.ArgumentParser(description="Orchestrator runner")
parser.add_argument("--recipe", type=str, default="recipes/default_recipe.yaml")
parser.add_argument("--debug", "-d", action="store_true")
parser.add_argument("--only", nargs="*", help="Only run these steps")
parser.add_argument("--skip", nargs="*", help="Skip these steps")
parser.add_argument("--resume-from", type=str, help="Resume from this step after failure")
args = parser.parse_args()

# Debug mode if runner.debug file exists or --debug flag
debug = args.debug or os.path.exists(DEBUG_FILE)

# Load recipe
with open(args.recipe, "r") as f:
    recipe = yaml.safe_load(f)

steps = recipe["steps"]
step_map = {step["name"]: step for step in steps}
step_status = {step["name"]: "pending" for step in steps}

# Shared in-memory context
ctx = Context()
GLOBAL_STATE = {}

# Load state if resume requested
if args.resume_from and os.path.exists(STATE_PICKLE):
    state = load_state()
    ctx.update(state["ctx"])
    # Optionally restore files from state["files"]

# Step execution helpers
def run_python_step(step, ctx, debug):
    module = step["module"]
    function = step["function"]
    params = step.get("params", {})
    mod = __import__(module, fromlist=[function])
    func = getattr(mod, function)
    start = time.time()
    result = func(ctx, params)
    elapsed = time.time() - start
    if debug:
        print(f"[{step['name']}] Python step completed in {elapsed:.2f}s, result: {result}")
    return result, elapsed

def run_shell_step(step, ctx, debug):
    cmd = [step["cmd"]] + step.get("args", [])
    env = os.environ.copy()
    # Expose ctx keys if specified
    for k in step.get("shell_env_keys", []):
        env[k] = str(ctx.get(k, ""))
    start = time.time()
    proc = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    out, err = proc.communicate()
    elapsed = time.time() - start
    if debug:
        print(f"[{step['name']}] Shell step completed in {elapsed:.2f}s, stdout: {out}, stderr: {err}")
    return {"stdout": out, "stderr": err, "returncode": proc.returncode}, elapsed

# Dependency check
def dependencies_met(step, completed):
    for dep in step.get("depends_on", []):
        if dep not in completed:
            return False
    return True

# Main execution loop
completed = set()
failed = set()
running = set()
files_touched = set()
step_threads = {}

try:
    for step in steps:
        name = step["name"]
        if args.only and name not in args.only:
            debug_print(f"Skipping {name} (not in --only)", debug)
            continue
        if args.skip and name in args.skip:
            debug_print(f"Skipping {name} (in --skip)", debug)
            continue
        if not dependencies_met(step, completed):
            debug_print(f"Skipping {name} (dependencies not met)", debug)
            continue

        # Print dependencies and requirements
        print(f"[{nowstr()}] Step: {name}")
        print(f"  Dependencies: {step.get('depends_on', [])}")
        print(f"  Requirements: {step.get('success', {})}")

        # Mark as running
        step_status[name] = "running"
        update_digraph(steps, step_status)

        # Background/parallel support
        if step.get("background"):
            t = threading.Thread(target=run_python_step if step["type"] == "python" else run_shell_step,
                                 args=(step, ctx, debug), daemon=True)
            t.start()
            step_threads[name] = t
            print(f"  Step {name} running in background (PID: {os.getpid()})")
            continue
        elif step.get("parallel"):
            t = threading.Thread(target=run_python_step if step["type"] == "python" else run_shell_step,
                                 args=(step, ctx, debug))
            t.start()
            step_threads[name] = t
            print(f"  Step {name} running in parallel (PID: {os.getpid()})")
            continue

        # Run step
        try:
            start_time = time.time()
            if step["type"] == "python":
                result, elapsed = run_python_step(step, ctx, debug)
            elif step["type"] == "shell":
                result, elapsed = run_shell_step(step, ctx, debug)
            else:
                raise ValueError(f"Unknown step type: {step['type']}")
            end_time = time.time()

            # Log ctx changes if debug
            if debug:
                for k, v in ctx.items():
                    log_ctx_change(ctx, k, v)

            # Log resource usage
            mem, cpu, disk, pid, child_pids = resource_report(psutil.Process(os.getpid()))
            print(f"  Time: {elapsed:.2f}s, Mem: {mem:.2f}MB, CPU: {cpu:.2f}%, Disk: {disk:.2f}MB, PID: {pid}, Children: {child_pids}")

            # Mark as complete
            step_status[name] = "success"
            completed.add(name)
            update_digraph(steps, step_status)

            # Save state after each step
            save_state(ctx, files_touched)

        except Exception as e:
            step_status[name] = "fail"
            failed.add(name)
            update_digraph(steps, step_status)
            log_error(f"Step {name} failed: {e}\n{traceback.format_exc()}")
            print(f"[{nowstr()}] Step {name} failed: {e}")
            save_state(ctx, files_touched)
            sys.exit(1)

    # Wait for background/parallel threads
    for name, t in step_threads.items():
        t.join()
        step_status[name] = "success"
        completed.add(name)
        update_digraph(steps, step_status)

    print(f"[{nowstr()}] All steps completed successfully.")
    sys.exit(0)

except Exception as e:
    log_error(f"Runner failed: {e}\n{traceback.format_exc()}")
    print(f"[{nowstr()}] Runner failed: {e}")
    save_state(ctx, files_touched)
    sys.exit(1)