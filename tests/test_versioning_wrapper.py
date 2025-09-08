from orchestrator.runner_v2 import run_recipe_v2
def test_versioning_v2():
    ctx = run_recipe_v2("orchestrator/recipes/08_versioning_wrapper.yaml", set(), set(), None)
    assert ctx.get("metrics.summary_v1")
    assert ctx.get("metrics.summary_v2")
