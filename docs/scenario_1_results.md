# Scenario 1 Results: Natural Language → CLI Command Execution

**Date:** 2026-03-12  
**Status:** Validated  

## Commands / Prompts Used

```
User: "Check disk space and show me a summary."
```

## Agent Flow Observed

1. Intent classified as `shell_cmd` via `/agent/intent/classify`
2. Command validated against allowlist: `df -h` — ALLOWED
3. Shell executed: `df -h`
4. Output returned as structured JSON to user
5. Audit log entry written to `audit/audit.jsonl`

## Sample Output

```
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda1        50G   18G   30G  38% /
tmpfs           3.9G     0  3.9G   0% /dev/shm
```

## Artifacts

- Audit log: `audit/audit.jsonl` (entries: intent_classified, shell_executed)
- Response body: `{ "output": "<df -h output>", "exit_code": 0 }`

## Pass/Fail Criteria

- [x] Disk space info returned without errors
- [x] No blocked or unsafe commands executed
- [x] Audit log has entries for intent → command → output
