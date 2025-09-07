Goal
- Create a single-responsibility Python scriptlet at orchestrator/scriptlets/python/steps/<snake_name>.py with BaseScriptlet, validate/run, CLI, and strict JSON-serializable ctx writes.

Inputs
- params JSON schema: <define>
- outputs (ctx keys): <define>
- success: files_exist and/or ctx_has_keys

Expectations
- Use ctx.set(key, value, who="<step_name>"); verify json-serializability before setting.
- Print only final JSON to stdout; send logs to stderr/Logger with resource/timing decorator.
- Include docstring with usage, description, limitations, and examples.
- Do not modify existing helpers; add wrappers or versioned modules to preserve backward compatibility.
