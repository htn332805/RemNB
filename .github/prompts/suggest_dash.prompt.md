---
mode: 'agent'
description: 'Generate a clear code with examples'
---

Key goals:
- Encapsulate UI into reusable factories so the main layout becomes a simple composition of functions, not hard-coded dbc trees.
- Isolate callbacks per component to avoid cross-file coupling and to allow flexible redesign of interactions.
- Support dynamic instances via pattern-matching IDs, enabling additional modules without rewriting layout or callback bindings.
- Prepare for multi-page growth with a clean app structure so feature additions live in their own files.

Recommended folder structure:
- app.py: creates Dash app, theme, top-level layout callable, imports and registers blueprints/modules.
- data/api.py: data fetch and transforms..
- components/
    * cards.py: card factory for KPIs.
    * table.py: table factory and its callback(s).
    * navbar.py: navbar factory.
- pages/ or modules/: optional multi-page or feature modules (each with layout() and register_callbacks()).
- assets/: CSS, favicon, images for styling.

Dynamic layout via factories:
- Define small functions that return dbc components, parameterized by data and options. The main layout simply calls these factories, so new modules can be appended without editing existing code.
-   KPI card factory:
    * Input: title, total, delta, color
    * Output: dbc.Card([...]) ready for composition in rows/cols.
-   Table region factory:
    * Input: id_prefix
    * Output: Div with dropdown + DataTable wired by pattern-matching IDs; supports multiple instances.
- Using factories means app.layout = make_layout() rather than a hard-coded tree; the function can compute, fetch, or assemble children dynamically at runtime.
- Decouple callbacks with pattern-matching IDs
    * Dash pattern-matching callbacks (MATCH/ALL) allow callbacks to be written once and applied to any number of instances created by a factory. This prevents tight coupling between the main layout and specific IDs.
    * Use dict IDs like {"type":"country-dropdown","module":"country-table","index": n} for Dropdown and {"type":"country-table","module":"country-table","index": n} for DataTable.
    * Write one callback with Input({"type":"country-dropdown","module":MATCH,"index":MATCH}, "value") → Output({"type":"country-table","module":MATCH,"index":MATCH}, "data").
    * New table modules can be added by calling the factory again with a new index, with no changes to the callback.

setting the host ip to 192.168.1.102
