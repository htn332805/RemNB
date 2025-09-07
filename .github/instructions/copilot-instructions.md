# Automation Orchestrator – Assistant Authoring Guidelines (Trimmed)

Canonical, source‑of‑truth engineering specification: [.github/prompts/automation.prompt.md](.github/prompts/automation.prompt.md)  
(This file is a lightweight layer for editor assistants to avoid duplicating the full spec. Update the canonical file first; only adjust this mirror to reflect high‑level changes.)

---

## 1. Purpose (Assistant Scope)
Provide precise, minimal, non‑divergent code or documentation changes that:
- Conform to the canonical spec
- Preserve backward compatibility (versioned keys `_vN`)
- Emit exactly one JSON result line per scriptlet execution
- Avoid speculative features not yet implemented

---

## 2. Core Generation Rules (Authoring Fast Reference)
| Topic | Rule |
|-------|------|
| Output | One JSON line to stdout: success or error schema (see §3) |
| Keys | `namespace.entity_vN` (add new version instead of mutating) |
| Context Writes | Always `ctx.set(key, value, who="<script_name>")` |
| Validation | Perform before side effects; raise early |
| Stdout vs Stderr | Only the final JSON on stdout; all logs to stderr |
| Scriptlet Class | One public class extending `BaseScriptlet` |
| Shell Steps | `set -euo pipefail`; same one‑line JSON contract |
| Parallelism Flags | Use only if step is pure & safe (`parallelizable`, `requires_process`) |
| Artifacts | Write under repo (e.g. `orchestrator/Data/`), then expose path key |
| Tests | Add/modify unit + recipe integration tests for every new scriptlet |
| Version Evolution | Add `_v2` file/key; do not alter `_v1` schema |

---

## 3. JSON Schemas (Exact)
Success (minimum):
```json
{"status":"ok","outputs":["domain.key_v1"]}
```
Extended (optional fields):
```json
{"status":"ok","outputs":["domain.key_v1"],"artifacts":["orchestrator/Data/report.md"],"metrics":{"duration_ms":12},"warnings":["optional"]}
```
Error:
```json
{"status":"error","reason":"human-readable","exit_code":1,"step":"step_name","ts":"2025-01-31T21:30:45Z"}
```

---

## 4. Python Scriptlet Template (Minimal)

```python
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
import json, sys, time, traceback

class ExampleStep(BaseScriptlet):
    def validate(self, ctx: Context, params: dict):
        if "src" not in params:
            raise ValueError("src required")

    def run(self, ctx: Context, params: dict):
        start = time.time()
        try:
            self.validate(ctx, params)
            # compute (pure where possible)
            result = {"count": 3}
            ctx.set("example.result_v1", result, who="example_step")
            duration = int((time.time() - start) * 1000)
            print(json.dumps({"status": "ok", "outputs": ["example.result_v1"], "metrics": {"duration_ms": duration}}))
            return 0
        except Exception as e:
            print(json.dumps({
                "status": "error",
                "reason": str(e),
                "exit_code": 1,
                "step": "example_step"
            }))
            return 1
```

---

## 5. Recipe Step Schema (Subset)
```yaml
- idx: 1
  name: compute_numbers
  type: python
  module: orchestrator.scriptlets.python.steps.compute_numbers
  function: ComputeNumbers
  args:
    src: orchestrator/Data/numbers.csv
  cache_key: "compute_numbers_v1"     # optional
  success:
    ctx_has_keys:
      - numbers.stats_v1
```

---

## 6. Assistant DO / DO NOT
DO:
- Reuse existing utility imports (`logging_util`, `resource`) when beneficial.
- Add new versioned keys instead of modifying existing outputs.
- Keep responses concise; reference canonical spec for details.

DO NOT:
- Introduce unversioned schema changes.
- Emit multiple JSON objects.
- Add side effects (file writes, network) before validation.
- Duplicate full spec text here.

---

## 7. Contribution Checklist (Abbreviated)
Before proposing code:
- Version-safe? (No mutation of existing `_vN`)
- Single JSON line success/error?
- `ctx.set(..., who=...)` used for each produced key?
- Tests updated/added?
- No stray stdout prints?
- Relative paths only?
- Docstring includes purpose, params, outputs, version?

(Full checklist: see canonical spec §16–17 in [.github/prompts/automation.prompt.md](.github/prompts/automation.prompt.md))

---

## 8. Quick Reference Commands
| Action | Command |
|--------|---------|
| Run recipe | `python orchestrator/runner_v2.py --recipe orchestrator/recipes/01_basic_number.yaml` |
| Only subset | `... --only aggregate_metrics generate_report` |
| Resume | `... --resume-from compute_metrics_v2` |
| Skip | `... --skip inject_failure` |
| Tests | `pytest -q` |

---

## 9. When In Doubt
Defer to canonical: [.github/prompts/automation.prompt.md](.github/prompts/automation.prompt.md).  
If a rule appears ambiguous, prefer backward-compatible, additive design.

---

## 10. Change Log (Local to This Mirror)
- v1 (trim): Converted from full duplicate to lean assistant guidance referencing canonical