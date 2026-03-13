# AIOS — Test Suite & Code Quality

**Generated:** 2026-03-12

---

## 1. Overview

**Test location:** `tests/`  
**Framework:** pytest 8.3.2 + pytest-asyncio 0.23.8  
**Total test files:** 74  
**Total test functions:** 375+ (across all modules)  
**Total test code:** ~5,618 lines

The test suite **mirrors the source module hierarchy** exactly — every source package has a corresponding test package:

```
tests/
├── agent/
│   ├── scheduler/ (4 test files)
│   └── test_*.py  (8 test files)
├── dashboard/     (7 test files)
├── knowledge_analytics/ (1 test file)
├── learning_loop/ (8 test files)
├── mcp/           (6 test files)
├── memory/        (9 test files)
├── notification/  (3 test files)
├── output_engine/ (7 test files)
├── pipeline/      (5 test files)
├── security/      (8 test files)
└── vault_cli/     (8 test files)
```

---

## 2. Test Inventory by Module

### 2.1 Agent Tests (`tests/agent/`)

| File | What It Tests |
|---|---|
| `test_intent_api.py` | `/agent/intent/classify` endpoint, LLM confirmation flow, low-confidence escalation |
| `test_pattern_matcher.py` | All 8 intent labels, edge cases (empty query, all-caps, mixed keywords) |
| `test_llm_confirm.py` | LLM call, JSON parse success/failure, fallback behavior |
| `test_session_manager.py` | Workspace directory creation, idempotency |
| `test_dashboard_endpoint.py` | Dashboard generation endpoint integration |
| `test_openwebui_integration.py` | OpenWebUI tool call wrapper |
| `test_sse_endpoint.py` | SSE stream connection, event delivery, heartbeat, disconnection |
| `test_global_error_handler.py` | 500 response shape for unhandled exceptions |

### 2.2 Scheduler Tests (`tests/agent/scheduler/`)

| File | What It Tests |
|---|---|
| `test_service.py` | Singleton behavior, start/stop, idempotent start |
| `test_registry.py` | add/get/list/delete/update operations, ID generation |
| `test_models.py` | Pydantic validation of ScheduleTaskRequest/Response |
| `test_api_integration.py` | Full CRUD REST cycle via TestClient |
| `test_executor.py` | Job execution routing by intent |

### 2.3 Security Tests (`tests/security/`)

| File | Key Assertions |
|---|---|
| `test_command_validator.py` | Blocked patterns, allowed commands, inject chains (`;`, `&&`, `\|\|`, `$()`, backticks) |
| `test_output_sanitizer.py` | All 14 dangerous regex patterns, clean inputs return SAFE |
| `test_scope_enforcer.py` | Path traversal (`../../etc/passwd`), workspace boundary checks, approved read paths |
| `test_audit_logger.py` | Entry format, file rotation by date, directory auto-creation |
| `test_confirmation_gate.py` | Interactive y/n prompt, non-interactive auto-reject |
| `test_dry_run.py` | Dry-run mode activation, no subprocess spawned |
| `test_timeout.py` | Timeout guard raises TimeoutError, process tree killed |
| `test_resource_monitor.py` | CPU/memory/files breach detection, abort callback invocation |

### 2.4 MCP Tests (`tests/mcp/`)

| File | Key Assertions |
|---|---|
| `test_shell_executor.py` | execute_local happy path, blocked command, dry-run, timeout |
| `test_ssh_executor.py` | Paramiko mocked: success, retry on connection error, blocked command |
| `test_telnet_executor.py` | Telnet flow mocked: connect, send, receive, disconnect |
| `test_filesystem.py` | read_file scope enforcement, write_file workspace isolation |
| `test_obsidian_sync.py` | Stub behavior (sync_to_vault returns src_path) |
| `test_qdrant_client.py` | Client construction, collection constant values |

### 2.5 Memory Tests (`tests/memory/`)

| File | Key Assertions |
|---|---|
| `test_store.py` | upsert_documents calls qdrant.upsert, semantic_search roundtrip with mock |
| `test_embedder.py` | embed_texts calls Ollama API, vector shape |
| `test_collections.py` | init_all_collections creates missing collections, skips existing |
| `test_context_budget.py` | Slot priority ordering (A before F), token budget enforcement |
| `test_context_provider.py` | Multi-slot context assembly |
| `test_compressor.py` | Compression triggered when tokens exceed budget |
| `test_alignment_checker.py` | LLM alignment score extraction |
| `test_search.py` | semantic_search parameter forwarding |
| `test_vault_watcher.py` | FileSystemEventHandler on_modified triggers upsert |

### 2.6 Learning Loop Tests (`tests/learning_loop/`)

