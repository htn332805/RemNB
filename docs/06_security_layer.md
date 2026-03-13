# AIOS — Security Layer (Phase 0)

**Generated:** 2026-03-12

---

## 1. Overview

**Module:** `agent/security/`  
**Implemented in:** Phase 0 (foundational safety layer)

The security layer provides **defense-in-depth** for all command and file operations. It is composed of eight independent modules, collectively referred to as SAFE-01 through SAFE-08. Every module is imported by executors directly — the security layer does **not** import from any other application module.

---

## 2. Security Architecture Summary

```
User Request
     │
     ▼
[SAFE-08] Output Sanitizer   ← regex injection scan on raw command string
     │ BLOCKED → reject
     ▼ SAFE
[SAFE-01] Command Validator  ← allowlist/blocklist YAML + injection check
     │ BLOCK → reject + audit log
     │ CONFIRM_REQUIRED → stop (non-interactive context)
     ▼ ALLOW
[SAFE-03] Dry-Run Gate       ← if dry_run=True, return without executing
     ▼ not dry-run
[Execute subprocess]
     │
     ├─ [SAFE-06] Timeout Guard     ← asyncio.wait_for / proc.communicate(timeout)
     ├─ [SAFE-07] Resource Monitor  ← psutil background thread (CPU/mem/files)
     ▼
[SAFE-02] Scope Enforcer     ← path boundary validation for file ops
[SAFE-04] Confirmation Gate  ← interactive y/n prompt (CLI mode only)
[SAFE-05] Audit Logger       ← append-only JSONL — every event recorded
```

---

## 3. Output Sanitizer (`agent/security/output_sanitizer.py`)

**SAFE-08 — Injection Pattern Scanner**

The first gate. Scans the raw command string for dangerous patterns using compiled regex.

### Dangerous Pattern Library

| Pattern | Threat |
|---|---|
| `` `...` `` | Backtick command substitution |
| `$(...)` | Shell `$()` command substitution |
| `; rm/dd/mkfs/shutdown/reboot/halt` | Semicolon-chained destructive commands |
| `&& rm/dd/...` | AND-chained destructive commands |
| `\|\| rm/dd/...` | OR-chained destructive commands |
| `> /dev/` | Write redirect to device files |
| `eval ` | Eval injection |
| `exec ` | Exec injection |
| `base64 --decode` / `base64 -d` | Base64-encoded payload execution |
| `curl ... \| bash` | Remote code execution via curl |
| `wget ... \| bash` | Remote code execution via wget |

**Return values:**
- `("SAFE", None)` — no match
- `("BLOCKED", pattern)` — writes audit event `injection_attempt` + returns blocking pattern

---

## 4. Command Validator (`agent/security/command_validator.py`)

**SAFE-01 — Command Allow/Block Enforcement**

Reads `configs/security/allowlist.yml` (lazy-loaded, cached after first load).

### Decision Logic

```
1. is_blocked(cmd)?     → BLOCK + audit log
2. has_injection(cmd)?  → BLOCK + audit log
3. is_confirm_required(cmd)? → CONFIRM_REQUIRED
4. is_allowed(cmd)?     → ALLOW
5. default              → BLOCK + audit log
```

### Allowlist Categories

**`allowed_local`** (prefix match on first token):
```
df, du, free, top, ps, cat, grep, awk, sed, ls, find,
python3, bash, echo, date, uptime, hostname, uname,
pwd, which, env, ping
```

**`blocked`** (substring match anywhere in command):
```
rm -rf, rm -r /, dd, mkfs, fdisk, format,
shutdown, reboot, halt, poweroff, kill -9 1,
chmod 777 /, /dev/sda, DROP TABLE, DROP DATABASE,
TRUNCATE, /dev/null 2>&1 && rm
```

**`confirm_required`** (prefix match):
```
rm, mv, chmod, chown, chgrp, systemctl, service,
crontab, ssh-keygen, passwd, useradd, userdel,
usermod, groupadd, iptables, ufw, apt, apt-get,
pip install, pip uninstall
```

