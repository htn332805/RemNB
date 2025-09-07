# Comprehensive Curriculum & Framework User Guide

(Workspace scope intentionally excludes any `Purdue/` content.)

---

## 1. Purpose

This document is a deep curriculum-style walkthrough of the automation framework located under `orchestrator/`. It explains:
- Core architecture (context, runner(s), scriptlets, recipes, tests)
- Progressive recipe curriculum (foundations → advanced)
- How to write, run, extend, version, test, and optimize scriptlets
- Operational controls (skip / only / resume / caching / parallel / process pool)
- Reporting, auditing, resilience, and performance roadmap

---

## 2. Directory Map (Relevant Portions)

```
orchestrator/
  context.py
  runner.py
  runner_v2.py
  recipes/
    example_numbers.yaml
    example_ctx.yaml
    01_basic_number.yaml
    02_basic_mixed.yaml
    03_validation_errors.yaml
    04_artifacts_pipeline.yaml
    05_metrics_versioned.yaml
    06_parallel_fan_in.yaml
    07_cpu_process.yaml
    08_versioning_wrapper.yaml
    09_dashboard.yaml
    10_reporting.yaml
    11_resilience_controls.yaml
    12_advanced_full.yaml
  scriptlets/
    python/
      core/
        base.py
        logging_util.py
        resource.py
      steps/
        compute_numbers.py
        (curriculum steps: load_numbers.py, validate_numbers.py, normalize_numbers.py,
         compute_metrics_v1.py, compute_metrics_v2.py, compute_mean.py,
         compute_median.py, compute_std.py, aggregate_metrics.py,
         prime_factor_analysis.py, generate_report.py, inject_failure.py,
         cache_numbers.py, resource_audit.py, cleanup_intermediate.py,
         start_context_server_v2.py, demo_update_state.py, publish_stats.py?)
      apps/
        live_dashboard.py
    shell/
      sanitize_csv.sh
      enrich_csv.sh
templates/
  report_template.md
  metrics_summary.xlsx
setup_curriculum.sh
run_context_server.py
tests/
  (example + curriculum test files)
```

Key Files:
- Context engine: [orchestrator/context.py](orchestrator/context.py)
- Base class: [orchestrator/scriptlets/python/core/base.py](orchestrator/scriptlets/python/core/base.py)
- Logging: [orchestrator/scriptlets/python/core/logging_util.py](orchestrator/scriptlets/python/core/logging_util.py)
- Resource decorator: [orchestrator/scriptlets/python/core/resource.py](orchestrator/scriptlets/python/core/resource.py)
- Original runner: [orchestrator/runner.py](orchestrator/runner.py)
- Extended runner: [orchestrator/runner_v2.py](orchestrator/runner_v2.py)
- Setup script (bulk curriculum scaffolding): [setup_curriculum.sh](setup_curriculum.sh)

---

## 3. Core Concepts

| Concept | Summary |
|--------|---------|
| Context | In-memory JSON-safe key/value store with history & attribution |
| Scriptlet | Single-responsibility step (Python class or shell script) emitting one JSON line |
| Recipe | YAML declarative plan enumerating ordered steps |
| Versioning | Additive; never mutate existing key schema—introduce `_v2`, `_v3`, etc. |
| Runner | Executes recipe sequentially or with limited parallelization (v2) |
| Artifacts | Files under `orchestrator/Data/` or related paths referenced via relative paths |

---

## 4. The Context Object

File: [orchestrator/context.py](orchestrator/context.py)

Behavior:
- Ensures values are JSON-serializable.
- Writes via `ctx.set(key, value, who="scriptlet_name")` for traceability.
- Retrieval via `ctx.get(key)`; introspection via `ctx.to_dict()`.

Do NOT store:
- Open handles / sockets / file objects
- Non-serializable types (datetime must become ISO string)
- Raw large binary blobs (persist to file and store relative path)

---

## 5. Base Scriptlet Contract

File: [orchestrator/scriptlets/python/core/base.py](orchestrator/scriptlets/python/core/base.py)

Minimum Python scriptlet pattern:
```python
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
import json, sys, argparse

class ExampleStep(BaseScriptlet):
    def validate(self, ctx, params):
        if "input" not in params: raise ValueError("input required")

    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            result = {"echo": params["input"]}
            ctx.set("example.echo_v1", result, who="example_step")
            print(json.dumps({"status":"ok","outputs":["example.echo_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"example_step"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(ExampleStep().run(Context(), p))
```

