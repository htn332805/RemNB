import json, argparse, sys
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class InjectFailure(BaseScriptlet):
    def validate(self, ctx, params): pass
    def run(self, ctx, params):
        try:
            if params.get("fail"):
                raise RuntimeError("Injected failure for resilience demo")
            ctx.set("resilience.inject_ok_v1", True, who="inject_failure")
            print(json.dumps({"status":"ok","outputs":["resilience.inject_ok_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"inject_failure"})); return 1
if __name__ == "__main__":
    import argparse, json, sys
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(InjectFailure().run(Context(), p))
