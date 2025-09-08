from orchestrator.runner_v2 import run_recipe_v2
def test_reporting():
    ctx = run_recipe_v2("orchestrator/recipes/10_reporting.yaml", set(), set(), None)
    assert ctx.get("report.path_v1")
