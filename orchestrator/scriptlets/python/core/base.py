### AUTOGEN(rebase_workspace.sh)
"""
BaseScriptlet – contract:
- validate(ctx, params) (optional override)
- run(ctx, params) -> exit code
"""
import json, sys, time
class BaseScriptlet:
    parallelizable = False
    requires_process = False
    def validate(self, ctx, params):
        return
    def run(self, ctx, params):
        raise NotImplementedError("Implement in subclass")
def _demo():
    from orchestrator.context import Context
    class _Example(BaseScriptlet):
        def run(self, ctx, params):
            ctx.set("example.echo_v1", {"echo": params}, who="example")
            print(json.dumps({"status":"ok","outputs":["example.echo_v1"]}))
            return 0
    _Example().run(Context(), {"msg":"hi"})
if __name__ == "__main__":
    _demo()
