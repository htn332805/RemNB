### AUTOGEN(rebase_workspace.sh)
#!/usr/bin/env bash
set -euo pipefail
IN="${1:-orchestrator/Data/numbers.csv}"
OUT="orchestrator/Data/numbers_sanitized.csv"
grep -v "^[[:space:]]*$" "$IN" | grep -v "^[[:space:]]*#" > "$OUT"
echo "{\"status\":\"ok\",\"outputs\":[\"$OUT\"],\"sanitized_path\":\"$OUT\"}"
