"""
NAME: demo_update_state.py
DESCRIPTION: Demonstrate interacting with the running context server via update_state.sh
             by performing a sequence of operations (set/append/get) and storing both
             the raw requests and responses in ctx for later inspection.
USAGE:
  python orchestrator/scriptlets/python/steps/demo_update_state.py --params '{"operations":[["set","demo.user","{\"name\":\"Alice\"}"]]}'
PARAMETERS:
  - operations: list[list[str]] (required)
       Each inner list: [op, path, value?] where op in {set, append, get, delete, pop, full}
       For get/full/delete/pop the value item may be omitted.
  - host: str (optional, default "127.0.0.1")
  - port: int (optional, default 8765)
RETURNS (stdout JSON):
  {"status":"ok","outputs":["demo_update_state.requests_v1","demo_update_state.responses_v1","demo_update_state.summary_v1"]}
LIMITATIONS:
  - Assumes update_state.sh is executable and context server is already running.
  - Does minimal retry; if server not ready, operations may fail.
EXAMPLES:
  python orchestrator/scriptlets/python/steps/demo_update_state.py --params '{
    "operations":[["set","demo.user","{\\"name\\":\\"Alice\\",\\"age\\":30}"],["get","demo.user"]]}'
"""
import argparse, json, os, subprocess, sys, time
from typing import Any, Dict, List

class DemoUpdateState:
    def validate(self, ctx, params: Dict[str, Any]) -> None:
        assert isinstance(params, dict), "params must be dict"
        ops = params.get("operations")
        assert isinstance(ops, list) and ops, "operations must be non-empty list"
        for item in ops:
            assert isinstance(item, list) and 1 <= len(item) <= 3, "each operation list length 1..3"
        if "port" in params:
            assert isinstance(params["port"], int), "port must be int"
        if "host" in params:
            assert isinstance(params["host"], str), "host must be str"
        if "probe_retries" in params:
            assert isinstance(params["probe_retries"], int) and params["probe_retries"] > 0, "probe_retries must be >0 int"
        if "probe_delay_sec" in params:
            assert isinstance(params["probe_delay_sec"], (int, float)) and params["probe_delay_sec"] > 0, "probe_delay_sec must be >0"

    def _run_op(self, script: str, op: List[str]) -> Dict[str, Any]:
        cmd_parts = ["bash", script] + op
        try:
            out = subprocess.check_output(cmd_parts, stderr=subprocess.DEVNULL, text=True, timeout=3)
            # Attempt to parse JSON line (may be blank on some commands)
            out_stripped = out.strip()
            parsed = json.loads(out_stripped) if out_stripped.startswith("{") else {"raw": out_stripped}
            return {"ok": True, "op": op, "response": parsed}
        except subprocess.CalledProcessError as e:
            return {"ok": False, "op": op, "error": f"exit {e.returncode}", "stdout": e.output}
        except Exception as e:
            return {"ok": False, "op": op, "error": str(e)}

    def run(self, ctx, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            operations: List[List[str]] = params["operations"]
            host = params.get("host", "127.0.0.1")
            port = params.get("port", 8765)
            probe_retries = params.get("probe_retries", 30)       # increased default (~15s with 0.5 delay)
            probe_delay = params.get("probe_delay_sec", 0.5)

            script_path = "./update_state.sh"
            assert os.path.exists(script_path), "update_state.sh not found"
            env = os.environ.copy()
            env["SHCTX_PORT"] = str(port)
            env["SHCTX_HOST"] = host  # ensure host exported if script expects it

            ready = False
            for _ in range(probe_retries):
                probe = subprocess.run(
                    ["bash", script_path, "full"],
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                    text=True, env=env
                )
                if probe.returncode == 0 and probe.stdout.strip():
                    ready = True
                    break
                time.sleep(probe_delay)
            if not ready:
                raise RuntimeError(f"Context server not responding after {probe_retries} retries (delay {probe_delay}s)")

            requests_serializable = []
            responses_serializable = []
            for op in operations:
                result = self._run_op(script_path, op)
                requests_serializable.append(op)
                responses_serializable.append(result)

            summary = {
                "total": len(responses_serializable),
                "success": sum(1 for r in responses_serializable if r.get("ok")),
                "failed": sum(1 for r in responses_serializable if not r.get("ok")),
                "host": host,
                "port": port
            }

            # JSON serializable enforcement
            json.dumps(requests_serializable)
            json.dumps(responses_serializable)
            json.dumps(summary)

            ctx.set("demo_update_state.requests_v1", requests_serializable, who="demo_update_state")
            ctx.set("demo_update_state.responses_v1", responses_serializable, who="demo_update_state")
            ctx.set("demo_update_state.summary_v1", summary, who="demo_update_state")

            result = {
                "status": "ok",
                "outputs": [
                    "demo_update_state.requests_v1",
                    "demo_update_state.responses_v1",
                    "demo_update_state.summary_v1"
                ]
            }
            print(json.dumps(result))
            return 0
        except Exception as e:
            err = {
                "status": "error",
                "reason": str(e),
                "exit_code": 1,
                "step": "demo_update_state"
            }
            print(json.dumps(err))
            return 1

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--params", required=True, help="JSON string of parameters")
    args = p.parse_args()
    params = json.loads(args.params)
    class _ShimCtx:
        def __init__(self): self._d = {}
        def set(self, k, v, who="demo_update_state"):
            json.dumps(v)
            self._d[k] = v
    shim_ctx = _ShimCtx()
    sys.exit(DemoUpdateState().run(shim_ctx, params))