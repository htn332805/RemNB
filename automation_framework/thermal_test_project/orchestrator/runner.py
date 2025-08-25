#!/usr/bin/env python3
"""
Runner with in-memory shared context (Option A).

Changes in this version:
- Replaces datetime.utcnow() with timezone-aware UTC helper now_utc_z().
- Keeps shared CTX for in-memory data passing between Python scriptlets.
- Shell steps can receive CTX values via environment (shell_env_keys) or stdin (stdin_from_ctx)
  and can write results back into CTX via stdout_to_ctx without touching disk.
"""

import os
import sys
import json
import time
import pickle
import importlib
import subprocess
from datetime import datetime, timezone
from typing import Any, Dict, List

import yaml

from context import Context, expose_for_shell  # shared in-memory context helpers

# -----------------------------------------------------------------------------
# Time utilities (fixes deprecation warnings by using timezone-aware UTC)
# -----------------------------------------------------------------------------
def now_utc_z() -> str:
    """
    Return current UTC time as ISO 8601 string with 'Z' suffix, e.g., 2025-08-23T23:45:26.019285Z
    """
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

# -----------------------------------------------------------------------------
# Global in-memory context and state
# -----------------------------------------------------------------------------
CTX = Context()  # shared, in-memory only (Option A)

BASE = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE, "Data")
LOGS_DIR = os.path.join(BASE, "Logs")
ASSETS_DIR = os.path.join(BASE, "Assets")

os.makedirs(DATA_DIR, exist_ok=True)
os.makedirs(LOGS_DIR, exist_ok=True)
os.makedirs(ASSETS_DIR, exist_ok=True)

STORE_ON_RAM_PATH = os.path.join(BASE, "store_on_ram.txt")
PICKLE_STATE_PATH = os.path.join(BASE, "state.pkl")  # used only for GLOBAL_STATE when RAM mode is enabled

GLOBAL_STATE: Dict[str, Any] = {
    "steps": {},    # step_name -> {"status": "...", "stdout", "stderr", "started_at", "ended_at"}
    "meta": {},
    "errors": [],
    "log": []
}

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------
def load_recipe(path: str) -> Dict[str, Any]:
    with open(path, "r") as f:
        return yaml.safe_load(f)

def read_control_file():
    if not os.path.exists(STORE_ON_RAM_PATH):
        return {"ram": False, "debug": False}
    with open(STORE_ON_RAM_PATH, "r") as f:
        txt = f.read().strip().lower()
    return {"ram": True, "debug": ("debug" in txt)}

def persist_state(ctrl):
    """
    Persist GLOBAL_STATE only when RAM mode is enabled.
    CTX is intentionally not persisted in Option A.
    """
    if not ctrl["ram"]:
        return
    with open(PICKLE_STATE_PATH, "wb") as pf:
        pickle.dump(GLOBAL_STATE, pf, protocol=pickle.HIGHEST_PROTOCOL)

def log_event(msg: str):
    rec = f"[{now_utc_z()}] {msg}"
    print(rec)
    GLOBAL_STATE["log"].append(rec)
    ctrl = read_control_file()
    if not ctrl["ram"]:
        with open(os.path.join(LOGS_DIR, "orchestrator.log"), "a", encoding="utf-8") as lf:
            lf.write(rec + "\n")

def files_exist(paths: List[str]) -> bool:
    # Utility for success checks: verify all files exist
    def _exists(p):
        ap = os.path.join(BASE, p) if not os.path.isabs(p) else p
        return os.path.exists(ap)
    return all(_exists(p) for p in paths)

