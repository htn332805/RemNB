# Copilot Prompt: Workspace Overview for Automation Orchestrator

## INTENT
Orient the assistant or new contributors to the core structure and concepts of this automation/test orchestrator.

## DIRECTORY MAP (condensed)
orchestrator/
    - context.py # JSON-safe context store
    - runner.py # Recipe runner (sequential)
    - runner_v2.py # Runner with dependency, parallel, process, cache
    - recipes/ # YAML files defining pipelines
    - scriptlets/
        - -python/
            - core/
                - base.py # BaseScriptlet contract
                - logging_util.py
                - resource.py
                - steps/ # One file per Python scriptlet
                    - compute_numbers.py
                    - ... (other steps)
            - shell/ # Shell steps (sanitizers, etc.)
            - templates/
            - report_template.md
metrics_summary.xlsx
setup_curriculum.sh
run_context_server.py
tests/ # pytest test suite


## CORE CONCEPTS
- **Context**: In-memory JSON key-value, passed across all steps, versioned key discipline
- **Scriptlet**: One atomic operation (Python class or shell script), outputs exactly one JSON line
- **Recipe**: Sequential YAML plan, each step declared and asserted
- **Artifact**: File produced, referred by relpath in outputs/context

## KEY POLICIES
- Never break or mutate existing context keys (version with `_v2`, `_v3`)
- One class per scriptlet file; one recipe per YAML
- Logging goes to stderr, machine output to stdout (always JSON)

## QUICK ACTIONS
- Add scriptlet: use template, follow key naming, put in steps/
- Extend recipe: edit YAML in recipes/, always assert outputs in `success: ctx_has_keys`
- Add or run test: use pytest suite

## WHERE TO START
- Browse docs and `tests/` for patterns.
- Copy patterns from compute_numbers.py or example recipes for new work.

## EXTENSION PRINCIPLES
- Always version new logic
- Prefer composition (fan-in/fan-out) using `depends_on`
- Write integration tests for every new pipeline extension

---

Happy iterating!
# END OF PROMPT
