"""
ctx_step_dependency.py
---------------------
Analyzes and lists step dependencies in a recipe.

Usage:
    run(ctx, {"recipe": "recipes/my_recipe.yaml", "out": "step_deps"})

Description:
    - Stores a dict of step dependencies in ctx[out].
"""
import yaml

def run(ctx, params):
    with open(params["recipe"]) as f:
        recipe = yaml.safe_load(f)
    steps = recipe.get("steps", [])
    deps = {}
    for step in steps:
        name = step["name"]
        after = step.get("after", [])
        deps[name] = after
    ctx[params["out"]] = deps
    return {"dependencies": params["out"]}