#!/usr/bin/env bash
# Remove blank/comment (#) lines from CSV.
set -euo pipefail
INPUT="${1:-orchestrator/Data/numbers.csv}"
OUT="orchestrator/Data/numbers_sanitized.csv"
grep -v '^[[:space:]]*$' "$INPUT" | grep -v '^[[:space:]]*#' > "$OUT"
echo "{\"status\":\"ok\",\"outputs\":[\"$OUT\"],\"sanitized_path\":\"$OUT\"}"
