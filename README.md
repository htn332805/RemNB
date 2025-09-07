# Automation Framework Skeleton

Implements initial structure per automation guidelines. Includes:
- Context with history tracking
- BaseScriptlet abstraction
- Resource tracking decorator
- Sample Python and shell steps
- Example recipe

Run example:
```
python orchestrator/runner.py --recipe orchestrator/recipes/example.yaml
```
# Automation Framework User Guide

Comprehensive reference for the repository. Covers structure, concepts, execution, extension, and best practices.

---

## 1. Purpose and Philosophy

This repository provides a minimal but extensible automation orchestration framework with:
- A shared in‑memory contextual state (`Context`) for step-to-step communication
- Python and shell "scriptlets" (atomic steps) runnable standalone or via a pipeline runner
- YAML recipes describing ordered execution
- Logging + resource usage tracking
- Examples for extending with new steps, recipes, and (future) Dash apps

Design goals:
- Small, composable, single-responsibility building blocks
- JSON-only state sharing (portable, inspectable, testable)
- Deterministic, auditable pipelines
- Easy onboarding for new steps without altering core code

---

## 2. Directory Overview

```
.
├── README.md
├── user_guide.md                # (this file)
├── run_context_server.py
├── update_state.sh
├── test.sh
├── simulate_cdu_poll.sh         # (referenced in scripts; not shown above if omitted)
├── orchestrator/
│   ├── context.py
│   ├── runner.py
│   ├── recipes/
│   │   └── example.yaml
│   ├── scriptlets/
│   │   ├── shell/
│   │   │   └── sample.sh
│   │   └── python/
│   │       ├── core/
│   │       │   ├── base.py
│   │       │   ├── logging.py
│   │       │   └── resource.py
│   │       └── steps/
│   │           └── sample_step.py
│   ├── Data/        # (empty placeholder — persist artifacts here)
│   ├── Logs/        # runtime logs
│   ├── Assets/      # visual artifacts / graphs (future)
│   ├── templates/   # reporting templates (future)
│   └── tools/       # utility scripts (future)
└── tests/
    └── test_sample_step.py
```

---

## 3. Core Components (File-by-File)

| Component | File | Purpose |
|-----------|------|---------|
| Context | [orchestrator/context.py](orchestrator/context.py) | In-memory, thread-safe JSON state with history |
| Runner | [orchestrator/runner.py](orchestrator/runner.py) | Executes YAML recipe steps (Python class/function or shell) |
| Scriptlet Base | [orchestrator/scriptlets/python/core/base.py](orchestrator/scriptlets/python/core/base.py) | Abstract interface for Python scriptlets |
| Logging | [orchestrator/scriptlets/python/core/logging.py](orchestrator/scriptlets/python/core/logging.py) | Central logger factory |
| Resource Tracking | [orchestrator/scriptlets/python/core/resource.py](orchestrator/scriptlets/python/core/resource.py) | Decorator: CPU/memory/time metrics |
| Sample Python Step | [orchestrator/scriptlets/python/steps/sample_step.py](orchestrator/scriptlets/python/steps/sample_step.py) | Demo context writer |
| Sample Shell Step | [orchestrator/scriptlets/shell/sample.sh](orchestrator/scriptlets/shell/sample.sh) | Demo shell JSON emitter |
| Example Recipe | [orchestrator/recipes/example.yaml](orchestrator/recipes/example.yaml) | Demonstrates two steps in sequence |
| Test | [tests/test_sample_step.py](tests/test_sample_step.py) | Validates sample Python step |
| Context TCP Helper | [update_state.sh](update_state.sh) | CLI to interact with a (future) context server |
| Manual Scenario Script | [test.sh](test.sh) | Example chaining context operations |
| Long-running Context Server | [run_context_server.py](run_context_server.py) | Keeps background threads alive (placeholder) |

---

## 4. The Context Object

File: [orchestrator/context.py](orchestrator/context.py)

Features:
- Thread-safe via `RLock`
- JSON serializability enforced on `set`
- History per key with timestamp, author (`who`), old and new values
- Accessors: `set`, `get`, `get_history`, `last_modified`, `to_dict`

Example (Python):
```python
from orchestrator.context import Context
ctx = Context()
ctx.set("metrics.mean_v1", 1.23, who="compute_metrics")
value = ctx.get("metrics.mean_v1")
history = ctx.get_history("metrics.mean_v1")
print(value)     # 1.23
print(history)   # [{'ts':..., 'who':'compute_metrics', 'old':None, 'new':1.23}]
```

Key invariants:
- Only JSON-serializable values accepted (primitives, list, dict)
- History never pruned (potential extension: retention policies)
- No deep mutation tracking (re-set full value after modifying structures externally)

---

## 5. Runner Workflow

File: [orchestrator/runner.py](orchestrator/runner.py)

