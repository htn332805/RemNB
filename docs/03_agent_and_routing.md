# AIOS — Agent Core, Intent Routing & Scheduler

**Generated:** 2026-03-12

---

## 1. Agent Overview

**Module:** `agent/`  
**Entry point:** `agent/main.py`  
**Runtime:** FastAPI + Uvicorn, port 8080

The agent is the single entry point into the AIOS system. It exposes an HTTP API that:
- Accepts structured JSON requests from OpenWebUI, REST clients, or internal callers
- Routes requests to the appropriate subsystem after safety validation
- Streams real-time status updates via SSE
- Provides bidirectional communication via WebSocket
- Returns all errors in a standard structured shape

---

## 2. FastAPI Application (`agent/main.py`)

### Application Bootstrap

```python
app = FastAPI(title="AIOS Agent API", version="1.0.0")
```

On startup, the APScheduler background service is started:
```python
@app.on_event("startup")
async def _startup():
    from agent.scheduler.service import start_scheduler
    start_scheduler()
```

### Global Exception Handler

All unhandled exceptions are caught by a global handler that:
- Logs the full traceback (server-side only — never exposed to client)
- Returns a **structured 500 JSON response** — never raw tracebacks

```json
{
  "error_code": "internal_error",
  "message": "An unexpected error occurred.",
  "details": "<exception string>",
  "task_id": null
}
```

---

## 3. REST API Endpoints

### Health Check
| Method | Path | Description |
|---|---|---|
| GET | `/health` | Liveness check — returns `{"status": "ok"}` |

### Intent Classification
| Method | Path | Description |
|---|---|---|
| POST | `/agent/intent/classify` | Classify a natural language query into a structured intent |

**Request model:**
```json
{
  "username": "tester",
  "query": "check disk space on all servers",
  "session_id": null
}
```

**Response model:**
```json
{
  "intent": "shell_cmd",
  "confidence": 0.67,
  "requires_confirmation": true,
  "metadata": {}
}
```

### Shell Execution
| Method | Path | Description |
|---|---|---|
| POST | `/agent/shell/run` | Execute a shell command locally with full safety gates |

**Request model:**
```json
{
  "username": "tester",
  "cmd": "df -h",
  "dry_run": false
}
```

Post-execution hooks:
1. Desktop notification (success / failure)
2. Task completion marker file written
3. Learning loop triggered asynchronously

### Remote Command Execution
| Method | Path | Description |
|---|---|---|
| POST | `/agent/remote/run-command` | Execute a command on all user-defined servers |

Loads `workspace/{username}/configs/servers.yml` → runs via `pipeline.multi_server_runner` → collects per-server results → logs failures.

### Scheduler CRUD
| Method | Path | Description |
|---|---|---|
| POST | `/agent/schedule` | Create a scheduled task |
| GET | `/agent/schedule` | List all scheduled tasks |
| GET | `/agent/schedule/{id}` | Get one task by ID |
| DELETE | `/agent/schedule/{id}` | Delete a task and unschedule its job |
| PATCH | `/agent/schedule/{id}` | Partial update (toggles live scheduler on `enabled` change) |

### SSE Streaming
| Method | Path | Description |
|---|---|---|
| GET | `/agent/stream/{task_id}` | Server-Sent Events stream for a task |

- Producers call `push_sse_event(task_id, event_dict)` to enqueue events
- Sentinel event `{"type": "done"}` closes the stream
- Heartbeat comment sent every 30s to keep connection alive
- Queue cleaned up after stream closes

### WebSocket
| Method | Path | Description |
|---|---|---|
| WS | `/agent/ws` | Bidirectional WebSocket for interactive sessions |

Supported message types:
- `{"type": "ping"}` → responds `{"type": "pong"}`
- `{"type": "cancel", "task_id": "..."}` → injects `done/cancelled` event into SSE queue

---

## 4. Intent Classification System

### Pattern Matcher (`agent/router/pattern_matcher.py`)

Intent classification uses a keyword-scoring algorithm:

**Intent Labels:**
| Label | Description |
|---|---|
| `shell_cmd` | Local shell command execution |
| `remote_exec` | SSH/Telnet remote command |
| `report_gen` | Report/summary generation |
| `test_plan` | Test plan or procedure |
| `schedule_task` | Recurring/cron task creation |
| `search_knowledge` | Vault/Qdrant knowledge search |
| `dashboard_gen` | Dashboard generation |
| `vault_action` | Obsidian vault operation |
| `unknown` | No confident match |

**Scoring Algorithm:**
```python
score = min(1.0, keyword_hits / 3.0)
```
Each intent has a set of keywords; hits are accumulated and normalized. The highest-scoring intent wins. If all scores are 0.0, intent is `"unknown"` with confidence 0.0.

**`requires_confirmation`** is set to `True` for:
- `shell_cmd`
- `remote_exec`
- `schedule_task`

### LLM Confirmation (`agent/router/llm_confirm.py`)

When pattern matcher confidence is below `LLM_CONFIRM_THRESHOLD = 0.6` OR intent is `"unknown"`, the result is escalated to an LLM:

- **Model:** `claude-3-5-sonnet-ollama` (Anthropic SDK pointing at local Ollama endpoint)
- **Token budget:** 256 tokens
- **System prompt:** Forces structured JSON response with `intent`, `confidence`, and `reason` keys
- **Fallback:** On any parse error, returns original pattern-matcher result

---