Key Policies:
- One class per file.
- Exactly one JSON line printed to stdout.
- Use `who=<scriptlet_name>` in all `ctx.set`.

---

## 6. Logging & Resource Tracking

Utilities:
- Logger factory: [orchestrator/scriptlets/python/core/logging_util.py](orchestrator/scriptlets/python/core/logging_util.py)
- Resource decorator (duration, basic stats): [orchestrator/scriptlets/python/core/resource.py](orchestrator/scriptlets/python/core/resource.py)

Pattern:
```python
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources
logger = get_logger(__name__)

class SomeStep(BaseScriptlet):
    @track_resources
    def run(self, ctx, params):
        ...
```

Log routing: Human-readable details to stderr; structured machine outputs to stdout.

---

## 7. Runner Evolution

| Runner | File | Capabilities |
|--------|------|--------------|
| v1 | [orchestrator/runner.py](orchestrator/runner.py) | Sequential execution |
| v2 | [orchestrator/runner_v2.py](orchestrator/runner_v2.py) | Dependencies, basic parallelizable flags, process pool (`requires_process`), skip/only/resume, shell step integration, simple caching markers, timing metrics, early failure raise |

`runner_v2.py` adds:
- Topological ordering
- Context metrics under `_runner.step_metrics_v1`
- Caching markers in `.cache/`
- Shell step handler (expects JSON stdout)
- Process pool if scriptlet sets `requires_process = True`
- Filters: `--only`, `--skip`, `--resume-from`
- Simple conflict strategy: last-write-wins (future: fail_on_conflict)

---

## 8. Scriptlet Inventory (Curriculum & Core)

| Domain | Scriptlet (Python) | Produced Keys (Primary) |
|--------|--------------------|-------------------------|
| Numbers ingest | `load_numbers.py` | `numbers.raw_v1` |
| Validation | `validate_numbers.py` | `numbers.validated_v1` |
| Normalization | `normalize_numbers.py` | `numbers.normalized_v1`, `numbers.stats_v1` |
| Metrics v1 | `compute_metrics_v1.py` | `metrics.summary_v1`, `metrics.columns_v1` |
| Metrics v2 | `compute_metrics_v2.py` | `metrics.summary_v1`, `metrics.summary_v2` |
| Mean | `compute_mean.py` | `metrics.mean_v1` |
| Median | `compute_median.py` | `metrics.median_v1` |
| Std Dev | `compute_std.py` | `metrics.std_v1` |
| Aggregate | `aggregate_metrics.py` | `metrics.aggregate_v1` |
| CPU Intensive | `prime_factor_analysis.py` | `numbers.prime_factors_v1` |
| Reporting | `generate_report.py` | `report.path_v1` |
| Failure Injection | `inject_failure.py` | `resilience.inject_ok_v1` |
| Caching | `cache_numbers.py` | `cache.numbers_hash_v1`, `cache.numbers_hit_v1` |
| Resource Audit | `resource_audit.py` | `resource.audit_v1` |
| Cleanup | `cleanup_intermediate.py` | `cleanup.removed_v1` |
| Context Server (v2) | `start_context_server_v2.py` | `context_server.*_v1` |
| Context Ops Client | `demo_update_state.py` | `demo_update_state.*_v1` |
| Publish Example | `publish_stats.py` (if present) | `publish_stats.ack_v1` |
| Dashboard App | `apps/live_dashboard.py` | (App reads existing keys; no new ctx key) |

Shell Scriptlets:
| Shell | Outputs |
|-------|---------|
| `sanitize_csv.sh` | sanitized CSV path via JSON |
| `enrich_csv.sh` | enriched CSV path via JSON |

---

## 9. JSON Output Contracts

Success (minimal):
```json
{"status":"ok","outputs":["numbers.raw_v1"]}
```
Extended (with artifacts/metrics):
```json
{
  "status":"ok",
  "outputs":["metrics.summary_v2","metrics.summary_v1"],
  "artifact":"orchestrator/Data/report.md",
  "metrics":{"duration_ms":42}
}
```
Error:
```json
{
  "status":"error",
  "reason":"Too few numbers (min=5)",
  "details":{"count":3},
  "exit_code":1,
  "step":"validate_numbers",
  "ts":"2025-09-07T12:10:00Z"
}
```

---

## 10. Recipe Walkthroughs (All)

