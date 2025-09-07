"""Standalone runner to keep shared context server alive without launching Dash UI."""
import time  # noqa
import data.shared_context  # starts threads

print("Shared context server running on 127.0.0.1:8765. Press Ctrl+C to stop.")
try:
    while True:
        time.sleep(3600)
except KeyboardInterrupt:
    print("Exiting context server runner.")
