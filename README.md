# Automation Framework (Minimal Example Edition)

A lightweight, extensible automation/testing framework scaffold. It demonstrates how to build and run modular "scriptlets" (small, single‑purpose steps) orchestrated by YAML recipes, sharing state through a JSON‑serializable context object and validated by tests.

> This repository currently contains a minimal vertical slice (Context + Runner + 1 Python scriptlet + 1 recipe + 1 test). It is intentionally lean so you can extend it safely.

---
## 1. Brief Description
**Purpose:** Execute ordered (later: parallel) steps defined in a YAML recipe. Each step is a self-contained scriptlet that:
- Accepts parameters (JSON) 
- Performs a focused task
- Emits machine-readable JSON to stdout
- Writes structured results into a shared Context

**Core Flow:**
1. You write a scriptlet (e.g. compute statistics on a CSV)
2. You reference it in a recipe YAML
3. The runner loads the recipe and executes each step in order
4. The `Context` captures outputs (versioned keys) for later steps/tests

**Inputs:** YAML recipe + referenced data files + optional CLI params.
**Outputs:** Updated in-memory context + scriptlet stdout JSON + potential artifacts under `orchestrator/Data/` (future: Logs, Reports, etc.).

**Key Dependencies:**
- Python 3.11+
- PyYAML (recipe parsing)
- pytest (testing)

---
## 2. Directory Layout (Current State)
```
orchestrator/
  context.py                 # Shared state container
  runner.py                  # Minimal recipe executor
  Data/
    numbers.csv              # Example input data
  recipes/
    example_numbers.yaml     # Recipe invoking the compute_numbers step
  scriptlets/
    python/
      core/
        base.py              # BaseScriptlet abstract class
        logging_util.py      # Simple stderr logger
        resource.py          # Timing decorator
      steps/
        compute_numbers.py   # Example scriptlet (statistics from CSV)

tests/
  test_example_numbers.py    # Demonstrates running the recipe & asserting context
```

Future (not yet implemented here) per architecture guidelines: `Logs/`, `templates/`, `shell/`, `apps/`, richer success criteria, dependency graphs, parallelism, resource tracking, error code taxonomy.

---
## 3. Core Components & Data Flow
| Component | Responsibility | Input | Output |
|-----------|----------------|-------|--------|
| Context (`context.py`) | Store JSON-serializable key/value pairs + change history | Key, value, actor | In-memory state + history list |
| Scriptlet (`compute_numbers.py`) | Read CSV, compute stats, set context key | Params `{src}` + file | Context key `numbers.stats_v1` + JSON stdout |
| Runner (`runner.py`) | Parse recipe, dynamically load & run scriptlets | `--recipe path` | Final context + stdout JSON summary |
| Recipe (`example_numbers.yaml`) | Declarative definition of steps | YAML | Execution order + validation cues |
| Test (`test_example_numbers.py`) | Assert expected context state | Recipe execution | Pass/fail report |

**Data Flow Example:**
```
CSV (numbers.csv) --> compute_numbers --> stats dict --> ctx['numbers.stats_v1'] --> test asserts
```

---
## 4. Detailed Code Explanations (Line-by-Line, Beginner Friendly)
### 4.1 `orchestrator/context.py`
```python
class Context:
    def __init__(self) -> None:        # When we create a Context, we set up empty storage structures
        self._data: Dict[str, Any] = {} # Holds all key/value pairs set by scriptlets
        self._history: List[Dict[str, Any]] = [] # Keeps a chronological list of every change

    def set(self, key: str, value: Any, who: str) -> None:
        try:
            json.dumps(value)          # We try to serialize the value to ensure it's JSON-safe
        except TypeError as e:
            raise TypeError(...)       # If not serializable, we fail early with a clear message
        old = self._data.get(key)      # Capture the previous value (may be None)
        self._data[key] = value        # Store/overwrite the new value
        self._history.append({         # Append a record capturing what changed and by whom
            'ts': time.time(),
            'key': key,
            'who': who,
            'old': old,
            'new': value,
        })

    def get(self, key: str, default: Any = None) -> Any:
        return self._data.get(key, default)  # Safely fetch a value or a default if missing

    def to_dict(self) -> Dict[str, Any]:
        return dict(self._data)              # Return a shallow copy so callers can't mutate internal dict accidentally

    @property
    def history(self) -> List[Dict[str, Any]]:
        return list(self._history)           # Provide a copy of history records
```

