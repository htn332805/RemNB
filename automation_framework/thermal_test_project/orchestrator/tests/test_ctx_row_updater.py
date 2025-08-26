from scriptlets.python.steps import ctx_row_updater
import time

def test_ctx_row_updater_run():
    ctx = {"foo": []}
    updater = ctx_row_updater.run(ctx, {"key": "foo", "n": 2, "interval_sec": 0.1})
    time.sleep(0.3)
    assert len(ctx["foo"]) >= 2