## 5. Session Management (`agent/session/session_manager.py`)

Minimal session management creates per-user workspace directories on demand:

```python
def ensure_workspace_exists(username: str) -> Path:
    root = Path.home() / "aios" / "workspace" / username
    root.mkdir(parents=True, exist_ok=True)
    return root
```

Called at the start of every `/intent/classify` request to guarantee the user's workspace exists before any downstream operation.

---

## 6. Scheduler Module (`agent/scheduler/`)

The scheduler manages recurring automated tasks using APScheduler's `BackgroundScheduler`.

### Models (`agent/scheduler/models.py`)

```
ScheduleTaskRequest:
  name:     str         — human-readable task name
  username: str         — task owner
  cron:     str         — cron expression (e.g., "0 * * * *")
  intent:   str         — AIOS intent label
  payload:  Dict        — additional parameters
  enabled:  bool = True

ScheduleTaskResponse:
  id:               str
  name:             str
  username:         str
  cron:             str
  intent:           str
  dry_run_verified: bool
  enabled:          bool
```

### Registry (`agent/scheduler/registry.py`)

In-memory dictionary mapping task IDs to task dicts. Provides:
- `add_task(data)` — assigns UUID id, stores
- `get_task(task_id)` — lookup by ID
- `list_tasks()` — return all tasks
- `delete_task(task_id)` — remove
- `update_task(task_id, updates)` — partial update via dict merge

> **Note:** Registry is in-memory only; tasks are lost on process restart. Persistence via a file/DB backend is a future improvement.

### Service (`agent/scheduler/service.py`)

Manages a singleton `BackgroundScheduler` instance:
- `start_scheduler()` — idempotent start
- `schedule_job(task)` — registers job from task dict using `CronTrigger` or `IntervalTrigger`
- `remove_job(task_id)` — unschedule a job (silent if not present)

### Executor (`agent/scheduler/executor.py`)

`run_scheduled_task(task)` — called by APScheduler for each fired job. Executes the task payload by routing to the appropriate AIOS subsystem based on `task["intent"]`.

### API (`agent/scheduler/api.py`)

RESTful CRUD operations mounted at `/agent/schedule`:

| Endpoint | Task Ref | Description |
|---|---|---|
| `POST /` | task_028 | Create + optionally schedule |
| `GET /` | task_029 | List all tasks |
| `GET /{id}` | task_030 | Get one task |
| `DELETE /{id}` | task_031 | Delete + unschedule |
| `PATCH /{id}` | task_032 | Update + toggle live scheduler |

---

## 7. Configuration (`agent/config/settings.py`)

All settings are loaded from environment variables at import time:

| Setting | Env Var | Default | Description |
|---|---|---|---|
| `AIOS_ROOT` | `AIOS_ROOT` | `~/aios` | Project root path |
| `APPROVED_READ_PATHS` | — | `/aios/`, `/var/log/`, `/proc/`, `/sys/`, `/etc/hostname` | Paths readable by agent |
| `WORKSPACE_WRITE_ROOT` | — | `~/aios/workspace/{username}/` | Per-user write root |
| `OBSIDIAN_WRITE_ROOT` | — | `~/aios/obsidian/vault/` | Vault write root |
| `ABSOLUTE_WRITE_BLOCK` | — | `/etc/`, `/boot/`, `/sys/`, `/proc/`, `/root/`, `/bin/`, `/sbin/`, `/usr/`, `/lib/` | Unconditionally blocked paths |
| `DEFAULT_TASK_TIMEOUT_SECONDS` | `DEFAULT_TASK_TIMEOUT_SECONDS` | `300` | Per-task timeout cap |
| `MAX_TASK_TIMEOUT_SECONDS` | `MAX_TASK_TIMEOUT_SECONDS` | `3600` | Absolute maximum timeout |
| `RESOURCE_LIMITS` | `RESOURCE_CPU_MAX`, `RESOURCE_MEM_MB_MAX`, etc. | cpu=85%, mem=512MB, files=100 | Resource usage limits |
| `DRY_RUN_DEFAULT` | — | `False` | Global dry-run toggle |

---

## 8. Request/Response Model Summary

| Pydantic Model | Module | Purpose |
|---|---|---|
| `QueryRequest` | `agent/main.py` | Intent classify request |
| `IntentResponse` | `agent/main.py` | Intent classify response |
| `ShellRequest` | `agent/main.py` | Shell execution request |
| `RemoteCommandRequest` | `agent/main.py` | Remote command request |
| `ScheduleTaskRequest` | `agent/scheduler/models.py` | Create scheduled task |
| `ScheduleTaskResponse` | `agent/scheduler/models.py` | Scheduled task data |
| `MCPResult` (TypedDict) | `mcp/common/types.py` | All tool execution results |

---

## 9. Known Limitations & Design Notes

| Area | Note |
|---|---|
| Scheduler persistence | In-memory only; restarting the agent loses all scheduled tasks |
| Session management | Minimal — workspace directory only; no auth tokens or session state |
| LLM confirmation | Requires Ollama running locally with `claude-3-5-sonnet-ollama` model loaded |
| SSE queue | In-memory `asyncio.Queue` per task; does not survive process restart |
| Pattern matcher | Simple keyword frequency scoring; not context-aware |
| `enable_learning` flag | Checked via `req.model_dump().get("enable_learning", True)` but `ShellRequest` model does not declare this field — it silently defaults to `True` |
