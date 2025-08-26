This guide explains how to use the provided workspace to build reusable test sequences, automate execution, analyze results with AI, store data in SQLite, generate Excel and reports, and monitor live data in a Dash app. This framework is layout and structred as follow:
    - This README layout the guideline for using and expanding the framework and must be the criteria that should be inherit for any further snippet/recipe addition and exapansion
    - The absolute path to the working folder is /Users/hain2/RemNB/automation_framework/thermal_test_project
    - All Persistent data for shareing should be store and locate orchestrator/Data
    - All log should be store and located in orchestrator/Logs
    - All snippet/scripts should be locate in orchestrator/scriptlets and its coressponding subfolder "python" "shell", etc.
    - All recipes used in the runner as argument from cli are located in orchestrator/recipes with YAML compatible format
    - All report templates, excel templates are located in orchestrator/templates
    - All scriptlets should be able to run as cli command independently of the recipe or the orchestrator runner
    - All scriptlets/snippet and recipe content should have detail comment on every line with the examples of its usages, descriptions, limitattion should be available at the top as docstring or multiblock comment
    - Orchestrator core
        a) The file runner.py ishe main pipeline runner with shared in-memory ctx capability along with ability to launch parallel step in the background using multithread technology
        b) context.py — defines Context and helpers to expose selected ctx values to shell environments
        c) The orchestrator executes a list of scriptlets defined in a recipe YAML. It:
            + Loads test_meta into ctx and GLOBAL_STATE
            + Runs steps in order/parallel depending on the recipe, checking dependencies
            + Captures stdout/stderr per step into Logs/<timestamp>_<step>.log
            + Evaluates success criteria (files_exist and/or ctx_has_keys)
            + Updates a directed graph at Assets/step_sequence.png showing progress
            + Honors RAM vs disk logging behavior based on store_on_ram.txt
            + Basic CLI:
                    python runner.py
                    Default recipe: recipes/default_recipe.yaml
                    python runner.py --recipe recipes/test_mem.yaml
                Filters:
                    --only STEP1 STEP2
                    --skip STEP_NAME
                    --resume-from STEP_NAME
                Typical exits:
                    Exit code 0: success
                    Exit code 1: a step failed
                    Exit code 2: dependency not satisfied

        Recipes (declarative execution plans)
            A recipe defines:
                - test_meta block: test_id, tester, description — written to DB and used in reports
                - steps: ordered list of scriptlets
                - idx: order
                - name: unique step name
                - type: shell | python
                - cmd/module/function: how to execute
                - args/params: inputs for the step
                - depends_on: list of step names that must succeed first
                - success: files_exist and/or ctx_has_keys

            On-the-fly adjustments:
            - Reorder steps by changing idx values
            - Insert or remove steps
            - Add success criteria for robust gating
            - Use --only or --skip or --resume-from without editing the file

    Tips: 
        Keep scriptlets small and composable with the focus on doing/performing single task only. Each step should do one thing well. It content are well comment in detail for every single line of code
        -Do Not Return or Store Non-Serializable Objects in ctx
        -Never store or return objects that are not JSON serializable
        - Data, Logs, Assets
            - Data/ — input/output artifacts: CSV, JSON metrics, Excel, report.docx, tests.db
            - Logs/ — orchestrator logs + per-step logs
            - Assets/graph.png — execution digraph, updated after each step

    - Tools
        - miscelenous snippet for house cleaning
        -tools/clean_pristine.sh — cleanup/reset convenience script
    - Control/state files
        - store_on_ram.txt — when present, orchestrator keeps logs/state in RAM and pickles GLOBAL_STATE to state.pkl
        - If it contains “debug”, increase verbosity stored in memory
        - state.pkl — pickled GLOBAL_STATE when RAM mode is enabled

    Quick-start scenarios:
        Full file-based pipeline (default_recipe.yaml)
        Purpose: run a typical pipeline with files on disk
        Steps:
            1. Preprocess raw CSV → Data/clean.csv
            2. Sanitize with awk → Data/sanitized.csv
            3. Normalize with sed → Data/normalized.csv
            4. Compute metrics → Data/metrics.json
            5. AI analysis → Data/ai_summary.md
            6. Store in DB → Data/tests.db
            7. Export Excel with charts → Data/output.xlsx
            8. Generate docx report → Data/report.docx
            - Command:
            - cd orchestrator
            - python runner.py
            - Inspect outputs in Data/
            - Check execution graph at Assets/graph.png
            - Logs are in Logs/

    In-memory ctx pipeline (test_mem.yaml)
        - Purpose: demonstrate sharing without intermediate files
        - Steps:
            1. compute_metrics_mem: read Data/normalized.csv and store results in ctx["metrics"], ctx["metrics_columns"]
            2. ai_analyze_mem: read ctx["metrics"], produce ctx["ai_summary"]
            3. shell_bridge_demo: pipe ctx["metrics_columns"] to a small inline Python that counts them, capture stdout back to ctx["metric_col_count"]
            4. debug_print_ctx: log ctx keys and samples

    CRITERIA MUST BE HONOR AS ALWAYS BY ALL SCRIPTLETS AND THE EXPANSION/MODIFICATION OF WORKFLOW:
        1) all scriptlets must be independent of each others and can be run with cli for easy debug
        2) all scriptlets must have detailed comment while having the example of usage, descriptions, and limitation/requirement provide on the top of the file as docstring or multi comment block
        3) all scriptlets must as small and resuable as possible for portability and scalability
        4) passing for updating the ctx content with serialized json format or json compabtible string instead of objects. Only serialized objects should be return.
        5) Any modification of existing code should be keep and minimal while ensuring backward compatibility where it can be run, support and compatible wit old and existing workflow or dash app
        5) Any major over haul of the scriptlets are highly encourage to be create a new or a separate improve version of the scriptlet thus ensuring backward compatibilty
        6) Shared in-memory context (ctx) for inter-step communication.
        7) Support for both Python and shell scriptlets.
        8) Logging and error handling are present.
        9) Scriptlets should not be monolithic and be reusable
        10) clear separation between orchestration logic and step execution logic.
        11) Logging, error handling, and resource tracking are standardized.
        12) utilizing async or process-based parallelism for heavy computation.
        13) caching or batching for repeated/expensive operations.
        12) some adapbility of standardized frame for unit test and debug
        13) clear traceability of ctx changes (who/what/when).
        14) scriptlets/python/core/ for reusable core utilities (ctx helpers, logging, resource tracking, etc.).
        15) scriptlets/python/steps/ for atomic, single-responsibility step implementations.
        16) criptlets/python/apps/ for Dash and other long-running apps.
        17) BaseScriptlet class (Python) with standard run(ctx, params) and optional validate(ctx, params) methods.
        18) Use a registry or entry-point system so new scriptlets can be auto-discovered and listed.
        19) For shell, use a naming convention and a wrapper to standardize environment variable handling and logging.
        20) Implement a Context class with:
                -Type-checked, versioned, and timestamped modifications.
                -Change history (who/what/when/old/new).
                -Optional hooks for validation, logging, and event triggers.
                -Provide a ctx.get_history(key) and ctx.last_modified(key) API.
        21) Logging, Error Handling, and Resource Tracking
                -Centralize logging with a Logger utility (supporting step-level, ctx-level, and global logs).
                -Standardize error handling with custom exceptions and error codes.
                -Use psutil and tracemalloc for resource tracking, and log resource usage per step.
                -Add a decorator for timing and resource usage on all step functions.
        22) Parallelism and Performance
                -Use concurrent.futures.ThreadPoolExecutor or ProcessPoolExecutor for parallel/async steps.
                -For heavy computation, allow steps to declare requires_process=True to run in a separate process.
                -Batch or cache expensive operations (e.g., data flattening, stats calculation).
                -Use async Dash callbacks for live apps if possible.
        23) Testing and CI
                -Add unit tests for all core utilities and scriptlets.
                -Add integration tests for recipes.
        24) Documentation and Discoverability
                -Auto-generate documentation for all scriptlets and their parameters.
                -Provide a CLI command to list all available steps and their docs.
        25) No scriptlets/helper should be rewrite but having a wrapper and decorator to override its functionality or creating new helpers that leverage existing one with minor/minimal changes/alteration
        
    Best Practices for Using ctx
        - Always validate data before storing in ctx.
        - Never store non-serializable objects (e.g., open files, sockets, custom classes).
        - Use clear, descriptive keys for each data item.
        - Log all changes to ctx for traceability (already supported in debug mode).
        - Export only necessary keys to shell scriptlets using expose_for_shell.
        - Document every scriptlet with usage and limitations.


    ## Directory Structure
    - `context.py` — Shared in-memory context utilities.
    - `runner.py` — Main pipeline runner.
    - `scriptlets/`
        - `python/core/` — Core utilities (ctx helpers, logging, etc.)
        - `python/steps/` — Atomic, single-responsibility step implementations.
        - `python/apps/` — Dash and other long-running apps.
        - `shell/` — Shell scriptlets.
    - `recipes/` — YAML recipes for orchestrator.
    - `tests/` — Unit and integration tests.
    - `Data/`, `Logs/`, `Assets/`, `templates/`, `tools/` — Data, logs, assets, templates, and tools.

    ## Scriptlet Guidelines

    - Each scriptlet must be independently runnable as a CLI.
    - Each scriptlet must have a detailed docstring with usage, description, and limitations.
    - Scriptlets must be small, composable, and reusable.
    - Only JSON-serializable objects may be stored in ctx.
    - All ctx changes must be traceable.
    - See `tests/` for usage examples.

        
  