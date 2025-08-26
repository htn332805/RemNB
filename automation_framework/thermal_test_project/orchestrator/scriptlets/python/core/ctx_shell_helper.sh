#!/usr/bin/env bash
# ctx_shell_helper.sh
# Manipulate ctx data passed via environment variables.
# Shell scriptlet for ctx manipulation (pretty-print, filter, etc.).
# Pretty-print all CTX_ env vars as JSON
echo "{"
env | grep '^CTX_' | while IFS='=' read -r key val; do
  printf '  "%s": %s,\n' "${key#CTX_}" "$val"
done
echo "}"