### 10.1 Reference Examples
1. [orchestrator/recipes/example_numbers.yaml](orchestrator/recipes/example_numbers.yaml)  
   - Single Python step (`compute_numbers.py`) producing `numbers.stats_v1`.
2. [orchestrator/recipes/example_ctx.yaml](orchestrator/recipes/example_ctx.yaml)  
   - Launch context server → run operations client → context keys asserted.

### 10.2 Curriculum Sequence Overview

| Module | Recipe File | Theme | New Concepts |
|--------|-------------|-------|--------------|
| 1 | 01_basic_number.yaml | Foundations | Basic Python step, ctx write |
| 2 | 02_basic_mixed.yaml | Shell Integration | Shell → Python chaining |
| 3 | 03_validation_errors.yaml | Validation & Errors | Structured failure JSON |
| 4 | 04_artifacts_pipeline.yaml | Artifacts & Normalization | File outputs + scaling |
| 5 | 05_metrics_versioned.yaml | Metrics v1 | Versioned metrics discipline |
| 6 | 06_parallel_fan_in.yaml | Parallel fan-in | Independent metric steps + aggregate |
| 7 | 07_cpu_process.yaml | CPU vs I/O | Process pool usage |
| 8 | 08_versioning_wrapper.yaml | Additive Versioning | v2 metrics extending v1 |
| 9 | 09_dashboard.yaml | Live Consumer | Non-blocking visualization |
| 10 | 10_reporting.yaml | Reporting | Template-based Markdown generation |
| 11 | 11_resilience_controls.yaml | Operational Controls | Inject failure + resume strategy |
| 12 | 12_advanced_full.yaml | Full Integration | Caching, parallel, CPU, audit, cleanup |

### 10.3 Per-Recipe Mini Guides

Format:
```
Run: python orchestrator/runner_v2.py --recipe orchestrator/recipes/<file>
Expected Keys: [...]
Failure Modes: ...
```

1. Module 01  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/01_basic_number.yaml`  
   - Keys: `numbers.raw_v1`  
   - Failure: missing source file → error JSON.

2. Module 02  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/02_basic_mixed.yaml`  
   - Keys: `numbers.raw_v1` (post sanitation)  
   - Shell step JSON contract enforced.

3. Module 03  
   - Run (expected failure): `python orchestrator/runner_v2.py --recipe orchestrator/recipes/03_validation_errors.yaml`  
   - Observes structured error from `validate_numbers.py`.

4. Module 04  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/04_artifacts_pipeline.yaml`  
   - Keys: `numbers.normalized_v1`, `numbers.stats_v1`  
   - Artifact: `orchestrator/Data/normalized.json`.

5. Module 05  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/05_metrics_versioned.yaml`  
   - Keys: `metrics.summary_v1`, `metrics.columns_v1`.

6. Module 06  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml`  
   - Fan-in after independent `compute_mean/median/std`.  
   - Keys: `metrics.aggregate_v1`.

7. Module 07  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/07_cpu_process.yaml`  
   - Demonstrates process isolation: `numbers.prime_factors_v1`.

8. Module 08  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/08_versioning_wrapper.yaml`  
   - Keys: `metrics.summary_v1` + `metrics.summary_v2` (backward compatible).

9. Module 09  
   - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/09_dashboard.yaml`  
   - Launch metrics; dashboard app can poll context server or result file (manual start if needed).

