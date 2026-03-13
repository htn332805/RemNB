# Scenario 4 Results: Scheduled Automated Workflow with Learning Loop

**Date:** 2026-03-12  
**Status:** Validated  

## Commands / Prompts Used

```bash
# Create scheduled task
curl -X POST http://localhost:8080/agent/tasks/schedule \
  -H "Content-Type: application/json" \
  -d '{"command": "df -h", "interval_minutes": 5, "type": "shell"}'
```

## Agent Flow Observed

1. Scheduled task created and registered in scheduler
2. Scheduler triggered at next interval (5 min)
3. Agent executed `df -h`, saved output to `workspace/<user>/task_history/<task_id>.json`
4. Small Markdown report generated
5. Learning loop evaluated outcome via `evaluate_outcome` + `extract_lesson`
6. New lesson added to `workspace/<user>/review_queue.json`
7. Item visible via `aios-vault review`

## Artifacts

- `workspace/<user>/task_history/<task_id>.json`
- `workspace/<user>/outputs/<report>.md`
- `workspace/<user>/review_queue.json` (new entry appended)
- `obsidian/vault/Knowledge/PendingReview/<lesson_id>.json`

## Pass/Fail Criteria

- [x] Scheduler triggers at expected times
- [x] Reports are generated automatically
- [x] New lessons or review queue items appear after runs
