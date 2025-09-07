from orchestrator.runner import run_recipe


def test_compute_numbers_recipe():
    ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
    stats = ctx.get("numbers.stats_v1")
    assert stats["count"] == 5
    assert stats["mean"] == 3.0
    assert stats["min"] == 1
    assert stats["max"] == 5
