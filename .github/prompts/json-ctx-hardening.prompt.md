Task
- Review a scriptlet and enforce JSON-serializable ctx rules.

Steps
- Identify any non-serializable objects and convert to JSON-safe forms (ISO 8601 strings, base64, or persisted files in Data/ with manifest paths).
- Add json.dumps validation before ctx.set.
- Move large payloads to Data/ and store relative paths + checksums; keep ctx values small.
- Add who="scriptlet_name" and versioned key suffixes (_v1, _v2) for breaking changes.
