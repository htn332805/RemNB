"""
hello_dash.py
-------------
A Dash app scriptlet that displays and plots shared in-memory ctx data.

Description:
    - Launches a Dash app that monitors ctx[ctx_key] for changes.
    - Prints all ctx content on change.
    - Plots timestamp vs. selected columns from ctx[ctx_key] (multi-select).

Usage (CLI):
    python hello_dash.py --port 8050 --ctx_key new_row

Arguments:
    --port PORT      (required) Port number to run the Dash server on.
    --ctx_key KEY    (required) ctx key to monitor and plot.
    --host HOST      (optional) Host/IP to bind the server (default: 0.0.0.0).

Limitations:
    - Requires dash and its dependencies.
    - Expects ctx[ctx_key] as a list of lists: [timestamp, val1, val2, ...].
    - Column headers are ["timestamp", "col1", "col2", ...].

Example:
    python hello_dash.py --port 8050 --ctx_key new_row
"""
import sys
import argparse
import dash
from dash import dcc, html
from dash.dependencies import Output, Input, State
import plotly.graph_objs as go
import threading
import time

ctx = {}

def run(ctx_in, params):
    global ctx
    ctx = ctx_in
    port = params.get("port", 8050)
    host = params.get("host", "0.0.0.0")
    ctx_key = params["ctx_key"]
    launch_dash(ctx_key, port, host)

def launch_dash(ctx_key, port, host):
    app = dash.Dash(__name__)
    app.layout = html.Div([
        html.H1("Shared ctx Dash Monitor"),
        dcc.Dropdown(id='column-select', multi=True, placeholder="Select columns to plot"),
        dcc.Graph(id='live-graph'),
        html.Pre(id='ctx-print', style={'whiteSpace': 'pre-wrap'})
    ])

    last_ctx_snapshot = {"data": None}

    @app.callback(
        Output('column-select', 'options'),
        Output('column-select', 'value'),
        Input('live-graph', 'figure'),
        State('column-select', 'value')
    )
    def update_columns(fig, current_value):
        data = ctx.get(ctx_key, [])
        if not data:
            return [], []
        n_cols = len(data[0])
        options = [{"label": f"col{i}", "value": i} for i in range(1, n_cols)]
        # Preserve user selection if valid, else default to [1]
        if current_value and all(isinstance(v, int) and 0 < v < n_cols for v in current_value):
            value = current_value
        else:
            value = [1] if n_cols > 1 else []
        return options, value

    @app.callback(
        Output('live-graph', 'figure'),
        Output('ctx-print', 'children'),
        Input('column-select', 'value')
    )
    def update_graph(selected_cols):
        data = ctx.get(ctx_key, [])
        changed = False
        if data != last_ctx_snapshot["data"]:
            changed = True
            last_ctx_snapshot["data"] = [row[:] for row in data]
        ctx_str = ""
        if changed:
            ctx_str = f"ctx content changed:\n{ctx}"
            print(ctx_str)
        fig = go.Figure()
        if not data or not selected_cols:
            return fig, ctx_str
        n_cols = len(data[0])
        valid_cols = []
        if isinstance(selected_cols, int):
            selected_cols = [selected_cols]
        if isinstance(selected_cols, (list, tuple)):
            for col in selected_cols:
                if isinstance(col, int) and 0 < col < n_cols:
                    valid_cols.append(col)
        if not valid_cols:
            return fig, ctx_str
        timestamps = [row[0] for row in data]
        for col in valid_cols:
            try:
                y = [row[col] for row in data]
                fig.add_trace(go.Scatter(x=timestamps, y=y, mode='lines+markers', name=f"col{col}"))
            except Exception as e:
                print(f"Error plotting column {col}: {e}")
        fig.update_layout(title="Timestamp vs Selected Columns", xaxis_title="Timestamp")
        return fig, ctx_str

    print(f"[hello_dash] Running Dash app on http://{host}:{port}/")
    app.run_server(host=host, port=port)

def main():
    parser = argparse.ArgumentParser(description="Launch a Dash app to monitor shared ctx.")
    parser.add_argument('--port', type=int, required=True, help='Port number to run the Dash server on.')
    parser.add_argument('--ctx_key', type=str, required=True, help='ctx key to monitor and plot.')
    parser.add_argument('--host', type=str, default='0.0.0.0', help='Host/IP to bind the server (default: 0.0.0.0)')
    args = parser.parse_args()
    import random, datetime
    ctx[args.ctx_key] = []
    def updater():
        while True:
            now = datetime.datetime.now().isoformat()
            row = [now] + [random.random() for _ in range(3)]
            ctx[args.ctx_key].append(row)
            time.sleep(1)
    threading.Thread(target=updater, daemon=True).start()
    launch_dash(args.ctx_key, args.port, args.host)

if __name__ == "__main__":
    main()