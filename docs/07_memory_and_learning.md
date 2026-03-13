# AIOS — Memory Layer, Learning Loop & Knowledge Analytics

**Generated:** 2026-03-12

---

## 1. Overview

The memory and learning subsystems form the **intelligence feedback loop** of AIOS:

1. **Memory layer** (`memory/`) stores and retrieves knowledge as semantic vectors in Qdrant plus markdown in Obsidian
2. **Learning loop** (`learning_loop/`) evaluates task outcomes, extracts structured lessons, detects conflicts, and indexes validated knowledge
3. **Knowledge analytics** (`knowledge_analytics/`) tracks growth and health of the knowledge base over time
4. **Vault CLI** (`vault_cli/`) provides human-in-the-loop operations on the knowledge vault

---

## 2. Memory Layer (`memory/`)

### 2.1 Qdrant Client & Collections (`memory/qdrant_client.py`, `memory/collections.py`)

**Connection factory:**
```python
def get_qdrant_client() -> QdrantClient:
    return QdrantClient(host=QDRANT_HOST, port=QDRANT_PORT)  # env-driven
```

**Six predefined collections:**

| Collection | Constant | Purpose |
|---|---|---|
| `reports` | `COLLECTION_REPORTS` | Generated pipeline/test reports |
| `test_plans` | `COLLECTION_TEST_PLANS` | Test procedures and plans |
| `logs` | `COLLECTION_LOGS` | Harvested system/service logs |
| `knowledge` | `COLLECTION_KNOWLEDGE` | Validated lessons, best practices, observations |
| `cmd_history` | `COLLECTION_CMD_HISTORY` | Command execution history |
| `scripts` | `COLLECTION_SCRIPTS` | Scripts and automation procedures |

**Initialization:**
```python
init_all_collections(vector_size=768)
```
Creates all six collections if not already present, using cosine distance similarity.

---

### 2.2 Embedder (`memory/embedder.py`)

Converts text to vectors using the `nomic-embed-text` model via Ollama's HTTP API:

```python
EMBED_MODEL = "nomic-embed-text"

def _ollama_embed(text: str) -> List[float]:
    # POST http://localhost:11434/api/embeddings
    # Returns: data["embedding"] → List[float] of length 768
```

`embed_texts(texts: List[str]) → List[List[float]]`
- Sequential calls (one per text)
- 30-second HTTP timeout per call
- Returns a list of uniform-length float vectors

---

### 2.3 Document Store (`memory/store.py`)

**`upsert_documents(collection, payloads)`**

1. Extracts `payload["content"]` from each item for embedding
2. Generates embeddings via `embed_texts`
3. Assigns a new UUID to each point (always insert, never overwrite)
4. Calls `client.upsert(collection_name, points)` to Qdrant

**`semantic_search(collection, query, top_k=5)`**

1. Embeds the query string
2. Calls `client.search(collection, query_vector, limit=top_k)`
3. Returns `[{score, id, payload}]` sorted by score descending

---

### 2.4 Context Budget Manager (`memory/context_budget.py`)

Implements a **slot-based token budget** for assembling LLM context windows.

**Context Slots (priority order A → F):**

| Slot | Priority | Content Type |
|---|---|---|
| `A_current_task` | 1 (highest) | Current task input / spec |
| `B_recent_runs` | 2 | Recent related pipeline runs |
| `C_best_practices` | 3 | Best practice knowledge entries |
| `D_error_patterns` | 4 | Known error patterns / failure modes |
| `E_scripts` | 5 | Scripts and procedures |
| `F_misc` | 6 (lowest) | Miscellaneous low-priority items |

**Selection algorithm (`select_context_slots`):**
1. Sort candidates by (slot_rank ASC, score DESC)
2. Accumulate items until next item would exceed `max_tokens` (default: 3000)
3. Token estimation: `len(text.split()) × 1.3`

---

### 2.5 Context Provider (`memory/context_provider.py`)

Assembles a complete multi-slot context for LLM calls:
1. Queries Qdrant for recent runs, best practices, error patterns, scripts
2. Assigns each result to the appropriate slot
3. Calls `select_context_slots` to fit within budget
4. Returns formatted text for injection into LLM system/user prompt

---

### 2.6 Compressor (`memory/compressor.py`)

Handles large context windows that exceed token limits:
1. Detects if raw context is too long
2. Calls local LLM to produce a compressed summary
3. Returns the summary as a replacement context fragment

---

### 2.7 Alignment Checker (`memory/alignment_checker.py`)

Verifies that a produced output (lesson, report) is consistent with stored best practices:
1. Retrieves relevant best-practice entries from Qdrant
2. Asks LLM to judge alignment on a 0.0–1.0 scale
3. Returns alignment score used in TQS computation (`DIM_ALIGNMENT`)

---

