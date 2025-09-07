from orchestrator.runner_v2 import run_recipe_v2
def test_parallel_metrics():
    ctx = run_recipe_v2("orchestrator/recipes/06_parallel_fan_in.yaml", set(), set(), None)
    assert ctx.get("metrics.aggregate_v1")
