# Documentation for Orchestrator Workspace

---

## context.py

### Description
Defines the `Context` class for in-memory data sharing between orchestrator steps. Provides utilities to expose selected ctx values to shell environments.

### Objective
- Enable safe, JSON-serializable, shared state (`ctx`) for all orchestrator steps.
- Allow exporting ctx keys as environment variables for shell scriptlets.

### Applications, Integrations, and Usage

- Used by `runner.py` to maintain shared state.
- Used by all Python scriptlets to read/write shared data.
- Used by shell scriptlets via `expose_for_shell` to pass ctx data as environment variables.

**Example:**
```python
from context import Context, expose_for_shell
ctx = Context()
ctx["foo"] = 123
env = expose_for_shell(ctx, ["foo"])
# env["foo"] == "123"
```

### Limitations
- Only JSON-serializable objects can be stored in ctx.
- Not persistent unless explicitly saved by the orchestrator.
- No built-in versioning or history (extend as needed).

### Requirements/Dependencies
- Python 3.6+
- Standard library: `json`, `typing`

---

## runner.py

### Description
Main orchestrator pipeline runner. Executes recipes step-by-step, manages ctx, logs, dependencies, parallel/background steps, and error handling.

### Objective
- Automate execution of complex test/analysis pipelines.
- Provide robust logging, error handling, and resource tracking.
- Support parallel and background execution of steps.

### Applications, Integrations, and Usage

- Run a recipe:
  ```sh
  python runner.py --recipe recipes/demo_screen_dash_ctx.yaml
  ```
- Run with debug:
  ```sh
  python runner.py --debug
  ```
- Run only specific steps:
  ```sh
  python runner.py --only step1 step2
  ```
- Skip steps:
  ```sh
  python runner.py --skip step3
  ```
- Resume from failure:
  ```sh
  python runner.py --resume-from step4
  ```

### Limitations
- Only supports YAML recipes in `orchestrator/recipes`.
- Only JSON-serializable objects are stored in ctx.
- Digraph update is a stub (implement as needed).

### Requirements/Dependencies
- Python 3.8+
- `pyyaml`, `psutil`, `shutil`, `traceback`, `datetime`
- `context.py` (for Context class)

---

## scriptlets/python/steps/ctx_init.py

### Description
Initializes keys in the shared in-memory ctx as empty lists.

### Objective
- Prepare ctx for downstream steps by initializing required keys.

### Applications, Integrations, and Usage

- Used as a step in recipes to ensure ctx keys exist:
  ```yaml
  - name: start_shared_ctx
    type: python
    module: scriptlets.python.steps.ctx_init
    function: run
    params:
      keys: ["new_row"]
  ```
- CLI usage:
  ```sh
  python scriptlets/python/steps/ctx_init.py --keys foo bar
  ```

### Limitations
- Only initializes keys as empty lists.
- Keys must be valid Python identifiers.

### Requirements/Dependencies
- Python 3.6+
- Standard library: `argparse`

---

## scriptlets/python/steps/ctx_row_updater.py

### Description
Continuously appends rows `[timestamp, random1, ..., randomn]` to ctx[key] in a background thread.

### Objective
- Simulate or generate live data for Dash or other consumers.

### Applications, Integrations, and Usage

- Used in recipes to provide live data for Dash apps:
  ```yaml
  - name: update_ctx_background
    type: python
    module: scriptlets.python.steps.ctx_row_updater
    function: run
    params:
      key: "new_row"
      n: 3
      interval_sec: 1
  ```
- CLI usage:
  ```sh
  python scriptlets/python/steps/ctx_row_updater.py --key foo --n 2 --interval_sec 1
  ```

### Limitations
- Only works with JSON-serializable ctx.
- Intended for orchestrator or CLI demo.

### Requirements/Dependencies
- Python 3.6+
- Standard library: `threading`, `time`, `datetime`, `random`, `argparse`

---

## scriptlets/python/apps/hello_dash.py

### Description
A Dash app scriptlet that displays and plots shared in-memory ctx data. Monitors ctx for changes and allows interactive plotting.

### Objective
- Provide a live dashboard for monitoring and visualizing ctx data.
- Allow user to select columns to plot.

### Applications, Integrations, and Usage

- Used in orchestrator recipes to visualize live data:
  ```yaml
  - name: monitor_and_plot
    type: python
    module: scriptlets.python.apps.hello_dash
    function: run
    params:
      port: 8050
      ctx_key: "new_row"
  ```
- CLI usage:
  ```sh
  python scriptlets/python/apps/hello_dash.py --port 8050 --ctx_key new_row
  ```

### Limitations
- Requires Dash and Plotly.
- Expects ctx[ctx_key] as a list of lists: [timestamp, val1, val2, ...].
- Not suitable for very large datasets.