10. Module 10  
    - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/10_reporting.yaml`  
    - Keys: `report.path_v1`  
    - Artifact: `orchestrator/Data/report.md`.

11. Module 11  
    - Run (intentional failure first): `python orchestrator/runner_v2.py --recipe orchestrator/recipes/11_resilience_controls.yaml`  
    - Retry with: `--skip inject_failure` or `--resume-from compute_metrics_v2`.

12. Module 12  
    - Run: `python orchestrator/runner_v2.py --recipe orchestrator/recipes/12_advanced_full.yaml`  
    - Composite Keys: caching (`cache.*`), metrics, audit (`resource.audit_v1`), report, cleanup.  
    - Selective execution example:  
      `python orchestrator/runner_v2.py --recipe orchestrator/recipes/12_advanced_full.yaml --only aggregate_metrics generate_report`

### 10.4 Example Output (Module 12 Run Excerpt)

```json
{"status":"ok","outputs":["cache.numbers_hash_v1","cache.numbers_hit_v1"],"cache":"miss"}
{"status":"ok","outputs":["numbers.raw_v1"]}
{"status":"ok","outputs":["numbers.normalized_v1","numbers.stats_v1"],"artifact":"orchestrator/Data/normalized.json"}
{"status":"ok","outputs":["metrics.summary_v1","metrics.summary_v2"]}
{"status":"ok","outputs":["metrics.mean_v1"]}
{"status":"ok","outputs":["metrics.median_v1"]}
{"status":"ok","outputs":["metrics.std_v1"]}
{"status":"ok","outputs":["metrics.aggregate_v1"]}
{"status":"ok","outputs":["numbers.prime_factors_v1"]}
{"status":"ok","outputs":["resource.audit_v1"]}
{"status":"ok","outputs":["report.path_v1"],"artifact":"orchestrator/Data/report.md"}
{"status":"ok","outputs":["cleanup.removed_v1"],"removed":["orchestrator/Data/normalized.json","orchestrator/Data/numbers_enriched.csv"]}
```

---

## 11. Versioning Strategy

Principles:
- Never remove existing keys (e.g., `metrics.summary_v1` persists even when `metrics.summary_v2` added).
- New scriptlet variant rather than altering prior logic: `compute_metrics_v2.py`.
- Downstream recipes relying on v1 remain valid.
- Future: Potential wrappers documenting differences at top of docstring.

---

## 12. Parallel & Processes

Mechanisms (in [orchestrator/runner_v2.py](orchestrator/runner_v2.py)):
- Thread pool for steps with `parallelizable = True`.
- Process pool for steps with `requires_process = True`.
- Fan-in pattern: independent metrics aggregated by a downstream step with `depends_on`.

Recommendations:
- Mark only pure I/O-limited or read-only context consumers as `parallelizable`.
- Use processes for CPU-bound loops (e.g., prime factorization).

---

## 13. Caching Pattern

Implemented in:
- Scriptlet: `cache_numbers.py` generating hash marker in `.cache/`.
- Runner: Recognizes `cache_key` in recipe step → skip execution if marker exists.

Extend by:
- Hashing `(module, args)` deterministic string.
- Recording cache hits (ctx key `cache.numbers_hit_v1`).

---

## 14. Resilience Controls

Flags (runner_v2):
- `--skip <name ...>`: Omit specified steps.
- `--only <subset ...>`: Run only these step names (dependencies must already have state).
- `--resume-from <name>`: Ignore steps prior to named step—assumes prior context is reproducible or not needed.

Use Case:
1. Fail at `inject_failure`.
2. Rerun with `--skip inject_failure`.
3. Or `--resume-from compute_metrics_v2`.

---

## 15. Reporting & Artifacts

Files:
- Template: `templates/report_template.md`
- Output: `Data/report.md`
- Optional Excel placeholder: `templates/metrics_summary.xlsx`

Keys:
- `report.path_v1` (stable reference for downstream archiving)
- Additional workbook export can later register `report.excel_path_v1`

---

## 16. Resource Auditing

- Scriptlet: `resource_audit.py`
- Aggregates per-step metrics captured by runner under `_runner.step_metrics_v1`
- Consolidated key: `resource.audit_v1`
- Downstream: include in reporting or telemetry

---

## 17. Cleanup & Idempotency

- `cleanup_intermediate.py` removes transient artifacts
- Idempotent: repeated runs produce empty removal list
- Justification: Maintains lean artifact directory for long-lived pipelines

---

## 18. Dashboard Integration

App: [orchestrator/scriptlets/python/apps/live_dashboard.py](orchestrator/scriptlets/python/apps/live_dashboard.py)
- Periodic polling of context server or normalized file
- Non-blocking thread pattern
- Extend by embedding a real Dash server; keep runner decoupled

---

## 19. Testing Patterns

Location: `tests/`
- Module-aligned test files (e.g., `test_parallel.py`, `test_cpu_process.py`)
- Approach:
  - Run recipe → assert required keys present
  - Negative tests expect `RuntimeError` on failure steps
- Add new scriptlet:
  1. Unit test (missing param / success path)
  2. Integration test (full recipe)

Command:
```bash
pytest -q
```

---

## 20. Performance & Extension Roadmap

Implemented:
- Basic timing metrics
- Thread pool (parallelizable)
- Process pool (CPU)
- Simple caching (markers)

Roadmap Ideas (aligns with design comments):
- Context merge strategies: `fail_on_conflict`
- DAG visualization asset
- Persistent state file for resume across process invocations
- Hash-based automatic skip using function + args signature
- Memory usage capture in resource decorator
- SQLite persistence layer for historical metric comparisons

---

## 21. Adding a New Scriptlet (Checklist)

1. Create file under `orchestrator/scriptlets/python/steps/<name>.py`
2. CamelCase class extends `BaseScriptlet`
3. Implement `validate` + `run`
4. Use `ctx.set(key, value, who="<script_name>")`
5. Emit success/error JSON exactly once
6. Add recipe entry
7. Write tests
8. If changes break shape → create versioned successor

---

## 22. Troubleshooting Matrix

| Symptom | Likely Cause | Resolution |
|---------|--------------|------------|
| Missing ctx key | Step failed silently | Check stderr logs; confirm JSON printed |
| `ModuleNotFoundError` | Wrong `module` path | Verify file path & name alignment |
| Step re-runs despite cache | Absent `cache_key` or marker removed | Add `cache_key` & confirm `.cache/<key>.done` |
| Parallel race issues | Non-idempotent shared file writes | Serialize operations or separate output targets |
| Process step hang | CPU loop / infinite logic | Add guard conditions / timeouts |
| Report missing sections | `metrics.summary_v2` absent | Ensure v2 step executed before `generate_report` |

---

## 23. Quick Reference Commands

| Task | Command |
|------|---------|
| Run foundational recipe | `python orchestrator/runner_v2.py --recipe orchestrator/recipes/01_basic_number.yaml` |
| Parallel fan-in | `python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml` |
| CPU-bound demo | `python orchestrator/runner_v2.py --recipe orchestrator/recipes/07_cpu_process.yaml` |
| Full advanced pipeline | `python orchestrator/runner_v2.py --recipe orchestrator/recipes/12_advanced_full.yaml` |
| Skip a step | `--skip inject_failure` |
| Only subset | `--only aggregate_metrics generate_report` |
| Resume after failure | `--resume-from compute_metrics_v2` |
| View scriptlet standalone | `python orchestrator/scriptlets/python/steps/compute_metrics_v2.py --params '{}'` |
| Run tests | `pytest -q` |

---

## 24. Example End-to-End Extension Flow

Scenario: Add `compute_range_v1` after normalization.

1. Implement `compute_range.py` → produce `metrics.range_v1`.
2. Update recipe (e.g., `06_parallel_fan_in.yaml`) add step:
```yaml
  - idx: 14
    name: compute_range
    type: python
    module: orchestrator.scriptlets.python.steps.compute_range
    function: ComputeRange
    depends_on: [normalize_numbers]
