"""
recipe_merge.py
---------------
Merges multiple recipe YAML files or selected steps into a new recipe.
Merge multiple recipe YAML files or selected steps into a new recipe.

Usage:
    run(ctx, {"recipes": ["recipes/a.yaml", "recipes/b.yaml"], "out": "recipes/merged.yaml"})
    run(ctx, {"recipes": ["a.yaml", "b.yaml"], "steps": ["step1", "step2"], "out": "merged.yaml"})

Description:
    - Merges all steps, or only selected steps, from each recipe.
"""
import yaml

def run(ctx, params):
    all_steps = []
    for recipe_path in params["recipes"]:
        with open(recipe_path) as f:
            recipe = yaml.safe_load(f)
        steps = recipe.get("steps", [])
        if "steps" in params:
            steps = [s for s in steps if s.get("name") in params["steps"]]
        all_steps.extend(steps)
    out_recipe = {"steps": all_steps}
    with open(params["out"], "w") as f:
        yaml.safe_dump(out_recipe, f)
    return {"merged": params["out"], "step_count": len(all_steps)}