### Injection Detection (Secondary)

`has_injection(cmd)` performs additional injection detection via regex:
- `;` followed by non-whitespace
- `&&` followed by non-whitespace
- `||` followed by non-whitespace
- `$()` substitution
- Backtick substitution

> **Note:** Both the output sanitizer and command validator check for injections, providing redundant coverage with different pattern sets.

---

## 5. Scope Enforcer (`agent/security/scope_enforcer.py`)

**SAFE-04 — Path Boundary Validation**

The single entry point `validate_path(raw_path, operation, username)` returns `"ALLOW"` or `"BLOCK"`.

### Path Normalization

All paths go through:
```python
path = Path(raw_path).expanduser().resolve()
```
This eliminates `../` traversals and symlink-based escapes before any comparison.

### Decision Matrix

| Operation | Condition | Decision |
|---|---|---|
| Any | Path starts with any `ABSOLUTE_WRITE_BLOCK` prefix | `BLOCK` |
| `write` | Path within `workspace/{username}/` or `obsidian/vault/` | `ALLOW` |
| `write` | Path outside above | `BLOCK` |
| `read` | Path within `APPROVED_READ_PATHS` | `ALLOW` |
| `read` | Path outside approved read paths | `BLOCK` |
| unknown op | Any | `BLOCK` |

**`ABSOLUTE_WRITE_BLOCK` (unconditional blocks):**
```
/etc/, /boot/, /sys/, /proc/, /root/,
/bin/, /sbin/, /usr/, /lib/
```

**`APPROVED_READ_PATHS`:**
```
~/aios/, /var/log/, /proc/, /sys/, /etc/hostname
```

All blocks are audit-logged with the specific reason.

---

## 6. Audit Logger (`agent/security/audit_logger.py`)

**SAFE-05 — Immutable Audit Trail**

All security-relevant events are appended to daily JSONL files:
```
audit/audit_{YYYYMMDD}.jsonl
```

### Audit Entry Schema

```json
{
  "event_id":   "evt_abc123def456",
  "timestamp":  "2026-03-12T14:23:01.123456+00:00",
  "username":   "tester",
  "session_id": null,
  "task_id":    "shell_a1b2c3d4",
  "event_type": "command_executed",
  "tool":       "shell_executor",
  "action":     "df -h",
  "target":     null,
  "result":     "success",
  "reason":     null,
  "query_hash": null,
  "diff_ref":   null
}
```

### Event Types

| Event Type | Trigger |
|---|---|
| `command_executed` | After any shell/SSH/Telnet execution |
| `command_blocked` | Command validator BLOCK decision |
| `injection_attempt` | Output sanitizer BLOCKED |
| `path_blocked` | Scope enforcer BLOCK |
| `file_read` | Filesystem MCP read (success or blocked) |
| `file_written` | Filesystem MCP write (success, blocked, or dry_run) |
| `resource_limit_exceeded` | Resource monitor breach |
| `task_timeout` | Timeout guard exceeded |

The audit log directory is created automatically if absent (`mkdir -p`). Entries are **never deleted or modified**.

---

## 7. Resource Monitor (`agent/security/resource_monitor.py`)

**SAFE-07 — Runtime Resource Enforcement**

Runs as a background daemon thread alongside every subprocess execution.

### Monitored Metrics

| Metric | Env Variable | Default |
|---|---|---|
| CPU usage | `RESOURCE_CPU_MAX` | 85% |
| RSS memory | `RESOURCE_MEM_MB_MAX` | 512 MB |
| Open file descriptors | `RESOURCE_OPEN_FILES_MAX` | 100 |

### Behavior on Breach

1. Logs `resource_limit_exceeded` audit event
2. Sends desktop notification (urgency: critical)
3. Calls `abort_callback(limit_name, value)` → caller kills the process
4. Thread exits

`stop_monitoring(stop_event)` is called in `finally` to cleanly stop the thread.

---

## 8. Timeout Guard (`agent/security/timeout_guard.py`)

