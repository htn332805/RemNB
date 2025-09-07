"""Sample Python scriptlet.

NAME: sample_step.py
DESCRIPTION: Demonstrates a minimal scriptlet writing to context.
USAGE:
  python orchestrator/scriptlets/python/steps/sample_step.py --params '{"message":"Hi"}'
PARAMS:
  message: str - message to record.
RETURNS:
  {"status":"ok","recorded_key":"sample.message_v1"}
LIMITATIONS:
  - Demo only.
EXAMPLE:
  python orchestrator/scriptlets/python/steps/sample_step.py --params '{"message":"Hello"}'
"""
from __future__ import annotations
import argparse, json
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources

logger = get_logger(__name__)

class SampleStep(BaseScriptlet):
  def validate(self, ctx: Context, params: dict) -> None:
    assert isinstance(params, dict), "params must be dict"
    assert "message" in params, "message required"

  @track_resources
  def run(self, ctx: Context, params: dict) -> dict:
    self.validate(ctx, params)
    msg = str(params["message"])
    key = "sample.message_v1"
    ctx.set(key, msg, who="sample_step")
    logger.info(f"Recorded message {msg}")
    return {"status": "ok", "recorded_key": key}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--params", required=True)
    a = parser.parse_args()
    params = json.loads(a.params)
    ctx = Context()
    out = SampleStep().run(ctx, params)
    print(json.dumps(out))
