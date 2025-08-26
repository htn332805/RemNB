import pytest
from context import Context

def test_ctx_set_get():
    ctx = Context()
    ctx["foo"] = 123
    assert ctx["foo"] == 123

def test_ctx_json_serializable():
    ctx = Context()
    ctx["bar"] = {"a": [1, 2, 3]}
    assert ctx["bar"]["a"] == [1, 2, 3]

def test_ctx_non_serializable():
    ctx = Context()
    class NotSerializable:
        pass
    with pytest.raises(ValueError):
        ctx["bad"] = NotSerializable()