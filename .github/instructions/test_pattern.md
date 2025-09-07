# Copilot Prompt: Create Tests for Scriptlet or Recipe (pytest) in Automation Orchestrator

## SYSTEM CONTEXT
Testing in this framework is done with pytest under `tests/`.  
There are two test types:
- **Unit**: Direct calls to your scriptlet class.
- **Integration**: Run a full recipe with `run_recipe` and assert expected context keys.

## TASK
Write a pytest function to:
- Validate that your scriptlet or recipe produces the correct output key(s)
- Validate correct error handling on invalid input

## PATTERN

**Unit Test Example (New Scriptlet):**
```python
import pytest
from orchestrator.scriptlets.python.steps.compute_median import ComputeMedian
from orchestrator.context import Context

def test_compute_median_valid(tmp_path):
data = tmp_path / "nums.csv"
with open(data, "w") as f:
f.write("\n".join(["1", "3", "5"]))
ctx = Context()
code = ComputeMedian().run(ctx, {"src": str(data)})
assert code == 0
assert ctx.get("numbers.median_v1")["median"] == 3

def test_compute_median_missing_file(tmp_path):
ctx = Context()
code = ComputeMedian().run(ctx, {"src": str(tmp_path / "missing.csv")})
assert code == 1
assert ctx.get("numbers.median_v1") is None
```


**Integration Test Example (Recipe):**

```python
from orchestrator.runner import run_recipe

def test_recipe_end_to_end():
ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
assert ctx.get("numbers.stats_v1")["count"] > 0
```


## EXAMPLES OF USAGE
- Place new tests under `tests/`.
- Use function naming: `test_<scriptlet|recipe>_<case>()`.
- Use pytest `tmp_path` for file creation tests.
- Assert `ctx.get("<expected-key>")` or error codes / context absence.

## INSTRUCTION
Use/modify these for your own scriptlet or recipe integration as appropriate.
