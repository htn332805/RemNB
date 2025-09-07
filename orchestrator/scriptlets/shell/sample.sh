#!/usr/bin/env bash
# NAME: sample.sh
# DESC: Echo a provided text as JSON output.
# USAGE: ECHO_TEXT=Hello orchestrator/scriptlets/shell/sample.sh
# RETURNS: {"status":"ok","echo":ECHO_TEXT}
# LIMITATIONS: demo only
set -euo pipefail
TEXT="${ECHO_TEXT:-missing}"
>&2 echo "[sample.sh] echoing $TEXT"
printf '{"status":"ok","echo":"%s"}' "$TEXT"
