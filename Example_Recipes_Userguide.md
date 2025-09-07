# Example Recipes User Guide (Focused Walkthrough)

Scope: Detailed, line‑by‑line explanatory guide for:
- [orchestrator/recipes/example_numbers.yaml](orchestrator/recipes/example_numbers.yaml)
- [orchestrator/recipes/example_ctx.yaml](orchestrator/recipes/example_ctx.yaml)

Supporting scriptlets & core components:
- Context engine: [orchestrator/context.py](orchestrator/context.py)
- Runner: [orchestrator/runner.py](orchestrator/runner.py)
- Scriptlet base + utilities:
  - [orchestrator/scriptlets/python/core/base.py](orchestrator/scriptlets/python/core/base.py)
  - [orchestrator/scriptlets/python/core/logging_util.py](orchestrator/scriptlets/python/core/logging_util.py)
  - [orchestrator/scriptlets/python/core/resource.py](orchestrator/scriptlets/python/core/resource.py)
- Steps (Python scriptlets):
  - [orchestrator/scriptlets/python/steps/compute_numbers.py](orchestrator/scriptlets/python/steps/compute_numbers.py)
  - [orchestrator/scriptlets/python/steps/start_context_server_v2.py](orchestrator/scriptlets/python/steps/start_context_server_v2.py)
  - [orchestrator/scriptlets/python/steps/demo_update_state.py](orchestrator/scriptlets/python/steps/demo_update_state.py)

Folder Purdue/ intentionally ignored.

---

## 1. Framework Recap (Context for Recipes)

Execution model:
1. A YAML recipe is parsed by the runner ([orchestrator/runner.py](orchestrator/runner.py)).
2. Steps execute sequentially (future: dependency + parallel).
3. Each Python step is a class extending BaseScriptlet ([orchestrator/scriptlets/python/core/base.py](orchestrator/scriptlets/python/core/base.py)).
4. Shared state lives in a JSON‑safe context ([orchestrator/context.py](orchestrator/context.py)); writes via `ctx.set(key, value, who="<step_name>")`.
5. Each step prints exactly one JSON line to stdout describing success or failure:
   - Success (minimum): `{"status":"ok","outputs":["some.ctx.key_v1"]}`
   - Failure: `{"status":"error","reason":"...","exit_code":1,"step":"name"}`

Golden rules (reinforced by examples below):
- Namespaced + versioned keys (e.g. `numbers.stats_v1`).
- Validation early; fail fast.
- Only JSON‑serializable values.
- Backward compatibility preserved via versioned variants (`*_v2.py` etc.).

---

## 2. Recipe: example_numbers.yaml Deep Dive

File: [orchestrator/recipes/example_numbers.yaml](orchestrator/recipes/example_numbers.yaml)  
Purpose: Minimal numeric aggregation pipeline demonstrating a single Python scriptlet and success assertion.

### 2.1 Full Listing With Annotations

```yaml
test_meta:                           # (Section) Descriptive, non-executable metadata
  test_id: "NUM-001"                 # Unique identifier for traceability in logs & reporting
  tester: "demo"                     # Operator / automation tag (auditing)
  description: "Compute simple statistics from numbers.csv."  # Human-readable purpose

steps:                               # Ordered list of executable units
  - idx: 1                           # Explicit sequence index (stable ordering)
    name: compute_numbers            # Logical step name (used in depends_on, logs)
    type: python                     # Execution handler category (currently 'python')
    module: orchestrator.scriptlets.python.steps.compute_numbers   # Import path
    function: ComputeNumbers         # Class name inside module (must extend BaseScriptlet)
    args:                            # Arbitrary JSON-serializable parameter map passed to run()
      src: orchestrator/Data/numbers.csv   # Input file (relative path recommended)
    success:                         # Declarative post-step assertions (validated by runner roadmap)
      ctx_has_keys:                  # Keys that must exist in context on success
        - numbers.stats_v1
```

(If the actual file omits some comments above, they are added here for explanation.)

### 2.2 Field Semantics

