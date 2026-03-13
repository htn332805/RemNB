# AIOS — Codebase Analysis Master Index

**Generated:** 2026-03-12  
**Analyzed version:** 0.1.0  
**Total Python source files:** 181 (project-owned, excluding `.pyvenv`)  
**Total test functions:** 375+
**Total source lines (approx.):** 12,000+

---

## Report Sections

| # | File | Contents |
|---|---|---|
| 1 | [01_project_overview.md](01_project_overview.md) | Project identity, purpose, full tech stack, deployment topology, environment variables, phased development history |
| 2 | [02_architecture_and_structure.md](02_architecture_and_structure.md) | Architectural philosophy, full module dependency graph, all data flow diagrams, package tree, inter-module dependency rules, workspace isolation |
| 3 | [03_agent_and_routing.md](03_agent_and_routing.md) | FastAPI app bootstrap, all API endpoints, intent classification (pattern matcher + LLM confirmation), session management, scheduler CRUD, Pydantic models |
| 4 | [04_dashboard_builder.md](04_dashboard_builder.md) | DashboardSpec schema, code generator (template approach, chart types, data sources), launcher, registry, health checker, improver, catalog writer, end-to-end orchestrator |
| 5 | [05_pipeline_and_mcp.md](05_pipeline_and_mcp.md) | Pipeline abstractions (PipelineStep, PipelineContext, run_pipeline), multi-server runner, log harvester, data extractor; all MCP tools (shell, SSH, Telnet, filesystem, Qdrant, Obsidian); output engine (6 format builders, ReportSpec data model) |
| 6 | [06_security_layer.md](06_security_layer.md) | Full Phase 0 security layer: output sanitizer, command validator (allowlist YAML), scope enforcer, audit logger (JSONL format), resource monitor, timeout guard, confirmation gate, dry-run gate; security observations and recommendations |
| 7 | [07_memory_and_learning.md](07_memory_and_learning.md) | Qdrant client and 6 collections, embedder (nomic-embed-text), document store, context budget (slots A–F), context provider, compressor, alignment checker, vault watcher; full learning loop pipeline (TQS scoring, lesson extraction, quality gate, 6-stage conflict detection, review queue, insight indexer); knowledge analytics; vault CLI |
| 8 | [08_configuration_and_infrastructure.md](08_configuration_and_infrastructure.md) | All environment variables, YAML config files (allowlist, server registry, task schema), Docker Compose (Qdrant + OpenWebUI), pyproject.toml, requirements.txt, offline build cache, workspace layout, obsidian vault layout, audit directory, service port map, startup sequence |
| 9 | [09_testing_and_quality.md](09_testing_and_quality.md) | Full test inventory (74 files, 375+ functions), test patterns (unit/integration/mock), running instructions, verification scripts, scenario results docs, code quality observations |

---

## Quick Reference: Key Files

### Entry Points
| What | File |
|---|---|
| FastAPI Agent API | [agent/main.py](../agent/main.py) |
| Vault CLI | [vault_cli/cli.py](../vault_cli/cli.py) |
| Docker services | [docker-compose.yml](../docker-compose.yml) |
| Package definition | [pyproject.toml](../pyproject.toml) |

### Security
| What | File |
|---|---|
| Command allowlist | [configs/security/allowlist.yml](../configs/security/allowlist.yml) |
| Command validator | [agent/security/command_validator.py](../agent/security/command_validator.py) |
| Injection scanner | [agent/security/output_sanitizer.py](../agent/security/output_sanitizer.py) |
| Path scope enforcer | [agent/security/scope_enforcer.py](../agent/security/scope_enforcer.py) |
| Audit logger | [agent/security/audit_logger.py](../agent/security/audit_logger.py) |
| Resource monitor | [agent/security/resource_monitor.py](../agent/security/resource_monitor.py) |
| Timeout guard | [agent/security/timeout_guard.py](../agent/security/timeout_guard.py) |

### Core Execution
| What | File |
|---|---|
| Local shell execution | [mcp/shell_executor/executor.py](../mcp/shell_executor/executor.py) |
| SSH execution | [mcp/ssh_executor/executor.py](../mcp/ssh_executor/executor.py) |
| Telnet execution | [mcp/telnet_executor/executor.py](../mcp/telnet_executor/executor.py) |
| Filesystem ops | [mcp/filesystem/fs.py](../mcp/filesystem/fs.py) |
| Pipeline runner | [pipeline/orchestrator.py](../pipeline/orchestrator.py) |
| Multi-server fanout | [pipeline/multi_server_runner.py](../pipeline/multi_server_runner.py) |

### Intelligence
| What | File |
|---|---|
| Intent classification | [agent/router/pattern_matcher.py](../agent/router/pattern_matcher.py) |
| LLM confirmation | [agent/router/llm_confirm.py](../agent/router/llm_confirm.py) |
| Embeddings | [memory/embedder.py](../memory/embedder.py) |
| Vector store | [memory/store.py](../memory/store.py) |
| Context budget | [memory/context_budget.py](../memory/context_budget.py) |
| Learning loop | [learning_loop/loop_runner.py](../learning_loop/loop_runner.py) |
| TQS scoring | [learning_loop/outcome_evaluator.py](../learning_loop/outcome_evaluator.py) |
| Lesson extraction | [learning_loop/lesson_extractor.py](../learning_loop/lesson_extractor.py) |
| Conflict detection | [learning_loop/conflict_detector.py](../learning_loop/conflict_detector.py) |