### 2.8 Vault Watcher (`memory/vault_watcher.py`)

A `watchdog.FileSystemEventHandler` subclass that:
- Monitors the Obsidian vault directory for file changes
- On new/modified `.md` files: reads, embeds, upserts into Qdrant `knowledge` collection
- Enables continuous passive knowledge ingestion as vault notes are edited

---

### 2.9 Schemas (`memory/schemas.py`)

Defines constant keys for Qdrant payload fields (e.g., `"content"`, `"category"`, `"created_at"`, `"tags"`, `"status"`) to ensure consistent payload structure across all upsert operations.

---

## 3. Learning Loop (`learning_loop/`)

### 3.1 Entry Point (`learning_loop/loop_runner.py`)

`run_learning_loop(run_context)` is triggered after every interactive task:

```python
run_context = {
    "task_id":         str,
    "username":        str,
    "intent":          str,
    "exit_codes":      List[int],
    "criteria_results": List[str],  # "pass"/"fail"/"warn"
    "alignment_score": float,
}
```

**6-stage pipeline:**

```
evaluate_outcome(run_context)
    ↓
extract_lesson(run_context, tqs_result)
    ↓ None → discarded
classify_confidence(lesson) → tier
    ↓ "discard" → stop
    ↓ "pending_review"/"provisional" → add_to_review_queue
    ↓ "validated"
detect_conflicts(lesson)
    ↓ blocked → queue for review
    ↓ not blocked
index_lesson(lesson)  ← upsert to Qdrant "knowledge"
update_accumulator(username, ...)
```

Returns `{tqs, label, tier, action}`.

---

### 3.2 Outcome Evaluator (`learning_loop/outcome_evaluator.py`)

Computes a Task Quality Score (TQS) from three dimensions:

| Dimension | Weight | Source |
|---|---|---|
| `functional` | 50% | Exit code pass-rate: `passing_codes / total_codes` |
| `criteria` | 30% | Criteria pass-rate from `run_context["criteria_results"]` |
| `alignment` | 20% | Alignment checker score from `run_context["alignment_score"]` |

**TQS Formula:**
```
TQS = 0.5 × functional + 0.3 × criteria + 0.2 × alignment
```

**Labels:**
| TQS Range | Label |
|---|---|
| ≥ 0.7 | `pass` |
| 0.4 – 0.69 | `warn` |
| < 0.4 | `fail` |

---

### 3.3 Lesson Extractor (`learning_loop/lesson_extractor.py`)

Uses the Anthropic LLM (via Ollama) to extract a structured lesson from the run context:

**Lesson Schema:**
```json
{
  "lesson_type": "success|failure|warning",
  "category":    "best_practice|error_pattern|procedure|observation",
  "title":       "<short title>",
  "content":     "<detailed description>",
  "tags":        ["tag1", "tag2"],
  "confidence":  0.0-1.0
}
```

**Process:**
1. Build prompt including `task_id`, `intent`, TQS score, and recent errors
2. Call LLM at **temperature 0.1** (low for deterministic JSON)
3. Parse JSON response
4. On parse failure: retry once with explicit JSON reminder prompt
5. On second failure: add to review queue and return `None`

---

### 3.4 Quality Gate (`learning_loop/quality_gate.py`)

Classifies a lesson by its `confidence` field:

| Confidence | Tier |
|---|---|
| < 0.3 | `discard` — drop silently |
| 0.3 – 0.49 | `pending_review` — human review required |
| 0.5 – 0.7 | `provisional` — queue for review |
| > 0.7 | `validated` — proceed to conflict detection |

---

### 3.5 Conflict Detector (`learning_loop/conflict_detector.py`)

**6-stage pipeline** for detecting contradictions with existing knowledge:

| Stage | Task | Description |
|---|---|---|
| 1 | Candidate retrieval | Semantic search (top-10) filtered by tag overlap |
| 2 | Similarity thresholding | score > 0.92 → `near_duplicate`; score 0.70–0.92 → `conflict_candidate` |
| 3 | Scope boundary analysis | Different environments/OSes/services → may not conflict |
| 4 | Temporal aging check | Entries older than N days → lower conflict weight |
| 5 | LLM contradiction judgment | LLM decides if two statements contradict each other |
| 6 | Resolution routing | `near_duplicate` → skip index; `conflict_candidate` → review queue; no conflict → allow |

Returns `{blocked: bool, conflicts: List, reason: str}`.

Conflict entries are persisted as markdown files in `obsidian/vault/Knowledge/Conflicts/`.

---

### 3.6 Insight Indexer (`learning_loop/insight_indexer.py`)

`index_lesson(lesson)` — final step for validated lessons with no blocking conflicts:
1. Adds metadata: `created_at`, `status="validated"`, `username`
2. Calls `memory/store.upsert_documents("knowledge", [lesson])`

---

