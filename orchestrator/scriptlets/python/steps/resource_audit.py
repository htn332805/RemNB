import json, argparse, sys
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ResourceAudit(BaseScriptlet):
    def validate(self, ctx, params): pass
    def run(self, ctx, params):
        try:
            metrics = ctx.get("_runner.step_metrics_v1") or {}
            summary = {
                "total_steps": len(metrics),
                "steps": metrics
            }
            ctx.set("resource.audit_v1", summary, who="resource_audit")
            print(json.dumps({"status":"ok","outputs":["resource.audit_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"resource_audit"})); return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ResourceAudit().run(Context(), p))
