# RemNB Repository Overview

## Repository Summary

RemNB is a comprehensive **Automation Orchestrator Framework** designed for structured, reproducible test automation. The framework follows a modular architecture using **recipes → runner → scriptlets → context → tests** workflow, enabling flexible automation pipelines for data processing, metrics computation, validation, and reporting.

## Architecture Overview

```
orchestrator/                    # Core framework directory
├── context.py                   # Thread-safe JSON state management
├── runner.py                    # Sequential recipe executor (v1) 
├── runner_v2.py                 # Enhanced executor with parallel/shell support
├── recipes/                     # YAML workflow definitions (15 recipes)
├── scriptlets/                  # Executable automation units
│   ├── python/                  # Python-based scriptlets
│   │   ├── core/                # Base classes and utilities
│   │   ├── steps/               # Individual processing steps (20 scriptlets)
│   │   └── apps/                # Standalone applications
│   └── shell/                   # Shell-based scriptlets
├── Data/                        # Input/output data files
└── templates/                   # Report and output templates

tests/                           # Comprehensive test suite (14 test files)
run_context_server.py           # Shared context server
*.sh scripts                    # Setup, simulation, and utility scripts
```

## Key Components

### 1. Context Management (`orchestrator/context.py`)
- **Thread-safe JSON state store** with history tracking
- **Versioned keys** (e.g., `numbers.stats_v1`) for backward compatibility
- **Traceability** via `who` attribution for each state change
- **JSON-serializable only** - no complex objects, handles, or DataFrames

### 2. Execution Runners
- **`runner.py`**: Sequential execution for basic workflows
- **`runner_v2.py`**: Advanced executor supporting:
  - Parallel execution for independent steps
  - Shell scriptlet integration
  - Dependency management (`depends_on`)
  - Process pools for CPU-intensive tasks
  - Resource timing metrics
  - Caching capabilities

### 3. Scriptlets (Automation Units)
- **Single responsibility** executable components
- **Standardized interface** via `BaseScriptlet`
- **JSON output contract** for success/error reporting
- **Two types**:
  - **Python scriptlets**: Complex data processing, calculations
  - **Shell scriptlets**: File manipulation, system operations

## File Inventory

### Core Python Files (28 files)

#### Framework Core
| File | Purpose | Key Features |
|------|---------|-------------|
| `orchestrator/context.py` | State management | Thread-safe, JSON-only, versioned keys |
| `orchestrator/runner.py` | Basic executor | Sequential YAML recipe execution |
| `orchestrator/runner_v2.py` | Advanced executor | Parallel, shell, dependencies, caching |
| `run_context_server.py` | Shared context server | TCP JSON protocol, multi-client support |

#### Python Scriptlet Core
| File | Purpose | Key Features |
|------|---------|-------------|
| `orchestrator/scriptlets/python/core/base.py` | Base scriptlet class | Contract definition, validation pattern |
| `orchestrator/scriptlets/python/core/logging_util.py` | Logging utilities | Standardized stderr logging |
| `orchestrator/scriptlets/python/core/resource.py` | Resource tracking | Performance timing decorator |

#### Data Processing Scriptlets (20 files)
| File | Purpose | Inputs | Outputs |
|------|---------|---------|---------|
| `compute_numbers.py` | Basic statistics | CSV file | `numbers.stats_v1` |
| `load_numbers.py` | CSV data loading | CSV file | `numbers.raw_v1` |
| `normalize_numbers.py` | Min-max normalization | `numbers.raw_v1` | `numbers.normalized_v1`, `numbers.stats_v1` |
| `compute_mean.py` | Mean calculation | `numbers.normalized_v1` | `metrics.mean_v1` |
| `compute_median.py` | Median calculation | `numbers.normalized_v1` | `metrics.median_v1` |
| `compute_std.py` | Standard deviation | `numbers.normalized_v1` | `metrics.std_v1` |
| `compute_metrics_v1.py` | Basic metrics suite | `numbers.raw_v1` | `metrics.summary_v1`, `metrics.columns_v1` |
| `compute_metrics_v2.py` | Extended metrics | `numbers.raw_v1` | `metrics.summary_v2` |
| `aggregate_metrics.py` | Metrics consolidation | Various `metrics.*` | `metrics.aggregated_v1` |
| `prime_factor_analysis.py` | Prime factorization | `numbers.raw_v1` | `numbers.prime_factors_v1` |
| `validate_numbers.py` | Data validation | `numbers.raw_v1` | Validation results |
| `cache_numbers.py` | Data caching | `numbers.raw_v1` | Cached data management |
| `convert_to_summary.py` | Summary generation | Various inputs | Summary reports |
| `generate_report.py` | Report generation | Template + data | Formatted reports |
| `cleanup_intermediate.py` | Cleanup operations | Various | Cleanup status |
| `resource_audit.py` | Resource monitoring | System state | Resource metrics |
| `demo_update_state.py` | State demonstration | Demo parameters | State changes |
| `inject_failure.py` | Failure injection | Test parameters | Controlled failures |
| `start_context_server_v2.py` | Context server launcher | Server config | Server status |

#### Applications
| File | Purpose | Key Features |
|------|---------|-------------|
| `orchestrator/scriptlets/python/apps/live_dashboard.py` | Live monitoring dashboard | Real-time data visualization |

### Shell Scripts (20 files)