### 3.7 Review Queue (`learning_loop/review_queue.py`)

Maintains a JSONL file for items awaiting human review:
```
obsidian/vault/Knowledge/ReviewQueue/pending.jsonl
```

`add_to_review_queue(username, lesson, priority)` — appends:
```json
{
  "timestamp": "...",
  "username": "tester",
  "priority": 3,
  "lesson": { ... }
}
```

---

### 3.8 Context Accumulator (`learning_loop/context_accumulator.py`)

Per-user rolling log of learning loop events:
```
workspace/{username}/learning_accumulator.jsonl
```

Entries like: `[indexed] task=shell_a1b2 tqs=0.85`

Used by `vault_cli/insights.py` to surface recent activity patterns.

---

### 3.9 Error Pattern Seeder (`learning_loop/error_pattern_seeder.py`)

Seeds the Qdrant `knowledge` collection with an initial set of known error patterns before any real runs occur. This jump-starts the conflict detection system so it can recognize common errors from day one.

---

### 3.10 Escalation (`learning_loop/escalation.py`)

Routes high-priority conflicts or failures to notification channels (desktop notification, failure log). Called when conflict detection finds a critical contradiction with existing validated knowledge.

---

## 4. Knowledge Analytics (`knowledge_analytics/growth_reporter.py`)

### Growth Reporter

`generate_growth_report(since_date)` — returns per-collection entry counts for entries created since `since_date`:

```python
{
    "since":      "2026-03-01T00:00:00",
    "reports":    12,
    "knowledge":  47,
    "test_plans": 8,
    "logs":       31,
    "dashboards": 3,
    "total":      98
}
```

Uses Qdrant `scroll` API with Python-side `created_at` filtering (ISO string comparison).

`_count_dashboard_knowledge_entries(since_date)` — specialized counter for knowledge entries with `category='dashboard'`.

---

## 5. Vault CLI (`vault_cli/`)

**Entry point:** `vault_cli/cli.py` — installed as `aios-vault` command

### Subcommands

| Subcommand | Module | Purpose |
|---|---|---|
| `review` | `review.py` | Print and optionally approve pending review queue items |
| `insights` | `insights.py` | Extract and summarize recurring themes from Qdrant + vault files |
| `lessons` | `lessons.py` | Aggregate lessons from failure logs + task histories |
| `summarize <folder>` | `summarize.py` | Summarize all `.md` files in a vault subfolder via LLM |
| `reorganize [--apply]` | `reorganize.py` | Propose (or apply) vault folder restructuring |
| `analyze <file>` | `analyze.py` | Deep-analyze a single vault note for themes, quality, gaps |
| `diff --since <date>` | `diff.py` | Show vault file changes since an ISO date |

### `insights` Workflow

1. Fetch Qdrant `knowledge` entries modified in the last N days
2. Fetch vault markdown files from `LessonsLearned/` and `BestPractices/`
3. Combine all text content
4. Call Ollama LLM (`llama3.2:1b`) to identify 3–5 recurring themes
5. Print formatted insight report to stdout

### Data Flow: Knowledge Creation

```
Task execution
    ↓
learning_loop.run_learning_loop()
    ↓ validated lesson
insight_indexer.index_lesson()
    ↓
Qdrant "knowledge" collection
    ↓
vault_cli insights / aios-vault insights
    ↓ human-readable summary
stdout / Obsidian vault note
```

---

## 6. Qdrant Payload Schema (Knowledge Collection)

Standard payload for the `knowledge` collection:

| Field | Type | Description |
|---|---|---|
| `content` | str | Full lesson text (used for embedding) |
| `lesson_type` | str | `success` / `failure` / `warning` |
| `category` | str | `best_practice` / `error_pattern` / `procedure` / `observation` / `dashboard` |
| `title` | str | Short descriptive title |
| `tags` | List[str] | Topic tags for filtering |
| `confidence` | float | 0.0–1.0 confidence score |
| `status` | str | `validated` / `provisional` / `pending_review` |
| `username` | str | Originating user |
| `source_task_id` | str | Task ID that produced this lesson |
| `created_at` | str | ISO 8601 timestamp |

---

## 7. Integration with Other Modules

| Integration | Direction | Description |
|---|---|---|
| `agent/main.py` → `learning_loop` | After every shell/remote execution | Learning loop triggered post-task |
| `learning_loop` → `memory/store` | Index validated lessons | Upsert to Qdrant |
| `memory/vault_watcher` → `memory/store` | On vault file change | Auto-index updated vault notes |
| `dashboard/catalog_writer` → `memory/store` | After dashboard launch | Index dashboard metadata in knowledge collection |
| `vault_cli/insights` → Qdrant + vault | On demand | Read-only insight generation |
| `knowledge_analytics` → `memory/qdrant_client` | On demand | Count entries for growth reporting |
