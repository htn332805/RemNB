import json, argparse, sys, pathlib
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

TARGETS = [
    "orchestrator/Data/normalized.json",
    "orchestrator/Data/numbers_enriched.csv"
]

class CleanupIntermediate(BaseScriptlet):
    def validate(self, ctx, params): pass
    def run(self, ctx, params):
        removed=[]
        for t in TARGETS:
            p=pathlib.Path(t)
            if p.exists():
                try:
                    p.unlink()
                    removed.append(t)
                except Exception:
                    pass
        try:
            ctx.set("cleanup.removed_v1", removed, who="cleanup_intermediate")
            print(json.dumps({"status":"ok","outputs":["cleanup.removed_v1"],"removed":removed}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"cleanup_intermediate"})); return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(CleanupIntermediate().run(Context(), p))
