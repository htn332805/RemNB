#!/usr/bin/env bash
set -euo pipefail

# Rebuild / bootstrap automation orchestrator baseline (idempotent).
# Derived from documentation in:
#  - README.md
#  - Curriculum_userguide.md
#  - Example_Recipes_Userguide.md
#
# Usage:
#   ./rebase_workspace.sh                # Create missing files only
#   ./rebase_workspace.sh --force        # Overwrite managed files
#   ./rebase_workspace.sh --dry-run
#   ./rebase_workspace.sh --with-extra   # Include extended metric scriptlets / recipes
#
# Managed file marker: "### AUTOGEN(rebase_workspace.sh)"
#
# This script purposefully keeps implementations lean; enrich logic incrementally.

FORCE=0
DRY_RUN=0
WITH_EXTRA=0
QUIET=0

log() { [[ "$QUIET" -eq 1 ]] || echo "[rebase] $*"; }
warn() { [[ "$QUIET" -eq 1 ]] || echo "[rebase][warn] $*" >&2; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    --force) FORCE=1 ;;
    --dry-run) DRY_RUN=1 ;;
    --with-extra) WITH_EXTRA=1 ;;
    --quiet) QUIET=1 ;;
    -h|--help)
      cat <<EOF
Rebuild baseline workspace structure.

Flags:
  --force        Overwrite managed files
  --dry-run      Preview actions
  --with-extra   Include extended curriculum metric steps & recipes
  --quiet        Minimal output
EOF
      exit 0
      ;;
    *) warn "Unknown arg: $1"; exit 1 ;;
  esac
  shift
done

ROOT="$(pwd)"
MANAGED_TAG="### AUTOGEN(rebase_workspace.sh)"

ensure_dir() {
  local d="$1"
  if [[ ! -d "$d" ]]; then
    if [[ $DRY_RUN -eq 1 ]]; then log "MKDIR  $d"; else mkdir -p "$d"; log "created dir $d"; fi
  fi
}

write_file() {
  local path="$1"
  local content="$2"
  if [[ -f "$path" ]]; then
    if ! grep -q "$MANAGED_TAG" "$path"; then
      log "skip (unmanaged exists) $path"
      return
    fi
    if [[ $FORCE -ne 1 ]]; then
      log "keep (use --force to overwrite) $path"
      return
    fi
  fi
  if [[ $DRY_RUN -eq 1 ]]; then
    log "WRITE  $path"
  else
    printf "%s" "$content" > "$path"
    log "wrote $path"
  fi
}

append_if_absent() {
  local path="$1"
  local block="$2"
  if [[ ! -f "$path" ]]; then
    write_file "$path" "$block"
    return
  fi
  if grep -q "$MANAGED_TAG" "$path"; then
    if [[ $FORCE -eq 1 ]]; then
      write_file "$path" "$block"
    else
      log "managed file already present (no overwrite) $path"
    fi
    return
  fi
  if grep -q "BASELINE-RECON-SECTION" "$path"; then
    log "baseline marker already present in $path"
  else
    if [[ $DRY_RUN -eq 1 ]]; then
      log "APPEND baseline section to $path"
    else
      {
        echo ""
        echo "<!-- $MANAGED_TAG baseline injection -->"
        echo "$block"
      } >> "$path"
      log "appended baseline section to $path"
    fi
  fi
}

###############################################################################
# Directory scaffold
###############################################################################
DIRS=(
  orchestrator
  orchestrator/scriptlets
  orchestrator/scriptlets/python
  orchestrator/scriptlets/python/core
  orchestrator/scriptlets/python/steps
  orchestrator/scriptlets/python/apps
  orchestrator/scriptlets/shell
  orchestrator/recipes
  orchestrator/templates
  orchestrator/Data
  tests
  .github
  .github/instructions
  .github/prompts
)

for d in "${DIRS[@]}"; do ensure_dir "$d"; done

###############################################################################
# Core Python modules
###############################################################################
context_py=''"$MANAGED_TAG"'
"""
Context store (minimal) – version-safe key/value JSON-friendly.
"""
from __future__ import annotations
import copy, threading, json
class Context:
    def __init__(self):
        self._lock = threading.RLock()
        self._data = {}
        self._history = []
    def set(self, key, value, who="unknown"):
        if not isinstance(key, str):
            raise TypeError("key must be str")
        if key.count("_v") == 0:
            # Non-fatal warning pattern (could elevate later)
            pass
        with self._lock:
            self._data[key] = value
            self._history.append({"key": key, "who": who})
    def get(self, key, default=None):
        with self._lock:
            return self._data.get(key, default)
    def to_dict(self):
        with self._lock:
            return copy.deepcopy(self._data)
    def history(self):
        with self._lock:
            return list(self._history)
    def __repr__(self):
        return f"Context(keys={len(self._data)})"