### Configuration
| What | File |
|---|---|
| Settings | [agent/config/settings.py](../agent/config/settings.py) |
| Environment template | [.env.example](../.env.example) |
| Dependencies | [requirements.txt](../requirements.txt) |

---

## Module Statistics

| Module | Source Files | Test Files | Description |
|---|---|---|---|
| `agent/` | 15 | 12 | FastAPI app, router, scheduler, security, session |
| `mcp/` | 13 | 6 | Tool executors (shell, SSH, Telnet, FS, Qdrant, Obsidian) |
| `pipeline/` | 6 | 5 | Multi-server orchestration, log harvest, data extraction |
| `output_engine/` | 9 | 7 | 6-format report builders |
| `dashboard/` | 9 | 7 | Dash app lifecycle management |
| `memory/` | 11 | 9 | Qdrant-backed semantic memory |
| `learning_loop/` | 10 | 8 | Continuous learning pipeline |
| `knowledge_analytics/` | 2 | 1 | Knowledge base growth reporting |
| `notification/` | 3 | 3 | Desktop notifications, markers, failure logs |
| `vault_cli/` | 9 | 8 | `aios-vault` CLI commands |
| **Total** | **87** | **66** | |

---

## Architecture at a Glance

```
┌────────────────────────────────────────────────────────────────┐
│                     CLIENTS                                     │
│   OpenWebUI (port 3000)  │  REST  │  aios-vault CLI            │
└──────────────────────────┼────────────────────────────────────-┘
                           │
                           ▼ HTTP / WS / SSE
┌────────────────────────────────────────────────────────────────┐
│           AIOS AGENT API — FastAPI (port 8080)                  │
│                                                                  │
│  Intent Router ──► Pattern Matcher + LLM Confirmation           │
│                                                                  │
│  ┌──────────────────────────────────────────────────┐           │
│  │           PHASE 0 SAFETY LAYER                    │          │
│  │  Output Sanitizer → Command Validator →           │          │
│  │  Scope Enforcer → Dry-Run Gate →                  │          │
│  │  Confirmation Gate → Timeout Guard →              │          │
│  │  Resource Monitor → Audit Logger                  │          │
│  └──────────────────────────────────────────────────┘           │
│                                                                  │
│  ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌───────────┐            │
│  │ Shell   │ │ SSH /   │ │ Pipeline │ │ Dashboard │            │
│  │ Exec    │ │ Telnet  │ │ Runner   │ │ Builder   │            │
│  └────┬────┘ └────┬────┘ └─────┬────┘ └─────┬─────┘            │
│       │           │             │             │                  │
│  ┌────▼───────────▼─────────────▼─────────────▼───────────┐    │
│  │                 SCHEDULER (APScheduler)                   │   │
│  └───────────────────────────────────────────────────────-──┘   │
└────────────────────────────────────────────────────────────────-┘
                           │
           ┌───────────────┼───────────────┐
           ▼               ▼               ▼
┌──────────────┐  ┌────────────────┐  ┌──────────┐
│  QDRANT DB   │  │ OUTPUT ENGINE  │  │  OBSIDIAN│
│  (port 6333) │  │ CSV/XLSX/PDF/  │  │   VAULT  │
│  6 Collections│ │ HTML/DOCX/MD   │  │ Markdown │
└──────┬───────┘  └────────────────┘  └──────────┘
       │
       ▼
┌────────────────────────────────────────────────────────────┐
│              LEARNING LOOP                                  │
│  Outcome Evaluation → Lesson Extraction →                   │
│  Quality Gate → Conflict Detection → Knowledge Index        │
└────────────────────────────────────────────────────────────┘
```

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| **Single API entry point** | All requests flow through `agent/main.py` — simplifies security enforcement and audit |
| **Phase 0 safety layer is standalone** | `agent/security/` imports nothing from other app modules — no circular deps, independently testable |
| **In-memory scheduler registry** | Simple for Phase 1; documented limitation; persistence layer is planned |
| **Layered injection detection** | Both sanitizer (output_sanitizer) and validator (command_validator) check for injection — redundant but intentional |
| **nomic-embed-text via local Ollama** | No external API calls for embeddings — fully air-gappable |
| **Anthropic SDK pointing at Ollama** | Allows swapping between cloud and local LLMs without code changes |
| **Template-based dashboard generation** | No dynamic code execution; generated files are readable Python |
| **Qdrant for all semantic memory** | Consistent vector storage; all six collections use cosine similarity |
| **TQS multi-dimensional scoring** | Separates functional correctness (50%) from criteria (30%) and alignment (20%) |
| **6-stage conflict detection** | Prevents contradictory knowledge from accumulating without human review |

---

## Known Issues & Future Work

| Area | Issue / Gap |
|---|---|
| Scheduler persistence | Tasks lost on restart — no disk-backed registry |
| Obsidian sync | `mcp/obsidian_sync/sync.py` is a stub — full bi-directional sync not implemented |
| SSH host key verification | `AutoAddPolicy` accepts all keys — MITM risk on untrusted networks |
| `WEBUI_SECRET_KEY` default | Must be changed before any real deployment |
| Telnet security | Plaintext protocol — restrict to isolated network segments |
| `enable_learning` field | Not declared in `ShellRequest` model but referenced in handler |
| No conftest.py | Some test setup code is duplicated across test files |
| No coverage thresholds enforced in CI | Coverage not automatically gated |
| Context accumulator grows unbounded | No rotation or truncation implemented |
| Review queue is JSONL append-only | No expiry or cleanup mechanism |