### 4.2 `orchestrator/runner.py`
```python
import yaml                         # External library used to parse YAML recipe files
from orchestrator.context import Context

def _run_python_step(ctx, step):    # Helper to execute one python-type step from a recipe
    module_name = step['module']    # Module path string from YAML
    class_name = step['function']   # Class name implementing the step
    mod = importlib.import_module(module_name) # Dynamically load the module
    cls = getattr(mod, class_name)  # Get the class object by name
    inst = cls()                    # Instantiate the scriptlet
    params = step.get('args', {})   # Retrieve params (empty dict if none)
    code = inst.run(ctx, params)    # Execute the step, passing shared context & parameters
    if code != 0:                   # Non-zero exit signals failure
        raise RuntimeError(...)

def run_recipe(recipe_path: str) -> Context:
    spec = yaml.safe_load(open(recipe_path)) # Load the YAML recipe into a Python dict
    steps = spec.get('steps', [])            # Extract the ordered list of steps
    ctx = Context()                          # New context per invocation
    for step in steps:                       # Iterate steps sequentially
        if step.get('type') == 'python':     # Currently only python steps supported
            _run_python_step(ctx, step)
        else:
            raise NotImplementedError(...)
    return ctx                               # Return populated context for caller/tests
```

### 4.3 `compute_numbers.py`
```python
# High-level: Reads a CSV with a single column 'value', computes count/mean/min/max.
class ComputeNumbers(BaseScriptlet):
    def validate(self, ctx, params):         # Ensure required input exists and is well-typed
        if 'src' not in params or not isinstance(params['src'], str):
            raise ValueError("'src' (str) required")

    @track_resources                      # Decorator: records how long run() takes
    def run(self, ctx, params) -> int:
        self.validate(ctx, params)        # Always validate first
        src = params['src']               # Path to the CSV file
        values = []                       # We'll accumulate numeric values here
        with open(src, 'r', newline='') as f:
            reader = csv.DictReader(f)    # Parses CSV rows into dicts keyed by column name(s)
            fieldnames = reader.fieldnames or []
            if 'value' not in fieldnames: # Enforce contract: must have 'value' column
                raise ValueError("CSV must have 'value' column")
            for row in reader:            # Loop through each row
                if row['value'].strip():  # Skip blank lines or empty fields
                    values.append(float(row['value'])) # Convert to float and store
        if not values:                    # Defensive: ensure we got at least one number
            raise ValueError('No numeric values found')
        stats = {                         # Build a dictionary of simple statistics
            'count': len(values),
            'mean': float(statistics.fmean(values)),
            'min': float(min(values)),
            'max': float(max(values)),
        }
        json.dumps(stats)                 # Validates JSON serializability
        ctx.set('numbers.stats_v1', stats, who='compute_numbers') # Commit result to context
        print(json.dumps({'status': 'ok', 'outputs': ['numbers.stats_v1']})) # Machine-readable stdout
        return 0                          # 0 signals success
```

---
## 5. Implementation Example (End-to-End)
### 5.1 Prepare Environment
```bash
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
```

### 5.2 Run Scriptlet Directly
```bash
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
```
Expected stdout (example):
```json
{"status":"ok","outputs":["numbers.stats_v1"]}
```

