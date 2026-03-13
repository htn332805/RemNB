# AIOS — Dashboard Builder & Orchestration

**Generated:** 2026-03-12

---

## 1. Overview

**Module:** `dashboard/`  
**Implemented in:** Phase 10

The Dashboard module generates and launches interactive Plotly Dash web applications from a structured specification (`DashboardSpec` dict). The full lifecycle — code generation → registration → process launch → health verification — is orchestrated by a single entry point.

---

## 2. Module Inventory

| File | Purpose |
|---|---|
| `builder.py` | Converts a `DashboardSpec` dict into a Dash app `.py` file |
| `launcher.py` | Spawns the generated `.py` file as a subprocess |
| `registry.py` | JSON-backed registry of all launched dashboards |
| `tester.py` | HTTP health-check for a running Dash app |
| `improver.py` | LLM-driven iterative improvement of generated dashboards |
| `catalog_writer.py` | Writes dashboard metadata into the Qdrant knowledge catalog |
| `orchestrator.py` | End-to-end lifecycle: build → register → launch → health-check |
| `spec_examples.py` | Sample `DashboardSpec` dicts for testing |
| `versions/` | Directory of all generated Dash app `.py` files |

---

## 3. DashboardSpec Schema

A `DashboardSpec` is a Python dict with the following keys:

```python
{
    "title":        str,           # Dashboard window/page title
    "description":  str,           # Human-readable description
    "port":         int,           # Port for the Dash server (auto-allocated if absent)
    "data_queries": [              # List of data source descriptors
        {
            "name":   str,         # Variable name (→ df_<name>)
            "source": "csv"|"json"|"qdrant"|"inline",
            "path":   str,         # File path or Qdrant collection name
        }
    ],
    "charts": [                    # List of chart specifications
        {
            "id":    str,          # Unique chart component ID
            "type":  "bar"|"line"|"scatter"|"table",
            "x":     str,          # x-axis column name
            "y":     str,          # y-axis column name
            "title": str,          # Chart title
        }
    ],
    "layout": {}                   # Optional Dash layout overrides
}
```

---

## 4. Code Generator (`dashboard/builder.py`)

### Approach

The builder uses a **string template** (`DASH_APP_TEMPLATE`) rather than dynamic imports or AST manipulation. Placeholder sections are filled at generation time:

- `{data_section}` — Python code to load data into `df_<name>` DataFrames
- `{layout_section}` — Dash `html.Div` and `dcc.Graph` component tree

### Data Section Generation

| Source | Generated Code |
|---|---|
| `csv` | `df_<name> = pd.read_csv(r"<path>")` |
| `json` | `df_<name> = pd.read_json(r"<path>")` |
| `qdrant` | Placeholder `pd.DataFrame()` with TODO comment |
| `inline` (default) | Hardcoded `{"value": [0,1,2], "label": ["a","b","c"]}` |

The first query always also sets `df_main` as an alias.

### Layout Section Generation

Supported chart types:
| Chart Type | Plotly Express Call |
|---|---|
| `bar` | `px.bar(df_<name>, x=x_col, y=y_col)` |
| `line` | `px.line(df_<name>, x=x_col, y=y_col)` |
| `scatter` | `px.scatter(df_<name>, x=x_col, y=y_col)` |
| `table` | `go.Figure(go.Table(...))` |

All charts are wrapped in `dcc.Graph(id=chart_id, figure=fig)` and assembled into an `html.Div([...])` layout.

### Generated File Structure

```python
"""Auto-generated Dash dashboard: {title}"""
import dash
from dash import dcc, html
import plotly.express as px
import pandas as pd

# DATA SECTION
df_main = pd.read_csv("...")

# LAYOUT SECTION
layout = html.Div([
    html.H1("{title}"),
    dcc.Graph(id="chart_0", figure=fig_0),
    ...
])

def create_app():
    app = dash.Dash(__name__)
    app.title = "{title}"
    app.layout = layout
    return app

if __name__ == "__main__":
    app = create_app()
    app.run_server(host="0.0.0.0", port={port}, debug=False)
```

---

## 5. Launcher (`dashboard/launcher.py`)

Spawns the generated Python file as a background subprocess:

```python
pid = launch_dashboard(versioned_path, port)
```

