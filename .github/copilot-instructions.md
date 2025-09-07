# Copilot Instructions — Automation Framework

Purpose
- This repository hosts a reusable automation framework for building test sequences, automating execution, analyzing results with AI, persisting to SQLite, generating Excel and DOCX reports, and monitoring live data via Dash apps.
- Working directory: ~/automation_framework (orchestrator/ holds scripts, recipes, Data, Logs, Assets, templates, and tools).

Golden Rules (must follow)
- Scriptlets are independent CLI tools (Python or shell), small, composable, and single-responsibility.
- Every scriptlet has a top-level docstring with description, usage examples, parameters, return values, limitations/requirements, and line-by-line comments in code.
- Only JSON-serializable content may be written to ctx; never place non-serializable objects in ctx; log all ctx changes.
- Maintain strict separation between orchestration logic (runner, context) and step logic (scriptlets).
- Preserve backward compatibility: prefer wrappers/adapters or new versions over edits to existing helpers; do not rewrite existing helpers/snippets.
- Provide robust logging, error handling, and resource tracking; standardize exit codes (0 success, 1 failed step, 2 dependency not satisfied).
- Use async/threads/processes appropriately; heavy compute can declare requires_process=True to run out-of-process.
- Add tests for new utilities and steps; provide integration tests for recipes.

Key Locations (required)
- orchestrator/scriptlets/python/core/ — core utilities, ctx helpers, logging, resource tracking
- orchestrator/scriptlets/python/steps/ — atomic Python steps
- orchestrator/scriptlets/python/apps/ — Dash and long-running apps
- orchestrator/scriptlets/shell/ — shell steps
- orchestrator/recipes/ — YAML execution plans
- orchestrator/Data/, orchestrator/Logs/, orchestrator/Assets/, orchestrator/templates/, orchestrator/tools/

Runner/Context Expectations
- runner.py: Main pipeline runner with shared in-memory ctx, parallel/async steps, per-step stdout/stderr capture to Logs/<timestamp>_<step>.log, success checks (files_exist, ctx_has_keys), Assets/step_sequence.png updates, and RAM mode via store_on_ram.txt.
- context.py: Context implementation exposes selected keys to shell via environment; changes are type-checked, versioned, timestamped; retains who/what/when/old/new history with ctx.get_history(key) and ctx.last_modified(key).

Recipe Schema (YAML)
- test_meta: test_id, tester, description (persisted and used in reports)
- steps: list with fields: idx, name, type: (python|shell), how to run (module/function for Python, cmd for shell), args/params, depends_on, success (files_exist, ctx_has_keys)
- On-the-fly controls: --only, --skip, --resume-from; edits to idx reorder execution.

Code Generation Conventions — Python Step
- Create files in orchestrator/scriptlets/python/steps/<snake_name>.py.
- Implement class <CamelCaseName>(BaseScriptlet) with validate(ctx, params) and run(ctx, params) methods.
- Use a decorator for timing and resource tracking (psutil + tracemalloc).
- Use the centralized Logger utility for step-level and global logging.
- Update ctx only with JSON-serializable dict/list/str/number; use ctx.set(key, value, who="step_name") to record history and timestamps.
- Provide CLI entry: python <file>.py --params '{...}' and emit only JSON to stdout for machine use; human logs go to stderr or Logger.

Boilerplate — Python Step (template)

\"\"\" 
NAME: compute_metrics_mem.py
DESCRIPTION: Read Data/normalized.csv, compute metrics, and write to ctx['metrics'] and ctx['metrics_columns'].
USAGE:
  # CLI (independent of runner)
  python orchestrator/scriptlets/python/steps/compute_metrics_mem.py --params '{"src":"orchestrator/Data/normalized.csv"}'
LIMITATIONS:
  - Input must be a CSV with numeric columns.
  - Outputs are JSON-serializable only.
\"\"\"

import argparse, json
from scriptlets.python.core.base import BaseScriptlet
from scriptlets.python.core.logging import get_logger
from scriptlets.python.core.resource import track_resources
from context import Context

logger = get_logger(__name__)

