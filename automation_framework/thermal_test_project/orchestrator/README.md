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

    Tip: Keep scriptlets small and composable. Each step should do one thing well.
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
  