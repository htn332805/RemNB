import dash
from dash import dcc, html, Input, Output, State
import scriptlets.python.core.ctx_logger as ctx_logger
import scriptlets.python.core.ctx_debug_dump as ctx_debug_dump

# Assume ctx is imported or passed in from orchestrator
from context import ctx

app = dash.Dash(__name__)

app.layout = html.Div([
    dcc.Dropdown(
        id='item-dropdown',
        options=[{'label': f'Item {i}', 'value': i} for i in range(5)],
        value=0
    ),
    html.Button('Save Selection', id='save-btn', n_clicks=0),
    html.Div(id='save-status')
])


@app.callback(
    Output('save-status', 'children'),
    Input('save-btn', 'n_clicks'),
    State('item-dropdown', 'value')
)
def save_selection(n_clicks, selected_value):
    if n_clicks > 0:
        # Save selection to ctx
        ctx['user_selection'] = selected_value

        # Log the action using ctx_logger
        ctx_logger.run(ctx, {
            "msg": f"User selected {selected_value}",
            "level": "INFO"
        })

        # Optionally, dump ctx to file for debugging
        ctx_debug_dump.run(ctx, {"out": "Logs/ctx_after_selection.json"})

        return f"Selection {selected_value} saved to ctx!"
    return ""