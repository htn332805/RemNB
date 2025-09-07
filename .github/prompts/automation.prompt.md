# Automation Orchestrator – Consolidated Engineering Guidelines

## 1. Purpose
A modular, version-safe automation framework: recipes (YAML) → runner → scriptlets (Python / shell) → shared context → artifacts → tests.

## 2. Canonical Directory Layout
```
orchestrator/
  context.py
  runner.py
  runner_v2.py
  recipes/*.yaml
  scriptlets/
    python/core/{base.py,logging_util.py,resource.py}
    python/steps/*.py
    shell/*.sh
  templates/
  Data/
tests/
.github/
```

## 3. Core Concepts
| Concept | Definition |
|---------|------------|
| Context | JSON‑safe key/value store with attribution + history |
| Scriptlet | Single-responsibility executable producing exactly one structured JSON line |
| Recipe | Ordered YAML steps (future: full DAG) |
| Versioned Key | Name ends `_vN`; never mutate prior schema |
| Artifact | External file path written under repo (e.g. orchestrator/Data/normalized.json) |
| Fan-In | Aggregation of independent parallel step outputs |

## 4. Normative Specification (MUST / SHOULD / MAY)
| Area | Requirement | Level |
|------|-------------|-------|
| JSON Contract | Scriptlet prints exactly one line: `{"status":"ok"|"error", ...}` | MUST |
| Serialization | Only primitives / lists / dicts; no custom classes | MUST |
| Key Naming | `namespace.entity_vN` with additive versioning | MUST |
| Idempotency | Re-running a successful step yields same keys (unless designed otherwise) | MUST |
| Error Output | On failure include `reason`, `exit_code`, `step` | MUST |
| Logging | Non-JSON diagnostics to stderr (or structured logger) | SHOULD |
| Validation | `validate()` raises before mutation; no partial writes | MUST |
| Attribution | Every `ctx.set` supplies `who=` origin | MUST |
| Tests | New scriptlet: unit + integration (recipe) | MUST |
| Caching | Use explicit step + marker (e.g. `.cache/`) | SHOULD |
| Parallel Safety | No shared mutable external file writes without coordination | MUST |
| Process Isolation | CPU-bound steps flag `requires_process=True` | SHOULD |
| Resource Metrics | Runner records timing per step; extend for CPU/mem | SHOULD |
| Security | No dynamic import from untrusted paths | MUST |
| Version Evolution | Add `_v2` etc.; retain `_v1` until all consumers migrated | MUST |
| CLI Controls | `--skip`, `--only`, `--resume-from` semantics stable | MUST |
| Documentation | Each scriptlet top docstring: purpose, params, outputs | SHOULD |
| Async I/O | (Future) Only when runner supports event loop | MAY |

## 5. Scriptlet Authoring Standard
Template responsibilities:
1. `class <Name>(BaseScriptlet)`
2. `validate(ctx, params)` – raise on missing/invalid inputs.
3. `run(ctx, params)`:
   - Gather inputs from context + params.
   - Perform computation (pure if possible).
   - `ctx.set(<key>, <value>, who="<script_name>")`.
   - Print success JSON: `{"status":"ok","outputs":["<key1>","<key2>"]}`.
   - On exception: structured error JSON, return non‑zero exit code.

Success JSON minimal schema:
```
{"status":"ok","outputs":["domain.entity_v1"], ...optional metadata...}
```
Error JSON minimal schema:
```
{"status":"error","reason":"<human message>","exit_code":1,"step":"<step_name>"}
```

## 6. Recipe Schema (Current Subset)
```yaml
test_meta:
  test_id: STR
  description: STR
steps:
  - idx: INT
    name: STR (unique)
    type: python|shell
    module: <dotted.path>        # python
    function: <ClassName>        # python
    script: path/to/script.sh    # shell
    args: {k: v} | [list]        # optional
    depends_on: [step_name, ...] # optional
    cache_key: "identifier"      # optional
    success:
      ctx_has_keys:
        - namespace.entity_v1
```

## 7. Runner Guarantees
- Sequential by default; limited parallel groups where safe.
- Resilience flags:
  - `--skip name …`
  - `--only name …`
  - `--resume-from name`
- Caching: if step has `cache_key` and marker resolved → skip + annotate context.
- Collect per-step metrics: start/end timestamps (+ future CPU/mem).

## 8. Context Rules
- `ctx.set(key, value, who=origin)` records timestamp & provenance.
- Reading missing key returns `None`.
- No overwriting with incompatible schema for same version.
- Future: enforcement layer to detect conflicting writes (roadmap).

## 9. Versioning Policy
| Scenario | Action |
|----------|--------|
| Add new field to output | Introduce new key `_v2` (keep `_v1`) |
| Bug fix w/o schema change | Patch in place (document in changelog) |
| Breaking change needed | New scriptlet filename + class + key `_vN+1` |
| Deprecation | Mark old version in docs; remove only after consumer migration |

## 10. Performance & Resources
Consolidated expectations:
- Thread pool for I/O‑bound parallelizable steps (pure context reads).
- Process pool for CPU heavy (`requires_process=True`).
- Timing always captured; CPU/memory MAY be added via decorator.
- Caching: explicit markers; future automatic hash of (module, args).

## 11. Quality & Testing
| Item | Target |
|------|--------|
| Unit tests | Validate param errors + success |
| Integration | Run recipe; assert produced keys |
| Coverage | >=80% (once coverage tooling enabled) |
| Lint | Pass ruff/flake8 (add config) |
| CI | Fail on missing success keys or error JSON |

## 12. Logging & Error Handling
- Scriptlets: only final structured JSON to stdout; all auxiliary logs to stderr.
- Avoid partial stdout lines.
- Never swallow exceptions silently; map to structured error.

## 13. Security & Safety
- Allowlist imports (no user-provided arbitrary dotted paths without review).
- No eval/exec for recipe content.
- Paths must be relative inside workspace (no `/etc/...` writes).

## 14. Contribution Checklist
Before PR merge:
- [ ] Scriptlet file + docstring
- [ ] Adds keys with `_vN` suffix
- [ ] Tests (unit + recipe) green
- [ ] No duplicated logic (consider refactoring)
- [ ] Updated documentation (if public surface changed)
- [ ] No breaking removal of prior version keys
- [ ] Structured logging intact
- [ ] Caching / parallel flags justified (if used)

## 15. Roadmap (Non-Normative)
| Feature | Status |
|---------|--------|
| Automatic dependency graph validation | Planned |
| Retry / timeout per step | Planned |
| Context persistence across runs | Planned |
| Hash-based automatic caching | Planned |
| DAG visualization | Planned |
| Historical metric persistence (SQLite) | Planned |
| Async I/O pipeline | Exploratory |

## 16. Acceptance Criteria Summary
| Category | Criteria |
|----------|----------|
| Scriptlet Validity | Passes validation, emits single JSON line |
| Version Safety | No mutation of existing versioned schema |
| Test Integrity | All required keys asserted in tests |
| Performance Safety | Parallel steps do not race on same file |
| Error Transparency | All failures emit structured JSON |
| Documentation | New behaviors documented once (no duplication) |

## 17. Rationale for Consolidation
This single specification removes redundancy, clarifies mandatory vs aspirational features, and provides an actionable checklist for contributors and CI enforcement.

---