"""
chain_recipes.py
----------------
Chains two recipe YAML files or selected steps into a new recipe.

Usage:
    run(ctx, {"recipe1": "recipes/a.yaml", "recipe2": "recipes/b.yaml", "out": "recipes/ab.yaml"})

Description:
    - Merges steps from both recipes.
"""
import yaml

def run(ctx, params):
    with open(params["recipe1"]) as f1, open(params["recipe2"]) as f2:
        r1 = yaml.safe_load(f1)
        r2 = yaml.safe_load(f2)
    steps = r1.get("steps", []) + r2.get("steps", [])
    out_recipe = {"steps": steps}
    with open(params["out"], "w") as f:
        yaml.safe_dump(out_recipe, f)
    return {"chained": params["out"]}