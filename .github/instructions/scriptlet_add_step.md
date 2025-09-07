# Copilot Prompt: Add a New Scriptlet Step for Automation Orchestrator

## SYSTEM CONTEXT
You are contributing to the Automation Orchestrator framework.  
All computational steps are atomic "scriptlets" (one class per file), using only JSON-safe outputs via a shared Context (`orchestrator/context.py`). Each scriptlet must:
- Extend `BaseScriptlet` from `orchestrator/scriptlets/python/core/base.py`
- Print exactly one JSON line to stdout, indicating success or failure.
- Set outputs in context with a namespaced and versioned key, with `who=<scriptlet_name>`.
- Log human-readable info to stderr (use logger factory).

Refer to workspace directory:
- Scriptlets: `orchestrator/scriptlets/python/steps/`

## TASK
Generate a new scriptlet, with class (CamelCase), that:
- Reads from a file or context key given as a JSON parameter
- Performs a defined transformation/calculation on the input
- Writes a JSON-serializable value to the context using `who`
- Emits a correctly formatted JSON result on stdout

## PATTERN
1. Validate parameters and input file/context.
2. Use `@track_resources` for `run` when applicable.
3. Use `get_logger` for all logging (stderr).
4. Add exception handling to always print error JSON on failure.

## TEMPLATE

```python   
import json, sys, argparse
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources
from orchestrator.context import Context
import pathlib

logger = get_logger(name)

class ExampleStep(BaseScriptlet):
def validate(self, ctx, params):
if "src" not in params:
raise ValueError("missing src")
if not pathlib.Path(params["src"]).is_file():
raise ValueError("file not found: {}".format(params["src"]))

@track_resources
def run(self, ctx, params):
    try:
        self.validate(ctx, params)
        # Example logic
        data = [float(line.strip()) for line in open(params["src"])]
        result = {"sum": sum(data), "count": len(data)}
        ctx.set("numbers.sum_v1", result, who="example_step")
        print(json.dumps({"status": "ok", "outputs": ["numbers.sum_v1"]}))
        return 0
    except Exception as e:
        print(json.dumps({
            "status": "error",
            "reason": str(e),
            "exit_code": 1,
            "step": "example_step"
        }))
        return 1

if name == "main":
ap = argparse.ArgumentParser()
ap.add_argument("--params", required=True)
p = json.loads(ap.parse_args().params)
sys.exit(ExampleStep().run(Context(), p))
```

## EXAMPLES
- Add `compute_range.py` (producing `metrics.range_v1`) or `compute_std.py` (producing `metrics.std_v1`).
- Output JSON contract: `{"status":"ok","outputs":["<key>"]}` or structured error as above.

## INSTRUCTION
Replicate this pattern for your operation, update class/key names, and change logic as required.