if __name__ == "__main__":
    c=Context(); c.set("demo.key_v1", {"ok":True}, who="selftest"); print(json.dumps(c.to_dict()))
'

base_py=''"$MANAGED_TAG"'
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
'

logging_util_py=''"$MANAGED_TAG"'
import logging, sys
def get_logger(name: str):
    logger = logging.getLogger(name)
    if not logger.handlers:
        h = logging.StreamHandler(sys.stderr)
        fmt = logging.Formatter("[%(levelname)s] %(name)s %(message)s")
        h.setFormatter(fmt)
        logger.addHandler(h)
        logger.setLevel(logging.INFO)
    return logger
'

resource_py=''"$MANAGED_TAG"'
import time, functools, sys, json
def track_resources(fn):
    def wrapper(*a, **kw):
        start = time.time()
        try:
            return fn(*a, **kw)
        finally:
            dur = time.time() - start
            print(f"[resource] duration_sec={dur:.4f}", file=sys.stderr)
    return functools.wraps(fn)(wrapper)
'

runner_py=''"$MANAGED_TAG"'
"""
Sequential runner (baseline).
"""
import json, importlib, sys, pathlib, argparse
from orchestrator.context import Context
def load_recipe(path):
    import yaml
    return yaml.safe_load(open(path))
def _invoke_python_step(ctx, step):
    mod = importlib.import_module(step["module"])
    cls = getattr(mod, step["function"])
    inst = cls()
    params = step.get("args", {})
    return inst.run(ctx, params)
def run_recipe(recipe_path):
    rp = pathlib.Path(recipe_path)
    if not rp.is_file():
        raise FileNotFoundError(recipe_path)
    recipe = load_recipe(str(rp))
    ctx = Context()
    for step in recipe.get("steps", []):
        if step["type"] != "python":
            raise NotImplementedError("Only python steps supported in baseline")
        code = _invoke_python_step(ctx, step)
        if code != 0:
            raise RuntimeError(f"Step failed: {step['name']}")
    return ctx
if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--recipe", required=True)
    a = ap.parse_args()
    ctx = run_recipe(a.recipe)
    print(json.dumps({"status":"ok","ctx_keys":list(ctx.to_dict().keys())}))
'

runner_v2_py=''"$MANAGED_TAG"'
"""
Extended runner (placeholder) – dependency / parallel hooks could be layered here.
"""
from .runner import run_recipe  # Re-export for now
'

compute_numbers_py=''"$MANAGED_TAG"'
"""
ComputeNumbers – reference numeric scriptlet
Outputs: numbers.stats_v1
"""
import json, sys, pathlib, statistics
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class ComputeNumbers(BaseScriptlet):
    def validate(self, ctx, params):
        if "src" not in params: raise ValueError("src required")
        if not pathlib.Path(params["src"]).is_file(): raise ValueError("file not found")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            nums = [float(l.strip()) for l in open(params["src"]) if l.strip() and not l.startswith("#")]
            if not nums:
                raise ValueError("no numeric data")
            stats = {
                "count": len(nums),
                "sum": sum(nums),
                "mean": sum(nums)/len(nums),
                "min": min(nums),
                "max": max(nums),
            }
            ctx.set("numbers.stats_v1", stats, who="compute_numbers")
            print(json.dumps({"status":"ok","outputs":["numbers.stats_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_numbers"}))
            return 1
if __name__ == "__main__":
    import argparse
    ap=argparse.ArgumentParser(); ap.add_argument("--params", required=True)
    p=json.loads(ap.parse_args().params)
    sys.exit(ComputeNumbers().run(Context(), p))
'

start_context_server_v2_py=''"$MANAGED_TAG"'
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
'

demo_update_state_py=''"$MANAGED_TAG"'
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
'

sanitize_shell=''"$MANAGED_TAG"'
#!/usr/bin/env bash
set -euo pipefail
IN="${1:-orchestrator/Data/numbers.csv}"
OUT="orchestrator/Data/numbers_sanitized.csv"
grep -v "^[[:space:]]*$" "$IN" | grep -v "^[[:space:]]*#" > "$OUT"
echo "{\"status\":\"ok\",\"outputs\":[\"$OUT\"],\"sanitized_path\":\"$OUT\"}"
'

###############################################################################
# Extended (optional) scriptlets
###############################################################################
compute_mean_py=''"$MANAGED_TAG"'
import json, sys, statistics, pathlib
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class ComputeMean(BaseScriptlet):
    def validate(self, ctx, params):
        if "stats_key" not in params: raise ValueError("stats_key required")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            stats = ctx.get(params["stats_key"])
            if not stats: raise ValueError("missing stats")
            ctx.set("metrics.mean_v1", {"mean": stats["mean"]}, who="compute_mean")
            print(json.dumps({"status":"ok","outputs":["metrics.mean_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_mean"})); return 1
'

compute_median_py=''"$MANAGED_TAG"'
import json, sys, pathlib, statistics
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class ComputeMedian(BaseScriptlet):
    def validate(self, ctx, params):
        if "src" not in params: raise ValueError("src required")
        if not pathlib.Path(params["src"]).is_file(): raise ValueError("file not found")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            nums=[float(l.strip()) for l in open(params["src"]) if l.strip() and not l.startswith("#")]
            ctx.set("metrics.median_v1", {"median": statistics.median(nums)}, who="compute_median")
            print(json.dumps({"status":"ok","outputs":["metrics.median_v1"]})); return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_median"})); return 1
'

compute_std_py=''"$MANAGED_TAG"'
import json, sys, pathlib, statistics, math
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class ComputeStd(BaseScriptlet):
    def validate(self, ctx, params):
        if "src" not in params: raise ValueError("src required")
    def run(self, ctx, params):
        try:
            self.validate(ctx, params)
            nums=[float(l.strip()) for l in open(params["src"]) if l.strip() and not l.startswith("#")]
            m=sum(nums)/len(nums)
            var=sum((x-m)**2 for x in nums)/len(nums)
            ctx.set("metrics.std_v1", {"std": math.sqrt(var)}, who="compute_std")
            print(json.dumps({"status":"ok","outputs":["metrics.std_v1"]})); return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"compute_std"})); return 1
