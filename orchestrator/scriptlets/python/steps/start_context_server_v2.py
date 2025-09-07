"""
NAME: start_context_server_v2.py
DESCRIPTION: Enhanced (backward-compatible) launcher for run_context_server.py in a detached GNU
             screen session (or fallback background) with an optional readiness wait on host:port.
DELTA vs v1:
  - Adds working_dir param (default: directory of script)
  - Adds wait_for_port (bool), wait_host, wait_port, wait_timeout_sec for positive readiness
  - Uses 'cd <working_dir> && <python> <script>' inside screen to preserve imports
  - Records readiness under context_server.ready_v1 key
USAGE:
  python orchestrator/scriptlets/python/steps/start_context_server_v2.py --params '{"screen_name":"ctx_server","script":"run_context_server.py","wait_for_port":true,"wait_port":8765}'
PARAMETERS:
  - screen_name: str (required) name of GNU screen session
  - script: str (required) path to Python server script
  - working_dir: str (optional) directory to cd into before launching (default: dirname(script))
  - wait_for_port: bool (optional, default false) enable active polling for socket readiness
  - wait_host: str (default "127.0.0.1") host to poll
  - wait_port: int (default 8765) port to poll
  - wait_timeout_sec: float (default 30.0) overall timeout for readiness
RETURNS (stdout JSON):
  {"status":"ok","outputs":["context_server.session_v1","context_server.launch_cmd_v1","context_server.ready_v1"]}
LIMITATIONS:
  - Requires 'screen' for detached session (otherwise falls back to background process)
  - Does not terminate the server; manual cleanup required
EXAMPLES:
  python orchestrator/scriptlets/python/steps/start_context_server_v2.py --params '{"screen_name":"ctx_server","script":"run_context_server.py","wait_for_port":true}'
"""
import argparse, json, os, shutil, subprocess, sys, time, socket, shlex
from typing import Any, Dict

class StartContextServerV2:
    def validate(self, ctx, params: Dict[str, Any]) -> None:
        assert "screen_name" in params and isinstance(params["screen_name"], str), "screen_name required"
        assert "script" in params and isinstance(params["script"], str), "script required"
        assert os.path.exists(params["script"]), f"script not found: {params['script']}"
        if "working_dir" in params:
            assert os.path.isdir(params["working_dir"]), "working_dir must exist"
        if "wait_for_port" in params:
            assert isinstance(params["wait_for_port"], bool), "wait_for_port must be bool"

    def _screen_exists(self, name: str) -> bool:
        try:
            out = subprocess.check_output(["screen", "-ls"], text=True, stderr=subprocess.DEVNULL)
            return f".{name}\t" in out or f".{name}\n" in out
        except Exception:
            return False

    def _wait_for_port(self, host: str, port: int, timeout: float) -> bool:
        deadline = time.time() + timeout
        while time.time() < deadline:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.settimeout(0.5)
                try:
                    if s.connect_ex((host, port)) == 0:
                        return True
                except Exception:
                    pass
            time.sleep(0.5)
        return False

    def run(self, ctx, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            screen_name = params["screen_name"]
            script_abs = os.path.abspath(params["script"])
            working_dir = params.get("working_dir") or os.path.dirname(script_abs) or "."
            python_exec = sys.executable
            base_cmd = f"cd {shlex.quote(working_dir)} && {shlex.quote(python_exec)} {shlex.quote(script_abs)}"
            launch_cmd: str
            used_screen = False

            if shutil.which("screen"):
                if not self._screen_exists(screen_name):
                    subprocess.check_call(["screen", "-S", screen_name, "-dm", "bash", "-lc", base_cmd])
                    time.sleep(1.0)
                used_screen = self._screen_exists(screen_name)
                launch_cmd = f"screen -S {screen_name} -dm bash -lc '{base_cmd}'"
            else:
                proc = subprocess.Popen(
                    ["bash", "-lc", base_cmd],
                    cwd=working_dir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                launch_cmd = f"(fallback background pid={proc.pid}) {base_cmd}"
                try:
                    ctx.set("context_server.pid_v1", proc.pid, who="start_context_server_v2")
                except Exception:
                    pass

            # Optional readiness wait
            ready = False
            if params.get("wait_for_port"):
                host = params.get("wait_host", "127.0.0.1")
                port = int(params.get("wait_port", 8765))
                timeout_sec = float(params.get("wait_timeout_sec", 30.0))
                ready = self._wait_for_port(host, port, timeout_sec)
                if not ready:
                    # Still emit partial metadata but error exit
                    ctx.set("context_server.session_v1", {"screen_name": screen_name, "used_screen": used_screen}, who="start_context_server_v2")
                    ctx.set("context_server.launch_cmd_v1", launch_cmd, who="start_context_server_v2")
                    ctx.set("context_server.ready_v1", {"ready": False, "wait_host": host, "wait_port": port}, who="start_context_server_v2")
                    print(json.dumps({
                        "status": "error",
                        "reason": f"Port {host}:{port} not ready within timeout",
                        "exit_code": 1,
                        "step": "start_context_server_v2"
                    }))
                    return 1
            else:
                ready = True  # not enforced

            ctx.set("context_server.session_v1", {"screen_name": screen_name, "used_screen": used_screen}, who="start_context_server_v2")
            ctx.set("context_server.launch_cmd_v1", launch_cmd, who="start_context_server_v2")
            ctx.set("context_server.ready_v1", {"ready": ready}, who="start_context_server_v2")

            print(json.dumps({
                "status": "ok",
                "outputs": ["context_server.session_v1", "context_server.launch_cmd_v1", "context_server.ready_v1"]
            }))
            return 0
        except Exception as e:
            print(json.dumps({
                "status": "error",
                "reason": str(e),
                "exit_code": 1,
                "step": "start_context_server_v2"
            }))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    args = ap.parse_args()
    params = json.loads(args.params)
    class _ShimCtx:
        def __init__(self): self._d = {}
        def set(self, k, v, who="?"):
            json.dumps(v); self._d[k] = v
    sys.exit(StartContextServerV2().run(_ShimCtx(), params))