### 5.3 Run via Recipe
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```
Sample stdout:
```json
{"status": "ok", "ctx_keys": ["numbers.stats_v1"]}
```

### 5.4 Validate with Tests
```bash
pytest -q
```
Output should show: `1 passed`.

### 5.5 Inspect Context (Interactive)
```bash
python - <<'PY'
from orchestrator.runner import run_recipe
ctx = run_recipe('orchestrator/recipes/example_numbers.yaml')
print('Stats:', ctx.get('numbers.stats_v1'))
print('History:', ctx.history)
PY
```

---
## 6. Edge Cases & Handling
| Scenario | Behavior |
|----------|----------|
| Missing file | FileNotFoundError surfaces (not yet custom-wrapped) |
| Missing 'value' column | Raises ValueError with clear message |
| Non-numeric row entries | Throws ValueError/float conversion error (future: sanitize) |
| Empty CSV (besides header) | "No numeric values found" error |
| Duplicate key sets | History records each overwrite (latest value accessible) |

---
## 7. Best Practices & Guidelines
- Version keys (e.g., `numbers.stats_v1`, later `numbers.stats_v2`) to avoid breaking downstream consumers.
- Keep scriptlets single responsibility: one clear input/output contract.
- Always validate params early before heavy work.
- Only write machine-readable JSON to stdout; human logs to stderr (logger).
- Ensure context values are JSON-serializable (convert datetimes → ISO strings, etc.).
- Add tests for every new scriptlet; integration tests for multi-step recipes.

### Common Pitfalls
| Pitfall | Avoid By |
|---------|----------|
| Storing complex objects in context | Serialize or write to file + store path |
| Silent failures | Always return non-zero on errors & emit structured error JSON |
| Overloaded scriptlets | Split into smaller steps (normalize → compute → aggregate) |
| Unversioned breaking changes | Introduce new `_v2` key or new scriptlet module |

### Optimization Opportunities (Future)
- Caching intermediate results (hash inputs → reuse)
- Parallel step execution (Thread/Process pools) where dependencies allow
- Structured resource metrics (CPU %, memory deltas) instead of simple timing
- Retry wrappers for transient I/O failures

### Alternative Approaches
| Approach | Trade-off |
|----------|----------|
| Monolithic pipeline script | Simpler initial code, harder to extend/test |
| Orchestrator + separate microservices | Scales horizontally, higher complexity |
| Makefile / shell-only | Good for trivial automation, lacks structured context |

---
## 8. Adding a New Scriptlet (Example Walkthrough)
### Goal: Normalize numeric values (0–1 scaling) before computing stats.

1. Create `orchestrator/scriptlets/python/steps/normalize_numbers.py`:
```python
"""Normalize values in a CSV and write a new CSV."""
from __future__ import annotations
import csv, json, argparse, sys
from pathlib import Path
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger

logger = get_logger(__name__)

class NormalizeNumbers(BaseScriptlet):
    def validate(self, ctx, params):
        if 'src' not in params or 'out' not in params:
            raise ValueError("'src' and 'out' required")

    def run(self, ctx, params) -> int:
        try:
            self.validate(ctx, params)
            src, out = params['src'], params['out']
            values = []
            with open(src) as f:
                r = csv.DictReader(f)
                if 'value' not in (r.fieldnames or []):
                    raise ValueError("need 'value' column")
                for row in r:
                    if row['value'].strip():
                        values.append(float(row['value']))
            if not values:
                raise ValueError('no values to normalize')
            lo, hi = min(values), max(values)
            span = hi - lo or 1.0
            norm_rows = [(v - lo)/span for v in values]
            # Write new CSV
            Path(out).parent.mkdir(parents=True, exist_ok=True)
            with open(out, 'w', newline='') as f:
                w = csv.writer(f)
                w.writerow(['value'])
                for v in norm_rows:
                    w.writerow([v])
            ctx.set('numbers.normalized_path_v1', out, who='normalize_numbers')
            print(json.dumps({"status":"ok","outputs":["numbers.normalized_path_v1"]}))
            return 0
        except Exception as e:  # minimal error handling
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"normalize_numbers"}))
            logger.exception('normalize failed')
            return 1

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--params', required=True)
    params = json.loads(ap.parse_args().params)
    ctx = Context()
    sys.exit(NormalizeNumbers().run(ctx, params))
