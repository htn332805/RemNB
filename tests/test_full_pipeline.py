from orchestrator.runner_v2 import run_recipe_v2
def test_full_pipeline():
    ctx = run_recipe_v2("orchestrator/recipes/12_advanced_full.yaml", set(), set(), None)
    assert ctx.get("report.path_v1")
    assert "resource.audit_v1" in ctx.to_dict()
