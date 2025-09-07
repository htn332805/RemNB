"""Minimal recipe runner to execute python scriptlet steps defined in a YAML recipe."""
from __future__ import annotations
import argparse
import importlib
import json
from pathlib import Path
from typing import Any, Dict

import yaml

from orchestrator.context import Context


def _run_python_step(ctx: Context, step: Dict[str, Any]) -> None:
    module_name = step["module"]
    class_name = step["function"]
    mod = importlib.import_module(module_name)
    cls = getattr(mod, class_name)
    inst = cls()
    params = step.get("args", {})
    code = inst.run(ctx, params)
    if code != 0:
        raise RuntimeError(f"Step {step['name']} failed with exit code {code}")


def run_recipe(recipe_path: str) -> Context:
    with open(recipe_path, "r") as f:
        spec = yaml.safe_load(f)
    steps = spec.get("steps", [])
    ctx = Context()
    for step in steps:
        if step.get("type") == "python":
            _run_python_step(ctx, step)
        else:
            raise NotImplementedError(f"Unsupported step type: {step.get('type')}")
    return ctx


def main():  # pragma: no cover - CLI wrapper
    parser = argparse.ArgumentParser()
    parser.add_argument("--recipe", required=True, help="Path to recipe YAML")
    args = parser.parse_args()
    ctx = run_recipe(args.recipe)
    print(json.dumps({"status": "ok", "ctx_keys": list(ctx.to_dict().keys())}))


if __name__ == "__main__":  # pragma: no cover
    main()
    #python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
    #python -m orchestrator.runner --recipe orchestrator/recipes/example_numbers.yaml