```
2. Add a recipe:
```yaml
steps:
  - idx: 1
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
    args:
      src: orchestrator/Data/numbers.csv
      out: orchestrator/Data/numbers_normalized.csv
    success:
      ctx_has_keys:
        - numbers.normalized_path_v1
  - idx: 2
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args:
      src: orchestrator/Data/numbers_normalized.csv
    depends_on: [normalize_numbers]
    success:
      ctx_has_keys:
        - numbers.stats_v1
```
3. Run it:
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/normalize_and_compute.yaml
```

---
## 9. Testing Patterns
Basic test structure (`pytest`):
```python
def test_stats_mean():
    from orchestrator.runner import run_recipe
    ctx = run_recipe('orchestrator/recipes/example_numbers.yaml')
    stats = ctx.get('numbers.stats_v1')
    assert abs(stats['mean'] - 3.0) < 1e-9
```
Add edge tests (missing file, invalid CSV) by creating temporary files and asserting raised exceptions or error JSON.

---
## 10. Error Handling (Current vs Target)
| Aspect | Current | Target (Future) |
|--------|---------|-----------------|
| JSON error schema | Basic per scriptlet | Standardized (status, reason, details, exit_code, ts) |
| Exit codes | 0 success, 1 generic error | Expanded taxonomy (dependency unsatisfied, validation error, etc.) |
| Logging | Stderr prints + simple logger | Structured log files per step under `Logs/` |

---
## 11. Extensibility Roadmap
1. Add shell scriptlet support (mirroring Python interface)  
2. Implement success validation (files exist, ctx keys present, custom expressions)  
3. Add parallel execution with dependency graph resolution  
4. Introduce artifact management (hashing, dedupe)  
5. Integrate resource metrics (RSS, CPU %) and thresholds  
6. Reporting exporters (Excel, DOCX, HTML)  
7. Dash live monitoring app (consumes context snapshots)  
8. Versioned migration helpers (e.g. unify statistics schemas)  

---
## 12. Troubleshooting
| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| `ModuleNotFoundError: orchestrator` | Not on PYTHONPATH | Run from repo root / ensure tests add root to path |
| `CSV must have 'value' column` | Wrong file schema | Add header row `value` |
| Mean not as expected | Non-numeric lines | Clean input or add validation scriptlet |
| Test can't find recipe | Wrong relative path | Use path relative to repo root |

---
## 13. FAQ
**Q: How do I prevent breaking changes?**  
A: Introduce new keys (`*_v2`) or duplicate scriptlet with version suffix.

**Q: Can I store a pandas DataFrame in context?**  
A: No—serialize (to CSV) and store the path.

**Q: How to add timing/metrics?**  
A: Wrap logic with `@track_resources`; extend decorator to capture memory.

---
## 14. Glossary
- Scriptlet: A small, atomic executable unit of work.
- Recipe: Declarative sequence of scriptlets.
- Context: In-memory, JSON-serializable state store with history.
- Artifact: (Future) A persisted file produced by a step.

---
## 15. Quick Reference Commands
```bash
# Run example recipe
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml

# Directly run scriptlet
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'

# Run tests
pytest -q
```

---
## 16. License & Contribution
(Adjust this section as needed.) Contributions: follow versioning & non-breaking principles; add tests for every new feature.

---
## 17. Summary
You now have a functioning backbone: Context + Runner + Scriptlet + Recipe + Test. Extend by adding more scriptlets, recipes, and validation layers while preserving backward compatibility and clean interfaces.

Happy automating!