Execution Flow:
1. Load YAML recipe via `load_recipe`
2. Instantiate shared `Context`
3. Sort steps by `idx`
4. For each step:
   - Type `shell` → run in subprocess, expect JSON stdout
   - Type `python` → dynamic import:
     - If `class` specified: instantiate & call `run(ctx, params)`
     - Else call function (variant supporting `(ctx, params)` or `(params)` signatures)
5. Abort on first non-success (`status` not `ok` / `success`)

Recipe Example: [orchestrator/recipes/example.yaml](orchestrator/recipes/example.yaml)
```yaml
steps:
  - idx: 1
    name: sample_python
    type: python
    module: orchestrator.scriptlets.python.steps.sample_step
    class: SampleStep
    params:
      message: "Hello"
  - idx: 2
    name: sample_shell
    type: shell
    cmd: orchestrator/scriptlets/shell/sample.sh
    args:
      ECHO_TEXT: "World"
```

Run:
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example.yaml
```

Output (logs):
```
INFO | Running python step sample_python ...
INFO | Step sample_python output: {'status': 'ok', 'recorded_key': 'sample.message_v1'}
INFO | Running shell step sample_shell ...
INFO | Step sample_shell output: {'status': 'ok','echo':'World'}
INFO | Pipeline complete
```

---

## 6. Python Scriptlets

Pattern:
- File under `orchestrator/scriptlets/python/steps/`
- Inherit from [`BaseScriptlet`](orchestrator/scriptlets/python/core/base.py)
- Implement `validate(ctx, params)` (optional) and `run(ctx, params)`
- Use logger from [`logging.py`](orchestrator/scriptlets/python/core/logging.py)
- Optionally decorate `run` with [`track_resources`](orchestrator/scriptlets/python/core/resource.py)

Example: [`SampleStep`](orchestrator/scriptlets/python/steps/sample_step.py)
```python
class SampleStep(BaseScriptlet):
    def validate(self, ctx, params):
        assert "message" in params

    @track_resources
    def run(self, ctx, params):
        msg = str(params["message"])
        key = "sample.message_v1"
        ctx.set(key, msg, who="sample_step")
        return {"status": "ok", "recorded_key": key}
```

Standalone CLI:
```bash
python orchestrator/scriptlets/python/steps/sample_step.py --params '{"message":"CLI hello"}'
```

Expected stdout:
```json
{"status": "ok", "recorded_key": "sample.message_v1"}
```

---

## 7. Shell Scriptlets

Pattern:
- File under `orchestrator/scriptlets/shell/`
- Executable (`chmod +x`)
- Emit JSON to stdout
- Accept environment variables or flags

Example: [`sample.sh`](orchestrator/scriptlets/shell/sample.sh)
```bash
ECHO_TEXT="Hello" orchestrator/scriptlets/shell/sample.sh
# {"status":"ok","echo":"Hello"}
```

Integration in recipe: Provide `args` → sets environment variables.

---

## 8. Adding a New Python Scriptlet (Hands-on Guide)

Goal: Create a step that writes a computed length of a string.

File suggestion: `orchestrator/scriptlets/python/steps/compute_message_length.py`
```python
"""
NAME: compute_message_length.py
DESCRIPTION: Compute length of a provided message and store in ctx.
USAGE:
  python orchestrator/scriptlets/python/steps/compute_message_length.py --params '{"message":"Hello"}'
RETURNS:
  {"status":"ok","key":"metrics.message_length_v1","length":5}
LIMITATIONS:
  - message must be a string
"""
import json, argparse
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources
logger = get_logger(__name__)

class ComputeMessageLength(BaseScriptlet):
    def validate(self, ctx, params):
        assert isinstance(params, dict), "params must be dict"
        assert "message" in params, "message required"
        assert isinstance(params["message"], str), "message must be string"

    @track_resources
    def run(self, ctx, params):
        self.validate(ctx, params)
        length = len(params["message"])
        key = "metrics.message_length_v1"
        ctx.set(key, length, who="compute_message_length")
        return {"status":"ok","key":key,"length":length}

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    args = ap.parse_args()
    params = json.loads(args.params)
    ctx = Context()
    out = ComputeMessageLength().run(ctx, params)
    print(json.dumps(out))
```

Add to a recipe step:
```yaml
- idx: 3
  name: compute_msg_len
  type: python
  module: orchestrator.scriptlets.python.steps.compute_message_length
  class: ComputeMessageLength
  params:
    message: "Framework"
