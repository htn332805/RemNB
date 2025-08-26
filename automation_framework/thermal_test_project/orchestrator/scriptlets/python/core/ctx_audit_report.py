"""
ctx_audit_report.py
-------------------
Generates a human-readable audit report from ctx["audit"].

Usage:
    run(ctx, {"out": "Data/audit_report.md"})

Description:
    - Writes a Markdown report summarizing all audited steps.
"""
def run(ctx, params):
    out = params["out"]
    audit = ctx.get("audit", [])
    lines = ["# Audit Report", ""]
    for entry in audit:
        lines.append(f"## Step: {entry['step']}")
        lines.append(f"- Start: {entry.get('start')}")
        lines.append(f"- End: {entry.get('end')}")
        lines.append(f"- Duration: {entry.get('duration'):.2f} sec")
        lines.append(f"- Status: {entry.get('status')}")
        if "error" in entry:
            lines.append(f"- Error: {entry['error']}")
        lines.append("")
    with open(out, "w") as f:
        f.write("\n".join(lines))
    return {"written": out}