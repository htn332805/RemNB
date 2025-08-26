from scriptlets.python.steps import ctx_init

def test_ctx_init_run():
    ctx = {}
    result = ctx_init.run(ctx, {"keys": ["foo", "bar"]})
    assert "foo" in ctx and "bar" in ctx
    assert ctx["foo"] == []
    assert result["initialized"] == ["foo", "bar"]