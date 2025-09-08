import json, argparse, sys, math
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

def factors(n:int):
    f=[]
    d=2
    while d*d<=n:
        while n%d==0:
            f.append(d); n//=d
        d+=1
    if n>1: f.append(n)
    return f

class PrimeFactorAnalysis(BaseScriptlet):
    requires_process = True   # runner_v2 picks process pool
    def validate(self, ctx, params):
        if ctx.get("numbers.raw_v1") is None: raise ValueError("raw list missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            raw = ctx.get("numbers.raw_v1")
            result = {str(n): factors(max(2,n)) for n in raw}
            ctx.set("numbers.prime_factors_v1", result, who="prime_factor_analysis")
            print(json.dumps({"status":"ok","outputs":["numbers.prime_factors_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"prime_factor_analysis"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(PrimeFactorAnalysis().run(Context(), p))