#### Framework Shell Scripts
| File | Purpose | Key Features |
|------|---------|-------------|
| `orchestrator/scriptlets/shell/sanitize_csv.sh` | CSV cleaning | Removes blanks/comments |
| `orchestrator/scriptlets/shell/enrich_csv.sh` | CSV enhancement | Adds derived columns |

#### Setup and Management Scripts
| File | Purpose | Key Features |
|------|---------|-------------|
| `rebase_workspace.sh` | Framework bootstrapping | Idempotent setup, managed files |
| `setup_curriculum.sh` | Curriculum creation | Educational examples, sample data |
| `tmux_layout.sh` | Session management | Multi-pane development environment |
| `update_state.sh` | Context manipulation | CLI for shared state management |
| `simulate_cdu_poll.sh` | Data simulation | Generates sensor data streams |
| `test.sh` | Integration testing | End-to-end workflow validation |

### Test Files (14 files)
| File | Purpose | Coverage |
|------|---------|----------|
| `tests/test_example_numbers.py` | Basic integration | Core recipe execution |
| `tests/test_parallel.py` | Parallel execution | Concurrent processing |
| `tests/test_shell_integration.py` | Shell scriptlets | Mixed Python/shell workflows |
| `tests/test_validation.py` | Data validation | Error handling patterns |
| `tests/test_cpu_process.py` | Process pools | CPU-intensive tasks |
| `tests/test_resilience_controls.py` | Error resilience | Failure handling |
| `tests/test_artifacts_metrics.py` | Artifact generation | File outputs |
| `tests/test_full_pipeline.py` | Complete workflows | End-to-end scenarios |
| `tests/test_reporting_stub.py` | Reporting features | Output generation |
| `tests/test_dashboard_stub.py` | Dashboard features | UI components |
| `tests/test_ctx_foundation.py` | Context operations | State management |
| `tests/test_versioning_wrapper.py` | Version compatibility | Backward compatibility |
| `tests/test_parallel_metrics_recipe.py` | Parallel metrics | Complex parallel flows |

## Recipe Workflows (15 YAML files)

### Basic Workflows
- `example_numbers.yaml`: Simple statistics computation
- `01_basic_number.yaml`: Single-step number processing
- `02_basic_mixed.yaml`: Shell + Python integration

### Advanced Workflows
- `06_parallel_fan_in.yaml`: Parallel metric computation with aggregation
- `12_advanced_full.yaml`: Comprehensive multi-step pipeline
- `04_artifacts_pipeline.yaml`: File artifact generation

### Specialized Workflows
- `03_validation_errors.yaml`: Error handling demonstration
- `07_cpu_process.yaml`: Process pool utilization
- `11_resilience_controls.yaml`: Failure recovery patterns
- `09_dashboard.yaml`: Live dashboard integration
- `10_reporting.yaml`: Report generation workflows

## Key Features

### 1. Modularity and Extensibility
- **Pluggable architecture**: Easy to add new scriptlets
- **Standardized interfaces**: Consistent patterns across components
- **Version compatibility**: Backward-compatible changes via versioning

### 2. Robustness and Reliability
- **Structured error handling**: JSON error contracts
- **Resource tracking**: Performance monitoring
- **State validation**: JSON-safe guarantees
- **Test coverage**: Comprehensive test suite

### 3. Flexibility
- **Multiple execution modes**: Sequential and parallel
- **Language support**: Python and shell scriptlets
- **Dependency management**: Complex workflow orchestration
- **Caching support**: Performance optimization

### 4. Real-world Integration
- **TCP context server**: Multi-client shared state
- **File artifact management**: Persistent outputs
- **Template system**: Customizable reports
- **Simulation capabilities**: Data generation for testing

## Usage Patterns

### 1. Data Processing Pipelines
```bash
python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml
```

### 2. Mixed Language Workflows
```bash
python orchestrator/runner_v2.py --recipe orchestrator/recipes/02_basic_mixed.yaml
```

### 3. Standalone Scriptlet Execution
```bash
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"data.csv"}'
```

### 4. Context Server Operations
```bash
./update_state.sh set data.example '{"value": 42}'
./update_state.sh get data.example
```

## Development Environment

### Setup Commands
```bash
./setup_curriculum.sh      # Initialize sample data and examples
./rebase_workspace.sh       # Bootstrap framework components
python -m pytest tests/     # Run test suite
```

### Development Tools
```bash
./tmux_layout.sh -w 3 -p 2                    # Multi-pane development
python run_context_server.py                   # Shared context service
./simulate_cdu_poll.sh 5                       # Generate test data
```

## Repository Statistics

- **Total Files**: 48 code files (Python + Shell)
- **Python Files**: 28 (Framework + Scriptlets + Tests + Apps)
- **Shell Scripts**: 20 (Utilities + Setup + Integration)
- **Recipe Definitions**: 15 YAML workflow files
- **Test Coverage**: 14 comprehensive test files
- **Documentation**: Extensive inline and README documentation

## Architecture Benefits

1. **Maintainability**: Clear separation of concerns, standardized patterns
2. **Testability**: Comprehensive test suite with integration and unit tests
3. **Scalability**: Parallel execution support and modular design
4. **Flexibility**: Support for multiple languages and execution modes
5. **Robustness**: Structured error handling and state validation
6. **Extensibility**: Plugin architecture for easy feature addition

This repository represents a production-ready automation framework suitable for data processing, testing, monitoring, and reporting applications in enterprise environments.