# -----------------------------------------------------------------------------
# Step executors
# -----------------------------------------------------------------------------
def run_shell(step: Dict[str, Any]) -> Dict[str, Any]:
    """
    Execute a shell step with optional env injection and stdin/stdout ctx bridging.

    Optional step fields:
    - shell_env_keys: ["test_id", "threshold"] -> exposes CTX_TEST_ID, CTX_THRESHOLD
    - stdin_from_ctx: "some_key" -> json.dumps(CTX["some_key"]) is piped to stdin
    - stdout_to_ctx: "dest_key" -> stdout is decoded and stored into CTX["dest_key"]
    - stdout_to: "path" -> also write stdout to a file on disk if desired
    """
    cmd = [step["cmd"]] + step.get("args", [])
    stdout_to = step.get("stdout_to")

    # Prepare environment with selected CTX keys
    env = os.environ.copy()
    for k, v in expose_for_shell(CTX, step.get("shell_env_keys", [])).items():
        env[k] = v

    # Prepare stdin payload if requested
    stdin_data = None
    stdin_key = step.get("stdin_from_ctx")
    if stdin_key:
        if stdin_key not in CTX:
            raise RuntimeError(f"stdin_from_ctx requested key '{stdin_key}' not found in CTX")
        try:
            stdin_data = json.dumps(CTX[stdin_key]).encode("utf-8")
        except Exception as e:
            raise RuntimeError(f"Failed to JSON-encode CTX['{stdin_key}']: {e}")

    log_event(f"Running shell: {' '.join(cmd)}")

    proc = subprocess.Popen(
        cmd,
        cwd=BASE,
        env=env,
        stdin=(subprocess.PIPE if stdin_data is not None else None),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=False  # capture raw bytes; decode explicitly
    )
    out, err = proc.communicate(input=stdin_data)
    rc = proc.returncode

    # Handle stdout destinations
    if stdout_to:
        out_path = os.path.join(BASE, stdout_to) if not os.path.isabs(stdout_to) else stdout_to
        with open(out_path, "wb") as f:
            f.write(out or b"")

    # Store stdout back into CTX if requested
    if step.get("stdout_to_ctx"):
        CTX[step["stdout_to_ctx"]] = (out or b"").decode("utf-8", errors="replace").strip()

    return {
        "returncode": rc,
        "stdout": (out or b"").decode("utf-8", errors="replace"),
        "stderr": (err or b"").decode("utf-8", errors="replace")
    }

def run_python(step: Dict[str, Any]) -> Dict[str, Any]:
    """
    Import the module and function, then call run(ctx, params).
    Backward compatibility: if the function doesn't accept ctx, call run(params).
    """
    mod_name = step["module"]
    func_name = step["function"]
    params = step.get("params", {})
    log_event(f"Running python: {mod_name}.{func_name}({params})")

    mod = importlib.import_module(mod_name)
    func = getattr(mod, func_name)

    try:
        result = func(CTX, params)  # preferred signature
    except TypeError:
        result = func(params)       # legacy support

    return {"returncode": 0, "stdout": json.dumps(result, indent=2), "stderr": ""}

def check_success(step: Dict[str, Any]) -> bool:
    """
    Check success criteria declared in YAML:
    - files_exist: ["path1", "path2"]
    - ctx_has_keys: ["key1", "key2"]
    """
    succ = step.get("success", {})
    ok = True
    if "files_exist" in succ:
        ok = ok and files_exist(succ["files_exist"])
    if "ctx_has_keys" in succ:
        for k in succ["ctx_has_keys"]:
            if k not in CTX:
                ok = False
                break
    return ok

def render_graph(steps: List[Dict[str, Any]]):
    """
    Render a colored digraph to Assets/graph.png.
    Node colors:
      pending=white, running=yellow, success=green, failed=red
    """
    try:
        import graphviz
    except ImportError:
        log_event("graphviz not installed; skipping digraph render")
        return

    dot = graphviz.Digraph(format="png")
    status_color = {"pending": "white", "running": "yellow", "success": "green", "failed": "red"}

    for s in steps:
        nm = s["name"]
        idx = s["idx"]
        st = GLOBAL_STATE["steps"].get(nm, {}).get("status", "pending")
        color = status_color.get(st, "white")
        label = f"{idx}: {nm}"
        dot.node(nm, label=label, style="filled", fillcolor=color)

    for s in steps:
        for dep in s.get("depends_on", []) or []:
            dot.edge(dep, s["name"])

    out_path = os.path.join(ASSETS_DIR, "graph")
    dot.render(out_path, cleanup=True)
    log_event(f"Digraph updated at {out_path}.png")

def ensure_log_files():
    open(os.path.join(LOGS_DIR, "orchestrator.log"), "a").close()

