### AUTOGEN(rebase_workspace.sh)
"""
DemoUpdateState – simulates operations against a (stub) context server.
Outputs: demo_update_state.requests_v1, demo_update_state.responses_v1, demo_update_state.summary_v1
"""
import json, sys, time
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class DemoUpdateState(BaseScriptlet):
    def validate(self, ctx, params):
        if "operations" not in params: raise ValueError("operations required")
        if not isinstance(params["operations"], list): raise ValueError("operations must list")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            reqs = params["operations"]
            responses=[]
            store={}
            for op in reqs:
                if not isinstance(op, list) or len(op) < 2:
                    responses.append({"error":"malformed"})
                    continue
                verb, key = op[0], op[1]
                if verb == "set":
                    val_json = op[2] if len(op)>2 else "null"
                    try:
                        val = json.loads(val_json)
                    except Exception:
                        val = val_json
                    store[key]=val
                    responses.append({"ok":True})
                elif verb == "append":
                    val_json = op[2] if len(op)>2 else "null"
                    try: val = json.loads(val_json)
                    except Exception: val=val_json
                    arr = store.setdefault(key, [])
                    if not isinstance(arr, list):
                        responses.append({"error":"type"})
                    else:
                        arr.append(val)
                        responses.append({"ok":True})
                elif verb == "get":
                    responses.append({"value":store.get(key)})
                else:
                    responses.append({"error":"unknown_verb"})
            summary = {"total_ops": len(reqs), "keys": sorted(store.keys())}
            ctx.set("demo_update_state.requests_v1", reqs, who="demo_update_state")
            ctx.set("demo_update_state.responses_v1", responses, who="demo_update_state")
            ctx.set("demo_update_state.summary_v1", summary, who="demo_update_state")
            print(json.dumps({"status":"ok","outputs":["demo_update_state.summary_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"demo_update_state"}))
            return 1
if __name__ == "__main__":
    print("Run via recipe", file=sys.stderr)
