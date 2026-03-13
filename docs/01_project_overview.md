# AIOS — Project Overview

**Generated:** 2026-03-12  
**Version:** 0.1.0  
**Language:** Python 3.11  
**Package:** `aios` (installed as editable via `pyproject.toml`)

---

## 1. What Is AIOS?

AIOS (AI Operating System) is a self-hosted, locally-deployed AI agent platform. It acts as an intelligent middleware layer between a user (via OpenWebUI, REST API, or CLI) and a fleet of managed systems. The agent accepts natural language or structured requests, classifies intent, routes work to the appropriate subsystem, enforces safety controls, executes tasks, and feeds outcomes back into a persistent knowledge vault so that future runs improve over time.

---

## 2. Core Purpose

| Capability | Description |
|---|---|
| **Natural Language Task Dispatch** | Classify a user query and dispatch to the right executor (shell, remote, report, scheduler, dashboard) |
| **Multi-Server Infrastructure Management** | Execute commands across SSH/Telnet hosts and collect structured results |
| **Report Generation** | Produce output files in Markdown, CSV, Excel, HTML, PDF, DOCX |
| **Scheduled Automation** | Create, update, and manage cron-driven or interval-driven background tasks |
| **Interactive Dashboards** | Generate and launch Plotly Dash applications from a spec |
| **Semantic Memory** | Store and retrieve knowledge as vector embeddings in Qdrant |
| **Continuous Learning** | Extract structured lessons from completed tasks and feed them back into the knowledge base |
| **Vault Management** | Manage an Obsidian markdown vault through a dedicated CLI (`aios-vault`) |

---

## 3. Technology Stack

### Core Runtime
| Component | Library / Version |
|---|---|
| Web Framework | FastAPI 0.115.0 |
| ASGI Server | Uvicorn 0.30.6 |
| Data Validation | Pydantic v2 |
| Task Scheduling | APScheduler 3.10.4 |

### AI / ML
| Component | Library / Version |
|---|---|
| LLM Client (local Ollama proxy) | Anthropic SDK 0.34.0 |
| LLM Runtime | Ollama 0.3.3 |
| Embedding Model | `nomic-embed-text` (via Ollama HTTP) |
| Default Dev Model | `claude-3-5-sonnet-ollama` / `llama3.2:1b` |

### Data & Storage
| Component | Library / Version |
|---|---|
| Vector Database | Qdrant (`qdrant-client` 1.10.1) |
| Knowledge Vault | Obsidian (markdown files) |
| Relational-ish | JSON files on disk (journals, registries) |

### Infrastructure / Networking
| Component | Library / Version |
|---|---|
| SSH | Paramiko 3.4.1 |
| Telnet | Telnetlib3 2.0.4 |
| HTTP Requests | Requests 2.32.5 |
| Docker Services | docker-compose.yml (Qdrant + OpenWebUI) |

### Output Generation
| Format | Library |
|---|---|
| Excel (xlsx) | XlsxWriter 3.2.0, openpyxl 3.1.5 |
| Word (docx) | python-docx 1.1.2 |
| PDF | WeasyPrint 62.3 |
| Dashboard | Plotly Dash (via generated Python files) |

### Security & Monitoring
| Component | Library |
|---|---|
| Cryptography | cryptography 43.0.1 |
| Process Monitoring | psutil 6.0.0 |
| Desktop Notifications | notify2 0.3.1 |
| Filesystem Watching | watchdog 4.0.1 |

### Testing
| Component | Library |
|---|---|
| Test Framework | pytest 8.3.2 |
| Async Test Support | pytest-asyncio 0.23.8 |

---

## 4. Project Structure (Top Level)

```
aios/
├── agent/              ← FastAPI app, intent router, scheduler, security, session
├── configs/            ← YAML configuration: server registry, security allowlist
├── dashboard/          ← Plotly Dash builder, launcher, registry, tester
├── knowledge_analytics/← Qdrant growth reporting
├── learning_loop/      ← Outcome evaluation, lesson extraction, conflict detection
├── mcp/                ← MCP tool wrappers (shell, SSH, telnet, filesystem, Qdrant)
├── memory/             ← Vector memory layer (Qdrant client, embeddings, search)
├── notification/       ← Desktop notifications, task markers, failure logs
├── obsidian/           ← Obsidian vault root (markdown knowledge base)
├── output_engine/      ← Multi-format report builders
├── pipeline/           ← Multi-server orchestration, log harvesting, data extraction
├── tests/              ← Full pytest test suite (mirrors source structure)
├── vault_cli/          ← `aios-vault` CLI (review, insights, lessons, summarize…)
├── audit/              ← Append-only JSONL audit logs
├── data/               ← Qdrant persistent data, OpenWebUI cache/db
├── docs/               ← Documentation (this analysis, scenario results, etc.)
├── workspace/          ← Per-user output directories
├── docker-compose.yml  ← Qdrant + OpenWebUI containers
├── pyproject.toml      ← Package metadata, entry points
├── requirements.txt    ← Python dependencies
└── .env.example        ← Required environment variables
```