```

---

## 9. Adding a New Shell Scriptlet (Hands-on Guide)

Goal: Count lines in a file.

File: `orchestrator/scriptlets/shell/count-lines.sh`
```bash
#!/usr/bin/env bash
# NAME: count-lines.sh
# DESC: Count lines in a text file and emit JSON.
# USAGE: INPUT=orchestrator/Data/file.txt orchestrator/scriptlets/shell/count-lines.sh
# RETURNS: {"status":"ok","lines":<int>}
set -euo pipefail
INPUT="${INPUT:?missing INPUT}"
if [[ ! -f "$INPUT" ]]; then
  echo "{\"status\":\"error\",\"reason\":\"missing file\"}" ; exit 1
fi
LINES=$(wc -l < "$INPUT" | tr -d ' ')
echo "{\"status\":\"ok\",\"lines\":$LINES}"
```

Recipe addition:
```yaml
- idx: 4
  name: count_lines
  type: shell
  cmd: orchestrator/scriptlets/shell/count-lines.sh
  args:
    INPUT: "orchestrator/Data/file.txt"
```

---

## 10. Writing a New Recipe

Create: `orchestrator/recipes/demo_metrics.yaml`
```yaml
# Purpose: Demonstrates chaining sample + compute length
test_meta:
  test_id: DEMO1
  tester: user
  description: Demo pipeline with two python steps
steps:
  - idx: 1
    name: sample_python
    type: python
    module: orchestrator.scriptlets.python.steps.sample_step
    class: SampleStep
    params:
      message: "Hello World"
  - idx: 2
    name: compute_msg_len
    type: python
    module: orchestrator.scriptlets.python.steps.compute_message_length
    class: ComputeMessageLength
    params:
      message: "Hello World"
```

Run:
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/demo_metrics.yaml
```

---

## 11. Testing

Existing test: [tests/test_sample_step.py](tests/test_sample_step.py)
```python
def test_sample_step():
    ctx = Context()
    out = SampleStep().run(ctx, {"message": "Hello"})
    assert out["status"] == "ok"
    assert ctx.get("sample.message_v1") == "Hello"
```

Add new test (example stub):
```python
def test_compute_message_length():
    from orchestrator.scriptlets.python.steps.compute_message_length import ComputeMessageLength
    ctx = Context()
    out = ComputeMessageLength().run(ctx, {"message":"XYZ"})
    assert out["length"] == 3
    assert ctx.get("metrics.message_length_v1") == 3
```

Run all tests:
```bash
pytest -q
```

---

## 12. Logging & Resource Metrics

Decorator: [`track_resources`](orchestrator/scriptlets/python/core/resource.py)
Emits JSON log entry like:
```json
{
  "metric":"resource_usage",
  "function":"run",
  "seconds":0.000321,
  "rss_delta":4096,
  "heap_current":12345,
  "heap_peak":23456
}
```

Logger standard format (see [`logging.py`](orchestrator/scriptlets/python/core/logging.py)):
```
YYYY-MM-DDTHH:MM:SS | LEVEL | module.name | message
```

Best practices:
- Avoid logging large raw data
- Use structured JSON when aggregating metrics
- Ensure stdout strictly reserved for final result JSON when running scriptlets directly

---

## 13. Context Interaction via `update_state.sh`

File: [update_state.sh](update_state.sh) — interacts with a TCP JSON protocol (server not fully shown here).

Supported operations:
- `full`
- `get <path>`
- `set <path> <json_value>`
- `append <path> <json_value>`
- `delete <path>`
- `pop <path> [index]`

Example:
```bash
./update_state.sh set data.user '{"name":"Alice"}'
./update_state.sh get data.user
./update_state.sh append data.numbers '1'
./update_state.sh append data.numbers '2'
./update_state.sh get data.numbers
./update_state.sh delete data.user
./update_state.sh full
```

See [test.sh](test.sh) for integrated usage pattern.

---

## 14. Patterns & Anti-Patterns

Recommended:
- Version keys: `metrics.summary_v1`
- Split transformations into multiple steps for clarity
- Validate params early with clear assertion messages
- Return minimal JSON payload (status + key refs)

Avoid:
- Storing raw large tables directly in context
- Printing logs to stdout
- Hidden side effects (undocumented file writes)
- Multi-purpose "god" scriptlets

---

## 15. Extensibility Roadmap (Suggested)

| Feature | Approach |
|---------|----------|
| Step discovery CLI | Scan `steps` folder and list classes subtyping `BaseScriptlet` |
| Success criteria expansion | Add `success` processing in runner (ctx_has_keys, files_exist) |
| Parallel execution | Introduce dependency graph + thread/process pools |
| Persistent context | Serialize snapshots to `orchestrator/Data/context_<ts>.json` |
| Dash monitoring | Add app under `scriptlets/python/apps/` reading `Data/` or context API |

---

