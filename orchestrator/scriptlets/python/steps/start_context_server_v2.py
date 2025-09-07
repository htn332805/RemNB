### AUTOGEN(rebase_workspace.sh)
"""
StartContextServerV2 – stub (no real server; placeholder for docs)
Produces: context_server.session_v1, context_server.ready_v1
"""
import json, time, sys
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class StartContextServerV2(BaseScriptlet):
    def run(self, ctx, params):
        try:
            sess = {"session":"stub", "ts": time.time()}
            ctx.set("context_server.session_v1", sess, who="start_context_server_v2")
            ctx.set("context_server.launch_cmd_v1", {"cmd":"python run_context_server.py"}, who="start_context_server_v2")
            ctx.set("context_server.ready_v1", {"ready": True, "ts": time.time()}, who="start_context_server_v2")
            print(json.dumps({"status":"ok","outputs":["context_server.session_v1","context_server.ready_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"start_context_server_v2"}))
            return 1
if __name__ == "__main__":
    print("Run via recipe", file=sys.stderr)
