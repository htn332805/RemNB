#!/usr/bin/env bash
set -euo pipefail

echo "[INIT] Creating directories..."
mkdir -p orchestrator/scriptlets/python/steps
mkdir -p orchestrator/scriptlets/shell
mkdir -p orchestrator/scriptlets/python/apps
mkdir -p orchestrator/recipes
mkdir -p orchestrator/templates
mkdir -p orchestrator/Data
mkdir -p tests
mkdir -p .cache

echo "[SEED] Sample input data (numbers.csv)..."
cat > orchestrator/Data/numbers.csv <<'EOF'
# demo numbers
1
2
3
4
5
EOF

echo "[SEED] Bad numbers file for validation failures..."
cat > orchestrator/Data/numbers_bad.csv <<'EOF'
# too few + out of range
-5
9999
EOF

echo "[TEMPLATE] Report markdown template..."
cat > orchestrator/templates/report_template.md <<'EOF'
# Automated Metrics Report

Generated: {{ generated_ts }}

## Summary (v1)
Count: {{ metrics.summary_v1.count }}
Mean: {{ metrics.summary_v1.mean }}
Min: {{ metrics.summary_v1.min }}
Max: {{ metrics.summary_v1.max }}

## Extended (v2 if present)
{% if metrics.summary_v2 %}
P50: {{ metrics.summary_v2.p50 }}
P90: {{ metrics.summary_v2.p90 }}
P99: {{ metrics.summary_v2.p99 }}
{% endif %}

## Aggregated
{{ metrics.aggregate_v1 | tojson }}

## Resources
{{ resource.audit_v1 | tojson }}

(End of Report)
EOF

echo "[STUB] Excel metrics summary placeholder..."
echo "Stub XLSX (replace with real export later)" > orchestrator/templates/metrics_summary.xlsx

echo "[APP] live_dashboard (stub polling context server or file)..."
cat > orchestrator/scriptlets/python/apps/live_dashboard.py <<'EOF'
"""
Module 9: Dash(-like) live dashboard stub.
Polls either:
 - Context server via host/port (JSON API assumed)
 - Fallback: normalized JSON file.

Non-blocking launch expected (runner_v2 spawns).
"""
import os, json, time, threading, http.client
from pathlib import Path

NORMALIZED = Path("orchestrator/Data/normalized.json")

def fetch_ctx(host, port):
    try:
        conn = http.client.HTTPConnection(host, port, timeout=1)
        conn.request("GET", "/full")
        resp = conn.getresponse()
        if resp.status == 200:
            return json.loads(resp.read().decode())
    except Exception:
        return {}
    return {}

def loop(host, port, interval=2.0):
    while True:
        data = fetch_ctx(host, port)
        if not data and NORMALIZED.is_file():
            try:
                data = json.loads(NORMALIZED.read_text())
            except Exception:
                data = {}
        print("[dashboard] snapshot keys:", list(data.keys())[:10])
        time.sleep(interval)

def main():
    host = os.environ.get("DASH_HOST", "127.0.0.1")
    port = int(os.environ.get("DASH_PORT", "8765"))
    t = threading.Thread(target=loop, args=(host, port), daemon=True)
    t.start()
    # Keep alive minimal
    try:
        while True:
            time.sleep(60)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()
EOF

echo "[SHELL] sanitize_csv.sh (Module 2)..."
cat > orchestrator/scriptlets/shell/sanitize_csv.sh <<'EOF'
#!/usr/bin/env bash
# Remove blank/comment (#) lines from CSV.
set -euo pipefail
INPUT="${1:-orchestrator/Data/numbers.csv}"
OUT="orchestrator/Data/numbers_sanitized.csv"
grep -v '^[[:space:]]*$' "$INPUT" | grep -v '^[[:space:]]*#' > "$OUT"
echo "{\"status\":\"ok\",\"outputs\":[\"$OUT\"],\"sanitized_path\":\"$OUT\"}"
EOF
chmod +x orchestrator/scriptlets/shell/sanitize_csv.sh