| Field | Meaning | Notes |
|-------|---------|-------|
| test_meta | Non-executable metadata | Free form but stable keys aid future reporting |
| steps | List[Step] | Executed sequentially |
| idx | Int ordering | Must be unique; gaps allowed but discouraged |
| name | Step handle | Used in dependency graphs (future), success messages, and context attribution |
| type | Execution adapter | 'python' → dynamic import & class instantiation |
| module | Fully qualified module path | Must be importable by Python runtime |
| function | Class name | Must inherit BaseScriptlet |
| args | Input parameters | Passed intact to `run(ctx, params)` |
| success.ctx_has_keys | Postcondition expectations | Runner will (future) enforce; currently illustrative |

### 2.3 Scriptlet: compute_numbers

Location: [orchestrator/scriptlets/python/steps/compute_numbers.py](orchestrator/scriptlets/python/steps/compute_numbers.py)

Responsibilities (typical pattern):
- Parse `src` file of numeric lines.
- Compute descriptive stats (count, sum, mean, min, max, maybe variance).
- Validate file existence & numeric conversion.
- Store result into context under `numbers.stats_v1`.
- Emit JSON success envelope:  
  `{"status":"ok","outputs":["numbers.stats_v1"]}`

Key internal calls (expected pattern):
- Logger from [orchestrator/scriptlets/python/core/logging_util.py](orchestrator/scriptlets/python/core/logging_util.py).
- Optional resource decorator from [orchestrator/scriptlets/python/core/resource.py](orchestrator/scriptlets/python/core/resource.py).
- `ctx.set("numbers.stats_v1", stats_dict, who="compute_numbers")`.

### 2.4 Example: Running This Recipe

CLI:
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```

Programmatic:
```python
from orchestrator.runner import run_recipe
ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
print(ctx.get("numbers.stats_v1"))
```

Dry-run style (conceptual future flag):
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml --plan
```

### 2.5 Extending This Recipe

Add a derived median step (new file `compute_median.py` then modify recipe):

```yaml
  - idx: 2
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    args:
      src: orchestrator/Data/numbers.csv
    depends_on:            # (Future capability)
      - compute_numbers
    success:
      ctx_has_keys:
        - numbers.median_v1
```

### 2.6 Common Failure Scenarios

| Symptom | Cause | Mitigation |
|---------|-------|------------|
| FileNotFoundError | Wrong relative path | Confirm under orchestrator/Data/ |
| Empty stats | Input file blank | Provide seed data |
| Non-numeric line | Validation not strict | Update scriptlet to skip or fail |
| Missing ctx key | Scriptlet forgot ctx.set | Ensure `who` + proper key string |

---

## 3. Recipe: example_ctx.yaml Deep Dive

File: [orchestrator/recipes/example_ctx.yaml](orchestrator/recipes/example_ctx.yaml)  
Purpose: Demonstrate multi-step orchestration, inter-step dependency (`depends_on`), launching a persistent context server, then exercising it through scripted “operations” producing summarized context trace keys.

### 3.1 Full Listing With Inline Commentary

(Reconstructed with explanatory comments.)

