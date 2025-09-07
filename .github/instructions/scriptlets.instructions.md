---
applyTo: "orchestrator/scriptlets/**"
---

# Copilot Instructions — Scriptlets focus

When editing or generating scriptlets:
- Prefer creating new wrappers/adapters or versioned scripts over modifying existing helpers; preserve 100% backward compatibility.
- Python steps go to python/steps/ with BaseScriptlet, validate(ctx, params), run(ctx, params), resource/timing decorator, and JSON-only ctx updates.
- Shell steps go to shell/ with a standard wrapper, machine-readable JSON on stdout, and logs to Logs/.
- Every scriptlet is a runnable CLI with argparse (Python) or env vars (shell) and a comprehensive docstring/header comment including usage examples.
- Do not store non-serializable objects in ctx; expose minimal keys for shell via context helpers; log changes for traceability.

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

# Python Step Template and example

```python
"""
NAME: compute_metrics_mem.py
DESCRIPTION: Compute basic metrics from a CSV and store results in ctx['metrics'] and ctx['metrics_columns'].
USAGE:
  python orchestrator/scriptlets/python/steps/compute_metrics_mem.py --params '{"src":"orchestrator/Data/normalized.csv"}'
LIMITATIONS:
  - Input must be a CSV with numeric columns.
  - Writes only JSON-serializable results to ctx.
EXAMPLES:
  python ... --params '{"src":"orchestrator/Data/normalized.csv"}'
RETURNS (stdout JSON):
  {"status":"ok","outputs":["metrics","metrics_columns"]}
"""
import argparse, json
from scriptlets.python.core.base import BaseScriptlet
from scriptlets.python.core.logging import get_logger
from scriptlets.python.core.resource import track_resources
from context import Context

logger = get_logger(__name__)

def _ensure_jsonable(obj):
    json.dumps(obj)  # raises TypeError if not serializable
    return obj

class ComputeMetricsMem(BaseScriptlet):
    def validate(self, ctx, params):
        assert isinstance(params, dict), "params must be dict"
        assert "src" in params and isinstance(params["src"], str), "params.src required"

    @track_resources
    def run(self, ctx, params):
        self.validate(ctx, params)
        src = params["src"]
        # ... read CSV, compute metrics ...
        metrics = {"count": 100, "mean": 1.23}  # JSON-serializable only
        columns = ["count", "mean"]
        _ensure_jsonable(metrics)
        _ensure_jsonable(columns)
        ctx.set("metrics.summary_v1", metrics, who="compute_metrics_mem")
        ctx.set("metrics.columns_v1", columns, who="compute_metrics_mem")
        return {"status": "ok", "outputs": ["metrics.summary_v1", "metrics.columns_v1"]}

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--params", required=True, help="JSON string of parameters")
    args = p.parse_args()
    params = json.loads(args.params)
    out = ComputeMetricsMem().run(Context(), params)
    print(json.dumps(out))

# Shell Step Template and example

```shell
#!/usr/bin/env bash
# NAME: sanitize_csv.sh
# DESC: Sanitize CSV into orchestrator/Data/sanitized.csv
# USAGE: ./orchestrator/scriptlets/shell/sanitize_csv.sh INPUT=orchestrator/Data/clean.csv OUTPUT=orchestrator/Data/sanitized.csv
set -euo pipefail
INPUT="${INPUT:?missing INPUT}"
OUTPUT="${OUTPUT:?missing OUTPUT}"
mkdir -p "$(dirname "$OUTPUT")"
# Single responsibility: sanitize rows
awk -F, 'NR==1 || $0 !~ /forbidden/' "$INPUT" > "$OUTPUT"
# Machine-readable JSON to stdout only
echo "{\"status\":\"ok\",\"output\":\"$OUTPUT\"}"
```