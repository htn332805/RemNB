import dash
from dash import dcc, html, Input, Output, State
import dash_bootstrap_components as dbc
import threading
import time
import os
import pandas as pd
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import plotly.graph_objs as go

# ---------- Shared in-memory context ----------
shared_ctx = {
    "lines": [],
    "modified": False,
    "df": pd.DataFrame()  # To hold parsed CSV data
}

FILE_TO_WATCH = "cdu.log"

# ---------- File Watcher ----------
class FileChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path.endswith(FILE_TO_WATCH):
            try:
                # Read full CSV file
                df = pd.read_csv(FILE_TO_WATCH, quotechar='"')
                if not df.equals(shared_ctx.get("df", pd.DataFrame())):
                    shared_ctx["df"] = df
                    # Update lines as raw text for live monitor tab
                    shared_ctx["lines"] = open(FILE_TO_WATCH).read().splitlines()
                    shared_ctx["modified"] = True
            except Exception as e:
                print(f"Error reading file: {e}")

def start_file_watcher():
    observer = Observer()
    handler = FileChangeHandler()
    observer.schedule(handler, path='.', recursive=False)
    observer.start()
    print("File watcher started.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

# ---------- Dash App ----------
app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP],suppress_callback_exceptions=True)

# Get columns for dropdown (excluding timestamp)
def get_columns():
    if not shared_ctx["df"].empty:
        return [{"label": col, "value": col} for col in shared_ctx["df"].columns if col != "timestamp"]
    return []

# Layout with Tabs
app.layout = dbc.Container([
    html.H2("Live CDU Monitor with Plot"),
    dcc.Interval(id='interval-component', interval=2000, n_intervals=0),
    dbc.Tabs([
        dbc.Tab(label="Live File Monitor", tab_id="tab-live"),
        dbc.Tab(label="Data Plot", tab_id="tab-plot"),
    ], id="tabs", active_tab="tab-live"),
    html.Div(id="tab-content", className="p-4")
], fluid=True)

# Content callback for tabs
@app.callback(
    Output("tab-content", "children"),
    Input("tabs", "active_tab")
)
def render_tab_content(active_tab):
    if active_tab == "tab-live":
        return html.Pre(id='live-content', style={'whiteSpace': 'pre-wrap', 'border': '1px solid #ccc', 'padding': '10px'})
    elif active_tab == "tab-plot":
        # Multi-select dropdown + Graph
        return html.Div([
            dbc.Label("Select columns to plot:"),
            dcc.Dropdown(
                id='columns-dropdown',
                options=get_columns(),
                value=[],
                multi=True,
                placeholder="Select one or more columns"
            ),
            dcc.Graph(id='live-graph')
        ])
    return "No content available"

# Update raw text content
@app.callback(
    Output('live-content', 'children'),
    Input('interval-component', 'n_intervals'),
    State("tabs", "active_tab")
)
def update_live_content(n, active_tab):
    if active_tab != "tab-live":
        raise dash.exceptions.PreventUpdate
    if shared_ctx["modified"]:
        shared_ctx["modified"] = False
        return "\n".join(shared_ctx["lines"])
    raise dash.exceptions.PreventUpdate

# Update graph based on dropdown selections and timer
@app.callback(
    Output('live-graph', 'figure'),
    Input('interval-component', 'n_intervals'),
    Input('columns-dropdown', 'value'),
    State("tabs", "active_tab")
)
def update_graph(n, selected_cols, active_tab):
    if active_tab != "tab-plot":
        raise dash.exceptions.PreventUpdate
    if shared_ctx["df"].empty or not selected_cols:
        # Return empty figure with message
        return go.Figure(
            layout=go.Layout(
                title="Select columns to plot",
                xaxis={"title": "Timestamp"},
                yaxis={"title": "Values"}
            )
        )
    df = shared_ctx["df"]

    # Convert timestamp to datetime for better x-axis
    try:
        df['timestamp'] = pd.to_datetime(df['timestamp'])
    except Exception:
        pass  # If fails, keep as is

    fig = go.Figure()

    for col in selected_cols:
        if col in df.columns:
            fig.add_trace(go.Scatter(
                x=df['timestamp'],
                y=df[col],
                mode='lines+markers',
                name=col
            ))

    fig.update_layout(
        title="CDU Data Over Time",
        xaxis_title="Timestamp",
        yaxis_title="Value",
        hovermode='x unified'
    )

    return fig

# ---------- Run File Watcher in Background Thread ----------
watcher_thread = threading.Thread(target=start_file_watcher, daemon=True)
watcher_thread.start()

# ---------- Run App ----------
if __name__ == '__main__':
    app.run(debug=True)
