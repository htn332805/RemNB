from orchestrator.runner_v2 import run_recipe_v2
def test_dashboard_pipeline_stub():
    ctx = run_recipe_v2("orchestrator/recipes/09_dashboard.yaml", set(), set(), None)
    assert ctx.get("metrics.summary_v2")