# -----------------------------------------------------------------------------
# Main driver
# -----------------------------------------------------------------------------
def main(recipe_path: str, only: List[str] = None, skip: List[str] = None, resume_from: str = None):
    ensure_log_files()
    ctrl = read_control_file()
    recipe = load_recipe(recipe_path)

    # Seed CTX and GLOBAL_STATE with test metadata
    CTX["meta"] = recipe.get("test_meta", {})
    GLOBAL_STATE["meta"] = recipe.get("test_meta", {})

    steps = sorted(recipe["steps"], key=lambda x: x["idx"])
    name_to_step = {s["name"]: s for s in steps}

    # Initialize step statuses
    for s in steps:
        GLOBAL_STATE["steps"].setdefault(s["name"], {"status": "pending", "artifacts": []})

    # Apply CLI filters
    if only:
        steps = [s for s in steps if s["name"] in only]
    if skip:
        steps = [s for s in steps if s["name"] not in skip]
    if resume_from:
        passed = False
        new_steps = []
        for s in steps:
            if s["name"] == resume_from:
                passed = True
            if passed:
                new_steps.append(s)
        steps = new_steps

    render_graph(steps)

    # Execute steps sequentially
    for s in steps:
        name = s["name"]
        # Dependency check
        for d in s.get("depends_on", []) or []:
            dstat = GLOBAL_STATE["steps"].get(d, {}).get("status", "pending")
            if dstat != "success":
                msg = f"Dependency {d} not successful for step {name}"
                log_event(msg)
                GLOBAL_STATE["errors"].append({"step": name, "error": msg})
                GLOBAL_STATE["steps"][name]["status"] = "failed"
                persist_state(ctrl)
                render_graph(steps)
                sys.exit(2)

        # Mark running and render
        GLOBAL_STATE["steps"][name]["status"] = "running"
        GLOBAL_STATE["steps"][name]["started_at"] = now_utc_z()
        render_graph(steps)

        try:
            # Execute
            if s["type"] == "shell":
                res = run_shell(s)
            elif s["type"] == "python":
                res = run_python(s)
            else:
                raise RuntimeError(f"Unknown step type: {s['type']}")

            # Write per-step log (stdout/stderr)
            step_log_path = os.path.join(LOGS_DIR, f"{name}.log")
            with open(step_log_path, "a", encoding="utf-8") as lf:
                lf.write(f"=== {now_utc_z()} ===\n")
                if res.get("stdout"):
                    lf.write(res["stdout"])
                if res.get("stderr"):
                    lf.write("\n[stderr]\n" + res["stderr"])

            # Evaluate success
            ok = (res.get("returncode", 1) == 0) and check_success(s)
            if not ok:
                GLOBAL_STATE["steps"][name]["status"] = "failed"
                GLOBAL_STATE["steps"][name]["ended_at"] = now_utc_z()
                GLOBAL_STATE["steps"][name]["stdout"] = res.get("stdout", "")
                GLOBAL_STATE["steps"][name]["stderr"] = res.get("stderr", "")
                msg = f"Step {name} failed"
                log_event(msg)
                GLOBAL_STATE["errors"].append({"step": name, "error": msg})
                persist_state(ctrl)
                render_graph(steps)
                sys.exit(1)

            # Success
            GLOBAL_STATE["steps"][name]["status"] = "success"
            GLOBAL_STATE["steps"][name]["ended_at"] = now_utc_z()
            GLOBAL_STATE["steps"][name]["stdout"] = res.get("stdout", "")
            GLOBAL_STATE["steps"][name]["stderr"] = res.get("stderr", "")
            log_event(f"Step {name} succeeded")

        except Exception as e:
            GLOBAL_STATE["steps"][name]["status"] = "failed"
            GLOBAL_STATE["steps"][name]["ended_at"] = now_utc_z()
            GLOBAL_STATE["errors"].append({"step": name, "error": str(e)})
            log_event(f"Exception in step {name}: {e}")
            persist_state(ctrl)
            render_graph(steps)
            sys.exit(1)

        persist_state(ctrl)
        render_graph(steps)

    log_event("Pipeline completed successfully.")
    persist_state(ctrl)

if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("--recipe", "-r", default=os.path.join(BASE, "recipes", "default_recipe.yaml"))
    ap.add_argument("--only", nargs="*", help="Only run these steps by name")
    ap.add_argument("--skip", nargs="*", help="Skip these steps by name")
    ap.add_argument("--resume-from", dest="resume_from", help="Resume starting from this step name")
    args = ap.parse_args()
    main(args.recipe, args.only, args.skip, args.resume_from)