```yaml
test_meta:
  test_id: "CTX-DEMO-001"            # Unique identifier for this context demonstration
  tester: "demo"                     # Operator tag
  description: "Launch shared context server in a GNU screen session and exercise update_state.sh."

steps:
  - idx: 1
    name: start_context_server       # Step that boots an async/shared context service
    type: python
    module: orchestrator.scriptlets.python.steps.start_context_server_v2
    function: StartContextServerV2   # Class providing launch + readiness wait
    args:
      screen_name: "ctx_server"      # GNU screen session name (decouples lifetime)
      script: "run_context_server.py" # Entrypoint script (root-level file: run_context_server.py)
      wait_for_port: true            # Actively poll for TCP readiness
      wait_host: "127.0.0.1"         # Host to probe
      wait_port: 8765                # Port exposed by server
      wait_timeout_sec: 40           # Max seconds to wait for readiness
    success:
      ctx_has_keys:                  # Server scriptlet expected to register these context entries
        - "context_server.session_v1"
        - "context_server.launch_cmd_v1"
        - "context_server.ready_v1"

  - idx: 2
    name: demo_update_state          # Consumer step interacting with the running server
    type: python
    module: orchestrator.scriptlets.python.steps.demo_update_state
    function: DemoUpdateState
    depends_on:                      # Enforces ordering + (future) skip logic if upstream failed
      - start_context_server
    args:
      operations:                    # Declarative batch of state manipulations (executed sequentially)
        - ["set", "demo.user", '{"name":"Alice","age":30}']
        - ["append", "demo.numbers", "100"]
        - ["append", "demo.numbers", "200"]
        - ["set", "demo.user", '{"name":"Alice","age":31}']
        - ["get", "demo.user"]
        - ["get", "demo.numbers"]
      host: "127.0.0.1"              # Target server host (matches launch)
      port: 8765                     # Target server port
      probe_retries: 120             # Internal readiness checks for server (secondary safety net)
      probe_delay_sec: 0.5           # Sleep between retries (seconds)
    success:
      ctx_has_keys:
        - "demo_update_state.requests_v1"    # Raw executed request log
        - "demo_update_state.responses_v1"   # Raw server responses
        - "demo_update_state.summary_v1"     # Aggregated human/machine friendly summary
```

### 3.2 Step 1: start_context_server

Scriptlet: [orchestrator/scriptlets/python/steps/start_context_server_v2.py](orchestrator/scriptlets/python/steps/start_context_server_v2.py)

Expected behaviors:
- Build a launch command (e.g. using `screen -dmS ctx_server python run_context_server.py`).
- Spawn detached process (resilient to runner exit).
- Poll `wait_host:wait_port` until a TCP connection opens or timeout reached.
- Populate context:
  - `context_server.session_v1` – screen session name or PID metadata.
  - `context_server.launch_cmd_v1` – reproducible command line.
  - `context_server.ready_v1` – readiness marker (timestamp or boolean).

Failure exit codes:
- 1: Launch failure (exception building or executing command).
- 2: Timeout waiting for port readiness.

### 3.3 Step 2: demo_update_state

Scriptlet: [orchestrator/scriptlets/python/steps/demo_update_state.py](orchestrator/scriptlets/python/steps/demo_update_state.py)

Behavior:
- Confirm server reachable (host/port).
- Replay structured `operations` list.
  - Each entry: `[verb, key, value?]`
  - Supported verbs (illustrated): `set`, `append`, `get`, (optionally `delete` in future).
- Maintain ordered arrays:
  - `demo_update_state.requests_v1`
  - `demo_update_state.responses_v1`
- Derive `demo_update_state.summary_v1`: e.g. counts, final keys touched, maybe timing.

Internal design considerations:
- All network I/O wrapped in try/except; partial failures still logged.
- Uses context only for result capture (does not rely on global state).
- Serial JSON contract with server; values remain strings until parsed/validated.

### 3.4 Context Evolution Walkthrough

Operation sequence (simplified):

| Op | Action | Resulting Keys |
|----|--------|----------------|
| set demo.user {...age:30} | Creates object | demo.user |
| append demo.numbers 100 | Initializes list then appends | demo.numbers |
| append demo.numbers 200 | Appends second value | demo.numbers |
| set demo.user {...age:31} | Overwrites user | demo.user |
| get demo.user | Reads current | (no new key) |
| get demo.numbers | Reads list | (no new key) |

Scriptlet-level context keys added:
- `demo_update_state.requests_v1`
- `demo_update_state.responses_v1`
- `demo_update_state.summary_v1`

Server-level keys (if it mirrors state internally) are not directly required in recipe assertions—only the scriptlet’s meta outputs.

### 3.5 Typical Extensions

Add a delete operation:
```yaml
      operations:
        - ["delete", "demo.user"]
        - ["get", "demo.user"]
```
Then `summary_v1` should reflect a missing/None result.

