# .github/prompts/new-python-step.prompt.md

Goal
- Create a new Python step at orchestrator/scriptlets/python/steps/<snake_name>.py that implements <ShortPurpose> as a BaseScriptlet with validate/run, CLI, and JSON-only ctx updates.

Context
- Inputs: <params JSON schema>
- Outputs: <ctx keys list>
- Success: <files_exist/ctx_has_keys>

Expectations
- Include full docstring (usage, description, limitations), line-by-line comments, logging, resource decorator, and tests under tests/.
- Do not modify existing helpers; add wrappers/adapters if needed to preserve backward compatibility.
