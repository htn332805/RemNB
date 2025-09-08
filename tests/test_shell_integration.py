from orchestrator.runner_v2 import run_recipe_v2
def test_shell_integration():
    ctx = run_recipe_v2("orchestrator/recipes/02_basic_mixed.yaml", set(), set(), None)
    assert ctx.get("numbers.raw_v1")