Add validation assertions (future schema):
```yaml
    success:
      ctx_has_keys:
        - demo_update_state.summary_v1
      ctx_match:
        - key: demo_update_state.summary_v1.total_ops
          eq: 7
```

### 3.6 Failure Patterns

| Stage | Failure | Mitigation |
|-------|---------|------------|
| Server launch | Port never opens | Check script path, firewall, screen availability |
| Server step | screen not installed | Pre-install dependency or fallback to subprocess |
| update_state | Connection refused | Increase `probe_retries`, verify host/port |
| update_state | Malformed operation tuple | Add validation: length & verb registry |
| Assertions | Key missing | Ensure scriptlet sets context before printing success JSON |

---

## 4. Comparing Both Recipes

| Aspect | example_numbers.yaml | example_ctx.yaml |
|--------|----------------------|------------------|
| Complexity | Single step | Multi-step dependency |
| External Process | No | Yes (context server) |
| Data Source | Static file | Live stateful service |
| Demonstrates | Basic scriptlet stats pattern | Orchestration + dependency + batching operations |
| Keys Produced | numbers.stats_v1 | context_server.* + demo_update_state.* |

---

## 5. Scriptlet Authoring Patterns Illustrated

| Pattern | Seen In | Principle |
|---------|---------|-----------|
| Single responsibility | compute_numbers | Keep scriptlet narrow |
| Versioned evolution | start_context_server_v2 | Add `_v2` instead of mutating original |
| Batched operations | demo_update_state | Declarative arrays > imperative loops in recipes |
| Context namespacing | All | Prevent collisions (`domain.entity_v1`) |
| Success assertions | Both recipes | Formalize expected side-effects |

---

## 6. Creating a New Derived Recipe (Composite Example)

Goal: Chain numeric stats + context operations (illustrative only).

```yaml
test_meta:
  test_id: "COMBO-001"
  tester: "dev"
  description: "Numbers pipeline plus context interaction"

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

  - idx: 2
    name: start_context_server
    type: python
    module: orchestrator.scriptlets.python.steps.start_context_server_v2
    function: StartContextServerV2
    args:
      screen_name: "ctx_server"
      script: "run_context_server.py"
      wait_for_port: true
      wait_host: "127.0.0.1"
      wait_port: 8765
      wait_timeout_sec: 40
    success:
      ctx_has_keys:
        - context_server.ready_v1

  - idx: 3
    name: publish_stats
    type: python
    module: orchestrator.scriptlets.python.steps.publish_stats   # (New scriptlet to implement)
    function: PublishStats
    depends_on:
      - compute_numbers
      - start_context_server
    args:
      stat_key: numbers.stats_v1
      host: 127.0.0.1
      port: 8765
    success:
      ctx_has_keys:
        - publish_stats.ack_v1
```

---

## 7. Boilerplate Python Scriptlet Template (Aligned with Examples)

```python
# Minimal pattern consistent with existing steps
import json, sys, argparse, statistics
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources
from orchestrator.context import Context

logger = get_logger(__name__)

class PublishStats(BaseScriptlet):
    def validate(self, ctx, params):
        if "stat_key" not in params:
            raise ValueError("stat_key required")

    @track_resources
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            stats = ctx.get(params["stat_key"])
            if not stats:
                raise ValueError(f"Missing stats at {params['stat_key']}")
            # Simulated publish
            ack = {"published": True, "source": params["stat_key"]}
            ctx.set("publish_stats.ack_v1", ack, who="publish_stats")
            print(json.dumps({"status": "ok", "outputs": ["publish_stats.ack_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status": "error", "reason": str(e), "exit_code": 1, "step": "publish_stats"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(PublishStats().run(Context(), p))
```

---

## 8. Testing Patterns Derived From Recipes

Integration test for `example_numbers.yaml` (see [tests/test_example_numbers.py](tests/test_example_numbers.py)):

```python
def test_numbers_recipe_end_to_end():
    from orchestrator.runner import run_recipe
    ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
    stats = ctx.get("numbers.stats_v1")
    assert stats and "count" in stats
```

