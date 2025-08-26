import subprocess

def test_hello_dash_cli_help():
    result = subprocess.run(
        ["python", "scriptlets/python/apps/hello_dash.py", "--help"],
        capture_output=True, text=True
    )
    assert "usage" in result.stdout.lower()