class ComputeMetricsMem(BaseScriptlet):
    def validate(self, ctx, params):
        assert isinstance(params, dict), "params must be dict"
        assert "src" in params, "params.src required"

    @track_resources
    def run(self, ctx, params):
        self.validate(ctx, params)
        src = params["src"]
        # Load, compute metrics...
        metrics = {"count": 100, "mean": 1.23}
        metrics_columns = ["count", "mean"]
        ctx.set("metrics", metrics, who="compute_metrics_mem")
        ctx.set("metrics_columns", metrics_columns, who="compute_metrics_mem")
        return {"status": "ok", "outputs": ["metrics", "metrics_columns"]}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--params", required=True, help="JSON string of parameters")
    args = parser.parse_args()
    params = json.loads(args.params)
    ctx = Context()
    out = ComputeMetricsMem().run(ctx, params)
    print(json.dumps(out))

Code Generation Conventions — Shell Step
- Create files in orchestrator/scriptlets/shell/<kebab-name>.sh with a standard wrapper for env loading, logging, and exit codes.
- Accept inputs via environment variables or JSON params; write machine-readable JSON to stdout only; logs to stderr or Logs/.
- Never mutate ctx directly; use expose_for_shell utilities through the Context export path.

Boilerplate — Shell Step (template)

#!/usr/bin/env bash
# NAME: sanitize_csv.sh
# DESC: Sanitize CSV into orchestrator/Data/sanitized.csv
# USAGE: ./orchestrator/scriptlets/shell/sanitize_csv.sh INPUT=orchestrator/Data/clean.csv OUTPUT=orchestrator/Data/sanitized.csv
set -euo pipefail
INPUT="${INPUT:?missing INPUT}"
OUTPUT="${OUTPUT:?missing OUTPUT}"
mkdir -p "$(dirname "$OUTPUT")"
awk -F, 'NR==1 || $0 !~ /forbidden/' "$INPUT" > "$OUTPUT"
echo "{\"status\":\"ok\",\"output\":\"$OUTPUT\"}"

Boilerplate — Recipe Step (YAML snippet)

# orchestrator/recipes/default_recipe.yaml (excerpt)
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

Testing & CI
- Add unit tests under tests/ for core utilities and steps; add integration tests for key recipes.
- Provide a CLI to list available steps with auto-discovered registry and emit markdown docs for each.

Performance & Parallelism
- Use ThreadPoolExecutor for IO-bound tasks; allow requires_process=True to route heavy compute to a ProcessPoolExecutor.
- Cache repeated expensive operations and batch where feasible.

Dash Apps
- Place apps in orchestrator/scriptlets/python/apps/; use async callbacks when possible; do not block the runner; consume ctx or Data/ artifacts.

Operational Notes
- Respect store_on_ram.txt: when present, keep logs/state in RAM and pickle GLOBAL_STATE as state.pkl; if content includes "debug", increase verbosity.
- Always emit per-step logs to orchestrator/Logs/, update Assets/step_sequence.png after each step, and ensure clean exits (0/1/2).

## Scriptlet Modularity & JSON-ctx Guardrails

Goals
- Keep every scriptlet small, single-purpose, and independently runnable from the CLI.
- Enforce strict JSON-serializable ctx usage, with clear traceability of changes and zero leakage of non-serializable objects.

Design Rules (must follow)
- Single responsibility: each scriptlet performs one concrete task end-to-end (e.g., “normalize CSV,” “compute metrics”). If scope grows, split into multiple steps and compose via recipes.
- Independent CLI: every scriptlet runs directly via CLI (argparse for Python, env vars for shell) and prints machine-readable JSON to stdout; human-friendly logs go to stderr or the Logger.
- Composition over mutation: avoid shared globals; read inputs via params/ctx keys; write outputs via ctx.set with who/what/when tracking; do not depend on in-memory globals between runs.
- Backward compatibility: never rewrite core helpers; create wrappers or versioned variants (e.g., my_helper_v2.py) and preserve 100% behavior of existing helpers used by current recipes and Dash apps.
- Idempotent when feasible: re-running produces the same outputs for the same inputs, unless the step is explicitly time- or randomness-dependent (document such cases in the docstring).

JSON-Serializable ctx (strict)
- Only store JSON-safe primitives: null, bool, number, string, list, dict. No custom classes, generators, open files, sockets, DB cursors, NumPy arrays, Pandas DataFrames, or datetime objects.
- Convert complex types:
  - datetime → ISO 8601 string (UTC) like "2025-09-07T00:00:00Z".
  - bytes/binary → base64 or save to Data/ and store a file path + checksum in ctx.
  - DataFrame/array → list[dict] or file path to CSV/Parquet; never store the object itself.
