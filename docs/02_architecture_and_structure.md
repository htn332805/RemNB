# AIOS — Architecture & Codebase Structure

**Generated:** 2026-03-12

---

## 1. Architectural Philosophy

AIOS follows a **layered dispatch architecture** where:

1. **All requests enter through one surface** — the FastAPI agent at `agent/main.py`.
2. **Every operation is safety-filtered** before execution (Phase 0 safety layer).
3. **Execution results are audit-logged** immutably to JSONL files.
4. **Outcomes feed the knowledge vault** (Qdrant + Obsidian) via the learning loop.
5. **Real-time updates stream** to clients via SSE and WebSocket.

The codebase is organized as **feature modules** (each a Python package with `__init__.py`), not as layered tiers. Every module is independently testable and has a corresponding test package under `tests/`.

---

## 2. Module Dependency Graph

```
agent/main.py
├── agent/router/           ← intent classification
│   ├── pattern_matcher.py  ← keyword scoring
│   └── llm_confirm.py      ← Anthropic LLM confirmation
│
├── agent/security/         ← all security gates (imported by MCP executors)
│   ├── command_validator.py
│   ├── output_sanitizer.py
│   ├── scope_enforcer.py
│   ├── audit_logger.py
│   ├── confirmation_gate.py
│   ├── dry_run_gate.py
│   ├── timeout_guard.py
│   └── resource_monitor.py
│
├── agent/scheduler/        ← APScheduler integration
│   ├── api.py              ← REST CRUD routes
│   ├── service.py          ← scheduler lifecycle
│   ├── executor.py         ← job execution
│   ├── models.py           ← Pydantic models
│   └── registry.py         ← in-memory task store
│
├── agent/session/          ← workspace per-user isolation
│   └── session_manager.py
│
├── agent/config/           ← settings loaded from env
│   └── settings.py
│
├── mcp/                    ← tool executors (consume security layer)
│   ├── shell_executor/
│   ├── ssh_executor/
│   ├── telnet_executor/
│   ├── filesystem/
│   ├── obsidian_sync/
│   ├── qdrant_tool/
│   └── openwebui_tools/
│
├── pipeline/               ← multi-server orchestration
│   ├── orchestrator.py
│   ├── multi_server_runner.py
│   ├── log_harvester.py
│   ├── data_extractor.py
│   ├── test_suite_runner.py
│   └── stress_orchestrator.py
│
├── output_engine/          ← report builders
│   ├── models.py           ← ReportSpec, TableSpec, ChartSpec
│   ├── csv_builder.py
│   ├── xlsx_builder.py
│   ├── md_builder.py
│   ├── html_builder.py
│   ├── pdf_builder.py
│   ├── docx_builder.py
│   └── workspace_wiring.py
│
├── dashboard/              ← Dash app lifecycle
│   ├── builder.py
│   ├── launcher.py
│   ├── registry.py
│   ├── tester.py
│   ├── improver.py
│   └── orchestrator.py
│
├── memory/                 ← Qdrant-backed semantic memory
│   ├── qdrant_client.py    ← connection + collection constants
│   ├── collections.py      ← collection initialization
│   ├── embedder.py         ← nomic-embed-text via Ollama
│   ├── store.py            ← upsert with embedding
│   ├── search.py           ← semantic search
│   ├── schemas.py          ← payload schema constants
│   ├── compressor.py       ← context window compression
│   ├── context_budget.py   ← token-budget slot selection
│   ├── context_provider.py ← assemble context for LLM calls
│   ├── alignment_checker.py← verify lesson alignment
│   └── vault_watcher.py    ← watchdog-based file event sync
│
├── learning_loop/          ← continuous improvement pipeline
│   ├── loop_runner.py      ← orchestrator (main entry point)
│   ├── outcome_evaluator.py← TQS scoring
│   ├── lesson_extractor.py ← LLM lesson extraction
│   ├── quality_gate.py     ← confidence classification
│   ├── conflict_detector.py← 6-stage conflict pipeline
│   ├── insight_indexer.py  ← Qdrant upsert
│   ├── review_queue.py     ← pending human review queue
│   ├── context_accumulator.py← per-user event log
│   ├── escalation.py       ← alert routing
│   └── error_pattern_seeder.py← seed initial error patterns
│
├── knowledge_analytics/
│   └── growth_reporter.py  ← per-collection entry counting
│
├── notification/           ← user notification subsystem
│   ├── notifier.py         ← notify2 desktop notifications
│   ├── markers.py          ← task completion marker files
│   └── failure_logger.py   ← structured failure JSONL log
│
└── vault_cli/              ← aios-vault CLI
    ├── cli.py              ← argparse entry point
    ├── review.py
    ├── insights.py
    ├── lessons.py
    ├── summarize.py
    ├── reorganize.py
    ├── analyze.py
    └── diff.py
```

