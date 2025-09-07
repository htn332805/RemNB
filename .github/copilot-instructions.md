Purpose & Scope
This repository provides a reusable automation framework for building test sequences, running steps, analyzing results (optionally with AI), persisting to SQLite, generating Excel/DOCX reports, and monitoring live data in Dash apps.

The working directory is ~/automation_framework with orchestrator/ containing scriptlets, recipes, Data, Logs, Assets, templates, tools, and tests to ensure consistent discovery and execution.

The framework is modular and CLI-first: small single-purpose scriptlets, YAML recipes for orchestration, strict JSON ctx, robust logging, and backward-compatible versioning and wrappers for safe evolution.

Quick Start
Python scriptlet: Create in orchestrator/scriptlets/python/steps/<snake_name>.py with a class <CamelCaseName>(BaseScriptlet) implementing validate(ctx, params) and run(ctx, params).

Shell scriptlet: Create in orchestrator/scriptlets/shell/<kebab-name>.sh that accepts inputs via environment variables and prints only JSON to stdout, logging to stderr.

Recipe: Add in orchestrator/recipes/*.yaml that wires steps with idx, name, type, args, depends_on, and success checks; then run the runner with --recipe path and optional --only/--skip/--resume-from.

Golden Rules
Scriptlets are small, single-responsibility CLI tools (Python or shell) and must be independently runnable with machine-readable stdout JSON.

Only JSON-serializable data may live in ctx; update with ctx.set(key, value, who="scriptlet_name") for full traceability (who/what/when).

Keep orchestration (runner/recipes/ctx) separate from step logic (scriptlets), never rely on shared mutable globals.

Preserve backward compatibility: add wrappers/adapters or versioned variants instead of modifying existing helpers or steps in place.

Provide robust logging, error handling, and resource tracking; standardize exit codes (0 success, 1 failed step, 2 dependency not satisfied, 3+ documented custom).

Use threads for I/O-bound work and processes for CPU-bound work; set requires_process=True for CPU-intensive steps.

Add tests for each new utility/step, and integration tests for recipes.

Directory Structure
All framework logic resides under orchestrator/, which contains:

scriptlets/: python/core/, python/steps/, python/apps/, shell/

recipes/: YAML execution plans

Data/, Logs/, Assets/, templates/, tools/: artifacts, logs, templates, admin scripts

tests/: for all unit and integration test scripts

Code Organization
scriptlets/python/core/: shared Python helpers: BaseScriptlet, context, logging, resource tracking

scriptlets/python/steps/: atomic Python scriptlets (single responsibility)

scriptlets/python/apps/: Dash/long-running apps

scriptlets/shell/: atomic shell scriptlets with std wrapper and JSON stdout

recipes/: YAML execution plans

Data/, Logs/, Assets/, templates/, tools/, tests/

Scriptlets (Python)
Location: orchestrator/scriptlets/python/steps/<snake_name>.py

Definition: One class <CamelCaseName>(BaseScriptlet) with validate(ctx, params) and run(ctx, params)

Logging: use Logger, decorate with resource/timing

Context: only JSON-serializable; use namespaced ctx keys and record history via ctx.set(who="...")

CLI: must emit JSON to stdout (machines) and write human logs to stderr/Logs

Scriptlets (Shell)
Location: orchestrator/scriptlets/shell/<kebab-name>.sh, executable, header for NAME, DESC, USAGE, LIMITATIONS, EXAMPLES, RETURNS

Inputs: environment variables or JSON, outputs only JSON to stdout, logs to stderr/Logs

Never mutate ctx directly — use context export utilities for shell

Context (ctx)
Store only JSON-serializable primitives: null, bool, number, string, list, dict

Convert complex types: datetimes to ISO 8601 string, artifacts to Data/ with path/checksum, tabular as list-of-dicts or file path

Traceability: always use ctx.set(key, val, who="step")

Recipes (YAML)
Schema: test_meta (test_id, tester, description) and ordered steps (idx, name, type, cmd/module, args, depends_on, success)

Controls: --only, --skip, --resume-from

Idempotency: Steps should be idempotent when feasible; if not, document why

Runner
Shared in-memory ctx, parallel/async steps, per-step stdout/stderr capture, updates Assets/step_sequence.png, RAM mode via store_on_ram.txt

If store_on_ram.txt present, keep logs in RAM and pickle state as state.pkl; increase log level if “debug” string found

Logging & Error Handling
Logger: centralized, standard levels (DEBUG, INFO, WARNING, ERROR, CRITICAL), per-step logs to Logs/, use resource/timing

Exit codes: 0 success, 1 failed, 2 dependency not satisfied, 3+ custom

Error JSON schema:

json
{
  "status": "error",
  "reason": "...",
  "details": {...},
  "exit_code": 1,
  "step": "name",
  "ts": "MM/DD/YYYY HH:MM:SS"   #sample ts 08/30/2025 21:30:45
}

Performance & Parallelism
ThreadPoolExecutor (I/O bound), ProcessPoolExecutor (requires_process=True for CPU)

Atomic ctx writes: namespaced keys, versioned key suffixes prevent conflict

Efficiency: cache, batch; document any locking/resource needs in docstrings

Dash Apps
Place under scriptlets/python/apps/, avoid blocking runner, use async callbacks, consume ctx or Data/, degrade gracefully, log/retry as needed

Security & Secrets
Never hard-code credentials/tokens; use env vars or config files ignored by VCS

Never log secrets

Redact sensitive info in error/debug context

Environment & Compatibility
Python 3.8+, bash, UTF-8, PEP 8

Prefer standard libs/open-source packages

Type hints required, no global state, prefer context or params, strive for idempotency

Naming & Discovery
Python steps: lower_snake_case, one class, BaseScriptlet

Shell steps: lower-kebab-case, doc header, executable

Discovery: auto-registry for python, list/sh-validate for shell

Backward Compatibility & Versioning
Never break existing helpers/scriptlets/core APIs

Wrappers/adapters/versioned variants for any change (e.g., normalize_csv_v2.py)

Document wrappers/adapters: what is extended, limitations, new behaviors, versioning

Only retire after all usages have migrated

Testing & CI
Add tests for all new/changed scriptlets and utilities

Integration tests for recipes/runner

Mock data under tests/ for deterministic/faster CI pipelines

Acceptance Checklist
Structure: directory, naming, single responsibility

Interface: CLI, JSON parameters/output

Ctx: JSON-serializable, namespaced, who/when/traceable

Quality: logging, error handling, resource tracking, exit codes, tests

Compatibility: wrappers/versioned as needed; no breaking changes

Anti-patterns
Do not store DataFrames, NumPy arrays, datetime objects, open files/sockets/DB cursors/Path in ctx

No multi-purpose steps or hidden outputs/side effects

No unstructured logs to stdout

No in-place rewrites of helpers; always create wrapper/versioned

Appendix: Standard Result/Output Schema (JSON)
Success:

json
{
  "status": "ok",
  "outputs": [...],
  "artifacts": [...],
  "metrics": {...},
  "warnings": [...]
}
Only JSON-serializable values and relative file paths.

Error:

json
{
  "status": "error",
  "reason": "...",
  "details": {...},
  "exit_code": 1,
  "step": "name",
  "ts": "YYYY-MM-DDTHH:MM:SSZ"
}
No secrets exposed.

Appendix: Python Scriptlet Template
```python
"""
NAME: compute_metrics_mem.py
DESCRIPTION: Read Data/normalized.csv, compute metrics, write to ctx['metrics'] and ctx['metrics_columns'].
USAGE:
  python orchestrator/scriptlets/python/steps/compute_metrics_mem.py --params '{"src":"orchestrator/Data/normalized.csv"}'
PARAMETERS:
  - src: str, path to CSV under orchestrator/Data/
RETURNS:
  {"status":"ok","outputs":["metrics","metrics_columns"]}
LIMITATIONS:
  - Input must be a CSV with numeric columns.
  - Outputs must be JSON-serializable.
EXAMPLES:
  python orchestrator/scriptlets/python/steps/compute_metrics_mem.py --params '{"src":"orchestrator/Data/normalized.csv"}'
"""
import argparse, json, sys
from scriptlets.python.core.base import BaseScriptlet
from scriptlets.python.core.logging import get_logger
from scriptlets.python.core.resource import track_resources
from context import Context

logger = get_logger(__name__)

class ComputeMetricsMem(BaseScriptlet):
    def validate(self, ctx, params):
        if not isinstance(params, dict):
            raise ValueError("params must be dict")
        if "src" not in params or not isinstance(params["src"], str):
            raise ValueError("params.src required and must be str")

    @track_resources
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            src = params["src"]
            # Load CSV, compute metrics (placeholder)
            metrics = {"count": 100, "mean": 1.23}
            metrics_columns = ["count", "mean"]
            json.dumps(metrics)
            json.dumps(metrics_columns)
            ctx.set("metrics.summary_v1", metrics, who="compute_metrics_mem")
            ctx.set("metrics.columns_v1", metrics_columns, who="compute_metrics_mem")
            result = {"status": "ok", "outputs": ["metrics.summary_v1", "metrics.columns_v1"]}
            print(json.dumps(result))
            return 0
        except Exception as e:
            err = {"status": "error", "reason": str(e), "exit_code": 1, "step": "compute_metrics_mem"}
            print(json.dumps(err))
            return 1

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--params", required=True, help="JSON string of parameters")
    args = parser.parse_args()
    params = json.loads(args.params)
    ctx = Context()
    sys.exit(ComputeMetricsMem().run(ctx, params))
```
Appendix: Shell Scriptlet Template
```bash
#!/usr/bin/env bash
# NAME: sanitize_csv.sh
# DESC: Sanitize CSV into orchestrator/Data/sanitized.csv
# USAGE: ./orchestrator/scriptlets/shell/sanitize_csv.sh INPUT=orchestrator/Data/clean.csv OUTPUT=orchestrator/Data/sanitized.csv
set -euo pipefail

INPUT="${INPUT:?missing INPUT}"
OUTPUT="${OUTPUT:?missing OUTPUT}"
mkdir -p "$(dirname "$OUTPUT")"
awk -F, 'NR==1 || $0 !~ /forbidden/' "$INPUT" > "$OUTPUT"
printf '{"status":"ok","output":"%s"}\n' "$OUTPUT"
```
Appendix: Recipe YAML Snippet
```yaml
 orchestrator/recipes/default_recipe.yaml (excerpt)
test_meta:
  test_id: "T-001"
  tester: "Alice"
  description: "Example pipeline"
steps:
  - idx: 1
    name: preprocess_csv
    type: shell
    cmd: orchestrator/scriptlets/shell/preprocess_csv.sh
    args:
      INPUT: "orchestrator/Data/raw.csv"
      OUTPUT: "orchestrator/Data/clean.csv"
    success:
      files_exist:
        - "orchestrator/Data/clean.csv"
  - idx: 2
    name: compute_metrics_mem
    type: python
    module: scriptlets.python.steps.compute_metrics_mem
    function: ComputeMetricsMem
    args:
      src: "orchestrator/Data/clean.csv"
    depends_on: [preprocess_csv]
    success:
      ctx_has_keys:
        - "metrics.summary_v1"
        - "metrics.columns_v1"
```
Enforcement
Only contributions that match these conventions are accepted by discovery utilities and CI; otherwise, they are excluded from the main repository until corrected.

Notes to Copilot & Contributors:

Prefer wrappers or versioned variants for any behavior change, always emit machine-readable JSON, and surface warnings when a suggestion risks breaking these rules.

Keep template comments practical and concise, and always link steps to tests using minimal reproducible mock fixtures.