- Reference large artifacts by path: place files in orchestrator/Data/ and keep ctx values as small summaries, checksums, schemas, or relative paths.
- Enforce round-trip safety: any ctx value must survive json.dumps + json.loads with no loss of meaning; add simple serializability checks in validate(ctx, params).
- Namespacing: use descriptive keys like "metrics.summary_v1", "files.normalized_csv_path", "ai.summary_md". Prefer version suffixes (…_v2) for breaking changes.
- Traceability: update via ctx.set(key, value, who="scriptlet_name") so history and last_modified are accurate; never mutate nested JSON in place without re-setting the whole value.

I/O and Logging
- Stdout: only final JSON result (e.g., {"status":"ok","outputs":["metrics","metrics_columns"]}).
- Stderr/Logger: progress, timing, resource usage, and human-readable diagnostics.
- Per-step logs must flow to orchestrator/Logs/; use the standard Logger utility and the resource/timing decorator.

Acceptance Criteria (for Copilot-generated code)
- The scriptlet:
  - Is placed in the correct folder (python/steps or shell/) and is single-purpose.
  - Has a top docstring/header with description, usage, params, returns, limitations, examples.
  - Validates inputs and ensures any ctx writes are JSON-serializable.
  - Uses ctx.set with who="…" and returns only JSON-serializable structures.
  - Emits machine-readable JSON to stdout; logs to stderr/Logger; no prints of Python objects to stdout.
  - Includes lightweight tests (unit for function logic, integration for recipe wiring when applicable).

Anti-Patterns (reject or refactor)
- Storing DataFrame/NumPy/datetime/Path objects directly in ctx.
- Writing multi-purpose “kitchen sink” scriptlets with many optional behaviors.
- Implicit side effects (hidden file writes, hidden environment mutations).
- Tight coupling to runner internals; scriptlet should not assume how it’s scheduled.
- Rewriting helpers; prefer small wrapper functions or versioned modules.

Refactoring Playbook
- If a step grows: split by concerns (I/O, transform, summarize) into separate steps.
- If non-serializable data is needed by multiple steps: persist as file in Data/ and store a compact JSON-safe manifest in ctx.
- If an API change is needed: add a v2 wrapper that adapts inputs/outputs; keep v1 intact for old recipes.

---
applyTo: "orchestrator/scriptlets/**"
---

# Scriptlet Modularity & JSON-ctx Enforcement