## 16. Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `Value ... not JSON-serializable` | Non-JSON type passed to `ctx.set` | Convert to primitive or persist externally |
| `ModuleNotFoundError` for new step | PYTHONPATH missing root | Ensure runner adds root (already handled) |
| Shell step returns `status:error` | Non-zero exit code or invalid JSON stdout | Validate script and sanitized output |
| Missing key in downstream step | Upstream step failed or key mismatch | Check logs, confirm key name consistency |
| Empty stdout from scriptlet | Did not `print(json.dumps(...))` | Add a final stdout emission |

---

## 17. FAQ

Q: How do I inspect all keys after a run?  
A: Modify runner or inside a diagnostic scriptlet: `print(ctx.to_dict())`.

Q: How do I version a breaking change to a step?  
A: Copy file → append `_v2` to filename and class → update recipe.

Q: Can steps share large data frames?  
A: Persist to CSV/Parquet in `orchestrator/Data/` and write the path + schema to context.

Q: Where do logs go per step?  
A: Currently to stderr; future enhancement could route to `orchestrator/Logs/` with file handlers.

---

## 18. Glossary

| Term | Definition |
|------|------------|
| Scriptlet | Atomic executable unit (Python or shell) forming part of a pipeline |
| Context (`ctx`) | Shared in-memory state with versioned history |
| Recipe | YAML file describing ordered steps |
| Runner | Engine that loads recipe and executes steps |
| Resource Tracking | Timing/memory instrumentation wrapping `run` |

---

## 19. Quick Reference Cheat Sheet

| Task | Command |
|------|---------|
| Run sample pipeline | `python orchestrator/runner.py --recipe orchestrator/recipes/example.yaml` |
| Run Python scriptlet | `python orchestrator/scriptlets/python/steps/sample_step.py --params '{"message":"Hi"}'` |
| Run shell scriptlet | `ECHO_TEXT=Hi orchestrator/scriptlets/shell/sample.sh` |
| Add context key (server) | `./update_state.sh set data.value '42'` |
| Get context key | `./update_state.sh get data.value` |
| Execute tests | `pytest -q` |

---

## 20. Minimal Template Snippets

Python step template:
```python
"""
NAME: <file>.py
DESCRIPTION: <single responsibility>
USAGE: python <path>/<file>.py --params '{"key":"value"}'
RETURNS: {"status":"ok",...}
LIMITATIONS: ...
"""
class <CamelCase>(BaseScriptlet):
    def validate(self, ctx, params): ...
    @track_resources
    def run(self, ctx, params): ...
```

Shell step template:
```bash
#!/usr/bin/env bash
# NAME: <name>.sh
# DESC: <single responsibility>
# USAGE: VAR1=... VAR2=... ./<name>.sh
set -euo pipefail
# logic...
echo '{"status":"ok"}'
```

Recipe template:
```yaml
test_meta:
  test_id: ID123
  tester: user
  description: Demo
steps:
  - idx: 1
    name: step_one
    type: python
    module: path.to.step
    class: StepClass
    params: { key: value }
```

---

## 21. Future Enhancements (Suggestions)

- Add `success` validation (ctx_has_keys / files_exist) inside runner
- Per-step log file handling
- Automatic DAG visualization into `Assets/`
- CLI: `python orchestrator/runner.py --list-steps`
- Context persistence for cross-process shell integration
- Dash app reading metrics snapshot

---

## 22. Conventions Summary

| Area | Rule |
|------|------|
| Keys | Namespaced + version suffix (`domain.metric_v1`) |
| Outputs | JSON only to stdout |
| Logging | Use central logger; no prints except final JSON |
| Step scope | Single responsibility |
| Backward compatibility | New versions instead of rewrites |
| Serialization | Strict JSON round-trip required |

---

## 23. Validation Checklist Before Adding a Step

- [ ] File under correct folder
- [ ] Class inherits `BaseScriptlet`
- [ ] Docstring includes NAME/DESCRIPTION/USAGE/RETURNS/LIMITATIONS
- [ ] `validate` method asserts required params
- [ ] All `ctx.set` calls use `who=<step_name>`
- [ ] No non-serializable objects passed to context
- [ ] Final return includes `"status":"ok"` on success
- [ ] CLI path tested standalone
- [ ] (Optional) Unit test added

---

## 24. Attribution

Seed components:
- Context / runner / sample steps authored per repository scaffold.
- This guide auto-generated to accelerate onboarding and extension.

---

## 25. Quick Start (5 Commands)

```bash
# 1. Run sample pipeline
python orchestrator/runner.py --recipe orchestrator/recipes/example.yaml

# 2. Inspect new context key (after python step)
grep -i sample.message_v1 orchestrator/Logs/* 2>/dev/null || true

# 3. Run sample python scriptlet alone
python orchestrator/scriptlets/python/steps/sample_step.py --params '{"message":"Solo"}'

# 4. Create & run a new step (see section 8)
python orchestrator/scriptlets/python/steps/compute_message_length.py --params '{"message":"Test"}'

# 5. Run tests
pytest -q
```

---

End