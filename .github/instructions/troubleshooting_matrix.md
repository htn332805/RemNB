# Copilot Prompt: Troubleshooting Matrix for Automation Orchestrator

## SYSTEM CONTEXT
If you encounter common errors while building or running scriptlets, recipes, or the runner, use this matrix to quickly narrow down root causes and remedies.

| Symptom                                     | Likely Cause                          | Resolution / Action                |
|----------------------------------------------|---------------------------------------|------------------------------------|
| Missing ctx key in output                    | Step failed silently                  | Check stderr logs, confirm scriptlet emits JSON to stdout, fix logic |
| `ModuleNotFoundError` or ImportError         | Wrong `module` path or missing file   | Verify file path, dotted path, and PYTHONPATH setup                 |
| Step re-runs even though cache should skip   | `cache_key` missing or marker deleted | Add correct `cache_key`, confirm `.cache/` content                   |
| Parallel output/data corrupt                 | Non-idempotent file write, race cond. | Serialize output or write to separate files                          |
| Step/process hangs indefinitely              | Infinite loop or blocking call        | Add timeout/guard, check logic, use @track_resources                 |
| TypeError: not JSON serializable             | Non-serializable type in context      | Convert to JSON-safe type before ctx.set                              |
| Report/file output missing sections          | Dependency not met                    | Ensure all producing steps executed before reporting                  |
| Empty recipe output/context                  | Upstream error or file not found      | Check for missing input files/data, rerun step or edit recipe         |
| Connection refused (context server steps)    | Server not started on port            | Confirm process launch, port, increase probe timeout if needed        |

## INSTRUCTION
- Find your symptom, review likely cause, and apply the suggested fix.
- For unresolved issues, check stderr output or rerun with extra logging enabled.