---

## 3. Data Flow — Standard Request

```
Request (HTTP POST /agent/intent/classify)
    │
    ▼
FastAPI pydantic validation → QueryRequest
    │
    ▼
session_manager.ensure_workspace_exists(username)
    │
    ▼
pattern_matcher.classify_intent(query)
    │ confidence < 0.6 or intent == "unknown"?
    ▼ YES
llm_confirm.confirm_intent_with_llm(query, intent, confidence)
    │
    ▼
IntentResponse JSON → client
```

---

## 4. Data Flow — Shell Execution

```
POST /agent/shell/run  {username, cmd, dry_run}
    │
    ▼
mcp/shell_executor/executor.execute_local(cmd, dry_run)
    │
    ├─► output_sanitizer.sanitize(cmd)          ← injection pattern scan
    │       │  BLOCKED → return MCPResult(error)
    │       ↓ SAFE
    ├─► command_validator.validate_command(cmd)  ← allowlist / blocklist
    │       │  BLOCK → return MCPResult(error)
    │       │  CONFIRM_REQUIRED → (non-interactive: abort)
    │       ↓ ALLOW
    ├─► dry_run check → return dry_run MCPResult if True
    │
    ├─► subprocess.Popen(shlex.split(cmd))
    ├─► resource_monitor.start_monitoring(pid)  ← background thread
    │
    ├─► proc.communicate(timeout=30)
    │       TimeoutExpired → kill + return error
    │       resource limit breach → kill + return error
    │
    ├─► audit_logger.log_event(...)
    ├─► notification.send_desktop_notification(...)
    ├─► markers.write_task_completion_marker(...)
    └─► learning_loop.run_learning_loop(run_context)
```

---

## 5. Data Flow — Learning Loop

```
run_learning_loop(run_context)
    │
    ▼
evaluate_outcome(run_context) → {tqs, label, functional, criteria, alignment}
    │
    ▼
extract_lesson(run_context, tqs_result) via LLM
    │ None → update_accumulator + return {action: "no_lesson"}
    ▼
classify_confidence(lesson) → tier
    │
    ├─ "discard" → update_accumulator + return
    ├─ "pending_review" | "provisional"
    │       → add_to_review_queue + update_accumulator + return
    │
    └─ "validated"
            │
            ▼
        detect_conflicts(lesson) → {blocked, conflicts, ...}
            │ blocked → add_to_review_queue + return
            ▼ not blocked
        index_lesson(lesson) → upsert into Qdrant "knowledge" collection
        update_accumulator(username, ...)
```

---

## 6. Package Layout — Full Source Tree