### Requirements/Dependencies
- Python 3.6+
- `dash`, `plotly`, `threading`, `argparse`

---

## scriptlets/shell/screen_script.sh

### Description
Manages GNU screen sessions: list, kill, create, recreate, send commands, and run with logging.

### Objective
- Automate screen session management for orchestrator steps and Dash app launching.

### Applications, Integrations, and Usage

- Used in recipes to manage screen sessions:
  ```yaml
  - name: create_screen_with_log
    type: shell
    cmd: scriptlets/shell/screen_script.sh
    args:
      - -r
      - test_screen
  ```
- CLI usage:
  ```sh
  ./scriptlets/shell/screen_script.sh -l
  ./scriptlets/shell/screen_script.sh -c mysession
  ./scriptlets/shell/screen_script.sh -k mysession
  ./scriptlets/shell/screen_script.sh -d mysession
  ./scriptlets/shell/screen_script.sh -s mysession "echo Hello"
  ./scriptlets/shell/screen_script.sh -r mysession
  ```

### Limitations
- Requires GNU screen installed and in PATH.
- Only supports session management by name, not by PID.

### Requirements/Dependencies
- Bash shell
- GNU screen

---

## recipes/demo_screen_dash_ctx.yaml

### Description
Recipe demonstrating launching a Dash app in a GNU screen session, sharing in-memory ctx, and updating ctx in the background.

### Objective
- Show integration of shell and Python scriptlets, ctx sharing, and live Dash monitoring.

### Applications, Integrations, and Usage

- Run the full pipeline:
  ```sh
  python runner.py --recipe recipes/demo_screen_dash_ctx.yaml
  ```
- Steps include:
  - Creating a screen session with logging.
  - Checking and attaching to the screen.
  - Launching Dash in the screen.
  - Initializing and updating ctx.
  - Monitoring and plotting with Dash.

### Limitations
- Assumes all referenced scriptlets exist and are executable.
- Assumes Dash and screen are installed.

### Requirements/Dependencies
- YAML format
- All referenced scriptlets and their dependencies

---

## tests/test_ctx.py

### Description
Unit tests for the Context class.

### Objective
- Ensure ctx supports set/get, JSON-serializability, and error on non-serializable objects.

### Applications, Integrations, and Usage

- Run with pytest:
  ```sh
  pytest tests/test_ctx.py
  ```

### Limitations
- Only tests core Context functionality.

### Requirements/Dependencies
- pytest
- context.py

---

## tests/test_ctx_init.py

### Description
Unit test for ctx_init scriptlet.

### Objective
- Ensure ctx_init initializes keys as empty lists.

### Applications, Integrations, and Usage

- Run with pytest:
  ```sh
  pytest tests/test_ctx_init.py
  ```

### Limitations
- Only tests Python API, not CLI.

### Requirements/Dependencies
- pytest
- scriptlets/python/steps/ctx_init.py

---

## tests/test_ctx_row_updater.py

### Description
Unit test for ctx_row_updater scriptlet.

### Objective
- Ensure ctx_row_updater appends rows to ctx in a background thread.

### Applications, Integrations, and Usage

- Run with pytest:
  ```sh
  pytest tests/test_ctx_row_updater.py
  ```

### Limitations
- Only tests Python API, not CLI.

### Requirements/Dependencies
- pytest
- scriptlets/python/steps/ctx_row_updater.py

---

## tests/test_hello_dash.py

### Description
CLI help test for hello_dash.py.

### Objective
- Ensure hello_dash.py responds to --help.

### Applications, Integrations, and Usage

- Run with pytest:
  ```sh
  pytest tests/test_hello_dash.py
  ```

### Limitations
- Only tests CLI help, not full Dash functionality.

### Requirements/Dependencies
- pytest
- scriptlets/python/apps/hello_dash.py

---

## tools/clean_pristine.sh

### Description
Shell script to clean/reset the workspace to a pristine state.

### Objective
- Remove generated data, logs, assets, caches, and state files.

### Applications, Integrations, and Usage

- Run to reset the workspace:
  ```sh
  bash tools/clean_pristine.sh
  ```

### Limitations
- Only works on Unix-like systems.
- May need adjustment for custom file patterns.

### Requirements/Dependencies
- Bash shell

---

## requirements.txt

### Description
Lists all Python dependencies for the orchestrator workspace.

### Objective
- Ensure all required packages are installed for orchestrator, Dash, and reporting.

### Applications, Integrations, and Usage

- Install dependencies:
  ```sh
  pip install -r requirements.txt
  ```

### Limitations
- Versions may need updating for compatibility.

### Requirements/Dependencies
- Python 3.8+