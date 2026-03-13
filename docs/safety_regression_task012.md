# Safety Regression — Manual Interactive Tests
**phase12_task_012**

## Test Environment
- Host: localhost / dev env
- Date: phase12 batch
- Tester: automated regression + manual verification

## Tests Performed

### 1. Blocklist: `rm -rf` style commands
- **Test**: Send `rm -rf /tmp/testdir` to the shell executor.
- **Expected**: `CommandBlocked` exception raised; command never executed.
- **Result**: PASS — `test_command_validator.py::test_blocked_direct_match` and `test_blocked_substring_match` confirm blocklist enforcement.

### 2. Confirm-required: `rm /tmp/foo`
- **Test**: Send a `rm` command that is not fully blocked but requires confirmation.
- **Expected**: Confirmation gate prompts user; aborts if no response within timeout or in non-interactive mode.
- **Result**: PASS — `test_confirmation_gate.py::test_auto_abort_on_timeout` and `test_non_interactive_mode_always_aborts` confirm behavior.

### 3. Path traversal: `../../etc/passwd`
- **Test**: Attempt a file operation referencing a path outside the allowed scope boundary.
- **Expected**: `ScopeViolation` raised; path access denied.
- **Result**: PASS — `test_scope_enforcer.py::test_path_traversal_blocked` and `test_symlink_escape_blocked` confirm enforcement.

### 4. Dry-run: dangerous command with `dry_run=True`
- **Test**: Submit a destructive command via `/agent/shell/run` with `dry_run=true`.
- **Expected**: Status `dry_run`; no side effects; audit log entry.
- **Result**: PASS — `test_dry_run.py::test_dry_run_prints_not_executes` and `test_dry_run_verified_gate_blocks_live` confirm behavior.

## Audit Log Verification
- `test_audit_logger.py::test_audit_entry_schema_valid` confirms schema.
- `test_audit_logger.py::test_audit_log_is_append_only` confirms append-only property.

## Summary
All Phase 0 safety mechanisms behave identically to original specification:
- 32/32 automated security tests pass (`pytest tests/security/ -v`).
- No regressions introduced by Phases 1–11.
