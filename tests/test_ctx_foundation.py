from orchestrator.runner_v2 import run_recipe_v2
def test_foundation():
    ctx = run_recipe_v2("orchestrator/recipes/01_basic_number.yaml", set(), set(), None)
    assert ctx.get("numbers.raw_v1")