'

aggregate_metrics_py=''"$MANAGED_TAG"'
import json, sys
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
class AggregateMetrics(BaseScriptlet):
    def run(self, ctx, params):
        try:
            agg = {
                "mean": (ctx.get("metrics.mean_v1") or {}).get("mean"),
                "median": (ctx.get("metrics.median_v1") or {}).get("median"),
                "std": (ctx.get("metrics.std_v1") or {}).get("std")
            }
            ctx.set("metrics.aggregate_v1", agg, who="aggregate_metrics")
            print(json.dumps({"status":"ok","outputs":["metrics.aggregate_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1,"step":"aggregate_metrics"})); return 1
'

###############################################################################
# Templates & Data
###############################################################################
numbers_csv=''"$MANAGED_TAG"'
# demo numbers
1
2
3
4
5
'

report_template_md=''"$MANAGED_TAG"'
# Automated Metrics Report

Generated: {{ generated_ts }}

## Summary (v1)
Count: {{ metrics.summary_v1.count }}
Mean: {{ metrics.summary_v1.mean }}
Min: {{ metrics.summary_v1.min }}
Max: {{ metrics.summary_v1.max }}

{% if metrics.summary_v2 %}
## Extended (v2)
p50: {{ metrics.summary_v2.p50 }}
{% endif %}
'

###############################################################################
# Recipes
###############################################################################
recipe_example_numbers=''"$MANAGED_TAG"'
test_meta:
  test_id: "NUM-001"
  description: "Compute simple statistics"
steps:
  - idx: 1
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args:
      src: orchestrator/Data/numbers.csv
    success:
      ctx_has_keys:
        - numbers.stats_v1
'

recipe_example_ctx=''"$MANAGED_TAG"'
test_meta:
  test_id: "CTX-001"
  description: "Context server stub + operations"
steps:
  - idx: 1
    name: start_context_server
    type: python
    module: orchestrator.scriptlets.python.steps.start_context_server_v2
    function: StartContextServerV2
  - idx: 2
    name: demo_update_state
    type: python
    module: orchestrator.scriptlets.python.steps.demo_update_state
    function: DemoUpdateState
    args:
      operations:
        - ["set","demo.user","{\\"name\\":\\"Alice\\",\\"age\\":30}"]
        - ["append","demo.nums","100"]
        - ["append","demo.nums","200"]
        - ["get","demo.user"]
        - ["get","demo.nums"]
    depends_on: [start_context_server]
    success:
      ctx_has_keys:
        - demo_update_state.summary_v1
'

recipe_parallel=''"$MANAGED_TAG"'
test_meta:
  test_id: "PAR-001"
  description: "Parallel fan-in style (conceptual – sequential in baseline runner)"
steps:
  - idx: 1
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 2
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 3
    name: compute_std
    type: python
    module: orchestrator.scriptlets.python.steps.compute_std
    function: ComputeStd
    args: { src: orchestrator/Data/numbers.csv }
  - idx: 4
    name: compute_mean
    type: python
    module: orchestrator.scriptlets.python.steps.compute_mean
    function: ComputeMean
    args: { stats_key: numbers.stats_v1 }
  - idx: 5
    name: aggregate_metrics
    type: python
    module: orchestrator.scriptlets.python.steps.aggregate_metrics
    function: AggregateMetrics
    success:
      ctx_has_keys:
        - metrics.aggregate_v1
'

###############################################################################
# Tests
###############################################################################
test_example_numbers=''"$MANAGED_TAG"'
import pytest
from orchestrator.runner import run_recipe
def test_example_numbers_recipe():
    ctx = run_recipe("orchestrator/recipes/example_numbers.yaml")
    stats = ctx.get("numbers.stats_v1")
    assert stats and "count" in stats
'

test_parallel=''"$MANAGED_TAG"'
from orchestrator.runner import run_recipe
def test_parallel_aggregate():
    ctx = run_recipe("orchestrator/recipes/parallel_fan_in.yaml")
    assert ctx.get("metrics.aggregate_v1")
'

###############################################################################
# Write core files
###############################################################################
write_file orchestrator/context.py "$context_py"
write_file orchestrator/scriptlets/python/core/base.py "$base_py"
write_file orchestrator/scriptlets/python/core/logging_util.py "$logging_util_py"
write_file orchestrator/scriptlets/python/core/resource.py "$resource_py"
write_file orchestrator/runner.py "$runner_py"
write_file orchestrator/runner_v2.py "$runner_v2_py"

write_file orchestrator/scriptlets/python/steps/compute_numbers.py "$compute_numbers_py"
write_file orchestrator/scriptlets/python/steps/start_context_server_v2.py "$start_context_server_v2_py"
write_file orchestrator/scriptlets/python/steps/demo_update_state.py "$demo_update_state_py"

write_file orchestrator/scriptlets/shell/sanitize_csv.sh "$sanitize_shell"
if [[ $DRY_RUN -ne 1 ]]; then chmod +x orchestrator/scriptlets/shell/sanitize_csv.sh; fi

write_file orchestrator/Data/numbers.csv "$numbers_csv"
write_file orchestrator/templates/report_template.md "$report_template_md"

write_file orchestrator/recipes/example_numbers.yaml "$recipe_example_numbers"
write_file orchestrator/recipes/example_ctx.yaml "$recipe_example_ctx"

if [[ $WITH_EXTRA -eq 1 ]]; then
  write_file orchestrator/scriptlets/python/steps/compute_mean.py "$compute_mean_py"
  write_file orchestrator/scriptlets/python/steps/compute_median.py "$compute_median_py"
  write_file orchestrator/scriptlets/python/steps/compute_std.py "$compute_std_py"
  write_file orchestrator/scriptlets/python/steps/aggregate_metrics.py "$aggregate_metrics_py"
  write_file orchestrator/recipes/parallel_fan_in.yaml "$recipe_parallel"
  write_file tests/test_parallel_fan_in.py "$test_parallel"
fi

write_file tests/test_example_numbers.py "$test_example_numbers"

###############################################################################
# Append baseline markers to top-level docs (non-destructive)
###############################################################################
BASELINE_APPEND="
<!-- BASELINE-RECON-SECTION -->
Rebase script executed: $(date -u +"%Y-%m-%dT%H:%M:%SZ")
Managed tag marker: $MANAGED_TAG
Use './rebase_workspace.sh --force' to refresh generated scaffolding.
"

append_if_absent README.md "$BASELINE_APPEND"
append_if_absent Curriculum_userguide.md "$BASELINE_APPEND"
append_if_absent Example_Recipes_Userguide.md "$BASELINE_APPEND"

###############################################################################
# Manifest
###############################################################################
if [[ $DRY_RUN -eq 1 ]]; then
  log "(dry-run) complete – no files written."
  exit 0
fi

log "MANIFEST (generated keys & paths):"
echo "  - orchestrator/context.py"
echo "  - orchestrator/runner.py"
echo "  - orchestrator/scriptlets/python/steps/compute_numbers.py"
echo "  - orchestrator/recipes/example_numbers.yaml"
[[ $WITH_EXTRA -eq 1 ]] && echo "  - extended metrics (mean/median/std/aggregate) + parallel_fan_in recipe"
log