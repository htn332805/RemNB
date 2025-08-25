#!/usr/bin/env bash
# Enable strict mode: exit on error (-e), undefined variables (-u), and fail pipelines (-o pipefail)
set -euo pipefail

# Check argument count: expect 2 arguments (input and output paths)
if [[ $# -ne 2 ]]; then
  echo "Usage: $0 <input_csv> <output_csv>" >&2
  exit 64
fi

# Assign positional arguments to named variables for clarity
in="$1"
out="$2"

# Create a temporary file for safe writing to avoid partial outputs
tmp="$(mktemp)"
# Ensure the temporary file is removed even if the script exits early
trap 'rm -f "$tmp"' EXIT

# Remove blank lines and comment lines, normalize CRLF to LF, write into temp file
grep -v '^[[:space:]]*$' "$in" | grep -v '^[[:space:]]*#' | sed 's/\r$//' > "$tmp"

# Basic header validation: first line must contain commas to look like CSV
if ! head -1 "$tmp" | grep -q ','; then
  echo "ERROR: header row missing or not CSV" >&2
  exit 65
fi

# Move the cleaned temp file to the final output path (atomic on same filesystem)
mv "$tmp" "$out"

# Informative message for logs
echo "Preprocess complete: $out"
