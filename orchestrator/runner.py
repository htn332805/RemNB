"""Pipeline runner skeleton.

Executes a YAML recipe of steps (python or shell). Minimal placeholder.
"""
from __future__ import annotations
import argparse, json, subprocess, sys, importlib, inspect, os, yaml
from typing import Any, Dict, List
# Ensure project root is on sys.path so 'orchestrator' can be imported even
# when invoking 'python orchestrator/runner.py' without setting PYTHONPATH.
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
if PROJECT_ROOT not in sys.path:
    sys.path.insert(0, PROJECT_ROOT)

from orchestrator.context import Context  # type: ignore
from orchestrator.scriptlets.python.core.logging import get_logger  # type: ignore
from orchestrator.scriptlets.python.core.base import BaseScriptlet  # type: ignore

logger = get_logger(__name__)

SUCCESS_EXIT = 0
FAILED_EXIT = 1
DEPENDENCY_EXIT = 2

def load_recipe(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)

def run_shell_step(step: Dict[str, Any]) -> Dict[str, Any]:
    cmd = step["cmd"]
    args_env = {**os.environ}
    for k, v in (step.get("args") or {}).items():
        if isinstance(v, (str, int, float)):
            args_env[k] = str(v)
    logger.info(f"Running shell step {step['name']}: {cmd}")
    proc = subprocess.run(cmd, shell=True, capture_output=True, text=True, env=args_env)
    if proc.returncode != 0:
        logger.error(proc.stderr)
        return {"status": "error", "returncode": proc.returncode, "stderr": proc.stderr}
    try:
        return json.loads(proc.stdout.strip() or '{}')
    except json.JSONDecodeError:
        return {"status": "error", "reason": "non-json stdout"}

def run_python_step(ctx: Context, step: Dict[str, Any]) -> Dict[str, Any]:
    mod_path = step["module"]
    cls_name = step.get("class")
    params = step.get("params") or {}
    logger.info(f"Running python step {step['name']}: {mod_path}")
    mod = importlib.import_module(mod_path)
    if cls_name:
        cls = getattr(mod, cls_name)
        inst = cls()
        assert isinstance(inst, BaseScriptlet)
        return inst.run(ctx, params)
    # function style
    func = getattr(mod, step["function"])
    sig = inspect.signature(func)
    if len(sig.parameters) == 2:  # assume (ctx, params)
        return func(ctx, params)
    return func(params)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--recipe", required=False, default="orchestrator/recipes/example.yaml")
    args = parser.parse_args()
    recipe = load_recipe(args.recipe)
    ctx = Context()
    steps: List[Dict[str, Any]] = recipe.get("steps", [])
    for step in sorted(steps, key=lambda s: s.get("idx", 0)):
        if step["type"] == "shell":
            out = run_shell_step(step)
        else:
            out = run_python_step(ctx, step)
        logger.info(f"Step {step['name']} output: {out}")
        if out.get("status") not in ("ok", "success"):
            logger.error(f"Stopping due to failure in {step['name']}")
            sys.exit(FAILED_EXIT)
    logger.info("Pipeline complete")

if __name__ == "__main__":
    main()