echo "[SHELL] enrich_csv.sh (Module 4)..."
cat > orchestrator/scriptlets/shell/enrich_csv.sh <<'EOF'
#!/usr/bin/env bash
# Adds a derived column (square) to sanitized CSV.
set -euo pipefail
INPUT="${1:-orchestrator/Data/numbers_sanitized.csv}"
OUT="orchestrator/Data/numbers_enriched.csv"
{
  echo "value,square"
  awk 'NR>0 && $1 !~ /^#/ && $1!="" {print $1","($1*$1)}' "$INPUT"
} > "$OUT"
echo "{\"status\":\"ok\",\"outputs\":[\"$OUT\"],\"enriched_path\":\"$OUT\"}"
EOF
chmod +x orchestrator/scriptlets/shell/enrich_csv.sh

echo "[SCRIPTLET] load_numbers.py (Module 1)..."
cat > orchestrator/scriptlets/python/steps/load_numbers.py <<'EOF'
"""
Module 1: Load numbers from a CSV into context.
Outputs: numbers.raw_v1 -> List[int]
"""
import json, argparse, sys, pathlib
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class LoadNumbers(BaseScriptlet):
    parallelizable = False
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        src = params.get("src")
        if not src or not pathlib.Path(src).is_file():
            raise ValueError("src file not found")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            src = params["src"]
            nums: List[int] = []
            with open(src) as f:
                for line in f:
                    line=line.strip()
                    if not line or line.startswith("#"): continue
                    nums.append(int(line.split(",")[0]))
            ctx.set("numbers.raw_v1", nums, who="load_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.raw_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"load_numbers"}))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(LoadNumbers().run(Context(), p))
EOF

echo "[SCRIPTLET] validate_numbers.py (Module 3)..."
cat > orchestrator/scriptlets/python/steps/validate_numbers.py <<'EOF'
"""
Module 3: Validate numbers constraints.
Errors produce structured JSON.
"""
import json, argparse, sys, statistics
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from datetime import datetime, timezone

class ValidateNumbers(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if "min_count" not in params: raise ValueError("min_count required")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            nums = ctx.get("numbers.raw_v1") or []
            min_count = params["min_count"]
            min_val = params.get("min_value", -10**9)
            max_val = params.get("max_value", 10**9)
            if len(nums) < min_count:
                raise ValueError(f"Too few numbers (min={min_count})")
            for n in nums:
                if n < min_val or n > max_val:
                    raise ValueError(f"Number {n} out of range [{min_val},{max_val}]")
            ctx.set("numbers.validated_v1", True, who="validate_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.validated_v1"]}))
            return 0
        except Exception as e:
            err = {
                "status":"error",
                "reason":str(e),
                "details":{"count": len(ctx.get('numbers.raw_v1') or [])},
                "exit_code":1,
                "step":"validate_numbers",
                "ts": datetime.now(timezone.utc).isoformat()
            }
            print(json.dumps(err))
            return 1

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(ValidateNumbers().run(Context(), p))
EOF

echo "[SCRIPTLET] normalize_numbers.py (Module 4)..."
cat > orchestrator/scriptlets/python/steps/normalize_numbers.py <<'EOF'
"""
Module 4: Normalize numbers (min-max 0..1) write file & ctx.
Outputs:
 - numbers.normalized_v1 (list[float])
 - numbers.stats_v1 (dict)
Artifact: orchestrator/Data/normalized.json
"""
import json, argparse, sys, statistics, pathlib
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

OUT_FILE = pathlib.Path("orchestrator/Data/normalized.json")

