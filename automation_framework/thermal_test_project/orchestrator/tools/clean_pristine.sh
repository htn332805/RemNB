#!/usr/bin/env bash
set -euo pipefail

# Resolve base as repo root if script is run from anywhere
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "[CLEAN] Starting clean slate reset at: $BASE"

# 1) Data folder cleanup: remove generated artifacts, keep optional fixtures folder
DATA_DIR="$BASE/orchestrator/Data"
if [[ -d "$DATA_DIR" ]]; then
  echo "[CLEAN] Purging Data/ generated files"
  find "$DATA_DIR" -maxdepth 1 -type f \
    ! -name ".gitkeep" \
    ! -name "chart_spec.json" \
    ! -name "meta.example.json" \
    -print -delete || true

  # Optionally preserve a dedicated fixtures subfolder
  if [[ -d "$DATA_DIR/fixtures" ]]; then
    echo "[CLEAN] Preserving Data/fixtures/"
  fi

  # Remove common generated outputs if nested
  rm -f "$DATA_DIR/metrics.json" \
        "$DATA_DIR/ai_summary.md" \
        "$DATA_DIR/output.xlsx" \
        "$DATA_DIR/report.docx" \
        "$DATA_DIR/tests.db" \
        "$DATA_DIR/normalized.csv" \
        "$DATA_DIR/sanitized.csv" \
        "$DATA_DIR/clean.csv" \
        "$DATA_DIR/raw_input.csv" 2>/dev/null || true
fi

# 2) Logs cleanup
LOGS_DIR="$BASE/orchestrator/Logs"
if [[ -d "$LOGS_DIR" ]]; then
  echo "[CLEAN] Purging Logs/"
  find "$LOGS_DIR" -type f -print -delete || true
fi

# 3) Assets cleanup: remove generated graph
ASSETS_DIR="$BASE/orchestrator/Assets"
if [[ -d "$ASSETS_DIR" ]]; then
  echo "[CLEAN] Removing generated digraph image"
  rm -f "$ASSETS_DIR/graph.png" "$ASSETS_DIR/graph" "$ASSETS_DIR/graph.gv" 2>/dev/null || true
fi

# 4) Runtime control/state
echo "[CLEAN] Removing runtime control/state files"
rm -f "$BASE/orchestrator/store_on_ram.txt" 2>/dev/null || true
rm -f "$BASE/orchestrator/state.pkl" 2>/dev/null || true

# 5) Python cache and temp files repository-wide
echo "[CLEAN] Removing Python caches and temp files"
find "$BASE" -type d -name "__pycache__" -prune -exec rm -rf {} + || true
find "$BASE" -type f \( -name "*.pyc" -o -name "*.pyo" -o -name "*.tmp" -o -name "*.bak" -o -name "*.log" \) -print -delete || true

# 6) Optional: recreate empty placeholders
touch "$LOGS_DIR/orchestrator.log"
touch "$ASSETS_DIR/.gitkeep"
touch "$DATA_DIR/.gitkeep"

echo "[CLEAN] Done. Repository is reset to pre-run state."
