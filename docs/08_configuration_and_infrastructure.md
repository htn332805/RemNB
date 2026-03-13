# AIOS — Configuration, Infrastructure & Deployment

**Generated:** 2026-03-12

---

## 1. Overview

This section covers all configuration, infrastructure, and deployment aspects of AIOS — including environment variables, YAML configuration files, Docker Compose services, Python packaging, and the installer.

---

## 2. Environment Variables (`.env.example`)

All runtime configuration is injected via environment variables. No credentials or environment-specific values are hardcoded.

| Variable | Default | Required | Description |
|---|---|---|---|
| `ANTHROPIC_BASE_URL` | `http://localhost:11434` | Yes | LLM API endpoint (Ollama proxy for Anthropic SDK) |
| `OLLAMA_MODEL_DEV` | `llama3.2:1b` | No | Default Ollama model for dev/lightweight tasks |
| `OLLAMA_EMBED_MODEL` | `nomic-embed-text` | Yes | Embedding model name |
| `QDRANT_HOST` | `localhost` | No | Qdrant vector DB hostname |
| `QDRANT_PORT` | `6333` | No | Qdrant HTTP API port |
| `AGENT_API_PORT` | `8080` | No | FastAPI agent server port |
| `AIOS_ROOT` | `/home/<user>/aios` | No | Absolute path to AIOS project root |
| `AUDIT_LOG_DIR` | `/home/<user>/aios/audit` | No | Audit log output directory |
| `RESOURCE_CPU_MAX` | `85` | No | Max CPU usage % before process abort |
| `RESOURCE_MEM_MB_MAX` | `512` | No | Max RSS memory MB before process abort |
| `RESOURCE_OPEN_FILES_MAX` | `100` | No | Max open file handles before abort |
| `RESOURCE_DISK_WRITE_MB_MAX` | `500` | No | Max disk write MB (checked by executor) |
| `DEFAULT_TASK_TIMEOUT_SECONDS` | `300` | No | Per-task execution timeout |
| `MAX_TASK_TIMEOUT_SECONDS` | `3600` | No | Absolute maximum timeout ceiling |
| `AIOS_DESKTOP_NOTIFICATIONS` | `1` | No | Set to `0` to disable desktop notifications |

### Usage in Code

Settings are loaded in `agent/config/settings.py` at module import time using `os.environ.get()` with defaults. The file is a simple module-level dictionary — no framework configuration management is used.

---

## 3. YAML Configuration Files (`configs/`)

### 3.1 Security Allowlist (`configs/security/allowlist.yml`)

The only file-based security policy. Loaded lazily (once) by `command_validator.py`:

```yaml
allowed_local:   # Commands permitted for local execution
  - df
  - ls
  - ...
blocked:         # Patterns that unconditionally block execution
  - rm -rf
  - DROP TABLE
  - ...
confirm_required: # Commands requiring explicit confirmation
  - rm
  - systemctl
  - apt
  - ...
```

Adding a new safe command: append its name to `allowed_local`.  
Blocking a new threat: append its pattern to `blocked`.  
Requiring review: append its prefix to `confirm_required`.

### 3.2 Server Registry (`configs/server_registry.yml`)

Global server definitions (template/example):
```yaml
servers:
  - name: "example-server"
    host: "192.168.1.10"
    protocol: "ssh"
    username: "admin"
    password: ""      # Loaded from env or vault in production
    port: 22
```

**Note:** Per-user server lists live at `workspace/{username}/configs/servers.yml` and override the global registry for per-user isolation.

### 3.3 Task Schema (`configs/task_templates/task_schema.yml`)

YAML schema reference for scheduled task payloads — documents the expected structure for task creation API calls.

---

## 4. Docker Compose Infrastructure (`docker-compose.yml`)

Two container services are managed by Docker Compose:

### 4.1 Qdrant (Vector Database)

```yaml
qdrant:
  image: qdrant/qdrant:latest
  container_name: qdrant
  restart: unless-stopped
  ports:
    - "6333:6333"   # HTTP REST API
    - "6334:6334"   # gRPC API
  volumes:
    - ./data/qdrant:/qdrant/storage  # Persistent volume
  environment:
    - QDRANT__SERVICE__HTTP_PORT=6333
    - QDRANT__SERVICE__GRPC_PORT=6334
```

Data persisted at `data/qdrant/` (included in repo `.gitignore` for large files). Qdrant metadata (`aliases/data.json`, `raft_state.json`) is tracked.

### 4.2 OpenWebUI (Chat UI)

```yaml
openwebui:
  image: ghcr.io/open-webui/open-webui:main
  container_name: openwebui
  restart: unless-stopped
  ports:
    - "3000:8080"    # UI on host port 3000
  volumes:
    - ./data/openwebui:/app/backend/data
  environment:
    - OLLAMA_BASE_URL=http://host.docker.internal:11434
    - WEBUI_SECRET_KEY=change-me-in-production  # ⚠ Must be changed!
  extra_hosts:
    - "host.docker.internal:host-gateway"
  depends_on:
    - qdrant
```

> **Security Note:** `WEBUI_SECRET_KEY=change-me-in-production` is the default value from the example. This **must** be replaced with a strong random secret before any deployment.

OpenWebUI data (vector DB, SQLite, model cache) persisted at `data/openwebui/`:
- `data/openwebui/webui.db` — SQLite user/conversation database
- `data/openwebui/vector_db/` — Chroma vector DB for OpenWebUI RAG
- `data/openwebui/cache/embedding/models/` — sentence-transformers model cache (`all-MiniLM-L6-v2`)

---

## 5. Python Package (`pyproject.toml`)

```toml
[project]
name = "aios"
version = "0.1.0"
requires-python = ">=3.11"

[project.scripts]
aios-vault = "vault_cli.cli:main"

[tool.setuptools.packages.find]
where = ["."]
```

