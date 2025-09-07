# Copilot Prompt: Extend or Create a Recipe (YAML) for Automation Orchestrator

## SYSTEM CONTEXT
Orchestrator recipes (under `orchestrator/recipes/`) are YAML files declaring an ordered pipeline of scriptlets.  
Each step:
- Specifies a unique idx, name, type (python/shell), module, function, args, and (optionally) dependencies.
- Uses `success: ctx_has_keys` to assert produced context outputs.

## TASK
Show how to extend an existing recipe OR write a new one including:
- Step(s) for additional metrics or transformation(s)
- Optional dependency chaining with `depends_on`
- Correct namespacing, output key and order

## RECIPE PATTERN

```yaml
test_meta:
test_id: UNIQUE-ID
tester: YOUR-NAME
description: Explain purpose of this recipe
steps:

idx: 1
name: compute_numbers
type: python
module: orchestrator.scriptlets.python.steps.compute_numbers
function: ComputeNumbers
args:
src: orchestrator/Data/numbers.csv
success:
ctx_has_keys:
- numbers.stats_v1

idx: 2
name: compute_median
type: python
module: orchestrator.scriptlets.python.steps.compute_median
function: ComputeMedian
args:
src: orchestrator/Data/numbers.csv
depends_on:

compute_numbers
success:
ctx_has_keys:

numbers.median_v1
```

## QUICK REFERENCE
- Always increment idx per step.
- Use fully qualified module path.
- Place new step files under `orchestrator/scriptlets/python/steps/`.
- For parallel/fan-in, use independent metrics steps, then aggregate.
- Use `depends_on` for explicit chaining.

## EXAMPLES

Add a new range calculation after median:
```yaml
idx: 3
name: compute_range
type: python
module: orchestrator.scriptlets.python.steps.compute_range
function: ComputeRange
args:
src: orchestrator/Data/numbers.csv
depends_on:

compute_median
success:
ctx_has_keys:

numbers.range_v1
````

## INSTRUCTION
Copy these structures into a `.yaml` file under `orchestrator/recipes/`, and replace step logic as required for your new or extended pipeline.
