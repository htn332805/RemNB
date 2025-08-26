"""
ctx_workflow_viz.py
-------------------
Generates a workflow/recipe graph visualization and saves as PNG/SVG.

Usage:
    run(ctx, {"recipe": "recipes/my_recipe.yaml", "out": "Data/recipe_graph.png"})

Description:
    - Uses graphviz to render the workflow DAG.
"""
import yaml
from graphviz import Digraph

def run(ctx, params):
    with open(params["recipe"]) as f:
        recipe = yaml.safe_load(f)
    steps = recipe.get("steps", [])
    dot = Digraph(comment="Workflow")
    for step in steps:
        dot.node(step["name"])
    for i in range(len(steps) - 1):
        dot.edge(steps[i]["name"], steps[i+1]["name"])
    out = params["out"]
    dot.render(out, format=out.split(".")[-1], cleanup=True)
    return {"graph": out}