**Installed via:** `pip install -e .` (editable install)

**CLI entry point:** `aios-vault` maps to `vault_cli.cli:main`

---

## 6. Dependencies (`requirements.txt`)

```
fastapi==0.115.0           # Web framework
uvicorn==0.30.6            # ASGI server
anthropic==0.34.0          # LLM client (Ollama proxy)
ollama==0.3.3              # Ollama Python SDK
qdrant-client==1.10.1      # Vector DB client
paramiko==3.4.1            # SSH
telnetlib3==2.0.4          # Telnet
openpyxl==3.1.5            # Excel read/write
xlsxwriter==3.2.0          # Excel with charts
python-docx==1.1.2         # DOCX generation
weasyprint==62.3           # PDF via HTML
python-dotenv==1.0.1       # .env file loading
apscheduler==3.10.4        # Task scheduling
watchdog==4.0.1            # Filesystem events
notify2==0.3.1             # Desktop notifications
psutil==6.0.0              # Process monitoring
cryptography==43.0.1       # Cryptographic operations
pytest==8.3.2              # Test framework
pytest-asyncio==0.23.8     # Async test support
requests>=2.31.0           # HTTP client
```

---

## 7. Offline Build Cache (`.build_vendor_cache/`)

A directory of pre-downloaded `.whl` files covering all dependencies. Enables fully offline installation — critical for air-gapped deployments:

Key cached packages:
- `fastapi-0.115.0`, `uvicorn-0.30.6`, `starlette-0.38.6`
- `anthropic-0.34.0`, `ollama-0.3.3`
- `qdrant_client-1.10.1`
- `pydantic-2.12.5`, `pydantic_core-2.41.5`
- `paramiko-3.4.1`, `telnetlib3-2.0.4`
- `numpy-2.4.3`, `pillow-12.1.1`, `weasyprint-62.3`
- `grpcio-1.78.0`, `protobuf-6.33.5`
- `huggingface_hub-1.6.0`, `tokenizers-0.22.2`
- All test, crypto, and utility packages

---

## 8. Virtual Environment (`.pyvenv/`)

A Python 3.11 virtual environment bundled with the project at `.pyvenv/`. Provides a consistent, isolated runtime. The `.pyvenv/lib/python3.11/site-packages/` contains all installed packages.

---

## 9. Installer (`aios-installer.run`)

A self-contained shell installer script that:
1. Creates/activates the virtual environment
2. Installs dependencies from `.build_vendor_cache/` (offline)
3. Runs `pip install -e .` for the `aios` package
4. Optionally starts the Docker services
5. Verifies the installation with a health check

---

## 10. Git Configuration

**`.gitignore`** excludes:
- `.pyvenv/` — virtual environment (large)
- `__pycache__/`, `*.pyc`, `*.pyo`
- `.env` (secrets)
- `data/openwebui/cache/` (large model blobs)
- `*.egg-info/`

**`.github/pull_request_template.md`** — PR template for contributors.

---

## 11. Workspace Directory Layout

Per-user workspace directories are created on first use:

```
workspace/
└── {username}/
    ├── configs/
    │   └── servers.yml          ← SSH/Telnet server definitions
    ├── outputs/                 ← Generated reports and files
    │   ├── report_2026-03-12.xlsx
    │   ├── report_2026-03-12.pdf
    │   └── ...
    ├── task_history/            ← Pipeline execution journals (JSON)
    │   ├── shell_a1b2c3d4.json
    │   └── ...
    ├── notifications/           ← Task completion marker files
    │   └── task_completed_{id}.marker
    ├── failure_logs.jsonl       ← Append-only failure records
    └── learning_accumulator.jsonl  ← Learning loop event log
```

---

## 12. Obsidian Vault Layout

```
obsidian/
└── vault/
    ├── Knowledge/
    │   ├── LessonsLearned/      ← Lesson markdown files
    │   ├── BestPractices/       ← Best practice notes
    │   ├── ErrorPatterns/       ← Known error pattern notes
    │   ├── Conflicts/           ← Conflict detection reports
    │   └── ReviewQueue/
    │       └── pending.jsonl    ← Items awaiting human review
    ├── Reports/                 ← Synced report documents
    └── Scripts/                 ← Script references
```

---

## 13. Audit Directory

```
audit/
└── audit_{YYYYMMDD}.jsonl   ← Daily append-only audit log
    (e.g., audit_20260312.jsonl)
```

Each day a new file is created. Files accumulate over time and should be archived if disk space is a concern.

---

## 14. Service Port Summary

| Service | Port | Protocol | Description |
|---|---|---|---|
| AIOS Agent API | 8080 | HTTP | FastAPI REST + SSE + WebSocket |
| OpenWebUI | 3000 | HTTP | Chat UI (Docker) |
| Qdrant HTTP | 6333 | HTTP | Vector DB REST API |
| Qdrant gRPC | 6334 | gRPC | Vector DB gRPC API |
| Ollama | 11434 | HTTP | LLM + embedding server (external) |
| Dash Dashboards | 8060–8089 | HTTP | Auto-allocated per generated dashboard |

---

## 15. Recommended Startup Sequence

```bash
# 1. Start infrastructure
docker compose up -d

# 2. Activate environment
source .pyvenv/bin/activate

# 3. Copy and configure environment
cp .env.example .env
# Edit .env with real paths and secrets

# 4. Initialize Qdrant collections
python3 -c "from memory.collections import init_all_collections; init_all_collections()"

# 5. Start the agent API
uvicorn agent.main:app --host 0.0.0.0 --port 8080 --reload

# 6. (Optional) Start vault file watcher
# python3 -m memory.vault_watcher
```
