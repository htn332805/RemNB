from orchestrator.runner_v2 import run_recipe_v2
def test_artifacts_pipeline():
    ctx = run_recipe_v2("orchestrator/recipes/04_artifacts_pipeline.yaml", set(), set(), None)
    assert ctx.get("numbers.normalized_v1")
    assert ctx.get("numbers.stats_v1")