| File | Key Assertions |
|---|---|
| `test_loop_runner.py` | Full pipeline: evaluate→extract→classify→detect conflicts→index |
| `test_outcome_evaluator.py` | TQS formula correctness, all three label thresholds |
| `test_lesson_extractor.py` | LLM call, JSON parse, retry on parse failure, review queue on failure |
| `test_quality_indexer_queue.py` | All four confidence tiers (discard/pending/provisional/validated) |
| `test_context_accumulator.py` | Append events, read history, per-user isolation |
| `test_escalation_and_conflicts.py` | 6-stage conflict pipeline, near-duplicate detection |
| `test_error_pattern_seeder.py` | Seed produces correct payload format |
| `test_vault_cli_review.py` | Review queue load, approve/reject flow |

### 2.7 Pipeline Tests (`tests/pipeline/`)

| File | Key Assertions |
|---|---|
| `test_orchestrator.py` | run_pipeline sequential execution, error stop, journal write |
| `test_multi_server_runner.py` | SSH + Telnet fanout, per-server result shape, dry-run propagation |
| `test_log_harvester.py` | SSH log fetch, file path collection |
| `test_data_extractor.py` | Log parsing → structured record extraction |
| `test_stress_and_suite.py` | Stress test and test suite runner behaviors |

### 2.8 Output Engine Tests (`tests/output_engine/`)

| File | Key Assertions |
|---|---|
| `test_csv_builder.py` | CSV row/column correctness, file creation |
| `test_xlsx_builder.py` | Sheet creation, data rows, embedded chart spec |
| `test_md_builder.py` | GFM table format, header row |
| `test_html_builder.py` | HTML table tags, column headings, styling |
| `test_pdf_builder.py` | PDF bytes output (via WeasyPrint), file size > 0 |
| `test_docx_builder.py` | python-docx table structure |
| `test_workspace_wiring.py` | Output path construction, directory auto-creation |

### 2.9 Dashboard Tests (`tests/dashboard/`)

| File | Key Assertions |
|---|---|
| `test_builder.py` | Code generation from spec (data section, layout section, all chart types) |
| `test_registry.py` | Register, version increment, load/save JSON |
| `test_launcher.py` | Subprocess spawn, PID returned |
| `test_tester.py` | Health check with HTTP mock |
| `test_orchestrator.py` | End-to-end lifecycle with all steps mocked |
| `test_improver.py` | LLM improvement loop |
| `test_catalog_writer.py` | Qdrant upsert called with category='dashboard' |

### 2.10 Notification Tests (`tests/notification/`)

| File | Key Assertions |
|---|---|
| `test_notifier.py` | notify2 mocked, urgency level mapping |
| `test_markers_and_logs.py` | Marker file written, failure JSONL appended |
| `test_integration.py` | End-to-end: shell run → marker + notification |

### 2.11 Knowledge Analytics Tests (`tests/knowledge_analytics/`)

| File | Key Assertions |
|---|---|
| `test_growth_reporter.py` | count_entries_since date filtering, generate_growth_report totals |

### 2.12 Vault CLI Tests (`tests/vault_cli/`)

| File | Key Assertions |
|---|---|
| `test_review_cli.py` | argparse dispatch to review.run_review() |
| `test_insights.py` | Qdrant scroll + vault file collection, LLM call, output format |
| `test_lessons.py` | Failure log aggregation, lesson formatting |
| `test_summarize.py` | Vault folder scan, LLM summarize call |
| `test_reorganize.py` | Dry-run proposal vs. apply mode |
| `test_analyze.py` | Single note analysis, theme extraction |
| `test_diff.py` | Date-filtered vault file change detection |
| `test_cli_integration.py` | Full CLI: all subcommands via argparse, help text |

---

## 3. Test Patterns and Conventions

### 3.1 Unit Test Pattern (Mock Everything External)

```python
from unittest.mock import patch, MagicMock

def test_upsert_documents_calls_qdrant_upsert():
    mock_client = MagicMock()
    
    with patch("memory.store.get_qdrant_client", return_value=mock_client), \
         patch("memory.store.embed_texts", side_effect=_fake_embed):
        upsert_documents("reports", [{"content": "hello world"}])
    
    mock_client.upsert.assert_called_once()
```

All I/O dependencies (Qdrant, Ollama, Paramiko, OS) are mocked. Tests are fast and offline-runnable.

### 3.2 Integration Test Pattern (TestClient)

```python
from fastapi.testclient import TestClient
from agent.main import app

client = TestClient(app)

def test_intent_classify_shell():
    resp = client.post("/agent/intent/classify",
                       json={"username": "test", "query": "run df -h"})
    assert resp.status_code == 200
    assert resp.json()["intent"] == "shell_cmd"
```

FastAPI's `TestClient` runs the full middleware stack synchronously.

### 3.3 LLM mock pattern

LLM calls are always mocked to avoid network dependency in tests:

