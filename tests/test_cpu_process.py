from orchestrator.runner_v2 import run_recipe_v2
def test_cpu_process():
    ctx = run_recipe_v2("orchestrator/recipes/07_cpu_process.yaml", set(), set(), None)
    assert ctx.get("numbers.prime_factors_v1")
