import json, argparse, sys, hashlib, pathlib
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

CACHE_DIR = pathlib.Path(".cache")
CACHE_DIR.mkdir(exist_ok=True)

class CacheNumbers(BaseScriptlet):
    def validate(self, ctx, params):
        if "src" not in params: raise ValueError("src required")

    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            src = params["src"]
            data = pathlib.Path(src).read_bytes()
            h = hashlib.sha256(data).hexdigest()[:16]
            marker = CACHE_DIR / f"numbers_{h}.done"
            if marker.exists():
                ctx.set("cache.numbers_hit_v1", True, who="cache_numbers")
                ctx.set("cache.numbers_hash_v1", h, who="cache_numbers")
                print(json.dumps({"status":"ok","outputs":["cache.numbers_hash_v1","cache.numbers_hit_v1"],"cache":"hit"}))
                return 0
            marker.write_text("ok")
            ctx.set("cache.numbers_hit_v1", False, who="cache_numbers")
            ctx.set("cache.numbers_hash_v1", h, who="cache_numbers")
            print(json.dumps({"status":"ok","outputs":["cache.numbers_hash_v1","cache.numbers_hit_v1"],"cache":"miss"}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"cache_numbers"})); return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(CacheNumbers().run(Context(), p))