```python
def test_intent_classify_triggers_llm():
    mock_response = '{"intent": "shell_cmd", "confidence": 0.75, "reason": "..."}'
    with patch("agent.router.llm_confirm._call_llm", return_value=mock_response):
        resp = client.post(...)
    assert resp.json()["intent"] == "shell_cmd"
```

### 3.4 Scheduler Cleanup Pattern

```python
def test_start_scheduler_singleton():
    import agent.scheduler.service as svc
    svc.scheduler = None  # Reset singleton before test
    
    sched1 = start_scheduler()
    sched2 = start_scheduler()
    assert sched1 is sched2
    
    sched1.shutdown(wait=False)  # ← Always clean up background threads
    svc.scheduler = None
```

---

## 4. Running the Test Suite

```bash
# Run all tests
cd /home/tester/aios
pytest

# Run with verbose output
pytest -v

# Run a specific module
pytest tests/security/

# Run a specific test
pytest tests/security/test_command_validator.py::test_blocked_direct_match

# Run with coverage (if pytest-cov installed)
pytest --cov=agent --cov=mcp --cov=memory --cov=learning_loop

# Run async tests (pytest-asyncio required)
pytest tests/agent/test_sse_endpoint.py
```

---

## 5. Test Configuration

No `pytest.ini` or `conftest.py` found at root level. Tests rely on:
- Standard pytest discovery (files named `test_*.py`)
- Module imports via editable install (`pip install -e .`)
- `pytest-asyncio` auto mode for async test functions

---

## 6. Phase-Level Verification Scripts

In addition to the pytest suite, the repo contains manual verification scripts at the root:

| Script | Purpose |
|---|---|
| `verify_038.py` | Verify Phase 0 task 038 implementation |
| `verify_039.py` | Verify Phase 0 task 039 implementation |
| `verify_038_043.py` | Verify tasks 038–043 as a batch |
| `verify_053_062.py` | Verify tasks 053–062 as a batch |
| `verify_063_072.py` | Verify tasks 063–072 as a batch |

These scripts are run during development to validate correct implementation before committing, and produce output to `phase0_test_results.txt`.

---

## 7. Scenario Test Results

The `docs/` directory contains documented test scenario results:

| File | Content |
|---|---|
| `docs/scenario_1_results.md` | Results of test scenario 1 (Phase 0 safety validation) |
| `docs/scenario_2_results.md` | Results of test scenario 2 (multi-server pipeline) |
| `docs/scenario_3_results.md` | Results of test scenario 3 (report generation) |
| `docs/scenario_4_results.md` | Results of test scenario 4 (dashboard creation) |
| `docs/scenarios.txt` | Scenario definitions and pass/fail summary |
| `docs/safety_regression_task012.md` | Safety regression test results for task 012 |
| `docs/safety_regression_task013.md` | Safety regression test results for task 013 |

---

## 8. Code Quality Observations

### Strengths
| Observation | Evidence |
|---|---|
| High test coverage breadth | 74 test files covering all 12 source modules |
| Clean mock patterns | All external I/O mocked; tests are offline-runnable |
| Security layer heavily tested | 8 dedicated security test files, all injection vectors covered |
| Happy path + error path coverage | Each module tests both success and failure branches |
| Integration tests present | FastAPI TestClient used for full API surface |
| Learning loop fully tested | Full pipeline mock (evaluate→extract→classify→index) |

### Areas for Improvement
| Finding | Recommendation |
|---|---|
| No `conftest.py` | Add shared fixtures (fake Qdrant client, temp workspace dirs) to reduce duplication |
| No coverage reporting in CI | Add `pytest-cov` to requirements; set coverage thresholds |
| Async tests (SSE/WS) | Ensure `asyncio_mode = "auto"` in pytest config; some async tests may need markers |
| Scheduler persistence not tested | No test for scheduler task loss on restart |
| SSH `AutoAddPolicy` not warned in tests | Security test could assert that known_hosts policy is warn-level |
| Telnet tests | Mock only basic connect flow; edge cases (login timeouts, banner variations) not covered |

---

## 9. Dependency on Externals in Tests

All tests mock external dependencies. No real I/O in `pytest`:

| External | Mock Strategy |
|---|---|
| Qdrant | `patch("memory.store.get_qdrant_client", return_value=MagicMock())` |
| Ollama (embeddings) | `patch("memory.store.embed_texts", side_effect=_fake_embed)` |
| Anthropic/Ollama (LLM) | `patch("agent.router.llm_confirm._call_llm", return_value="...")` |
| Paramiko (SSH) | `patch("paramiko.SSHClient")` |
| subprocess | `patch("subprocess.Popen")` |
| notify2 | `patch("notify2.Notification")` or env-var `AIOS_DESKTOP_NOTIFICATIONS=0` |
| psutil | `patch("psutil.Process")` |
| Filesystem | `tmp_path` pytest fixture or `patch("pathlib.Path.write_text")` |
