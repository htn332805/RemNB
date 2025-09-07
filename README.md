# Automation Orchestrator Framework (Workspace Documentation)

Comprehensive, example‑rich documentation for this workspace (excluding `Purdue/`).  
Focus: reproducible test automation via **recipes → runner → scriptlets → context → tests**.

---

## 1. High‑Level Overview

This framework loads a YAML *recipe*, executes ordered *scriptlet* steps (Python now, shell future), shares state through a JSON‑safe [`orchestrator.context`](orchestrator/context.py) object, and provides test coverage via `pytest`.

Core components:
- [`orchestrator/context.py`](orchestrator/context.py) – Context store with history & JSON safety.
- [`orchestrator/runner.py`](orchestrator/runner.py) – Sequential recipe executor (extensible).
- [`orchestrator/scriptlets/python/core/base.py`](orchestrator/scriptlets/python/core/base.py) – `BaseScriptlet` contract.
- [`orchestrator/scriptlets/python/core/logging_util.py`](orchestrator/scriptlets/python/core/logging_util.py) – Logger factory.
- [`orchestrator/scriptlets/python/core/resource.py`](orchestrator/scriptlets/python/core/resource.py) – `@track_resources` timing decorator.
- [`orchestrator/scriptlets/python/steps/compute_numbers.py`](orchestrator/scriptlets/python/steps/compute_numbers.py) – Reference step.
- [`orchestrator/recipes/example_numbers.yaml`](orchestrator/recipes/example_numbers.yaml) – Example recipe.
- [`tests/test_example_numbers.py`](tests/test_example_numbers.py) – Integration test.

---

## 2. Directory Map (Concise)

```
orchestrator/
  context.py
  runner.py
  recipes/
    example_numbers.yaml
  scriptlets/
    python/
      core/
        base.py
        logging_util.py
        resource.py
      steps/
        compute_numbers.py
tests/
  conftest.py
  test_example_numbers.py
```

---

## 3. Installation

```bash
python3 -m venv .venv
. .venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
```

---

## 4. Quick Starts

### 4.1 Run Full Recipe (CLI)
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```
Sample output:
```json
{"status":"ok","ctx_keys":["numbers.stats_v1"]}
```

### 4.2 Programmatic Use
```python
from orchestrator.runner import run_recipe
ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
print(ctx.get("numbers.stats_v1"))
```

### 4.3 Run Single Scriptlet Standalone
```bash
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
```

### 4.4 (Planned) Filtered Execution
Conceptual flags (future):
```bash
python orchestrator/runner.py --recipe r.yaml --only compute_numbers
python orchestrator/runner.py --recipe r.yaml --skip preprocess_csv
python orchestrator/runner.py --recipe r.yaml --resume-from compute_numbers
```

---

## 5. The Context Object

[`orchestrator.context`](orchestrator/context.py) guarantees:
- JSON‑serializable values only (primitives, lists, dicts).
- Namespaced dotted keys to avoid collisions (`numbers.stats_v1`).
- Traceability via `ctx.set(key, value, who="scriptlet_name")`.

Do NOT store: open handles, DataFrames, NumPy arrays (convert first), datetimes (serialize ISO string).

---

## 6. Scriptlet Anatomy

All Python scriptlets:
1 file → 1 class (CamelCase) → extends `BaseScriptlet` (see [`orchestrator/scriptlets/python/core/base.py`](orchestrator/scriptlets/python/core/base.py)).
Implements:
- `validate(ctx, params)` – fast failure.
- `run(ctx, params)` – must print exactly one JSON line to stdout.

Reference: [`orchestrator/scriptlets/python/steps/compute_numbers.py`](orchestrator/scriptlets/python/steps/compute_numbers.py)

Success JSON envelope:
```json
{"status":"ok","outputs":["numbers.stats_v1"]}
```
Error envelope pattern:
```json
{"status":"error","reason":"<msg>","exit_code":1,"step":"compute_numbers"}
```

---

## 7. Creating a New Python Scriptlet (Step‑By‑Step)

1. Copy template pattern from existing step.
2. Place file under `orchestrator/scriptlets/python/steps/<snake_name>.py`.
3. Class name must be CamelCase version.
4. Use `@track_resources` from [`orchestrator/scriptlets/python/core/resource.py`](orchestrator/scriptlets/python/core/resource.py) for heavier runs.
5. Add validations; ensure outputs use `ctx.set(..., who="<scriptlet_name>")`.
6. Print success or error JSON to stdout; send human logs to stderr via `get_logger`.

Example skeleton:

```python
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources
from orchestrator.context import Context
import json, sys, argparse, statistics, pathlib

logger = get_logger(__name__)

