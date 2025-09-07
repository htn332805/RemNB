# Imports
import dash
from dash import dcc, html, Input, Output, State, ALL, ctx
import dash_bootstrap_components as dbc
import threading
import time
import pandas as pd
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import plotly.graph_objs as go
import uuid  # For generating unique chart IDs

# ---------- Global State (shared across threads) ----------
shared_ctx = {
    "lines": [],         # raw lines from the monitored file
    "modified": False,   # flag if the file changed
    "df": pd.DataFrame() # dataframe with parsed data
}

FILE_TO_WATCH = "cdu.log"  # The file we're monitoring


# ---------- File Watcher Class ----------
class FileChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        # Called when a file is modified
        if event.src_path.endswith(FILE_TO_WATCH):
            try:
                df = pd.read_csv(FILE_TO_WATCH, quotechar='"')
                if not df.equals(shared_ctx["df"]):
                    shared_ctx["df"] = df
                    shared_ctx["lines"] = open(FILE_TO_WATCH).read().splitlines()
                    shared_ctx["modified"] = True
            except Exception as e:
                print(f"Error reading file: {e}")

# ---------- Start Watcher Thread ----------
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


# ---------- Dash App Setup ----------
app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP], suppress_callback_exceptions=True)

# Function to get dropdown options from dataframe columns
def get_columns():
    if not shared_ctx["df"].empty:
        return [{"label": col, "value": col} for col in shared_ctx["df"].columns if col != "timestamp"]
    return []

# ---------- App Layout ----------
app.layout = dbc.Container([
    html.H2("Live CDU Monitor with Dynamic Charts"),
    dcc.Interval(id='interval-component', interval=2000, n_intervals=0),  # updates every 2 seconds
    dbc.Tabs([
        dbc.Tab(label="Live File Monitor", tab_id="tab-live"),
        dbc.Tab(label="Data Plots", tab_id="tab-plot"),
    ], id="tabs", active_tab="tab-live"),
    html.Div(id="tab-content", className="p-4"),
    dcc.Store(id='charts-store', data=[]),  # store the list of chart configs
], fluid=True)

# ---------- Render Tab Content ----------
@app.callback(
    Output("tab-content", "children"),
    Input("tabs", "active_tab"),
    State("charts-store", "data")
)
def render_tab_content(active_tab, charts_data):
    if active_tab == "tab-live":
        # Tab showing raw file contents
        return html.Pre(id='live-content', style={'whiteSpace': 'pre-wrap', 'border': '1px solid #ccc', 'padding': '10px'})
    
    elif active_tab == "tab-plot":
        # Tab showing charts
        columns_options = get_columns()
        charts = []

        for chart in charts_data:
            chart_id = chart['id']
            selected_cols = chart.get('columns', [])

            charts.append(
                dbc.Card([
                    dbc.CardHeader([
                        html.Span(f"Chart {chart_id}", style={"fontWeight": "bold"}),
                        dbc.Button("Remove", color="danger", size="sm", id={'type': 'remove-chart-btn', 'index': chart_id}, style={"float": "right"})
                    ]),
                    dbc.CardBody([
                        dcc.Dropdown(
                            id={'type': 'columns-dropdown', 'index': chart_id},
                            options=columns_options,
                            value=selected_cols,
                            multi=True,
                            placeholder="Select columns to plot"
                        ),
                        dcc.Graph(id={'type': 'live-graph', 'index': chart_id}, style={"height": "300px"})
                    ])
                ], className="mb-3")
            )

        return html.Div([
            dbc.Button("Add New Chart", id="add-chart-btn", color="primary", className="mb-3", n_clicks=0),
            *charts
        ])
    
    return "No content available"

# ---------- Update Live File Monitor ----------
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

# ---------- Manage Add / Remove / Update Charts ----------
@app.callback(
    Output('charts-store', 'data'),
    [
        Input('add-chart-btn', 'n_clicks'),
        Input({'type': 'remove-chart-btn', 'index': ALL}, 'n_clicks'),
        Input({'type': 'columns-dropdown', 'index': ALL}, 'value')
    ],
    State('charts-store', 'data'),
    prevent_initial_call=True
)
def update_charts_store(add_click, remove_clicks, columns_values, charts_data):
    ctx_trigger = ctx.triggered_id

    if charts_data is None:
        charts_data = []

    if ctx_trigger == 'add-chart-btn':
        # Add a new chart config
        new_id = str(uuid.uuid4())[:8]
        charts_data.append({"id": new_id, "columns": []})
        return charts_data

    # Handle chart removal
    if isinstance(ctx_trigger, dict) and ctx_trigger.get('type') == 'remove-chart-btn':
        remove_id = ctx_trigger.get('index')
        charts_data = [c for c in charts_data if c['id'] != remove_id]
        return charts_data

    # Handle column dropdown change
    if isinstance(ctx_trigger, dict) and ctx_trigger.get('type') == 'columns-dropdown':
        changed_id = ctx_trigger.get('index')
        for i, chart in enumerate(charts_data):
            if chart['id'] == changed_id:
                charts_data[i]['columns'] = columns_values[i] if columns_values[i] else []
                break
        return charts_data

    raise dash.exceptions.PreventUpdate

# ---------- Update All Charts' Figures ----------
@app.callback(
    Output({'type': 'live-graph', 'index': ALL}, 'figure'),
    Input('interval-component', 'n_intervals'),
    Input('charts-store', 'data')
)
def update_charts(n_intervals, charts_data):
    if shared_ctx["df"].empty or not charts_data:
        empty_fig = go.Figure(layout=go.Layout(title="No data", xaxis_title="Time", yaxis_title="Value"))
        return empty_fig if len(charts_data) == 1 else [empty_fig] * len(charts_data)

    df = shared_ctx["df"].copy()

    try:
        df['timestamp'] = pd.to_datetime(df['timestamp'])
    except Exception:
        pass

    figures = []

    for chart in charts_data:
        selected_cols = chart.get('columns', [])
        if not selected_cols:
            fig = go.Figure(layout=go.Layout(title="Select columns", xaxis_title="Time", yaxis_title="Value"))
            figures.append(fig)
            continue

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
        figures.append(fig)

    return figures[0] if len(figures) == 1 else figures


# ---------- Start File Watcher in Background ----------
watcher_thread = threading.Thread(target=start_file_watcher, daemon=True)
watcher_thread.start()

# ---------- Run the Dash App ----------
if __name__ == '__main__':
    app.run(debug=True)