Hypothetical test for context recipe:

```python
def test_context_recipe():
    from orchestrator.runner import run_recipe
    ctx = run_recipe("orchestrator/recipes/example_ctx.yaml")
    assert ctx.get("context_server.ready_v1") is not None
    summary = ctx.get("demo_update_state.summary_v1")
    assert summary and summary.get("total") >= 6
```

---

## 9. Troubleshooting Matrix Focused on These Recipes

| Issue | Recipe | Root Cause | Fix |
|-------|--------|-----------|-----|
| `ModuleNotFoundError` | Both | Wrong module dotted path | Verify file exists & PYTHONPATH root |
| Missing `numbers.stats_v1` | example_numbers | Exception reading file | Validate numeric file contents |
| Context server timeout | example_ctx | Port mismatch or script failed | Confirm port 8765 open via `ss -ltn` |
| No responses list | example_ctx | Operations aborted early | Check stderr logs for connection errors |
| Unicode / serialization error | Both | Non-JSON value stored | Ensure only primitives/dicts/lists |

---

## 10. Design Principles Reinforced

Principle → Demonstrated By:
- Versioned evolution → `start_context_server_v2`
- Namespace discipline → `numbers.stats_v1`, `context_server.ready_v1`
- Declarative orchestration → YAML structure
- Separation of concerns → Compute vs. Server lifecycle vs. Client operations
- Deterministic repeatability → No random seeds, explicit inputs
- Extensibility path → Add new steps without mutating existing ones

---

## 11. Quick Reference Commands

| Task | Command |
|------|--------|
| Run numbers recipe | `python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml` |
| Run context recipe | `python orchestrator/runner.py --recipe orchestrator/recipes/example_ctx.yaml` |
| Inspect context stats | `jq . Logs/latest_run/context.json` (if such dump implemented) |
| Tail screen session | `screen -r ctx_server` |
| List screen sessions | `screen -ls` |

---

## 12. Checklist for Adding New Recipes

- Unique `test_id`.
- Each step has unique `idx` + `name`.
- Use versioned scriptlets if altering behavior.
- All produced keys asserted under `success.ctx_has_keys`.
- Paths relative to repository root (portable).
- No secrets or absolute machine-specific paths.
- Validate parameters inside scriptlets (not in YAML).

---

## 13. FAQ (Focused)

Q: Why version `start_context_server_v2`?  
A: Avoid breaking existing recipes depending on original semantics (backward compatibility rule).

Q: Why store summaries (`summary_v1`) instead of only raw requests/responses?  
A: Downstream steps/tests prefer condensed, stable schema for assertions & reporting.

Q: Can I reuse operations arrays across recipes?  
A: Yes—extract to a small helper scriptlet or template loader (future enhancement).

---

## 14. Roadmap Hooks Visible in These Recipes

| Future Feature | Where It Fits |
|----------------|---------------|
| Dependency graph validation | `depends_on` in `demo_update_state` |
| Retry / timeout per step | Long-running server launch |
| Parallel groups | Independent numeric + context initialization pipelines |
| Context persistence / resume | After server start, resume at consumer step |
| Success assertion engine | `ctx_has_keys` enforcement at runtime |

---

## 15. Summary

The two example recipes illustrate:
- Base numeric processing pattern (minimal single-step pipeline).
- Stateful multi-step orchestration with a launched auxiliary service and dependent interaction.
They collectively showcase key platform concepts: strict JSON context, versioned scriptlets, declarative recipes, reproducibility, and forward-compatible structure.

Use them as authoritative templates when designing new automation flows.
<!-- ### AUTOGEN(rebase_workspace.sh) baseline injection -->

<!-- BASELINE-RECON-SECTION -->
Rebase script executed: 2025-09-07T21:03:12Z
Managed tag marker: ### AUTOGEN(rebase_workspace.sh)
Use './rebase_workspace.sh --force' to refresh generated scaffolding.