```
agent/
    __init__.py
    main.py                   ← FastAPI app, all route registrations
    config/
        __init__.py
        settings.py           ← env-driven config constants
    router/
        __init__.py
        pattern_matcher.py    ← keyword-based intent scorer
        llm_confirm.py        ← LLM-backed intent confirmation
    scheduler/
        __init__.py
        api.py                ← REST CRUD: POST/GET/DELETE/PATCH /schedule
        service.py            ← APScheduler lifecycle
        executor.py           ← job runner
        models.py             ← Pydantic request/response models
        registry.py           ← in-memory task registry
    security/
        __init__.py
        audit_logger.py       ← append-only JSONL audit
        command_validator.py  ← allowlist/blocklist YAML enforcement
        confirmation_gate.py  ← interactive prompt for CONFIRM_REQUIRED
        dry_run_gate.py       ← global dry-run mode toggle
        output_sanitizer.py   ← regex injection pattern scanner
        resource_monitor.py   ← psutil background resource watcher
        scope_enforcer.py     ← path read/write boundary validation
        timeout_guard.py      ← asyncio timeout wrapper + process tree killer
    session/
        __init__.py
        session_manager.py    ← per-user workspace directory management

configs/
    server_registry.yml       ← global server definitions (example)
    security/
        allowlist.yml         ← command allow/block/confirm lists
    task_templates/
        task_schema.yml       ← task payload schema definition

dashboard/
    __init__.py
    builder.py                ← DashboardSpec → Dash Python file generator
    catalog_writer.py         ← catalog metadata writer
    improver.py               ← iterative dashboard improvement via LLM
    launcher.py               ← subprocess Dash app launch
    orchestrator.py           ← end-to-end: build → register → launch → health-check
    registry.py               ← JSON registry of launched dashboards
    spec_examples.py          ← example DashboardSpec dicts
    tester.py                 ← HTTP health check for launched apps
    versions/                 ← generated Dash app .py files

knowledge_analytics/
    __init__.py
    growth_reporter.py        ← count Qdrant entries by collection and date range

learning_loop/
    __init__.py
    conflict_detector.py      ← 6-stage knowledge conflict pipeline
    context_accumulator.py    ← per-user event accumulation log
    error_pattern_seeder.py   ← seed initial error patterns into Qdrant
    escalation.py             ← alert/escalation routing
    insight_indexer.py        ← Qdrant upsert for validated lessons
    lesson_extractor.py       ← LLM-driven structured lesson extraction
    loop_runner.py            ← main orchestrator (entry point)
    outcome_evaluator.py      ← TQS multi-dimensional scoring
    quality_gate.py           ← confidence threshold classification
    review_queue.py           ← pending-review JSONL queue

mcp/
    __init__.py
    common/
        __init__.py
        types.py              ← MCPResult TypedDict
    filesystem/
        __init__.py
        fs.py                 ← read_file / write_file with scope enforcement
        executor.py           ← higher-level filesystem operations
    obsidian_sync/
        __init__.py
        sync.py               ← sync_to_vault stub
    openwebui_tools/
        aios_agent_tool.py    ← OpenWebUI tool call wrapper
    qdrant_tool/
        __init__.py
        client.py             ← Qdrant MCP search/upsert wrapper
    shell_executor/
        __init__.py
        executor.py           ← execute_local with full safety gates
    ssh_executor/
        __init__.py
        executor.py           ← execute_ssh_command with retry
    telnet_executor/
        __init__.py
        executor.py           ← execute_telnet_command

memory/
    __init__.py
    alignment_checker.py      ← knowledge alignment verification
    collections.py            ← init all 6 Qdrant collections
    compressor.py             ← context window summarization
    context_budget.py         ← token-budget slot selection (Slots A–F)
    context_provider.py       ← assemble multi-slot LLM context
    embedder.py               ← nomic-embed-text via Ollama /api/embeddings
    qdrant_client.py          ← QdrantClient factory + collection name constants
    schemas.py                ← payload key constants
    search.py                 ← semantic_search wrapper
    store.py                  ← upsert_documents with auto-embedding
    vault_watcher.py          ← watchdog FileSystemEventHandler for vault sync

notification/
    __init__.py
    failure_logger.py         ← write JSONL failure record
    markers.py                ← write task completion marker file
    notifier.py               ← send_desktop_notification via notify2

output_engine/
    __init__.py
    models.py                 ← ReportSpec / TableSpec / ChartSpec TypedDicts
    csv_builder.py
    xlsx_builder.py           ← XlsxWriter with embedded charts
    md_builder.py
    html_builder.py
    pdf_builder.py            ← WeasyPrint HTML→PDF
    docx_builder.py           ← python-docx
    examples.py               ← sample ReportSpec data
    workspace_wiring.py       ← write outputs to workspace/<user>/outputs/

pipeline/
    __init__.py
    orchestrator.py           ← PipelineStep, run_pipeline, load_user_servers
    multi_server_runner.py    ← run_command_on_servers (SSH + Telnet)
    log_harvester.py          ← SSH log collection
    data_extractor.py         ← log parsing → structured records
    test_suite_runner.py      ← automated test run orchestration
    stress_orchestrator.py    ← load/stress test pipeline

vault_cli/
    __init__.py
    cli.py                    ← argparse entry point (aios-vault command)
    analyze.py                ← deep-analyze a single vault note
    diff.py                   ← show vault changes since a date
    insights.py               ← recurring theme extraction (Qdrant + vault)
    lessons.py                ← aggregate lessons from failure logs
    reorganize.py             ← propose/apply vault folder reorganization
    review.py                 ← review pending knowledge queue items
    summarize.py              ← summarize a vault folder

tests/                        ← mirrors source structure, one test file per module
    agent/ (scheduler/, test_*.py)
    dashboard/
    knowledge_analytics/
    learning_loop/
    mcp/
    memory/
    notification/
    output_engine/
    pipeline/
    security/
    vault_cli/
```