class ComputeMedian(BaseScriptlet):
    def validate(self, ctx, params):
        if not isinstance(params, dict): raise ValueError("params must be dict")
        if "src" not in params: raise ValueError("missing src")
        if not pathlib.Path(params["src"]).is_file(): raise ValueError("file not found")

    @track_resources
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            nums = [float(line.strip()) for line in open(params["src"]) if line.strip()]
            med = statistics.median(nums)
            result = {"median": med, "count": len(nums)}
            ctx.set("numbers.median_v1", result, who="compute_median")
            print(json.dumps({"status":"ok","outputs":["numbers.median_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_median"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(ComputeMedian().run(Context(), p))
```

---

## 8. Adding the New Step to a Recipe

Edit (or create) recipe YAML under `orchestrator/recipes/`:

```yaml
test_meta:
  test_id: NUM-002
  tester: user
  description: Compute median
steps:
  - idx: 1
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    args:
      src: orchestrator/Data/numbers.csv
    success:
      ctx_has_keys:
        - numbers.median_v1
```

Run:
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/median.yaml
```

---

## 9. Recipe Specification (Current)

Minimal schema (see [`orchestrator/recipes/example_numbers.yaml`](orchestrator/recipes/example_numbers.yaml)):

```yaml
test_meta:
  test_id: ID-123
  tester: user
  description: Demo
steps:
  - idx: 1
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args:
      src: orchestrator/Data/numbers.csv
    success:
      ctx_has_keys:
        - numbers.stats_v1
```

Planned extensions (roadmap):
- `depends_on: [...]`
- `retry: {max: 3, delay_sec: 2}`
- `timeout: 30`
- `parallel: true`
- `cache_key: "<hash expression>"`

---

## 10. Logging & Resource Tracking

Use `get_logger` from [`orchestrator/scriptlets/python/core/logging_util.py`](orchestrator/scriptlets/python/core/logging_util.py).  
Decorate `run` with `@track_resources` for a `[resource] duration_sec=<x>` line to stderr (from [`resource.py`](orchestrator/scriptlets/python/core/resource.py)).

---

## 11. Error Handling Pattern

Inside `run`:
1. Validate early.
2. Wrap operational logic in `try/except`.
3. Emit structured JSON (never stack traces to stdout).
4. Return non‑zero exit code on failure to enforce fail‑fast behavior in [`orchestrator.runner`](orchestrator/runner.py).

---

## 12. Testing

- Integration: [`tests/test_example_numbers.py`](tests/test_example_numbers.py) runs full recipe.
- Add a negative test per pattern:

```python
import pytest
from orchestrator.scriptlets.python.steps.compute_numbers import ComputeNumbers
from orchestrator.context import Context

def test_compute_numbers_missing_file(tmp_path):
    ctx = Context()
    code = ComputeNumbers().run(ctx, {"src": str(tmp_path/"missing.csv")})
    assert code == 1
    assert ctx.get("numbers.stats_v1") is None
```

Run:
```bash
pytest -q
```

---

## 13. Backward Compatibility

Golden rule: never break existing step or core API.  
If behavior must change, create a versioned variant (e.g. `compute_numbers_v2.py`), update new recipes gradually, then retire old version after migration.

---

## 14. Performance & Parallel Roadmap

Currently sequential; extension ideas:
- Thread pool for I/O bound steps.
- Process pool when step sets `requires_process=True`.
- Context merge strategies: `last_write_wins` vs `fail_on_conflict`.
- Hash‑based caching using `(module, args)` fingerprint.
- Persistent run state file for resume capabilities.

---

## 15. Security / Safety

- Do not execute untrusted recipe modules without allowlist.
- Avoid leaking secrets in logs.
- Only JSON to stdout; all verbose text to stderr.

---

## 16. Troubleshooting

| Symptom | Likely Cause | Action |
|---------|--------------|--------|
| Empty `ctx_keys` | Step failed silently | Check stderr logs / ensure JSON printed |
| `TypeError: not JSON serializable` | Complex object stored | Convert to dict/list/primitive |
| ImportError module path | Wrong `module` in recipe | Verify dotted path & file presence |
| Success key missing | Bad `ctx.set` key or validation | Confirm key string & `who` attribution |

---

## 17. Frequently Used Patterns

### 17.1 Retrieve All Keys
```python
ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
print(list(ctx.to_dict().keys()))
```

### 17.2 Standalone Debug Run With Pretty Output
```bash
python -m orchestrator.scriptlets.python.steps.compute_numbers --params '{"src":"orchestrator/Data/numbers.csv"}' | jq
```

### 17.3 Add Lightweight Derived Metric
Create `compute_range.py` reading `numbers.stats_v1` and writing `numbers.range_v1` (depends on prior step once dependency graph is implemented).

---

## 18. Contribution Checklist

- Single responsibility scriptlet.
- JSON‑safe outputs; keys namespaced with version suffix (`_v1`).
- Structured success & error JSON.
- Added/updated tests.
- No breaking API changes; versioned new behavior.
- Clear logging & resource timing where useful.

---

## 19. Example Extended Scenario (Conceptual)

Composite recipe (future):

```yaml
steps:
  - idx: 1
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    depends_on: [compute_numbers]
    success:
      ctx_has_keys:
        - numbers.median_v1
  - idx: 3
    name: aggregate_summary
    type: python
    module: orchestrator.scriptlets.python.steps.aggregate_summary
    function: AggregateSummary
    success:
      ctx_has_keys:
        - numbers.summary_table_v1
```

---

## 20. Minimal Glossary

| Term | Meaning |
|------|---------|
| Scriptlet | Atomic executable unit (Python class) |
| Recipe | Declarative sequence of steps |
| Context | Shared JSON‑safe state with history |
| Output Key | Namespaced dotted key written to context |
| Step Success | Assertion block verifying outcomes |

---

## 21. Quick Reference Commands

| Task | Command |
|------|---------|
| Run example | `python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml` |
| Run tests | `pytest -q` |
| Lint (add your tool) | `ruff check .` (if added) |
| Create new step | Copy template → edit params → add to recipe |
| Debug single step | `python path/to/step.py --params '{"..."}'` |

---

## 22. Roadmap Snapshot

1. Success assertion processor (enforce `ctx_has_keys`).
2. Shell step support with JSON stdout enforcement.
3. Parallel groups + dependency graph.
4. Context persistence & `--resume-from`.
5. Caching layer (hash inputs).
6. HTML / Markdown reporting (templates/).
7. Coverage metrics in CI.

---

## 23. Summary

This workspace provides a lean, extensible backbone for structured automation. Extend through **new scriptlets + recipes**, retain JSON discipline, and layer in performance, caching, and parallelism features iteratively.

Happy building!
<!-- BASELINE-RECON-SECTION -->
