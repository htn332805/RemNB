# AIOS — Pipeline, MCP Tools & Output Engine

**Generated:** 2026-03-12

---

## 1. Overview

This section covers three tightly related subsystems:

1. **Pipeline** (`pipeline/`) — orchestrates multi-server task execution flows
2. **MCP Tools** (`mcp/`) — modular executor adapters with safety gate integration
3. **Output Engine** (`output_engine/`) — converts structured data into documents

Together these form the **execution layer** of AIOS: pipelines coordinate what runs where, MCP tools handle the actual execution with safety enforcement, and the output engine materializes results into deliverable files.

---

## 2. Pipeline Module (`pipeline/`)

### 2.1 Core Abstractions (`pipeline/orchestrator.py`)

#### `PipelineStep`
An immutable named step:
```python
class PipelineStep:
    name: str
    func: Callable[[PipelineContext], PipelineContext]
```

#### `PipelineContext`
A `Dict[str, Any]` that is passed through and mutated by each step. Internal bookkeeping keys:
- `__steps__` — list of `{name, status}` records
- `__errors__` — list of `{step, error}` records

#### `run_pipeline(steps, ctx)`
Sequential executor:
1. Iterates steps, calls `step.func(ctx)` which returns the updated ctx
2. On exception: logs to `ctx["__errors__"]`, **stops** (fail-fast)
3. After all steps: writes pipeline journal to disk if `username` + `task_id` present

#### `write_pipeline_journal(username, task_id, ctx)`
Writes the final context as JSON to:
```
workspace/{username}/task_history/{task_id}.json
```

#### `load_user_servers(username)`
Reads `workspace/{username}/configs/servers.yml` and returns the `servers` list. Returns `[]` if file absent.

**Server entry schema (YAML):**
```yaml
servers:
  - name: "prod-01"
    host: "192.168.1.10"
    protocol: "ssh"       # or "telnet"
    username: "admin"
    password: "secret"
    port: 22
```

---

### 2.2 Multi-Server Runner (`pipeline/multi_server_runner.py`)

Implements a fanout pattern: execute one command on all user-defined servers.

**Context contract:**
```
Input keys:  ctx["username"], ctx["servers"], ctx["command"], ctx["dry_run"]
Output key:  ctx["results"] → List[per-server result dicts]
```

**Per-server result:**
```python
{
    "server_name": str,
    "host": str,
    "protocol": "ssh" | "telnet",
    "status": "success" | "error" | "dry_run",
    "stdout": str,
    "stderr": str,
    "exit_code": int
}
```

Protocol dispatch:
- `ssh` → `mcp.ssh_executor.execute_ssh_command(...)`
- `telnet` → `mcp.telnet_executor.execute_telnet_command(...)`
- Unknown → error result with "Unknown protocol" message

---

### 2.3 Log Harvester (`pipeline/log_harvester.py`)

SSH-connects to each server and retrieves log file contents. Used in test pipeline flows to collect diagnostic data before extraction.

---

### 2.4 Data Extractor (`pipeline/data_extractor.py`)

Parses raw log strings into structured records (dicts). Downstream consumers feed these into the output engine for report generation.

---

### 2.5 Test Suite Runner (`pipeline/test_suite_runner.py`)

Executes predefined test scripts across servers, collecting pass/fail results per test per server. Results feed TQS scoring in the learning loop.

---

### 2.6 Stress Orchestrator (`pipeline/stress_orchestrator.py`)

Coordinates load/stress test execution: sends high-volume commands to servers and collects latency and error rate statistics.

---

## 3. MCP Tools (`mcp/`)

MCP (Model Context Protocol) tools are modular execution adapters. **All tools share:**
- A common result type (`MCPResult`) from `mcp/common/types.py`
- Integration with the security layer (command validation + audit logging)
- Support for `dry_run` mode

### 3.1 Common Types (`mcp/common/types.py`)

```python
class MCPResult(TypedDict):
    status:    str          # "success" | "error" | "dry_run"
    stdout:    str
    stderr:    str
    exit_code: int
    metadata:  dict
```

---

### 3.2 Shell Executor (`mcp/shell_executor/executor.py`)

**Primary function:** `execute_local(cmd, dry_run=False, timeout=30)`

**Full execution pipeline:**

```
cmd
 │
 ├─ output_sanitizer.sanitize(cmd)        → BLOCKED? → MCPResult(error)
 │
 ├─ command_validator.validate_command(cmd)
 │     BLOCK           → MCPResult(error) + audit log
 │     CONFIRM_REQUIRED → MCPResult(error) + audit log  (non-interactive mode)
 │     ALLOW            → continue
 │
 ├─ dry_run=True → MCPResult("dry_run", stdout="[DRY-RUN] Would execute: cmd")
 │
 ├─ subprocess.Popen(shlex.split(cmd))
 ├─ resource_monitor.start_monitoring(pid)   ← background thread
 │
 ├─ proc.communicate(timeout=30)
 │     TimeoutExpired → proc.kill() → MCPResult(error, exit_code=124)
 │
 ├─ resource breach (aborted event) → proc.kill() → MCPResult(error)
 │
 ├─ audit_logger.log_event("command_executed", ...)
 └─ MCPResult(status="success"|"error", stdout, stderr, exit_code)
```

---

### 3.3 SSH Executor (`mcp/ssh_executor/executor.py`)

