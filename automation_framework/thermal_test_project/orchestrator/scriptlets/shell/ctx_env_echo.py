#!/usr/bin/env bash
# ctx_env_echo.sh
# Prints the value of a ctx key passed as an environment variable (JSON-safe).
# Purpose: Example shell scriptlet that prints JSON from env (for debugging ctx-to-shell integration).
# Usage: ctx_env_echo.sh
echo "CTX ENV VARS:"
env | grep '^CTX_' | while read -r line; do
  echo "$line"
done