import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import threading
import time
import os

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# ---------- Shared in-memory context ----------
shared_ctx = {
    "lines": [],
    "modified": False
}

FILE_TO_WATCH = "cdu.log"

# ---------- File Watcher ----------
class FileChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path.endswith(FILE_TO_WATCH):
            try:
                with open(FILE_TO_WATCH, 'r') as f:
                    lines = f.readlines()
                    if lines:
                        last_line = lines[-1].strip()
                        if last_line and last_line not in shared_ctx["lines"]:
                            shared_ctx["lines"].append(last_line)
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
app = dash.Dash(__name__)
app.layout = html.Div([
    html.H2("Live File Monitor"),
    dcc.Interval(id='interval-component', interval=2000, n_intervals=0),
    html.Pre(id='live-content', style={'whiteSpace': 'pre-wrap', 'border': '1px solid #ccc', 'padding': '10px'})
])

@app.callback(
    Output('live-content', 'children'),
    Input('interval-component', 'n_intervals')
)
def update_output(n):
    if shared_ctx["modified"]:
        shared_ctx["modified"] = False
        return "\n".join(shared_ctx["lines"])
    raise dash.exceptions.PreventUpdate

# ---------- Run File Watcher in Background Thread ----------
watcher_thread = threading.Thread(target=start_file_watcher, daemon=True)
watcher_thread.start()

# ---------- Run App ----------
if __name__ == '__main__':
    app.run(debug=True)