**Primary function:** `execute_ssh_command(host, username, password, cmd, dry_run, port=22)`

Features:
- Same sanitize → validate pipeline as shell executor
- **Retry loop** with configurable `SSH_MAX_RETRIES` (default 2) and `SSH_RETRY_DELAY` (default 2s)
- Uses `paramiko.SSHClient` with `AutoAddPolicy` for host key acceptance
- Returns `metadata={"attempts": N}` for observability
- Logs all connection attempts and outcomes

**`open_ssh_client(host, username, password, port, timeout=10)`**
- Sets `AutoAddPolicy` (accepts any host key — suitable for internal/trusted networks)
- `allow_agent=False`, `look_for_keys=False` — password-only auth

> **Security note:** `AutoAddPolicy` accepts all SSH host keys without verification, making it vulnerable to MITM attacks. Acceptable for trusted internal networks; should be hardened for internet-facing deployments.

---

### 3.4 Telnet Executor (`mcp/telnet_executor/executor.py`)

**Primary function:** `execute_telnet_command(host, port, username, password, cmd, dry_run=False)`

Uses `telnetlib3` for async Telnet communication. Same validation pipeline as SSH.

> **Security note:** Telnet transmits credentials and commands in plaintext. Should only be used on isolated networks or legacy devices without SSH support.

---

### 3.5 Filesystem Tool (`mcp/filesystem/fs.py`)

**Functions:**
- `read_file(path, username)` — validates `operation="read"` against approved read paths; raises `PermissionError` on BLOCK
- `write_file(path, content, username, dry_run=False)` — validates `operation="write"` against user workspace; creates parent dirs on ALLOW

All operations logged via `audit_logger.log_event`.

**`mcp/filesystem/executor.py`** — higher-level operations building on `fs.py` (move, copy, list, etc.)

---

### 3.6 Obsidian Sync Tool (`mcp/obsidian_sync/sync.py`)

Currently a **stub** implementation:
```python
def sync_to_vault(src_path: Path, vault_subpath: str) -> Path:
    return src_path   # Phase 2 stub — full implementation pending
```

---

### 3.7 Qdrant Tool (`mcp/qdrant_tool/client.py`)

Wraps `memory/store.py` and `memory/search.py` as MCP-compatible functions for use from OpenWebUI tool calls:
- `search_knowledge(query, collection, top_k)` → MCPResult-compatible
- `upsert_knowledge(content, collection, payload)` → MCPResult-compatible

---

### 3.8 OpenWebUI Tool (`mcp/openwebui_tools/aios_agent_tool.py`)

Provides an OpenWebUI-compatible function definition that forwards requests to the AIOS agent HTTP API. Enables OpenWebUI users to invoke AIOS capabilities from the chat interface.

---

## 4. Output Engine (`output_engine/`)

Converts structured `ReportSpec` data into files in six formats.

### 4.1 Data Models (`output_engine/models.py`)

```python
class TableSpec(TypedDict):
    name:    str            # Sheet/table label
    columns: List[str]      # Column header labels
    rows:    List[List[Any]]# Row data (aligned with columns)

class ChartSpec(TypedDict):
    title:      str
    table_name: str         # Must match a TableSpec name in same report
    x_column:   str
    y_column:   str
    chart_type: str         # "line" | "bar" | "scatter"

class ReportSpec(TypedDict):
    title:    str
    tables:   List[TableSpec]
    charts:   List[ChartSpec]
    metadata: Dict[str, Any]
```

---

### 4.2 Format Builders

| Builder | Module | Library | Output Format |
|---|---|---|---|
| CSV | `csv_builder.py` | stdlib `csv` | `.csv` per table |
| Excel | `xlsx_builder.py` | XlsxWriter 3.2.0 | `.xlsx` with sheets + embedded charts |
| Markdown | `md_builder.py` | stdlib | `.md` with GFM tables |
| HTML | `html_builder.py` | stdlib | `.html` with styled tables |
| PDF | `pdf_builder.py` | WeasyPrint 62.3 | `.pdf` via HTML→PDF |
| Word | `docx_builder.py` | python-docx 1.1.2 | `.docx` with tables |

Each builder accepts a `ReportSpec` and a target `Path` and returns the path of the generated file.

---

### 4.3 Workspace Wiring (`output_engine/workspace_wiring.py`)

Writes output files to the standardized per-user path:
```
workspace/{username}/outputs/{title}_{timestamp}.{ext}
```

Ensures the `outputs/` directory exists before writing.

---

### 4.4 Examples (`output_engine/examples.py`)

Provides sample `ReportSpec` dicts with mock data for testing all builders without a real pipeline run.

---

## 5. Pipeline ↔ Output Engine Integration

```
pipeline/test_suite_runner.py
    │ collects per-server test results
    ▼
pipeline/data_extractor.py
    │ parses into structured records
    ▼
output_engine/workspace_wiring.py
    │ calls xlsx_builder, pdf_builder, md_builder...
    ▼
workspace/{username}/outputs/
```

---

## 6. Observability

Every MCP tool execution:
1. Logs to `audit/{date}.jsonl` via `audit_logger.log_event`
2. Includes `tool`, `action` (command), `result` (success/error/blocked/dry_run), and `host` (for remote tools)
3. Failures trigger `notification/failure_logger.write_failure_log`
4. Desktop notifications sent for both success and failure on `/agent/remote/run-command`
