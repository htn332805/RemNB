import pytest
from orchestrator.runner_v2 import run_recipe_v2
def test_validation_fail():
    with pytest.raises(RuntimeError):
        run_recipe_v2("orchestrator/recipes/03_validation_errors.yaml", set(), set(), None)