---

## 5. Deployment Architecture

```
┌─────────────────────────────────────────────┐
│  User Interface Layer                        │
│  OpenWebUI (port 3000) ← Docker             │
│  REST API clients       ← HTTP              │
│  aios-vault CLI         ← terminal          │
└──────────────┬──────────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────────┐
│  AIOS Agent API — FastAPI (port 8080)        │
│  Endpoints: /agent/intent/classify           │
│             /agent/shell/run                 │
│             /agent/remote/run-command        │
│             /agent/schedule/*                │
│             /agent/stream/{task_id}  (SSE)  │
│             /agent/ws             (WebSocket)│
└──────────────┬──────────────────────────────┘
               │
       ┌───────┼───────────────┐
       ▼       ▼               ▼
 Safety    Scheduler      Subsystems
 Layer     APScheduler    (MCP Tools,
 (SAFE-*)  backgrounds    Pipeline,
           jobs           Dashboard,
                          Output Engine)
               │
               ▼
┌─────────────────────────────────────────────┐
│  Persistence                                 │
│  Qdrant (port 6333) — vector memory         │
│  workspace/<user>/ — output files            │
│  obsidian/vault/   — markdown knowledge     │
│  audit/*.jsonl     — immutable audit trail  │
└─────────────────────────────────────────────┘
```

---

## 6. Entry Points

| Entry Point | Target |
|---|---|
| `uvicorn agent.main:app --port 8080` | Launch the agent API |
| `aios-vault <subcommand>` | CLI vault management |
| `docker compose up -d` | Start Qdrant + OpenWebUI |

---

## 7. Key Environment Variables

| Variable | Default | Purpose |
|---|---|---|
| `ANTHROPIC_BASE_URL` | `http://localhost:11434` | LLM endpoint (Ollama proxy) |
| `OLLAMA_HOST` | `http://localhost:11434` | Embedding server |
| `QDRANT_HOST` | `localhost` | Qdrant host |
| `QDRANT_PORT` | `6333` | Qdrant port |
| `AGENT_API_PORT` | `8080` | Agent FastAPI port |
| `AIOS_ROOT` | `~/aios` | Project root path |
| `AUDIT_LOG_DIR` | `~/aios/audit` | Audit log directory |
| `DEFAULT_TASK_TIMEOUT_SECONDS` | `300` | Per-task timeout |
| `RESOURCE_CPU_MAX` | `85` | CPU usage limit (%) |
| `RESOURCE_MEM_MB_MAX` | `512` | Memory limit (MB) |

---

## 8. Phased Development History

The codebase was built in numbered phases (Phase 0 through Phase 12), each adding capability:

| Phase | Focus |
|---|---|
| Phase 0 | Safety layer (SAFE-01 through SAFE-08): command validation, dry-run, scope enforcement, audit, resource monitoring |
| Phase 1-2 | Agent API skeleton, pipeline orchestrator, shell/SSH/Telnet executors |
| Phase 3 | Multi-server runner, log harvesting, data extraction |
| Phase 4 | Output engine (CSV, Excel, PDF, HTML, DOCX, Markdown) |
| Phase 5 | Memory layer (Qdrant collections, embedder, context budget, alignment checker) |
| Phase 6 | Notification system, failure logger, task markers |
| Phase 7 | Obsidian vault watcher, MCP filesystem |
| Phase 8 | Learning loop (outcome evaluation, lesson extraction, conflict detection, knowledge analytics) |
| Phase 9 | SSE streaming, WebSocket, Docker infrastructure |
| Phase 10 | Dashboard builder/launcher/registry/improver |
| Phase 11 | Vault CLI (`aios-vault`), knowledge growth reporter |
| Phase 12 | Global error handler, scheduler finalization, documentation |
