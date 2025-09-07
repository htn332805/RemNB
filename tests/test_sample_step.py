import json
from orchestrator.context import Context
from orchestrator.scriptlets.python.steps.sample_step import SampleStep

def test_sample_step():
    ctx = Context()
    out = SampleStep().run(ctx, {"message": "Hello"})
    assert out["status"] == "ok"
    assert ctx.get("sample.message_v1") == "Hello"