- Uses `subprocess.Popen` with the current Python interpreter
- Returns the subprocess PID for tracking
- Does not block — the Dash app runs independently

---

## 6. Registry (`dashboard/registry.py`)

Maintains a JSON file (`dashboard/registry.json`) of all generated and launched dashboards.

Each registry entry contains:
```json
{
  "id":      "dash_<hexdigest>",
  "version": 1,
  "name":    "My Dashboard",
  "spec":    { ... },
  "path":    "dashboard/versions/dash_<id>_v1.py",
  "port":    8061,
  "created_at": "2026-03-12T..."
}
```

Functions:
- `register_dashboard(name, spec, path)` → creates entry, returns entry dict
- `load_registry()` → returns full registry dict
- `save_registry(registry)` → writes to JSON file
- Version numbering increments automatically per dashboard ID

---

## 7. Health Checker (`dashboard/tester.py`)

Attempts an HTTP GET to `http://localhost:{port}/` with a configurable timeout:

```python
health_ok = health_check_dashboard(port, timeout_seconds=8)
```

Returns `True` if the server responds with HTTP 200, `False` otherwise.

---

## 8. Improver (`dashboard/improver.py`)

Runs an iterative improvement loop:
1. Reads the generated Dash app source code
2. Sends it to Ollama LLM with instructions to improve it
3. Writes the improved version back to the `.py` file
4. Health-checks the improved version

Used for self-refining dashboard quality without human intervention.

---

## 9. Catalog Writer (`dashboard/catalog_writer.py`)

Writes dashboard metadata into the Qdrant `knowledge` collection after a dashboard is successfully launched:
```python
{
    "category": "dashboard",
    "content":  "<title> <description> charts: ...",
    "created_at": "...",
    ...
}
```

This makes dashboards discoverable through semantic search in the knowledge base.

---

## 10. Orchestrator (`dashboard/orchestrator.py`)

Single public entry point for external callers:

```python
result = create_and_launch_dashboard(username, spec)
```

**7-Step Lifecycle:**

| Step | Action |
|---|---|
| 1 | Choose a free TCP port in range 8060–8090 using socket bind probing |
| 2 | Ensure `dashboard/versions/` directory exists |
| 3 | Generate Dash app code to a temp file |
| 4 | Register in JSON registry → get permanent versioned ID |
| 5 | Rename temp file to `dashboard/versions/<id>_v<version>.py` |
| 6 | Launch via `launch_dashboard(path, port)` |
| 7 | Health-check with 8-second timeout |

**Return value:**
```python
{
    "dashboard_id": "dash_69b322edf8c5...",
    "version": 1,
    "port": 8061,
    "pid": 12345,
    "health_ok": True,
    "path": "dashboard/versions/dash_69b_v1.py"
}
```

---

## 11. Naming Convention for Generated Files

Generated files follow the pattern:
```
dashboard/versions/dash_{MD5_of_spec_content}_{v}{version}.py
```

This ensures:
- Same spec always gets the same ID base (deterministic)
- Multiple versions of the same dashboard are differentiated by version integer
- Files are easily traceable to their registry entry

---

## 12. Port Allocation Strategy

```python
_PORT_START = 8060
_PORT_END = 8090

def _find_free_port():
    for port in range(8060, 8090):
        try:
            s.bind(("0.0.0.0", port))
            return port
        except OSError:
            continue
    raise RuntimeError("No free port found")
```

Port range: **8060–8089** (30 simultaneous dashboards maximum).

---

## 13. Versioned Dashboard Example

The file `dashboard/versions/dash_69b322edf8c5474ab1f0ade7c0230f6e_v1.py` is the first real generated dashboard in the repository, showing a complete working Plotly Dash application with data loading, layout, and server startup code.

---

## 14. Integration Points

| Caller | Interaction |
|---|---|
| `agent/main.py` | Calls `dashboard/orchestrator.create_and_launch_dashboard` when intent = `dashboard_gen` |
| `knowledge_analytics/growth_reporter.py` | Counts dashboard Qdrant entries for analytics |
| Qdrant `knowledge` collection | Dashboard metadata stored via `catalog_writer` |
| `output_engine` | Dashboard artifacts can be referenced in generated reports |