class NormalizeNumbers(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.raw_v1") is None:
            raise ValueError("numbers.raw_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            nums = ctx.get("numbers.raw_v1")
            mn, mx = min(nums), max(nums)
            rng = mx - mn if mx != mn else 1
            normalized = [(n - mn)/rng for n in nums]
            stats = {
                "count": len(nums),
                "mean": statistics.fmean(nums),
                "min": mn,
                "max": mx
            }
            OUT_FILE.write_text(json.dumps({"normalized": normalized, "stats": stats}, indent=2))
            ctx.set("numbers.normalized_v1", normalized, who="normalize_numbers")
            ctx.set("numbers.stats_v1", stats, who="normalize_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.normalized_v1","numbers.stats_v1"],"artifact":str(OUT_FILE)}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"normalize_numbers"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(NormalizeNumbers().run(Context(), p))
EOF

echo "[SCRIPTLET] compute_metrics_v1.py (Module 5)..."
cat > orchestrator/scriptlets/python/steps/compute_metrics_v1.py <<'EOF'
"""
Module 5: Basic metrics v1.
Outputs: metrics.summary_v1, metrics.columns_v1
"""
import json, argparse, sys, statistics
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeMetricsV1(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.normalized_v1") is None:
            raise ValueError("numbers.normalized_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            vals: List[float] = ctx.get("numbers.normalized_v1")
            summary = {
                "count": len(vals),
                "mean": statistics.fmean(vals),
                "min": min(vals),
                "max": max(vals)
            }
            ctx.set("metrics.summary_v1", summary, who="compute_metrics_v1")
            ctx.set("metrics.columns_v1", list(summary.keys()), who="compute_metrics_v1")
            print(json.dumps({"status":"ok","outputs":["metrics.summary_v1","metrics.columns_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_metrics_v1"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params", required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeMetricsV1().run(Context(), p))
EOF

echo "[SCRIPTLETS] compute_mean / median / std (Module 6)..."
cat > orchestrator/scriptlets/python/steps/compute_mean.py <<'EOF'
import json, argparse, sys, statistics
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeMean(BaseScriptlet):
    parallelizable = True
    def validate(self, ctx, params): 
        if ctx.get("numbers.normalized_v1") is None: raise ValueError("normalized missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            data = ctx.get("numbers.normalized_v1")
            ctx.set("metrics.mean_v1", float(statistics.fmean(data)), who="compute_mean")
            print(json.dumps({"status":"ok","outputs":["metrics.mean_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_mean"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ComputeMean().run(Context(), p))
EOF

cat > orchestrator/scriptlets/python/steps/compute_median.py <<'EOF'
import json, argparse, sys, statistics
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeMedian(BaseScriptlet):
    parallelizable = True
    def validate(self, ctx, params):
        if ctx.get("numbers.normalized_v1") is None: raise ValueError("normalized missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            vals = ctx.get("numbers.normalized_v1")
            ctx.set("metrics.median_v1", float(statistics.median(vals)), who="compute_median")
            print(json.dumps({"status":"ok","outputs":["metrics.median_v1"]})); return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_median"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ComputeMedian().run(Context(), p))
EOF

cat > orchestrator/scriptlets/python/steps/compute_std.py <<'EOF'
import json, argparse, sys, statistics, math
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class ComputeStd(BaseScriptlet):
    parallelizable = True
    def validate(self, ctx, params):
        if ctx.get("numbers.normalized_v1") is None: raise ValueError("normalized missing")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            vals = ctx.get("numbers.normalized_v1")
            mean = statistics.fmean(vals)
            variance = sum((x-mean)**2 for x in vals)/len(vals)
            ctx.set("metrics.std_v1", math.sqrt(variance), who="compute_std")
            print(json.dumps({"status":"ok","outputs":["metrics.std_v1"]})); return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_std"})); return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(ComputeStd().run(Context(), p))
EOF

echo "[SCRIPTLET] aggregate_metrics.py (Module 6 fan-in)..."
cat > orchestrator/scriptlets/python/steps/aggregate_metrics.py <<'EOF'
import json, argparse, sys
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

class AggregateMetrics(BaseScriptlet):
    def validate(self, ctx, params):
        for k in ["metrics.mean_v1","metrics.median_v1","metrics.std_v1"]:
            if ctx.get(k) is None:
                raise ValueError(f"missing dependency key {k}")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            agg = {
                "mean": ctx.get("metrics.mean_v1"),
                "median": ctx.get("metrics.median_v1"),
                "std": ctx.get("metrics.std_v1")
            }
            ctx.set("metrics.aggregate_v1", agg, who="aggregate_metrics")
            print(json.dumps({"status":"ok","outputs":["metrics.aggregate_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"aggregate_metrics"}))
            return 1
if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params); sys.exit(AggregateMetrics().run(Context(), p))
EOF

echo "[SCRIPTLET] prime_factor_analysis.py (Module 7 CPU/process)..."
cat > orchestrator/scriptlets/python/steps/prime_factor_analysis.py <<'EOF'
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
EOF

echo "[SCRIPTLET] compute_metrics_v2.py (Module 8 version evolution)..."
cat > orchestrator/scriptlets/python/steps/compute_metrics_v2.py <<'EOF'
"""
Adds percentiles while preserving v1 keys.
Outputs (additive):
 - metrics.summary_v1 (unchanged shape)
 - metrics.summary_v2 (extended)
"""
import json, argparse, sys, statistics
from typing import Any, Dict, List
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet

def percentile(values: List[float], p: float) -> float:
    if not values: return 0.0
    k = (len(values)-1) * p
    f = int(k); c = min(f+1, len(values)-1)
    if f == c: return values[f]
    d = k - f
    return values[f] + (values[c]-values[f]) * d

class ComputeMetricsV2(BaseScriptlet):
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        if ctx.get("numbers.normalized_v1") is None:
            raise ValueError("numbers.normalized_v1 missing")

    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        try:
            self.validate(ctx, params)
            vals = list(ctx.get("numbers.normalized_v1"))
            vals_sorted = sorted(vals)
            summary_v1 = {
                "count": len(vals),
                "mean": statistics.fmean(vals),
                "min": min(vals),
                "max": max(vals)
            }
            summary_v2 = dict(summary_v1)
            summary_v2.update({
                "p50": percentile(vals_sorted, 0.5),
                "p90": percentile(vals_sorted, 0.9),
                "p99": percentile(vals_sorted, 0.99)
            })
            ctx.set("metrics.summary_v1", summary_v1, who="compute_metrics_v2")
            ctx.set("metrics.summary_v2", summary_v2, who="compute_metrics_v2")
            print(json.dumps({"status":"ok","outputs":["metrics.summary_v1","metrics.summary_v2"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_metrics_v2"}))
            return 1

if __name__ == "__main__":
    ap=argparse.ArgumentParser(); ap.add_argument("--params",required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeMetricsV2().run(Context(), p))
EOF

echo "[SCRIPTLET] generate_report.py (Module 10)..."
cat > orchestrator/scriptlets/python/steps/generate_report.py <<'EOF'
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
EOF

echo "[SCRIPTLET] inject_failure.py (Module 11)..."
cat > orchestrator/scriptlets/python/steps/inject_failure.py <<'EOF'
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
EOF

echo "[SCRIPTLET] cache_numbers.py (Module 12 caching)..."
cat > orchestrator/scriptlets/python/steps/cache_numbers.py <<'EOF'
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
EOF

echo "[SCRIPTLET] resource_audit.py (Module 12 observability)..."
cat > orchestrator/scriptlets/python/steps/resource_audit.py <<'EOF'
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
EOF

echo "[SCRIPTLET] cleanup_intermediate.py (Module 12 cleanup)..."
cat > orchestrator/scriptlets/python/steps/cleanup_intermediate.py <<'EOF'
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
EOF

echo "[RUNNER V2] Enhanced runner with shell, deps, parallel, process, skip/only/resume, caching, resource metrics..."
cat > orchestrator/runner_v2.py <<'EOF'
"""
runner_v2: Extended capabilities:
 - Shell steps (type: shell) expecting JSON stdout
 - Dependencies (depends_on)
 - Parallel execution for steps with parallelizable = True
 - Process pool for steps with requires_process = True
 - Skip / only / resume-from CLI filters
 - Basic caching via recipe 'cache_key'
 - Resource timing metrics -> ctx key: _runner.step_metrics_v1
 - Context merge strategy: last_write_wins (placeholder for alternative strategies)

Usage:
  python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml
"""
from __future__ import annotations
import argparse, importlib, json, time, subprocess, os, threading
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor, as_completed
from pathlib import Path
from typing import Any, Dict, List, Set

import yaml
from orchestrator.context import Context

LOCK = threading.Lock()

def _import_class(module: str, cls: str):
    mod = importlib.import_module(module)
    return getattr(mod, cls)

def _run_python_step(ctx: Context, step: Dict[str, Any]) -> int:
    cls = _import_class(step["module"], step["function"])
    inst = cls()
    params = step.get("args", {})
    start = time.time()
    code = inst.run(ctx, params)
    dur = (time.time() - start) * 1000
    with LOCK:
        m = ctx.get("_runner.step_metrics_v1") or {}
        m[step["name"]] = {"duration_ms": dur, "exit_code": code, "module": step["module"]}
        ctx.set("_runner.step_metrics_v1", m, who="runner_v2")
    return code

def _run_shell_step(ctx: Context, step: Dict[str, Any]) -> int:
    script = step["script"]
    args = step.get("args", [])
    start = time.time()
    try:
        out = subprocess.check_output(["bash", script, *args], text=True)
        line = out.strip().splitlines()[-1]
        data = json.loads(line)
        # Optionally push any outputs? We rely on contract only
        code = 0 if data.get("status") == "ok" else data.get("exit_code", 1)
    except Exception as e:
        data = {"status":"error","reason":str(e),"exit_code":1,"step":step["name"]}
        code = 1
        print(json.dumps(data))
    dur = (time.time() - start) * 1000
    with LOCK:
        m = ctx.get("_runner.step_metrics_v1") or {}
        m[step["name"]] = {"duration_ms": dur, "exit_code": code, "script": script}
        ctx.set("_runner.step_metrics_v1", m, who="runner_v2")
    return code

def topo_sort(steps: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    by_name = {s["name"]: s for s in steps}
    in_deg = {s["name"]: 0 for s in steps}
    for s in steps:
        for d in s.get("depends_on", []):
            in_deg[s["name"]] += 1
    ready = [n for n,v in in_deg.items() if v==0]
    order=[]
    while ready:
        n = ready.pop()
        order.append(by_name[n])
        for s in steps:
            if n in s.get("depends_on", []):
                in_deg[s["name"]] -=1
                if in_deg[s["name"]] ==0:
                    ready.append(s["name"])
    if len(order)!=len(steps):
        raise RuntimeError("Cycle detected in dependencies")
    return order

def filter_steps(steps, only: Set[str], skip: Set[str], resume_from: str|None):
    filtered=[]
    resume_found = resume_from is None
    for s in steps:
        nm = s["name"]
        if not resume_found:
            if nm == resume_from:
                resume_found = True
            else:
                continue
        if only and nm not in only: continue
        if nm in skip: continue
        filtered.append(s)
    return filtered

def run_recipe_v2(recipe_path: str, only: Set[str], skip: Set[str], resume_from: str|None) -> Context:
    spec = yaml.safe_load(Path(recipe_path).read_text())
    steps = spec.get("steps", [])
    steps = topo_sort(steps)
    steps = filter_steps(steps, only, skip, resume_from)
    ctx = Context()
    # simple caching skip
    for step in steps:
        ck = step.get("cache_key")
        if ck:
            marker = Path(".cache")/f"{ck}.done"
            if marker.exists():
                continue
        # gather runnable parallel batch
        # For simplicity execute sequential except fan-in of explicit parallelizable set with identical depends satisfaction
        # (Advanced scheduler omitted for brevity)
        if step.get("type") == "python":
            cls = _import_class(step["module"], step["function"])
            parallel = getattr(cls, "parallelizable", False)
            requires_proc = getattr(cls, "requires_process", False)
            if parallel:
                # collect consecutive parallelizable python steps with no unmet dependencies
                batch=[step]
                # (naive: just run single parallel or extended by lookahead)
                if requires_proc:
                    with ProcessPoolExecutor() as pe:
                        fut = pe.submit(_run_python_step, ctx, step)
                        code = fut.result()
                else:
                    with ThreadPoolExecutor() as te:
                        fut = te.submit(_run_python_step, ctx, step)
                        code = fut.result()
            else:
                if requires_proc:
                    with ProcessPoolExecutor() as pe:
                        fut = pe.submit(_run_python_step, ctx, step)
                        code = fut.result()
                else:
                    code = _run_python_step(ctx, step)
        elif step.get("type") == "shell":
            code = _run_shell_step(ctx, step)
        else:
            raise NotImplementedError(f"Unsupported step type {step.get('type')}")
        ck = step.get("cache_key")
        if ck and code==0:
            marker = Path(".cache")/f"{ck}.done"
            marker.parent.mkdir(exist_ok=True)
            marker.write_text("ok")
        if code != 0:
            raise RuntimeError(f"Step {step['name']} failed exit {code}")
    return ctx

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--recipe", required=True)
    ap.add_argument("--only", nargs="*", default=[])
    ap.add_argument("--skip", nargs="*", default=[])
    ap.add_argument("--resume-from")
    args = ap.parse_args()
    ctx = run_recipe_v2(args.recipe, set(args.only), set(args.skip), args.resume_from)
    print(json.dumps({"status":"ok","ctx_keys": list(ctx.to_dict().keys())}))
if __name__ == "__main__":
    main()
EOF

echo "[RECIPES] Generating module recipes..."
cat > orchestrator/recipes/01_basic_number.yaml <<'EOF'
test_meta: { test_id: "M01", description: "Foundations single step" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args:
      src: orchestrator/Data/numbers.csv
    success:
      ctx_has_keys: [numbers.raw_v1]
EOF

cat > orchestrator/recipes/02_basic_mixed.yaml <<'EOF'
test_meta: { test_id: "M02", description: "Shell sanitize + load" }
steps:
  - idx: 1
    name: sanitize_csv
    type: shell
    script: orchestrator/scriptlets/shell/sanitize_csv.sh
    args: [orchestrator/Data/numbers.csv]
  - idx: 2
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args:
      src: orchestrator/Data/numbers_sanitized.csv
EOF

cat > orchestrator/recipes/03_validation_errors.yaml <<'EOF'
test_meta: { test_id: "M03", description: "Validation & error handling" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers_bad.csv }
  - idx: 2
    name: validate_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.validate_numbers
    function: ValidateNumbers
    args: { min_count: 5, min_value: 0, max_value: 100 }
EOF

cat > orchestrator/recipes/04_artifacts_pipeline.yaml <<'EOF'
test_meta: { test_id: "M04", description: "Artifacts & normalization" }
steps:
  - idx: 1
    name: sanitize_csv
    type: shell
    script: orchestrator/scriptlets/shell/sanitize_csv.sh
    args: [orchestrator/Data/numbers.csv]
  - idx: 2
    name: enrich_csv
    type: shell
    script: orchestrator/scriptlets/shell/enrich_csv.sh
    args: [orchestrator/Data/numbers_sanitized.csv]
  - idx: 3
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers_enriched.csv }
  - idx: 4
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
EOF

cat > orchestrator/recipes/05_metrics_versioned.yaml <<'EOF'
test_meta: { test_id: "M05", description: "Metrics v1" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  - idx: 3
    name: compute_metrics_v1
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v1
    function: ComputeMetricsV1
EOF

cat > orchestrator/recipes/06_parallel_fan_in.yaml <<'EOF'
test_meta: { test_id: "M06", description: "Parallel fan-in metrics" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  - idx: 10
    name: compute_mean
    type: python
    module: orchestrator.scriptlets.python.steps.compute_mean
    function: ComputeMean
    depends_on: [normalize_numbers]
  - idx: 11
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    depends_on: [normalize_numbers]
  - idx: 12
    name: compute_std
    type: python
    module: orchestrator.scriptlets.python.steps.compute_std
    function: ComputeStd
    depends_on: [normalize_numbers]
  - idx: 13
    name: aggregate_metrics
    type: python
    module: orchestrator.scriptlets.python.steps.aggregate_metrics
    function: AggregateMetrics
    depends_on: [compute_mean, compute_median, compute_std]
EOF

cat > orchestrator/recipes/07_cpu_process.yaml <<'EOF'
test_meta: { test_id: "M07", description: "CPU heavy process step" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: prime_factor_analysis
    type: python
    module: orchestrator.scriptlets.python.steps.prime_factor_analysis
    function: PrimeFactorAnalysis
    depends_on: [load_numbers]
EOF

cat > orchestrator/recipes/08_versioning_wrapper.yaml <<'EOF'
test_meta: { test_id: "M08", description: "Metrics v2 additive" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  - idx: 3
    name: compute_metrics_v2
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v2
    function: ComputeMetricsV2
EOF

cat > orchestrator/recipes/09_dashboard.yaml <<'EOF'
test_meta: { test_id: "M09", description: "Dashboard integration (stub)" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  - idx: 3
    name: compute_metrics_v2
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v2
    function: ComputeMetricsV2
  # Dashboard launch would normally be a custom step (omitted practical binding here)
EOF

cat > orchestrator/recipes/10_reporting.yaml <<'EOF'
test_meta: { test_id: "M10", description: "Reporting pipeline" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  - idx: 3
    name: compute_metrics_v2
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v2
    function: ComputeMetricsV2
  - idx: 4
    name: generate_report
    type: python
    module: orchestrator.scriptlets.python.steps.generate_report
    function: GenerateReport
    depends_on: [compute_metrics_v2]
EOF

cat > orchestrator/recipes/11_resilience_controls.yaml <<'EOF'
test_meta: { test_id: "M11", description: "Resilience controls" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: inject_failure
    type: python
    module: orchestrator.scriptlets.python.steps.inject_failure
    function: InjectFailure
    args: { fail: true }
  - idx: 3
    name: compute_metrics_v2
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v2
    function: ComputeMetricsV2
EOF

cat > orchestrator/recipes/12_advanced_full.yaml <<'EOF'
test_meta: { test_id: "M12", description: "Full advanced pipeline" }
steps:
  - idx: 1
    name: cache_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.cache_numbers
    function: CacheNumbers
    args: { src: orchestrator/Data/numbers.csv }
    cache_key: "cache_numbers_v1"
  - idx: 2
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
    depends_on: [cache_numbers]
  - idx: 3
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
    depends_on: [load_numbers]
  - idx: 4
    name: compute_metrics_v2
    type: python
    module: orchestrator.scriptlets.python.steps.compute_metrics_v2
    function: ComputeMetricsV2
    depends_on: [normalize_numbers]
  - idx: 5
    name: compute_mean
    type: python
    module: orchestrator.scriptlets.python.steps.compute_mean
    function: ComputeMean
    depends_on: [normalize_numbers]
  - idx: 6
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    depends_on: [normalize_numbers]
  - idx: 7
    name: compute_std
    type: python
    module: orchestrator.scriptlets.python.steps.compute_std
    function: ComputeStd
    depends_on: [normalize_numbers]
  - idx: 8
    name: aggregate_metrics
    type: python
    module: orchestrator.scriptlets.python.steps.aggregate_metrics
    function: AggregateMetrics
    depends_on: [compute_mean, compute_median, compute_std]
  - idx: 9
    name: prime_factor_analysis
    type: python
    module: orchestrator.scriptlets.python.steps.prime_factor_analysis
    function: PrimeFactorAnalysis
    depends_on: [load_numbers]
  - idx: 10
    name: resource_audit
    type: python
    module: orchestrator.scriptlets.python.steps.resource_audit
    function: ResourceAudit
    depends_on: [aggregate_metrics, prime_factor_analysis]
  - idx: 11
    name: generate_report
    type: python
    module: orchestrator.scriptlets.python.steps.generate_report
    function: GenerateReport
    depends_on: [compute_metrics_v2, aggregate_metrics, resource_audit]
  - idx: 12
    name: cleanup_intermediate
    type: python
    module: orchestrator.scriptlets.python.steps.cleanup_intermediate
    function: CleanupIntermediate
    depends_on: [generate_report]
EOF

echo "[TESTS] Creating lightweight test stubs..."
cat > tests/test_ctx_foundation.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_foundation():
    ctx = run_recipe_v2("orchestrator/recipes/01_basic_number.yaml", set(), set(), None)
    assert ctx.get("numbers.raw_v1")
EOF

cat > tests/test_shell_integration.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_shell_integration():
    ctx = run_recipe_v2("orchestrator/recipes/02_basic_mixed.yaml", set(), set(), None)
    assert ctx.get("numbers.raw_v1")
EOF

cat > tests/test_validation.py <<'EOF'
import pytest
from orchestrator.runner_v2 import run_recipe_v2
def test_validation_fail():
    with pytest.raises(RuntimeError):
        run_recipe_v2("orchestrator/recipes/03_validation_errors.yaml", set(), set(), None)
EOF

cat > tests/test_artifacts_metrics.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_artifacts_pipeline():
    ctx = run_recipe_v2("orchestrator/recipes/04_artifacts_pipeline.yaml", set(), set(), None)
    assert ctx.get("numbers.normalized_v1")
    assert ctx.get("numbers.stats_v1")
EOF

cat > tests/test_parallel.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_parallel_metrics():
    ctx = run_recipe_v2("orchestrator/recipes/06_parallel_fan_in.yaml", set(), set(), None)
    assert ctx.get("metrics.aggregate_v1")
EOF

cat > tests/test_cpu_process.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_cpu_process():
    ctx = run_recipe_v2("orchestrator/recipes/07_cpu_process.yaml", set(), set(), None)
    assert ctx.get("numbers.prime_factors_v1")
EOF

cat > tests/test_versioning_wrapper.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_versioning_v2():
    ctx = run_recipe_v2("orchestrator/recipes/08_versioning_wrapper.yaml", set(), set(), None)
    assert ctx.get("metrics.summary_v1")
    assert ctx.get("metrics.summary_v2")
EOF

cat > tests/test_dashboard_stub.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_dashboard_pipeline_stub():
    ctx = run_recipe_v2("orchestrator/recipes/09_dashboard.yaml", set(), set(), None)
    assert ctx.get("metrics.summary_v2")
EOF

cat > tests/test_reporting_stub.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_reporting():
    ctx = run_recipe_v2("orchestrator/recipes/10_reporting.yaml", set(), set(), None)
    assert ctx.get("report.path_v1")
EOF

cat > tests/test_resilience_controls.py <<'EOF'
import pytest
from orchestrator.runner_v2 import run_recipe_v2
def test_resilience_failure_injected():
    with pytest.raises(RuntimeError):
        run_recipe_v2("orchestrator/recipes/11_resilience_controls.yaml", set(), set(), None)
EOF

cat > tests/test_full_pipeline.py <<'EOF'
from orchestrator.runner_v2 import run_recipe_v2
def test_full_pipeline():
    ctx = run_recipe_v2("orchestrator/recipes/12_advanced_full.yaml", set(), set(), None)
    assert ctx.get("report.path_v1")
    assert "resource.audit_v1" in ctx.to_dict()
EOF

echo "[DONE] Curriculum scaffolding complete."
echo "Try: python orchestrator/runner_v2.py --recipe orchestrator/recipes/05_metrics_versioned.yaml"