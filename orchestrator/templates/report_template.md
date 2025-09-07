### AUTOGEN(rebase_workspace.sh)
# Automated Metrics Report

Generated: {{ generated_ts }}

## Summary (v1)
Count: {{ metrics.summary_v1.count }}
Mean: {{ metrics.summary_v1.mean }}
Min: {{ metrics.summary_v1.min }}
Max: {{ metrics.summary_v1.max }}

{% if metrics.summary_v2 %}
## Extended (v2)
p50: {{ metrics.summary_v2.p50 }}
{% endif %}