---

## 7. Inter-Module Dependency Rules

| Consumer | Direct Dependencies |
|---|---|
| `agent/main.py` | router, scheduler, session, mcp/shell_executor, pipeline, notification, learning_loop |
| `mcp/shell_executor` | agent/security (all), mcp/common |
| `mcp/ssh_executor` | agent/security (validator, sanitizer, audit, settings), mcp/common |
| `mcp/telnet_executor` | agent/security (validator, sanitizer, audit), mcp/common |
| `mcp/filesystem` | agent/security (scope_enforcer, audit_logger) |
| `pipeline/multi_server_runner` | mcp/ssh_executor, mcp/telnet_executor |
| `learning_loop/loop_runner` | outcome_evaluator, lesson_extractor, quality_gate, conflict_detector, insight_indexer, review_queue, context_accumulator |
| `learning_loop/conflict_detector` | memory/store, memory/qdrant_client, learning_loop/review_queue |
| `dashboard/orchestrator` | dashboard/builder, launcher, registry, tester |
| `knowledge_analytics/growth_reporter` | memory/qdrant_client |
| `vault_cli/*` | memory/*, learning_loop/review_queue, notification |

> **Security module imports are one-directional**: the security layer (`agent/security/`) does not import from any other application module, preventing circular dependency.

---

## 8. Configuration and Secrets

All secrets and environment-specific values are loaded exclusively from environment variables (no hardcoded credentials). See `agent/config/settings.py` and `.env.example`. The YAML allowlist at `configs/security/allowlist.yml` is the only file-based security configuration and has no sensitive data.

---

## 9. Workspace Isolation

Every user gets an isolated directory tree:

```
workspace/
└── {username}/
    ├── configs/
    │   └── servers.yml          ← user's server definitions
    ├── outputs/                 ← output files (CSV, Excel, PDF…)
    ├── task_history/            ← pipeline journals (JSON)
    └── notifications/           ← task completion markers
```

The scope enforcer (`agent/security/scope_enforcer.py`) validates all file operations against `workspace/{username}/` and `obsidian/vault/` as the only approved write roots.
