import pytest
from orchestrator.runner_v2 import run_recipe_v2
def test_resilience_failure_injected():
    with pytest.raises(RuntimeError):
        run_recipe_v2("orchestrator/recipes/11_resilience_controls.yaml", set(), set(), None)