**SAFE-06 — Execution Time Ceiling**

Two mechanisms:

### Async Timeout (`wrap_with_timeout`)
```python
result = await wrap_with_timeout(coro, timeout_seconds)
```
Wraps any async coroutine. On `asyncio.TimeoutError`:
1. Logs `task_timeout` audit event
2. Sends desktop notification
3. Re-raises `TimeoutError`

### Subprocess Timeout
In `execute_local`, `proc.communicate(timeout=30)` enforces a hard 30-second subprocess timeout directly.

### Process Tree Killer (`kill_process_tree`)
Uses psutil to enumerate and kill the entire child process tree (not just the direct child), preventing orphaned grandchild processes.

---

## 9. Confirmation Gate (`agent/security/confirmation_gate.py`)

**SAFE-04 — Interactive Confirmation**

Handles `CONFIRM_REQUIRED` decisions in CLI/interactive contexts:
- Presents a `y/n` prompt to the user
- Returns `True` (approved) or `False` (rejected)
- In non-interactive mode (scheduler, agent API), always returns `False` → operation blocked

---

## 10. Dry-Run Gate (`agent/security/dry_run_gate.py`)

**SAFE-03 — Global Dry-Run Mode Toggle**

- Reads `DRY_RUN_DEFAULT` from settings
- All executors check `dry_run=True` before opening any subprocess or connection
- Dry-run executions return `MCPResult(status="dry_run", stdout="[DRY-RUN] Would execute: <cmd>")` and are audit-logged

---

## 11. Security Configuration File (`configs/security/allowlist.yml`)

The only external security configuration file. Contains three YAML lists:
- `allowed_local` — safe read-only/informational commands
- `blocked` — unconditionally dangerous patterns
- `confirm_required` — privileged-but-legitimate operations needing confirmation

**File location hardcoded** in `command_validator.py` relative to package root, making it consistently located regardless of CWD.

---

## 12. Security Event Audit File Sample

Real audit entry from `audit/audit_20260312.jsonl`:
```json
{
  "event_id": "evt_a7f3b2c1d4e5",
  "timestamp": "2026-03-12T13:59:47.123456+00:00",
  "username": "tester",
  "session_id": null,
  "task_id": "shell_9a1b2c3d",
  "event_type": "command_executed",
  "tool": "shell_executor",
  "action": "df -h",
  "target": null,
  "result": "success",
  "reason": null,
  "query_hash": null,
  "diff_ref": null
}
```

---

## 13. Security Observations & Recommendations

| Finding | Severity | Recommendation |
|---|---|---|
| SSH `AutoAddPolicy` accepts all host keys | Medium | Use `RejectPolicy` + known_hosts file for production |
| Telnet transmits plaintext credentials | High | Restrict to isolated network segments; prefer SSH |
| Scheduler in-memory only | Low | Add disk persistence to prevent task loss on restart |
| `allow_agent=False` in SSH | Good | Prevents SSH agent hijacking |
| Injection detection is layered (sanitizer + validator) | Good | Redundant coverage reduces bypass risk |
| Audit log directory created automatically | Note | Ensure audit log path is on tamper-resistant storage in production |
| `CONFIRM_REQUIRED` always blocks in API/scheduler context | Good | Prevents privileged ops without explicit human approval |
| Path traversal via `resolve()` before comparison | Good | Properly neutralizes `../` attacks |

---

## 14. Notification System (`notification/`)

Closely coupled to the security layer for user alerting.

### `notification/notifier.py`
- Sends desktop notifications via `notify2` (libnotify)
- Controlled by `AIOS_DESKTOP_NOTIFICATIONS` env var
- Silently fails on headless/no-DISPLAY environments
- Urgency levels: `low`, `normal`, `critical`

### `notification/markers.py`
- Writes task completion marker files under `workspace/{username}/notifications/`
- Used by external monitoring tools to detect task completion

### `notification/failure_logger.py`
- Appends failure records to `workspace/{username}/failure_logs.jsonl`
- Records: server, command, stderr output, retry count
