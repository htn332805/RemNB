# Test Framework Architecture and Development Guidelines

## Core Principles
1. Build modular, reusable test components that follow single responsibility principle
2. Maintain backward compatibility for all modifications
3. Use standardized interfaces for consistent integration
4. Implement comprehensive logging and error handling
5. Support parallel execution and resource optimization

## Framework Structure

### Directory Layout
```
~/automation_framework/
├── orchestrator/
│   ├── runner.py         # Main pipeline executor
│   ├── context.py        # Shared context management
│   ├── Data/             # Persistent data storage
│   ├── Logs/            # Execution logs
│   ├── scriptlets/      # Test components
│   │   ├── python/
│   │   │   ├── core/    # Shared utilities
│   │   │   ├── steps/   # Test implementations
│   │   │   └── apps/    # Dash applications
│   │   └── shell/       # Shell scripts
│   ├── recipes/         # YAML test definitions
│   └── templates/       # Report/output templates
```

## Development Requirements

### Scriptlet Design
- Independent CLI execution capability
- Comprehensive documentation with examples
- Single responsibility focus
- JSON-serializable context data only
- Standardized interfaces

### Context Management
- Type-checked and versioned modifications
- Complete change history tracking
- Validation and event hooks
- Serializable data only

### Quality Standards
- Unit tests for all components
- Integration tests for workflows
- Standard logging implementation
- Resource usage monitoring
- Error handling patterns

### Performance Optimization
- Support for parallel execution
- Resource-aware processing
- Caching mechanisms
- Async operations where applicable

## Integration Guidelines
1. Use BaseScriptlet class for consistency
2. Implement standardized logging
3. Follow naming conventions
4. Use wrapper patterns for modifications
5. Maintain backward compatibility

## Usage Instructions
```bash
# Basic execution
python runner.py

# With specific recipe
python runner.py --recipe recipes/test_mem.yaml

# Execution control
python runner.py --only STEP1 STEP2
python runner.py --skip STEP_NAME
python runner.py --resume-from STEP_NAME
```

For additional details, refer to the test framework documentation and example implementations in the `tests/` directory.