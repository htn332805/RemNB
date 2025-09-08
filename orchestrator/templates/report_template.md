# Automated Metrics Report

Generated: {{ generated_ts }}

## Summary (v1)
Count: {{ metrics.summary_v1.count }}
Mean: {{ metrics.summary_v1.mean }}
Min: {{ metrics.summary_v1.min }}
Max: {{ metrics.summary_v1.max }}

## Extended (v2 if present)
{% if metrics.summary_v2 %}
P50: {{ metrics.summary_v2.p50 }}
P90: {{ metrics.summary_v2.p90 }}
P99: {{ metrics.summary_v2.p99 }}
{% endif %}

## Aggregated
{{ metrics.aggregate_v1 | tojson }}

## Resources
{{ resource.audit_v1 | tojson }}

(End of Report)
