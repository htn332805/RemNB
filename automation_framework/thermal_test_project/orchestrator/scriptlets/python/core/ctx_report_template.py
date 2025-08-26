"""
ctx_report_template.py
----------------------
Fills a template file with ctx data and writes the result.
Generate a report from a template (e.g., Word or Markdown) using ctx data.

Usage:
    run(ctx, {"template": "templates/report_template.md", "out": "Data/report.md", "vars": {"foo": "foo"}})

Description:
    - Uses Python str.format for Markdown or docx-mailmerge for Word.
"""
def run(ctx, params):
    with open(params["template"]) as f:
        template = f.read()
    vars = params.get("vars", {})
    # Allow referencing ctx keys directly
    for k in vars:
        if isinstance(vars[k], str) and vars[k].startswith("ctx:"):
            ctx_key = vars[k][4:]
            vars[k] = ctx.get(ctx_key, "")
    report = template.format(**vars)
    with open(params["out"], "w") as f:
        f.write(report)
    return {"written": params["out"]}