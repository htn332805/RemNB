import json, argparse, sys, datetime, pathlib
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

TEMPLATE_PATH = pathlib.Path("orchestrator/templates/report_template.md")
OUT_PATH = pathlib.Path("orchestrator/Data/report.md")

class GenerateReport(BaseScriptlet):
    def validate(self, ctx, params):
        if ctx.get("metrics.summary_v1") is None:
            raise ValueError("metrics.summary_v1 missing")

    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            template = TEMPLATE_PATH.read_text()
            # naive placeholders
            subs = {
                "generated_ts": datetime.datetime.utcnow().isoformat(),
                "metrics.summary_v1.count": ctx.get("metrics.summary_v1")["count"],
                "metrics.summary_v1.mean": ctx.get("metrics.summary_v1")["mean"],
                "metrics.summary_v1.min": ctx.get("metrics.summary_v1")["min"],
                "metrics.summary_v1.max": ctx.get("metrics.summary_v1")["max"],
                "metrics.summary_v2.p50": (ctx.get("metrics.summary_v2") or {}).get("p50"),
            }
            out = template
            for k,v in subs.items():
                out = out.replace("{{ "+k+" }}", str(v))
            # crude cleanup of conditional blocks
            if ctx.get("metrics.summary_v2"):
                out = out.replace("{% if metrics.summary_v2 %}","").replace("{% endif %}","")
            else:
                # remove block contents
                import re
                out = re.sub(r"\{\% if metrics.summary_v2 \%\}.*?\{\% endif \%\}","",out,flags=re.S)
            OUT_PATH.write_text(out)
            ctx.set("report.path_v1", str(OUT_PATH), who="generate_report")
            print(json.dumps({"status":"ok","outputs":["report.path_v1"],"artifact":str(OUT_PATH)}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"generate_report"})); return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(GenerateReport().run(Context(), p))
