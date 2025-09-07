### AUTOGEN(rebase_workspace.sh)
"""
Sequential runner (baseline).
"""
import json, importlib, sys, pathlib, argparse
from orchestrator.context import Context
def load_recipe(path):
    import yaml
    return yaml.safe_load(open(path))
def _invoke_python_step(ctx, step):
    mod = importlib.import_module(step["module"])
    cls = getattr(mod, step["function"])
    inst = cls()
    params = step.get("args", {})
    return inst.run(ctx, params)
def run_recipe(recipe_path):
    rp = pathlib.Path(recipe_path)
    if not rp.is_file():
        raise FileNotFoundError(recipe_path)
    recipe = load_recipe(str(rp))
    ctx = Context()
    for step in recipe.get("steps", []):
        if step["type"] != "python":
            raise NotImplementedError("Only python steps supported in baseline")
        code = _invoke_python_step(ctx, step)
        if code != 0:
            raise RuntimeError(f"Step failed: {step[name]}")
    return ctx
if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--recipe", required=True)
    a = ap.parse_args()
    ctx = run_recipe(a.recipe)
    print(json.dumps({"status":"ok","ctx_keys":list(ctx.to_dict().keys())}))
