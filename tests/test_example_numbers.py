### AUTOGEN(rebase_workspace.sh)
import pytest
from orchestrator.runner import run_recipe
def test_example_numbers_recipe():
    ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
    stats = ctx.get("numbers.stats_v1")
    assert stats and "count" in stats
