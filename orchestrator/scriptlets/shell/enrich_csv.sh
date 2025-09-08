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