When generating or editing scriptlets under orchestrator/scriptlets/**:

Modularity
- Implement one focused operation per scriptlet; if multiple tasks appear, instruct to split and compose via recipes.
- Provide a CLI interface and comprehensive docstring (usage, params, returns, limitations, examples).
- Favor wrappers/adapters or versioned files over edits to existing helpers; keep backward compatibility.

JSON-Serializable ctx
- Store only JSON-safe values (null, bool, number, string, list, dict). Convert datetimes to ISO 8601 (UTC), binaries to base64 or persisted file paths.
- Keep large data off ctx; write to orchestrator/Data/ and store relative paths, sizes, and checksums in ctx.
- Use ctx.set(key, value, who="scriptlet_name"). Do not store non-serializable objects; verify via a quick json.dumps round-trip before setting.

I/O and Logs
- Print only final JSON to stdout; send diagnostics to stderr/Logger; include resource/timing via the standard decorator.
- Ensure per-step logs integrate with orchestrator/Logs/ conventions.

Checklist (must pass)
- [ ] Single-responsibility and independent CLI runnable
- [ ] Docstring with description, usage, params, returns, limitations, examples
- [ ] ctx writes are JSON-serializable (validated)
- [ ] Uses ctx.set(..., who="…") with meaningful, versioned keys
- [ ] Outputs minimal JSON to stdout; logs to stderr/Logger
- [ ] No edits to core helpers without wrapper/versioning

## Directory Structure & Key Locations

- `orchestrator/scriptlets/python/core/` — core Python utilities, context, logging, resource tracking
- `orchestrator/scriptlets/python/steps/` — atomic, single-responsibility step implementations in Python
- `orchestrator/scriptlets/python/apps/` — long-running and Dash apps
- `orchestrator/scriptlets/shell/` — atomic shell scriptlets
- `orchestrator/recipes/` — YAML workflow definitions
- `orchestrator/Data/` — persistent input/output artifacts, always referred to by recipe/scriptlet (never implicit)
- `orchestrator/Logs/` — per-step, per-run logs
- `orchestrator/Assets/` — execution graphs, illustrations
- `orchestrator/templates/` — Excel, report, docx, etc. templates
- `orchestrator/tools/` — admin utilities (e.g., cleaning scripts)
- `tests/` — CLI/unit/integration tests for all core utilities and steps

---

## Mandatory Scriptlet Guidelines

**Modularity & CLI-First:**
- Every scriptlet (Python or shell) is a small, composable, CLI-runnable unit, focused on a single task only. No monolithic or multipurpose scriptlets.
- All scriptlets include a usage-ready CLI interface: argparse (Python) or environment variables (shell). Print only JSON to `stdout`; all logs go to `stderr` or the framework logger.
- Place Python steps in `orchestrator/scriptlets/python/steps/` and shell in `orchestrator/scriptlets/shell/`.

**Documentation:**
- Begin each scriptlet with a detailed docstring/header block including:
    - Name, description, param/argument schema
    - Usage example(s)
    - Returns and success criteria
    - Limitations/requirements

**ctx Usage (JSON-Serializable Only):**
- Only JSON-serializable data may be stored in or returned via `ctx` (dict, list, str, int, float, bool, None).
- Never store classes, NumPy arrays, DataFrames, datetime objects, open files, sockets, or other non-serializable objects in `ctx`.
- Convert complex types as follows:
    - datetimes as `"2025-09-07T00:00:00Z"` (ISO 8601 string, UTC)
    - binary/large artifacts written to `Data/`, referenced in `ctx` by relative path/checksum
    - tabular data as list-of-dicts or persisted CSV/Parquet path, plus schema if needed
- Always validate serializability before saving to `ctx` using `json.dumps`.
- Use descriptive, namespaced keys (e.g., `"metrics.summary_v1"`, `"files.normalized_csv_path"`) and add new version-suffixed keys on breaking changes.

**Traceability:**
- All writes to `ctx` must use `ctx.set(key, value, who="scriptlet_name")` to enable change tracking/history. Never mutate nested objects in place.
- Ensure all `ctx` modifications are time-stamped and leave a last-modified trail; review via `ctx.get_history(key)` where available.

**Recipe Structure:**
- All recipes are YAML files in `orchestrator/recipes/` containing:
    - `test_meta` block: `test_id`, `tester`, `description`
    - Ordered `steps` with: `idx`, `name`, `type` (`shell` or `python`), `cmd` or module, `args`/params, `depends_on`, `success` (e.g. `files_exist`, `ctx_has_keys`).
- Steps declare dependencies using `depends_on` and check results via `success` (no implicit ordering).
- Command-line runner supports: `--recipe`, `--only`, `--skip`, `--resume-from`, and must log each run for reproducibility.

**Error Handling, Logging, and Resource Usage:**
- Use the centralized Logger utility (`orchestrator/scriptlets/python/core/logging.py`) for all logs, and decorate each step with the provided resource/timing decorator (`track_resources`).
- Errors must be explicit with meaningful exit codes: `0` success, `1` step failed, `2` dependency unsatisfied, additional codes as documented.
- Heavy computation can request isolation via `requires_process=True` (runs in a process pool).
- Always emit step and runner logs into `orchestrator/Logs/` and update `Assets/step_sequence.png` after each run.

**Backward Compatibility:**
- Never rewrite or break existing helpers or scriptlets. Add new wrappers or version-suffixed variants if you need to change behavior.
- All extensions and wrappers must preserve 100% behavior/compatibility for old and new code/recipes.
- Only expand or override; never refactor in-place unless explicitly allowed.

**Testing & CI:**
- Add and update unit test scripts in `tests/` for every new or changed scriptlet, and for any shared utility.
- Add integration tests for recipes and orchestrator logic.
- Use mock data in `tests/` and avoid reliance on external state.

**Documentation & Discoverability:**
- Every scriptlet, recipe, and utility must include usage and limitation documentation.
- Automatically document available scriptlets and their parameters; provide a CLI to list all available steps and show their help.
- Maintain and update this copilot-instructions.md to match changes in framework expectations.

---

## Generation Acceptance Checklist

Any code, snippet, or change suggested by Copilot **must**:

- Adhere to the prescribed directory structure and naming conventions
- Be a modular, single-purpose function, scriptlet, or recipe
- Only store/read JSON-serializable data in `ctx`, with full traceability on all keys/values
- Supply and validate a command-line or environment-variable interface with machine-readable output
- Attach comprehensive documentation and usage examples in every scriptlet and recipe
- Be extensible and backward compatible by design, favoring wrappers/adapters/versioning over breaking edits
- Use robust error handling, resource tracking, and standardized exit codes/logs
- Provide (where relevant) sample/additional test coverage in `tests/`

---

## Anti-patterns to Avoid

- Storing non-serializable objects in `ctx`
- Bundling multiple responsibilities or CLI modes in a single scriptlet
- Implicit outputs or hidden file/environment mutations
- Editing core helpers in-place instead of versioning
- Printing unstructured data (Python objects, logs) to stdout
- Missing or unclear usage examples/limitations in documentation

---
## Summary

- Copilot, follow these repository conventions for every generated script, recipe, and document, and always surface warnings if a suggestion would break the above rules.
- Use the provided directory layout, CLI entrypoints, JSON serialization conventions, and traceable context for every contribution.

## Parallelism, Logging, and Error Handling

### Parallelism

- Steps in a recipe may be executed in parallel if dependencies permit:
    - The runner uses `ThreadPoolExecutor` for I/O-bound steps and `ProcessPoolExecutor` for CPU-bound work.
    - Each step can declare `requires_process=True` to run safely in its own process.
    - Avoid non-threadsafe or mutable shared state. Scriptlets must not rely on global variables.
    - Parallel steps must operate independently and maintain idempotent result generation (same input yields same output).
    - If a step manages heavy computation or system resources, use explicit serial/release resource patterns and add notes in the scriptlet documentation.
    - All ctx writes must be atomic—conflicting writes are not allowed. Use step-specific keys and versioning if multiple steps produce related outputs.

### Logging

- All diagnostic and human-readable step/process logs must use the central Logger utility provided in `orchestrator/scriptlets/python/core/logging.py`.
    - Write all logs to `stderr` or Logger, never unstructured logs to `stdout`.
    - Append per-step logs to `orchestrator/Logs/<timestamp>_<step>.log` for every recipe run.
    - Critical errors, resource usage, and diagnostic metrics are always logged—with step name, parameters, and timestamp.
    - Use standard logging levels: `DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`.
    - Add a decorator for function timing and system resource usage (`track_resources`) for all step implementations.
    - When `store_on_ram.txt` contains “debug”, increase log verbosity for tracing and debugging complex issues.
    - Do not echo sensitive information or secrets in public logs.

### Error Handling

- Error handling and exception flow are standardized:
    - Catch and log all exceptions within scriptlets and steps, then emit a clear exit code and structured error message.
    - Output only JSON-formatted error or result info to `stdout`:
        - Success: `{"status":"ok",...}`
        - Failure: `{"status":"error","reason":"...","details":...}`
    - Use documented exit codes:
        - `0`: Successful execution
        - `1`: Step execution failed
        - `2`: Dependency not satisfied
        - `3+`: Custom failure codes (documented by scriptlet)
    - Avoid terminating the runner on the first error; log, update ctx with failure info, and continue if the recipe design allows.
    - For shell scriptlets, rely on `set -euo pipefail`, explicit exit codes, and always print a machine-readable JSON error summary.
    - Always propagate clear error context (step name, parameters, stack trace snippet if relevant) for downstream debugging.
    - Mark failed outputs in both the logs and ctx (e.g., `ctx.set("sanitization.status", "failed", who="sanitize_csv_sh")`).
    - Prefer specific custom exceptions for predictable error cases. Fall back to `RuntimeError` only for unexpected failures.
    - Validate all inputs before main execution—fail fast, log reason, avoid half-finished outputs.

---

### Guidance for Copilot

- When generating steps that support parallelism, always encapsulate resources and state, and explain any caveats in scriptlet documentation.
- Every scriptlet and step must use Logger-based logging and wrap core logic in comprehensive error-handling constructs.
- All scriptlets and recipes must make use of standardized exit codes and machine-readable error info, rejecting unhandled exceptions or ambiguous output flows.
- Add comments and structured logging for areas involving parallelism or race condition risks, so maintainers can easily troubleshoot or extend.

---

**Always apply these guidance points—in addition to all previous modularity and serializability rules—when Copilot generates any code, doc, or recipe for this automation framework.**

## Directory Structure & Naming Conventions for Scriptlet Discovery

### Top-Level Structure

- All framework logic is within `~/automation_framework/orchestrator/` or relative paths below.
- Scriptlets, core utilities, apps, data artifacts, templates, and recipes are always separated into the following locations:

orchestrator/
│
├── scriptlets/
│ ├── python/
│ │ ├── core/ # Shared helpers, BaseScriptlet, context, logging, resource
│ │ ├── steps/ # Atomic, single-responsibility Python scriptlets
│ │ └── apps/ # Long-running and Dash-based applications
│ └── shell/ # Atomic shell scriptlets (bash, sh, etc.)
│
├── recipes/ # All YAML pipeline/recipe definitions
│
├── Data/ # Shared or output data artifacts (CSV, JSON, Excel, etc.)
├── Logs/ # All logs and per-step run logs
├── Assets/ # Execution DAG graphs, visualizations
├── templates/ # Report/excel/etc. templates
├── tools/ # Housekeeping and admin scripts (e.g., clean_pristine.sh)
└── tests/ # All unit and integration test scripts

text

---

### Naming Conventions

- **Python scriptlets (steps):**
    - Located in `orchestrator/scriptlets/python/steps/`
    - Use lower_snake_case for file names, e.g. `normalize_csv.py`, `compute_metrics_mem.py`
    - Each file must define one main class named in UpperCamelCase and inheriting from `BaseScriptlet`, e.g. `NormalizeCsv`, `ComputeMetricsMem`
    - Avoid numbers or version suffixes in the initial file name—if a new version is required for backwards compatibility, use a suffix (`normalize_csv_v2.py`) and update imports and recipes accordingly.

- **Shell scriptlets:**
    - Located in `orchestrator/scriptlets/shell/`
    - Use lower-kebab-case for file names, e.g. `sanitize-csv.sh`, `preprocess-raw.sh`
    - Each file must be executable with `./<name>.sh` and begin with a descriptive header comment for docs/discovery.

- **Core utilities:**
    - Located in `orchestrator/scriptlets/python/core/`
    - Use lower_snake_case for modules: `base.py`, `context.py`, `logging.py`, `resource.py`
    - Helpers for discovery should scan core/ and steps/ for all possible scriptlets conforming to the BaseScriptlet interface or executable shell scriptlets.

- **Dash and long-running apps:**
    - Located in `orchestrator/scriptlets/python/apps/`
    - Use lower_snake_case for apps: `live_dashboard.py`

---

### Scriptlet Discovery & Documentation

- Python scriptlets in `steps/` must register with an auto-discovery registry (e.g., via module entry-points, decorators, or directory scan).
- Shell scriptlets are discovered by listing all `*.sh` in shell/; only scripts with a valid doc header and executable flag are shown in CLI listing.
- Each scriptlet (Python or shell) must supply a docstring/header block with:
    - NAME, DESCRIPTION, USAGE, LIMITATIONS, EXAMPLES, and expected RETURNS.
- Core helpers and discovery utilities should auto-generate documentation and provide a CLI to list all available scriptlets with usage/help.
- Only scriptlets conforming to these conventions are included in live dashboards, runner selection menus, or doc generators.

---

### Additional Notes

- Never place business logic or tests outside this structure—ad hoc, test, or demo scripts must go in `tests/`.
- Place wrappers, adapters, and new versions next to (not in place of) existing implementations with a clear version suffix.
- All directories and files must use ASCII-safe names and avoid spaces or special symbols for full cross-platform compatibility.

---

**Enforcement:**  
Copilot and contributors must strictly follow these directory and naming conventions. Any code, recipe, or documentation not matching these patterns will not be discoverable by auto-registry utilities or the orchestrator and should not be accepted in the main repo.

## Backward Compatibility and Wrapper Usage Rules

**1. Do Not Break Existing Code**

- Never modify or remove existing helpers, scriptlets, or core APIs in a way that would break current recipes, runner functionality, Dash apps, or integration tests.
- All updates or modifications must maintain the ability for legacy workflows, recipes, and interfaces to work as before, without mandatory update or rewrite.

**2. New Features via Wrappers/Adapters, Not In-Place Edits**

- If behavior change or extension is needed for any scriptlet, helper, or API:
    - **Do NOT** rewrite or edit the original code directly.
    - Instead, create a new wrapper or adapter that calls the original, extends functionality, or changes behavior as needed.
    - If a substantial change is required, implement a new versioned file, e.g., `my_helper_v2.py` or `normalize_csv_v2.py`, and update only the new recipes or steps to use this. Legacy scripts remain untouched.
    - Use Python decorators or context manager wrappers to inject new logging, validation, or error handling onto existing functions without altering the originals.

**3. Naming and Documentation for Wrappers**

- Wrappers and adapters should be clearly named (e.g., `my_helper_wrapper.py`, `normalize_csv_v3.py`) and reside alongside the original implementation.
- Always document in the wrapper’s docstring:
    - Which original helper or scriptlet is being adapted
    - Nature of the extension or override
    - Any differences in parameters, outputs, or side effects
    - Limitations and backward compatibility guarantees

**4. Versioning Conventions**

- When breaking changes are required (argument order, behavior, side effect, returned data, etc.), create a new file with an incremented version suffix.
    - e.g., `compute_metrics_mem_v2.py`
- All versioned files must have clear docstrings and usage examples (especially what has changed from previous versions).
- Recipes and step definitions should declare explicitly which version of a scriptlet/helper they call.
- Retire old versions only if all recipes and apps have migrated—otherwise, maintain all used legacy versions.

**5. Discovery and Registration**

- All wrappers and new versions must be registered with the auto-discovery mechanism, just like originals.
- Do not hide or remove old registrations unless the wrapper is a pure, backward-compatible superset.

**6. Extending Core Helpers**

- If updates to a core helper are required for new features or bugfixes:
    - Minimize the patch footprint—prefer subclassing, decorator-based enhancement, or external function wrapping.
    - Extend with minimal new code—inheriting all old behaviors by default.
    - Comprehensive tests for both old and new behaviors are required.

**7. When Direct Modification is Allowed**

- Only make in-place edits to old helpers when correcting critical security, data integrity, or unrecoverable bugs, and only if wrappers cannot resolve the issue.
- All such edits must be clearly documented and announced for all users of the codebase.

---

**Copilot/Contributors:**  
- Always propose wrappers, versioned copies, or adapters for enhancements or changes—not rewrites.
- When in doubt, preserve legacy code as primary and offer new features as opt-in upgrades.
- Changes that break recipes or Dash apps without explicit version migration are not allowed.

## Backward Compatibility and Wrapper Usage Rules

- **Never modify or remove existing helpers, scriptlets, or core APIs in ways that break existing recipes, runner logic, or downstream apps.**
- For any change that alters behavior, parameters, results, or side effects:
    - **DO NOT** rewrite the original scriptlet/helper.
    - **ALWAYS** create a new wrapper, decorator, or versioned scriptlet (e.g., `normalize_csv_v2.py`).
    - The wrapper should reside alongside the original. Name clearly (e.g., `my_helper_wrapper.py`, `my_scriptlet_v3.py`). Update only new recipes or steps to use it.
- Retain the full interface and functionality of old helpers. Never swap or remove legacy versions unless/until all usages are migrated.
- Document all wrappers/adapters: which original implementation is adapted, what is extended/overridden, any new parameters or behaviors, backward compatibility notes.
- Only update original files directly for critical security/data-integrity hotfixes—document this exception in the commit and pull request.
- All wrappers/versions must register for scriptlet discovery and appear in any CLI or auto-generated help lists.
- When new features or fixes are needed, prefer decorator/context manager patterns to add logging or pre/post logic around the unmodified core function.
- **Acceptance:** Copilot/completion suggestions must never break or remove old logic, should always offer opt-in upgrades (via wrappers/versions), and leave legacy workflows 100% operable.