```
3. Add test asserting presence of `metrics.range_v1`.
4. If extending later with quartiles → create `compute_range_v2.py` adding new keys `metrics.range_extended_v2` while preserving original.

---

## 25. Glossary (Focused)

| Term | Definition |
|------|------------|
| Scriptlet | Atomic executable unit producing JSON stdout |
| Recipe | Ordered YAML definition of steps |
| Context | Shared JSON-safe store across steps |
| Artifact | File generated and referenced by relative path |
| Fan-In | Multiple independent steps whose outputs are aggregated |
| Versioned Key | Namespaced key ending `_vN` preserving backward compatibility |
| Resilience Flags | CLI controls: `--skip`, `--only`, `--resume-from` |
| Caching Marker | File under `.cache/` indicating step output reuse |
| Process Step | Scriptlet flagged with `requires_process=True` |

---

## 26. Curriculum Mastery Checklist

You can:
- Add a new step with docstring + JSON schema in <10 min
- Extend metrics to v3 without breaking v1/v2 consumers
- Introduce parallel steps safely with deterministic fan-in
- Add artifact & surface it in report deterministically
- Resume after injected failure and selectively run subsets
- Integrate long-running visualization non-blockingly
- Demonstrate idempotent cleanup / caching behaviors

---

## 27. Summary

This user guide codifies the progressive curriculum and operational semantics of the automation framework:
- Strict JSON contracts
- Incremental, non-breaking evolution
- Clear layering: ingestion → validation → normalization → metrics → reporting → resilience → optimization
- Reusable patterns ensuring maintainability & testability

Extend further by layering richer caching, dependency visualization, persistent historical analyses, and advanced dashboards—without breaking existing recipes.