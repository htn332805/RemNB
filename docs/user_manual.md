# RemNB Framework User Manual

## Table of Contents

1. [Quick Start Guide](#quick-start-guide)
2. [Installation and Setup](#installation-and-setup)
3. [Core Framework Components](#core-framework-components)
4. [Python Scriptlets Guide](#python-scriptlets-guide)
5. [Shell Scriptlets Guide](#shell-scriptlets-guide)
6. [Recipe Workflow System](#recipe-workflow-system)
7. [Context Management](#context-management)
8. [Advanced Features](#advanced-features)
9. [Real-World Applications](#real-world-applications)
10. [Development and Extension Guide](#development-and-extension-guide)
11. [Troubleshooting](#troubleshooting)
12. [API Reference](#api-reference)

---

## Quick Start Guide

### Basic Usage

1. **Initialize the framework:**
```bash
./setup_curriculum.sh    # Creates sample data and directories
```

2. **Run a simple recipe:**
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```

3. **Run an advanced parallel recipe:**
```bash
python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml
```

4. **Execute a standalone scriptlet:**
```bash
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
```

---

## Installation and Setup

### Prerequisites
- Python 3.7+
- Bash shell
- Standard Unix tools (awk, grep, etc.)

### Basic Installation

```bash
# Clone the repository
git clone <repository-url>
cd RemNB

# Initialize the framework
./setup_curriculum.sh

# Install Python dependencies (if available)
pip install pyyaml pytest

# Bootstrap framework components
./rebase_workspace.sh
```

### Verification

```bash
# Run tests
python -m pytest tests/

# Verify basic functionality
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```

---

## Core Framework Components

### 1. Context System (`orchestrator/context.py`)

The Context is a thread-safe, JSON-serializable state store that enables data sharing between scriptlets.

**Key Features:**
- Thread-safe operations
- JSON-only values (no complex objects)
- History tracking with attribution
- Versioned keys for backward compatibility

**Basic Usage:**
```python
from orchestrator.context import Context

ctx = Context()
ctx.set("numbers.stats_v1", {"mean": 5.5, "count": 10}, who="my_script")
stats = ctx.get("numbers.stats_v1")
print(ctx.to_dict())  # All context data
```

**CLI Context Manipulation:**
```bash
# Start context server
python run_context_server.py &

# Interact with shared context
./update_state.sh set data.example '{"value": 42}'
./update_state.sh get data.example
./update_state.sh append data.items '100'
./update_state.sh full | jq .
```

### 2. Runners

#### Basic Runner (`orchestrator/runner.py`)
Sequential execution of YAML recipes.

```bash
python orchestrator/runner.py --recipe path/to/recipe.yaml
```

#### Advanced Runner (`orchestrator/runner_v2.py`)
Enhanced executor with advanced capabilities:

```bash
# Standard execution
python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml

# With filters
python orchestrator/runner_v2.py --recipe recipe.yaml --only step1,step2
python orchestrator/runner_v2.py --recipe recipe.yaml --skip cleanup
```

**Features:**
- Parallel execution
- Shell scriptlet support
- Dependency management
- Process pools
- Resource tracking
- Caching

---

## Python Scriptlets Guide

### Scriptlet Architecture

All Python scriptlets inherit from `BaseScriptlet` and follow a standard pattern:

```python
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.context import Context
import json, argparse, sys

class MyScriptlet(BaseScriptlet):
    # Optional flags
    parallelizable = True      # Can run in parallel
    requires_process = False   # Needs process pool
    
    def validate(self, ctx, params):
        """Validate inputs before execution"""
        if "required_param" not in params:
            raise ValueError("required_param missing")
    
    def run(self, ctx, params):
        """Main execution logic"""
        try:
            self.validate(ctx, params)
            # Your logic here
            result = {"status": "success"}
            ctx.set("my.result_v1", result, who="my_scriptlet")
            print(json.dumps({"status":"ok","outputs":["my.result_v1"]}))
            return 0
        except Exception as e:
            print(json.dumps({"status":"error","reason":str(e),"exit_code":1}))
            return 1

# CLI interface
if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--params", required=True)
    p = json.loads(ap.parse_args().params)
    sys.exit(MyScriptlet().run(Context(), p))
```

### Data Processing Scriptlets

#### 1. Data Loading (`load_numbers.py`)

**Purpose:** Load numeric data from CSV files into context

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/load_numbers.py --params '{"src":"orchestrator/Data/numbers.csv"}'
```

**Input:** CSV file with numeric data
**Output:** `numbers.raw_v1` - List of integers

**Example CSV:**
```
# Sample data
1
2
3
4
5
```

**Integration:**
```yaml
steps:
  - name: load_data
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args:
      src: orchestrator/Data/numbers.csv
```

#### 2. Data Normalization (`normalize_numbers.py`)

**Purpose:** Min-max normalization (0-1 scaling) of numeric data

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/normalize_numbers.py --params '{}'
```

**Inputs:** `numbers.raw_v1`
**Outputs:** 
- `numbers.normalized_v1` - Normalized values (0-1)
- `numbers.stats_v1` - Basic statistics

**Artifact:** Creates `orchestrator/Data/normalized.json`

#### 3. Statistical Computation (`compute_numbers.py`)

**Purpose:** Calculate basic statistics from numeric data

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/compute_numbers.py --params '{"src":"data.csv"}'
```

**Input:** CSV file path
**Output:** `numbers.stats_v1` with:
```json
{
  "count": 5,
  "sum": 15,
  "mean": 3.0,
  "min": 1,
  "max": 5
}
```

#### 4. Advanced Metrics (`compute_metrics_v2.py`)

**Purpose:** Extended statistics including percentiles

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/compute_metrics_v2.py --params '{}'
```

**Inputs:** `numbers.normalized_v1`
**Outputs:**
- `metrics.summary_v1` - Basic metrics
- `metrics.summary_v2` - Extended metrics with p50, p90, p99

### Specialized Scriptlets

#### 5. Prime Factor Analysis (`prime_factor_analysis.py`)

**Purpose:** Prime factorization of numeric data

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/prime_factor_analysis.py --params '{}'
```

**Process Pool:** Enabled (`requires_process = True`)
**Input:** `numbers.raw_v1`
**Output:** `numbers.prime_factors_v1`

**Example Output:**
```json
{
  "6": [2, 3],
  "8": [2, 2, 2],
  "15": [3, 5]
}
```

#### 6. Data Validation (`validate_numbers.py`)

**Purpose:** Validate numeric data against constraints

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/validate_numbers.py --params '{
  "min_count": 3,
  "min_value": -100,
  "max_value": 1000
}'
```

**Input:** `numbers.raw_v1`
**Output:** `numbers.validated_v1`
**Errors:** Structured JSON with validation details

#### 7. Report Generation (`generate_report.py`)

**Purpose:** Generate markdown reports from template and context data

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/generate_report.py --params '{}'
```

**Template:** `orchestrator/templates/report_template.md`
**Output:** `orchestrator/Data/report.md`
**Context:** `report.path_v1`

**Template Example:**
```markdown
# Metrics Report

Generated: {{ generated_ts }}

## Summary
- Count: {{ metrics.summary_v1.count }}
- Mean: {{ metrics.summary_v1.mean }}
- Range: {{ metrics.summary_v1.min }} - {{ metrics.summary_v1.max }}

{% if metrics.summary_v2 %}
## Percentiles
- P50: {{ metrics.summary_v2.p50 }}
- P90: {{ metrics.summary_v2.p90 }}
{% endif %}
```

### Parallel Processing Scriptlets

#### 8. Individual Metrics (`compute_mean.py`, `compute_median.py`, `compute_std.py`)

These scriptlets are designed for parallel execution:

**Common Properties:**
- `parallelizable = True`
- Independent execution
- Input: `numbers.normalized_v1`

**Usage Example:**
```bash
# Run in parallel recipe
python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml
```

#### 9. Metrics Aggregation (`aggregate_metrics.py`)

**Purpose:** Combine results from parallel metric computations

**Dependencies:** `metrics.mean_v1`, `metrics.median_v1`, `metrics.std_v1`
**Output:** `metrics.aggregate_v1`

**Example Output:**
```json
{
  "mean": 0.5,
  "median": 0.4,
  "std": 0.2887
}
```

### Application Scriptlets

#### 10. Live Dashboard (`orchestrator/scriptlets/python/apps/live_dashboard.py`)

**Purpose:** Real-time monitoring dashboard

**Usage:**
```bash
# As standalone application
python orchestrator/scriptlets/python/apps/live_dashboard.py

# With custom settings
DASH_HOST=127.0.0.1 DASH_PORT=8765 python orchestrator/scriptlets/python/apps/live_dashboard.py
```

**Features:**
- Polls context server for live data
- Fallback to file-based data
- Non-blocking operation for recipe integration

#### 11. Resource Auditing (`resource_audit.py`)

**Purpose:** Analyze execution performance and resource usage

**Usage:**
```bash
python orchestrator/scriptlets/python/steps/resource_audit.py --params '{}'
```

**Input:** `_runner.step_metrics_v1` (auto-generated by runner_v2)
**Output:** `resource.audit_v1`

**Example Output:**
```json
{
  "total_steps": 5,
  "steps": {
    "load_numbers": {"duration_ms": 12.5, "exit_code": 0},
    "normalize_numbers": {"duration_ms": 8.3, "exit_code": 0}
  }
}
```

---

## Shell Scriptlets Guide

Shell scriptlets follow the JSON output contract and can be integrated seamlessly with Python workflows.

### 1. CSV Sanitization (`orchestrator/scriptlets/shell/sanitize_csv.sh`)

**Purpose:** Remove blank lines and comments from CSV files

**Usage:**
```bash
# Direct execution
./orchestrator/scriptlets/shell/sanitize_csv.sh input.csv

# In recipe
python orchestrator/runner_v2.py --recipe orchestrator/recipes/02_basic_mixed.yaml
```

**Input:** CSV file path (default: `orchestrator/Data/numbers.csv`)
**Output:** `orchestrator/Data/numbers_sanitized.csv`

**JSON Output:**
```json
{"status":"ok","outputs":["orchestrator/Data/numbers_sanitized.csv"],"sanitized_path":"orchestrator/Data/numbers_sanitized.csv"}
```

### 2. CSV Enhancement (`orchestrator/scriptlets/shell/enrich_csv.sh`)

**Purpose:** Add derived columns to CSV data

**Usage:**
```bash
./orchestrator/scriptlets/shell/enrich_csv.sh input.csv
```

**Process:** Adds a "square" column with squared values
**Input:** Sanitized CSV file
**Output:** `orchestrator/Data/numbers_enriched.csv`

**Example Transformation:**
```
# Input
1
2
3

# Output
value,square
1,1
2,4
3,9
```

### System Utilities

#### 3. Context State Management (`update_state.sh`)

**Purpose:** CLI interface for shared context manipulation

**Usage Examples:**
```bash
# Set values
./update_state.sh set data.config '{"timeout": 30}'
./update_state.sh set data.name '"John Doe"'

# Get values
./update_state.sh get data.config
./update_state.sh full

# List operations
./update_state.sh append data.items '42'
./update_state.sh append data.items '"hello"'

# Cleanup
./update_state.sh delete data.config
./update_state.sh pop data.items 0
```

**JSON Protocol:**
```json
{"op": "set", "path": "data.key", "value": {"nested": "object"}}
{"op": "get", "path": "data.key"}
{"op": "append", "path": "data.list", "value": "item"}
{"op": "delete", "path": "data.key"}
```

#### 4. Data Simulation (`simulate_cdu_poll.sh`)

**Purpose:** Generate realistic sensor data for testing

**Usage:**
```bash
# Continuous polling (5-second intervals)
./simulate_cdu_poll.sh 5

# Pipe to state management
./simulate_cdu_poll.sh 1 | while read -r json; do
  [[ -n "$json" ]] && ./update_state.sh append sensor.readings "$json"
done
```

**Output Format:**
```json
{
  "timestamp": "03/15/2024 14:30:25",
  "supply_pressure": 15.47,
  "return_pressure": 12.83,
  "supply_temp": 21.25,
  "return_temp": 23.67
}
```

#### 5. Development Environment (`tmux_layout.sh`)

**Purpose:** Multi-pane development environment setup

**Usage:**
```bash
# Basic setup (1 window, 1 pane)
./tmux_layout.sh

# Advanced setup
./tmux_layout.sh -w 3 -p 2 -sn "dev_session"
./tmux_layout.sh -wn "edit,test,monitor" -pn "main,logs"
```

**Features:**
- Automatic log capture to `Logs/` directory
- Mouse support and visual styling
- Configurable windows and panes
- Terminal title setting

---

## Recipe Workflow System

### Recipe Structure

YAML recipes define execution workflows with metadata, steps, and success criteria:

```yaml
test_meta:
  test_id: "EXAMPLE-001"
  tester: "user"
  description: "Demonstration workflow"

steps:
  - idx: 1
    name: step_name
    type: python|shell
    module: path.to.module        # Python only
    function: ClassName           # Python only
    script: path/to/script.sh     # Shell only
    args:                         # Parameters
      param1: value1
      param2: value2
    depends_on: [other_step]      # Dependencies (runner_v2)
    cache_key: "cache_identifier" # Caching (runner_v2)
    success:                      # Validation (future)
      ctx_has_keys:
        - expected.output_v1
```

### Basic Recipes

#### 1. Simple Statistics (`example_numbers.yaml`)

```yaml
test_meta:
  test_id: "NUM-001"
  description: "Compute simple statistics"
steps:
  - idx: 1
    name: compute_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.compute_numbers
    function: ComputeNumbers
    args:
      src: orchestrator/Data/numbers.csv
    success:
      ctx_has_keys:
        - numbers.stats_v1
```

**Usage:**
```bash
python orchestrator/runner.py --recipe orchestrator/recipes/example_numbers.yaml
```

#### 2. Mixed Python-Shell (`02_basic_mixed.yaml`)

```yaml
test_meta: { test_id: "M02", description: "Shell sanitize + load" }
steps:
  - idx: 1
    name: sanitize_csv
    type: shell
    script: orchestrator/scriptlets/shell/sanitize_csv.sh
    args: [orchestrator/Data/numbers.csv]
  - idx: 2
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args:
      src: orchestrator/Data/numbers_sanitized.csv
```

### Advanced Recipes

#### 3. Parallel Fan-In (`06_parallel_fan_in.yaml`)

Complex workflow with parallel execution and dependency management:

```yaml
test_meta: { test_id: "M06", description: "Parallel fan-in metrics" }
steps:
  - idx: 1
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
  
  - idx: 2
    name: normalize_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.normalize_numbers
    function: NormalizeNumbers
  
  # Parallel metric calculations
  - idx: 10
    name: compute_mean
    type: python
    module: orchestrator.scriptlets.python.steps.compute_mean
    function: ComputeMean
    depends_on: [normalize_numbers]
    
  - idx: 11
    name: compute_median
    type: python
    module: orchestrator.scriptlets.python.steps.compute_median
    function: ComputeMedian
    depends_on: [normalize_numbers]
    
  - idx: 12
    name: compute_std
    type: python
    module: orchestrator.scriptlets.python.steps.compute_std
    function: ComputeStd
    depends_on: [normalize_numbers]
  
  # Aggregation step
  - idx: 13
    name: aggregate_metrics
    type: python
    module: orchestrator.scriptlets.python.steps.aggregate_metrics
    function: AggregateMetrics
    depends_on: [compute_mean, compute_median, compute_std]
```

**Execution:**
```bash
python orchestrator/runner_v2.py --recipe orchestrator/recipes/06_parallel_fan_in.yaml
```

#### 4. Complete Pipeline (`12_advanced_full.yaml`)

Full-featured pipeline with caching, dependencies, and resource monitoring:

```yaml
test_meta: { test_id: "M12", description: "Full advanced pipeline" }
steps:
  - idx: 1
    name: cache_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.cache_numbers
    function: CacheNumbers
    args: { src: orchestrator/Data/numbers.csv }
    cache_key: "cache_numbers_v1"
    
  - idx: 2
    name: load_numbers
    type: python
    module: orchestrator.scriptlets.python.steps.load_numbers
    function: LoadNumbers
    args: { src: orchestrator/Data/numbers.csv }
    depends_on: [cache_numbers]
    
  # ... additional processing steps ...
  
  - idx: 10
    name: resource_audit
    type: python
    module: orchestrator.scriptlets.python.steps.resource_audit
    function: ResourceAudit
    depends_on: [aggregate_metrics, prime_factor_analysis]
    
  - idx: 11
    name: generate_report
    type: python
    module: orchestrator.scriptlets.python.steps.generate_report
    function: GenerateReport
    depends_on: [compute_metrics_v2, aggregate_metrics, resource_audit]
```

### Recipe Execution Options

#### Standard Execution
```bash
python orchestrator/runner_v2.py --recipe path/to/recipe.yaml
```

#### Filtered Execution
```bash
# Execute only specific steps
python orchestrator/runner_v2.py --recipe recipe.yaml --only step1,step2

# Skip specific steps
python orchestrator/runner_v2.py --recipe recipe.yaml --skip cleanup_step

# Resume from specific step
python orchestrator/runner_v2.py --recipe recipe.yaml --resume-from step3
```

---

## Context Management

### Context Architecture

The Context system provides thread-safe, JSON-serializable state management across all framework components.

#### Key Principles

1. **JSON-Only Values:** No complex objects, handles, or non-serializable data
2. **Versioned Keys:** Use `_v1`, `_v2` suffixes for backward compatibility
3. **Attribution:** Track who modified each key via `who` parameter
4. **Thread Safety:** All operations are protected by locks

#### Context Operations

**Setting Values:**
```python
ctx = Context()
ctx.set("numbers.stats_v1", {"mean": 5.0, "count": 10}, who="my_script")
ctx.set("process.config_v1", {"timeout": 30}, who="config_loader")
```

**Retrieving Values:**
```python
stats = ctx.get("numbers.stats_v1")
config = ctx.get("process.config_v1", default={"timeout": 60})
```

**Context Inspection:**
```python
all_data = ctx.to_dict()
history = ctx.history()
print(f"Context contains {len(all_data)} keys")
```

### Shared Context Server

For multi-process and persistent contexts, use the shared context server:

#### Starting the Server
```bash
python run_context_server.py
```

#### Client Operations
```bash
# CLI interface
./update_state.sh set app.config '{"debug": true}'
./update_state.sh get app.config
./update_state.sh append app.logs '"Starting process"'
./update_state.sh full | jq .

# Programmatic access (Python)
import socket, json

def send_command(cmd):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(("127.0.0.1", 8765))
        s.sendall((cmd + "\n").encode())
        return json.loads(s.recv(8192).decode().strip())

# Set value
result = send_command('set data.example {"value": 42}')

# Get value
result = send_command('get data.example')
print(result["value"])
```

### Context Patterns

#### 1. Data Pipeline Context

```python
# Step 1: Data loading
ctx.set("data.raw_v1", load_data(), who="data_loader")

# Step 2: Processing
raw_data = ctx.get("data.raw_v1")
processed = process_data(raw_data)
ctx.set("data.processed_v1", processed, who="processor")

# Step 3: Analysis
processed_data = ctx.get("data.processed_v1")
results = analyze_data(processed_data)
ctx.set("analysis.results_v1", results, who="analyzer")
```

#### 2. Configuration Context

```python
# Global configuration
ctx.set("config.database_v1", {
    "host": "localhost",
    "port": 5432,
    "database": "myapp"
}, who="config_init")

# Feature flags
ctx.set("config.features_v1", {
    "parallel_processing": True,
    "cache_enabled": True,
    "debug_mode": False
}, who="feature_manager")
```

#### 3. Metrics and Monitoring

```python
# Performance metrics
ctx.set("metrics.performance_v1", {
    "execution_time": 12.5,
    "memory_usage": 256,
    "cpu_percent": 45.2
}, who="performance_monitor")

# Business metrics
ctx.set("metrics.business_v1", {
    "records_processed": 10000,
    "errors_count": 2,
    "success_rate": 99.98
}, who="business_monitor")
```

---

## Advanced Features

### 1. Parallel Execution

The framework supports parallel execution of independent steps via `runner_v2.py`:

#### Enabling Parallel Execution

**Scriptlet Level:**
```python
class MyScriptlet(BaseScriptlet):
    parallelizable = True  # Enable parallel execution
    requires_process = False  # Use thread pool (False) or process pool (True)
```

**Recipe Level:**
```yaml
steps:
  - name: step1
    type: python
    module: my.module
    function: MyScriptlet
    depends_on: [prerequisite_step]
  - name: step2
    type: python  
    module: my.other_module
    function: MyOtherScriptlet
    depends_on: [prerequisite_step]  # Can run parallel with step1
```

#### Process Pools for CPU-Intensive Tasks

```python
class CPUIntensiveScriptlet(BaseScriptlet):
    requires_process = True  # Use separate process
    
    def run(self, ctx, params):
        # CPU-intensive work (prime factorization, etc.)
        result = cpu_intensive_calculation()
        ctx.set("result.cpu_work_v1", result, who="cpu_worker")
        return 0
```

### 2. Dependency Management

Define complex workflows with step dependencies:

```yaml
steps:
  - idx: 1
    name: data_load
    # ... step definition ...
  
  - idx: 2
    name: data_clean
    depends_on: [data_load]
    # ... step definition ...
  
  - idx: 3
    name: feature_extract
    depends_on: [data_clean]
    # ... step definition ...
  
  - idx: 4
    name: model_train
    depends_on: [feature_extract]
    # ... step definition ...
    
  - idx: 5
    name: model_evaluate
    depends_on: [model_train]
    # ... step definition ...
```

### 3. Caching System

Implement caching to avoid redundant computations:

```yaml
steps:
  - idx: 1
    name: expensive_computation
    type: python
    module: my.module
    function: ExpensiveScriptlet
    cache_key: "expensive_comp_v1"  # Cache identifier
    args:
      input_file: data.csv
```

**Cache Implementation:**
```python
class CacheNumbers(BaseScriptlet):
    def run(self, ctx, params):
        cache_file = Path(f".cache/numbers_{hash(params['src'])}.done")
        if cache_file.exists():
            print(json.dumps({"status":"ok","cached":True}))
            return 0
        
        # Perform computation
        result = expensive_computation(params['src'])
        cache_file.touch()
        ctx.set("cached.result_v1", result, who="cache_numbers")
        print(json.dumps({"status":"ok","cached":False}))
        return 0
```

### 4. Resource Tracking

Monitor execution performance automatically:

```python
from orchestrator.scriptlets.python.core.resource import track_resources

class MyScriptlet(BaseScriptlet):
    @track_resources  # Automatic timing
    def run(self, ctx, params):
        # Your logic here
        time.sleep(2)  # Simulated work
        return 0
```

**Resource Metrics:**
The `runner_v2.py` automatically tracks metrics in `_runner.step_metrics_v1`:

```json
{
  "step_name": {
    "duration_ms": 2134.5,
    "exit_code": 0,
    "module": "path.to.module"
  }
}
```

### 5. Error Handling and Resilience

#### Structured Error Reporting

```python
def run(self, ctx, params):
    try:
        self.validate(ctx, params)
        # Processing logic
        return 0
    except Exception as e:
        error_details = {
            "status": "error",
            "reason": str(e),
            "exit_code": 1,
            "step": "my_step",
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "context": {"param1": params.get("param1")}
        }
        print(json.dumps(error_details))
        return 1
```

#### Failure Injection for Testing

```python
# inject_failure.py
class InjectFailure(BaseScriptlet):
    def run(self, ctx, params):
        failure_type = params.get("failure_type", "generic")
        if failure_type == "timeout":
            time.sleep(params.get("timeout", 30))
        elif failure_type == "exception":
            raise RuntimeError("Injected failure")
        return 1
```

**Usage in Recipe:**
```yaml
steps:
  - name: test_failure
    type: python
    module: orchestrator.scriptlets.python.steps.inject_failure
    function: InjectFailure
    args:
      failure_type: "exception"
```

---

## Real-World Applications

### 1. Data Processing Pipelines

#### Scientific Data Analysis
```bash
# Multi-stage data pipeline
python orchestrator/runner_v2.py --recipe pipelines/scientific_analysis.yaml
```

**Example Pipeline:**
1. **Data Ingestion:** Load experimental data from multiple sources
2. **Data Cleaning:** Remove outliers and normalize formats
3. **Feature Extraction:** Calculate derived metrics and features
4. **Statistical Analysis:** Compute descriptive and inferential statistics
5. **Visualization:** Generate plots and summary reports
6. **Quality Assurance:** Validate results against known benchmarks

#### Financial Data Processing
```yaml
test_meta: { description: "Daily financial data processing" }
steps:
  - name: fetch_market_data
    type: python
    module: finance.steps.fetch_data
    function: FetchMarketData
    args: { symbols: ["AAPL", "GOOGL", "MSFT"] }
  
  - name: calculate_indicators
    type: python
    module: finance.steps.indicators
    function: CalculateIndicators
    depends_on: [fetch_market_data]
  
  - name: risk_analysis
    type: python
    module: finance.steps.risk
    function: RiskAnalysis
    depends_on: [calculate_indicators]
  
  - name: generate_report
    type: python
    module: finance.steps.reporting
    function: GenerateReport
    depends_on: [risk_analysis]
```

### 2. Automated Testing and Quality Assurance

#### System Integration Testing
```yaml
test_meta: { description: "End-to-end system testing" }
steps:
  - name: setup_environment
    type: shell
    script: tests/setup_test_env.sh
  
  - name: run_api_tests
    type: python
    module: tests.steps.api_testing
    function: APITestSuite
    depends_on: [setup_environment]
  
  - name: run_ui_tests
    type: python
    module: tests.steps.ui_testing
    function: UITestSuite
    depends_on: [setup_environment]
  
  - name: performance_tests
    type: python
    module: tests.steps.performance
    function: PerformanceTestSuite
    depends_on: [run_api_tests, run_ui_tests]
  
  - name: generate_test_report
    type: python
    module: tests.steps.reporting
    function: TestReportGenerator
    depends_on: [performance_tests]
  
  - name: cleanup
    type: shell
    script: tests/cleanup_test_env.sh
    depends_on: [generate_test_report]
```

### 3. Monitoring and Alerting Systems

#### Infrastructure Monitoring
```bash
# Continuous monitoring pipeline
./simulate_cdu_poll.sh 30 | while read -r json; do
  echo "$json" | python monitors/process_metrics.py
done
```

**Monitoring Components:**
1. **Data Collectors:** Gather metrics from various sources
2. **Threshold Checkers:** Validate against operational limits  
3. **Anomaly Detectors:** Identify unusual patterns
4. **Alert Generators:** Send notifications for critical issues
5. **Dashboard Updates:** Real-time visualization

#### Business Process Monitoring
```yaml
test_meta: { description: "Daily business process monitoring" }
steps:
  - name: extract_business_data
    type: python
    module: business.steps.extraction
    function: DataExtractor
  
  - name: validate_data_quality
    type: python
    module: business.steps.validation
    function: DataQualityChecker
    depends_on: [extract_business_data]
  
  - name: calculate_kpis
    type: python
    module: business.steps.metrics
    function: KPICalculator
    depends_on: [validate_data_quality]
  
  - name: trend_analysis
    type: python
    module: business.steps.analysis
    function: TrendAnalyzer
    depends_on: [calculate_kpis]
  
  - name: alert_on_anomalies
    type: python
    module: business.steps.alerting
    function: AnomalyAlerter
    depends_on: [trend_analysis]
```

### 4. CI/CD Integration

#### Build and Deployment Pipeline
```yaml
test_meta: { description: "Application build and deployment" }
steps:
  - name: checkout_code
    type: shell
    script: ci/checkout.sh
    args: [${GIT_BRANCH}]
  
  - name: run_unit_tests
    type: python
    module: ci.steps.testing
    function: UnitTestRunner
    depends_on: [checkout_code]
  
  - name: build_application
    type: shell
    script: ci/build.sh
    depends_on: [run_unit_tests]
  
  - name: security_scan
    type: python
    module: ci.steps.security
    function: SecurityScanner
    depends_on: [build_application]
  
  - name: deploy_staging
    type: shell
    script: ci/deploy.sh
    args: [staging]
    depends_on: [security_scan]
  
  - name: integration_tests
    type: python
    module: ci.steps.integration
    function: IntegrationTestSuite
    depends_on: [deploy_staging]
  
  - name: deploy_production
    type: shell
    script: ci/deploy.sh
    args: [production]
    depends_on: [integration_tests]
```

### 5. Machine Learning Workflows

#### Model Training Pipeline
```yaml
test_meta: { description: "ML model training and evaluation" }
steps:
  - name: load_training_data
    type: python
    module: ml.steps.data_loading
    function: TrainingDataLoader
    args: { dataset: "customer_data.csv" }
  
  - name: feature_engineering
    type: python
    module: ml.steps.features
    function: FeatureEngineer
    depends_on: [load_training_data]
  
  - name: split_data
    type: python
    module: ml.steps.data_prep
    function: DataSplitter
    depends_on: [feature_engineering]
  
  - name: train_model
    type: python
    module: ml.steps.training
    function: ModelTrainer
    depends_on: [split_data]
    requires_process: true  # CPU-intensive
  
  - name: evaluate_model
    type: python
    module: ml.steps.evaluation
    function: ModelEvaluator
    depends_on: [train_model]
  
  - name: model_validation
    type: python
    module: ml.steps.validation
    function: ModelValidator
    depends_on: [evaluate_model]
  
  - name: deploy_model
    type: python
    module: ml.steps.deployment
    function: ModelDeployer
    depends_on: [model_validation]
```

### 6. Enterprise Integration Scenarios

#### ETL (Extract, Transform, Load) Processes
```yaml
test_meta: { description: "Daily ETL process" }
steps:
  - name: extract_from_database
    type: python
    module: etl.steps.extraction
    function: DatabaseExtractor
    args:
      connection: "postgresql://user:pass@host:5432/db"
      query: "SELECT * FROM daily_transactions"
  
  - name: extract_from_api
    type: python
    module: etl.steps.extraction
    function: APIExtractor
    args:
      endpoint: "https://api.external-service.com/data"
      auth_token: "${API_TOKEN}"
  
  - name: validate_extracted_data
    type: python
    module: etl.steps.validation
    function: DataValidator
    depends_on: [extract_from_database, extract_from_api]
  
  - name: transform_data
    type: python
    module: etl.steps.transformation
    function: DataTransformer
    depends_on: [validate_extracted_data]
    parallelizable: true
  
  - name: load_to_warehouse
    type: python
    module: etl.steps.loading
    function: WarehouseLoader
    depends_on: [transform_data]
    args:
      target: "data_warehouse"
      table: "processed_transactions"
  
  - name: update_metadata
    type: python
    module: etl.steps.metadata
    function: MetadataUpdater
    depends_on: [load_to_warehouse]
```

#### Multi-System Integration
```bash
# Orchestrate across multiple systems
python orchestrator/runner_v2.py --recipe integrations/multi_system.yaml

# Monitor integration health
python run_context_server.py &
python orchestrator/scriptlets/python/apps/live_dashboard.py &

# Process integration events
./simulate_cdu_poll.sh 10 | while read -r event; do
  ./update_state.sh append integration.events "$event"
done
```

---

## Development and Extension Guide

### Creating Custom Scriptlets

#### 1. Python Scriptlet Development

**Step 1:** Create the scriptlet file
```python
# orchestrator/scriptlets/python/steps/my_custom_step.py
"""
Custom processing step for business logic.
Inputs: business.data_v1
Outputs: business.processed_v1, business.summary_v1
"""
import json, argparse, sys
from typing import Any, Dict
from orchestrator.context import Context
from orchestrator.scriptlets.python.core.base import BaseScriptlet
from orchestrator.scriptlets.python.core.logging_util import get_logger
from orchestrator.scriptlets.python.core.resource import track_resources

logger = get_logger(__name__)

class MyCustomStep(BaseScriptlet):
    parallelizable = False  # Set to True if can run in parallel
    requires_process = False  # Set to True if CPU-intensive
    
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        """Validate inputs and parameters"""
        if not params.get("required_param"):
            raise ValueError("required_param is missing")
        
        if ctx.get("business.data_v1") is None:
            raise ValueError("business.data_v1 not found in context")
    
    @track_resources  # Optional: track execution time
    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        """Main execution logic"""
        try:
            self.validate(ctx, params)
            logger.info(f"Processing with params: {params}")
            
            # Get input data
            input_data = ctx.get("business.data_v1")
            
            # Process data
            processed_data = self.process_data(input_data, params)
            summary = self.create_summary(processed_data)
            
            # Store results
            ctx.set("business.processed_v1", processed_data, who="my_custom_step")
            ctx.set("business.summary_v1", summary, who="my_custom_step")
            
            # Success output
            print(json.dumps({
                "status": "ok",
                "outputs": ["business.processed_v1", "business.summary_v1"],
                "records_processed": len(processed_data)
            }))
            return 0
            
        except Exception as e:
            logger.error(f"Processing failed: {e}")
            print(json.dumps({
                "status": "error",
                "reason": str(e),
                "exit_code": 1,
                "step": "my_custom_step"
            }))
            return 1
    
    def process_data(self, data, params):
        """Custom processing logic"""
        # Your business logic here
        return [item for item in data if item.get("valid", True)]
    
    def create_summary(self, data):
        """Create summary statistics"""
        return {
            "total_records": len(data),
            "processed_at": datetime.now(timezone.utc).isoformat()
        }

# CLI interface
if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Custom processing step")
    ap.add_argument("--params", required=True, help="JSON parameters")
    args = ap.parse_args()
    
    params = json.loads(args.params)
    sys.exit(MyCustomStep().run(Context(), params))
```

**Step 2:** Test the scriptlet standalone
```bash
python orchestrator/scriptlets/python/steps/my_custom_step.py --params '{
  "required_param": "test_value",
  "optional_param": 42
}'
```

**Step 3:** Create recipe integration
```yaml
# orchestrator/recipes/my_custom_workflow.yaml
test_meta:
  test_id: "CUSTOM-001"
  description: "Custom business workflow"

steps:
  - idx: 1
    name: load_business_data
    type: python
    module: orchestrator.scriptlets.python.steps.load_business_data
    function: LoadBusinessData
    args:
      source: "business_data.csv"
  
  - idx: 2
    name: my_custom_step
    type: python
    module: orchestrator.scriptlets.python.steps.my_custom_step
    function: MyCustomStep
    args:
      required_param: "production_value"
      processing_mode: "enhanced"
    depends_on: [load_business_data]
    success:
      ctx_has_keys:
        - business.processed_v1
        - business.summary_v1
```

#### 2. Shell Scriptlet Development

**Step 1:** Create the shell script
```bash
# orchestrator/scriptlets/shell/my_custom_shell.sh
#!/usr/bin/env bash
# Custom shell processing step
# Usage: ./my_custom_shell.sh INPUT_FILE OUTPUT_FILE

set -euo pipefail

INPUT="${1:-input.txt}"
OUTPUT="${2:-output.txt}"

# Validate inputs
if [[ ! -f "$INPUT" ]]; then
    echo '{"status":"error","reason":"Input file not found","exit_code":1}' >&2
    exit 1
fi

# Create output directory
mkdir -p "$(dirname "$OUTPUT")"

# Processing logic
{
    echo "# Processed on $(date)"
    echo "# Source: $INPUT"
    
    # Custom processing (example: extract unique values)
    sort "$INPUT" | uniq -c | sort -nr
    
} > "$OUTPUT"

# Success output (JSON to stdout)
echo "{\"status\":\"ok\",\"outputs\":[\"$OUTPUT\"],\"processed_file\":\"$OUTPUT\"}"
```

**Step 2:** Make executable and test
```bash
chmod +x orchestrator/scriptlets/shell/my_custom_shell.sh
./orchestrator/scriptlets/shell/my_custom_shell.sh test_input.txt test_output.txt
```

**Step 3:** Integrate into recipe
```yaml
steps:
  - name: shell_processing
    type: shell
    script: orchestrator/scriptlets/shell/my_custom_shell.sh
    args: ["orchestrator/Data/input.txt", "orchestrator/Data/processed.txt"]
```

### Framework Extension Patterns

#### 1. Adding New Data Sources

Create specialized data loader scriptlets:

```python
class DatabaseLoader(BaseScriptlet):
    def run(self, ctx, params):
        import psycopg2  # Example database connection
        
        conn = psycopg2.connect(
            host=params["host"],
            database=params["database"],
            user=params["user"],
            password=params["password"]
        )
        
        with conn.cursor() as cur:
            cur.execute(params["query"])
            data = cur.fetchall()
        
        ctx.set("data.database_v1", data, who="database_loader")
        print(json.dumps({"status":"ok","outputs":["data.database_v1"]}))
        return 0
```

#### 2. Custom Output Formats

Extend the framework with new output capabilities:

```python
class JSONExporter(BaseScriptlet):
    def run(self, ctx, params):
        output_path = params["output_path"]
        data_key = params["data_key"]
        
        data = ctx.get(data_key)
        if data is None:
            raise ValueError(f"Data key {data_key} not found")
        
        with open(output_path, 'w') as f:
            json.dump(data, f, indent=2)
        
        ctx.set("export.json_path_v1", output_path, who="json_exporter")
        print(json.dumps({"status":"ok","artifact":output_path}))
        return 0
```

#### 3. Integration with External Systems

Create scriptlets for external system integration:

```python
class SlackNotifier(BaseScriptlet):
    def run(self, ctx, params):
        import requests
        
        webhook_url = params["webhook_url"]
        message = params["message"]
        
        # Include context data in message
        if params.get("include_summary"):
            summary = ctx.get("metrics.summary_v1", {})
            message += f"\n\nMetrics: {summary}"
        
        payload = {"text": message}
        response = requests.post(webhook_url, json=payload)
        response.raise_for_status()
        
        ctx.set("notifications.slack_sent_v1", True, who="slack_notifier")
        print(json.dumps({"status":"ok","response_code":response.status_code}))
        return 0
```

### Best Practices for Development

#### 1. Error Handling

```python
def run(self, ctx, params):
    try:
        self.validate(ctx, params)
        # Main logic
        return 0
    except ValidationError as e:
        print(json.dumps({
            "status": "error",
            "error_type": "validation",
            "reason": str(e),
            "exit_code": 1
        }))
        return 1
    except ExternalServiceError as e:
        print(json.dumps({
            "status": "error", 
            "error_type": "external_service",
            "reason": str(e),
            "retry_suggested": True,
            "exit_code": 2
        }))
        return 2
    except Exception as e:
        logger.exception("Unexpected error")
        print(json.dumps({
            "status": "error",
            "error_type": "unexpected",
            "reason": str(e),
            "exit_code": 1
        }))
        return 1
```

#### 2. Configuration Management

```python
class ConfigurableScriptlet(BaseScriptlet):
    def __init__(self):
        self.config = self.load_config()
    
    def load_config(self):
        config_path = os.environ.get("SCRIPTLET_CONFIG", "config.json")
        if os.path.exists(config_path):
            with open(config_path) as f:
                return json.load(f)
        return {}
    
    def run(self, ctx, params):
        # Use self.config for configuration
        timeout = self.config.get("timeout", 30)
        api_endpoint = self.config.get("api_endpoint", params["endpoint"])
        # ... rest of logic
```

#### 3. Testing Custom Scriptlets

```python
# test_my_custom_step.py
import pytest
from orchestrator.context import Context
from orchestrator.scriptlets.python.steps.my_custom_step import MyCustomStep

def test_my_custom_step_success():
    ctx = Context()
    ctx.set("business.data_v1", [{"id": 1, "valid": True}], who="test")
    
    scriptlet = MyCustomStep()
    result = scriptlet.run(ctx, {"required_param": "test"})
    
    assert result == 0
    assert ctx.get("business.processed_v1") is not None
    assert ctx.get("business.summary_v1")["total_records"] == 1

def test_my_custom_step_validation_failure():
    ctx = Context()
    scriptlet = MyCustomStep()
    
    result = scriptlet.run(ctx, {})  # Missing required_param
    assert result == 1

def test_my_custom_step_missing_data():
    ctx = Context()
    scriptlet = MyCustomStep()
    
    result = scriptlet.run(ctx, {"required_param": "test"})
    assert result == 1  # Missing business.data_v1
```

---

## Troubleshooting

### Common Issues and Solutions

#### 1. Import and Module Path Issues

**Problem:** `ImportError: No module named 'orchestrator'`

**Solution:**
```bash
# Ensure you're running from the repository root
cd /path/to/RemNB

# Set PYTHONPATH if needed
export PYTHONPATH=$PYTHONPATH:$(pwd)

# Run with python -m for module imports
python -m orchestrator.scriptlets.python.steps.compute_numbers --params '{...}'
```

#### 2. Context Key Not Found

**Problem:** `ValueError: Required context key not found`

**Debug Steps:**
```python
# Check available keys
ctx = run_recipe("recipe.yaml")
print("Available keys:", list(ctx.to_dict().keys()))

# Check context history
history = ctx.history()
for entry in history:
    print(f"Key: {entry['key']}, Set by: {entry['who']}")

# Validate key naming
assert "numbers.stats_v1" in ctx.to_dict(), "Expected key missing"
```

#### 3. JSON Serialization Errors

**Problem:** `TypeError: Object of type 'datetime' is not JSON serializable`

**Solution:**
```python
import datetime
from datetime import timezone

# Convert datetime to ISO string
timestamp = datetime.now(timezone.utc).isoformat()
ctx.set("process.timestamp_v1", timestamp, who="my_step")

# Convert complex objects to JSON-safe format
import numpy as np
array_data = np.array([1, 2, 3])
ctx.set("data.array_v1", array_data.tolist(), who="numpy_processor")

# Validate JSON serializability
import json
def ensure_json_safe(obj):
    try:
        json.dumps(obj)
        return obj
    except TypeError as e:
        raise ValueError(f"Object not JSON serializable: {e}")
```

#### 4. Shell Scriptlet Output Issues

**Problem:** Shell scriptlets not producing expected JSON output

**Debug:**
```bash
# Test shell script directly
./orchestrator/scriptlets/shell/my_script.sh args... 

# Check exit code
echo "Exit code: $?"

# Verify JSON output format
./orchestrator/scriptlets/shell/my_script.sh args... | jq .

# Check for stderr output
./orchestrator/scriptlets/shell/my_script.sh args... 2>&1
```

#### 5. Parallel Execution Issues

**Problem:** Parallel steps not executing as expected

**Debug:**
```python
# Check scriptlet parallel flags
class MyScriptlet(BaseScriptlet):
    parallelizable = True  # Must be True for parallel execution
    
    def run(self, ctx, params):
        import threading
        print(f"Running in thread: {threading.current_thread().name}")
        # ... rest of logic
```

**Recipe Debug:**
```yaml
# Ensure proper dependencies
steps:
  - name: step1
    depends_on: [prerequisite]  # Both can run after prerequisite
  - name: step2
    depends_on: [prerequisite]  # Both can run after prerequisite
  - name: step3
    depends_on: [step1, step2]  # Runs after both complete
```

#### 6. Context Server Connection Issues

**Problem:** `ConnectionRefusedError` when using shared context

**Solution:**
```bash
# Start context server
python run_context_server.py &

# Verify server is running
netstat -an | grep 8765
# or
ss -tuln | grep 8765

# Test connection
echo 'full' | nc 127.0.0.1 8765

# Check for port conflicts
lsof -i :8765
```

### Performance Troubleshooting

#### 1. Slow Scriptlet Execution

**Debug with Resource Tracking:**
```python
from orchestrator.scriptlets.python.core.resource import track_resources

class SlowScriptlet(BaseScriptlet):
    @track_resources
    def run(self, ctx, params):
        # Logic here - timing will be logged to stderr
        return 0
```

**Profile with runner_v2:**
```bash
# Run with resource tracking
python orchestrator/runner_v2.py --recipe recipe.yaml

# Check step metrics
python -c "
from orchestrator.runner_v2 import run_recipe_v2
ctx = run_recipe_v2('recipe.yaml', set(), set(), None)
metrics = ctx.get('_runner.step_metrics_v1', {})
for step, data in metrics.items():
    print(f'{step}: {data[\"duration_ms\"]}ms')
"
```

#### 2. Memory Usage Issues

**Monitor Memory:**
```python
import psutil
import os

class MemoryAwareScriptlet(BaseScriptlet):
    def run(self, ctx, params):
        process = psutil.Process(os.getpid())
        initial_memory = process.memory_info().rss
        
        # Your processing logic
        result = self.process_data(data)
        
        final_memory = process.memory_info().rss
        memory_delta = final_memory - initial_memory
        
        ctx.set("performance.memory_delta_v1", memory_delta, who="memory_tracker")
        return 0
```

#### 3. Dependency Resolution Issues

**Debug Dependencies:**
```python
# In runner_v2.py, add debug output
def resolve_dependencies(recipe):
    steps = recipe.get("steps", [])
    print("Dependency resolution:")
    for step in steps:
        deps = step.get("depends_on", [])
        print(f"Step {step['name']}: depends on {deps}")
```

### Debugging Workflows

#### 1. Recipe Execution Debug

```bash
# Add debug environment variable
export ORCHESTRATOR_DEBUG=1
python orchestrator/runner_v2.py --recipe recipe.yaml

# Test recipe syntax
python -c "
import yaml
with open('recipe.yaml') as f:
    recipe = yaml.safe_load(f)
print('Recipe valid:', bool(recipe))
"
```

#### 2. Context State Inspection

```python
def debug_context_state(ctx):
    """Helper function to inspect context state"""
    print("\n=== Context Debug Info ===")
    data = ctx.to_dict()
    print(f"Total keys: {len(data)}")
    
    for key, value in data.items():
        value_type = type(value).__name__
        if isinstance(value, (list, dict)):
            size = len(value)
            print(f"  {key}: {value_type}({size})")
        else:
            print(f"  {key}: {value_type} = {value}")
    
    print("\n=== Context History ===")
    for entry in ctx.history():
        print(f"  {entry['key']} <- {entry['who']}")
```

#### 3. Step-by-Step Execution

```bash
# Execute recipe with only specific steps
python orchestrator/runner_v2.py --recipe recipe.yaml --only step1
python orchestrator/runner_v2.py --recipe recipe.yaml --only step1,step2

# Skip problematic steps
python orchestrator/runner_v2.py --recipe recipe.yaml --skip problematic_step
```

---

## API Reference

### Core Classes

#### Context Class

```python
class Context:
    def __init__(self):
        """Initialize empty context with thread-safe storage"""
        
    def set(self, key: str, value: Any, who: str = "unknown") -> None:
        """Store value with attribution"""
        
    def get(self, key: str, default: Any = None) -> Any:
        """Retrieve value by key"""
        
    def to_dict(self) -> Dict[str, Any]:
        """Get all context data as dictionary"""
        
    def history(self) -> List[Dict[str, str]]:
        """Get modification history"""
```

#### BaseScriptlet Class

```python
class BaseScriptlet:
    parallelizable: bool = False      # Can run in parallel
    requires_process: bool = False    # Needs process pool
    
    def validate(self, ctx: Context, params: Dict[str, Any]) -> None:
        """Validate inputs before execution (override in subclass)"""
        pass
    
    def run(self, ctx: Context, params: Dict[str, Any]) -> int:
        """Main execution logic (must override in subclass)"""
        raise NotImplementedError("Implement in subclass")
```

### Runner Functions

#### Basic Runner

```python
def load_recipe(path: str) -> Dict[str, Any]:
    """Load YAML recipe from file"""
    
def run_recipe(recipe_path: str) -> Context:
    """Execute recipe sequentially, return final context"""
```

#### Advanced Runner

```python
def run_recipe_v2(
    recipe_path: str,
    only_steps: Set[str] = set(),
    skip_steps: Set[str] = set(),
    resume_from: Optional[str] = None
) -> Context:
    """Execute recipe with advanced features"""
```

### Utility Functions

#### Logging

```python
def get_logger(name: str) -> logging.Logger:
    """Get configured logger for scriptlet"""
```

#### Resource Tracking

```python
@track_resources
def my_function():
    """Decorator to track execution time"""
    pass
```

### Context Server Protocol

#### TCP JSON Commands

**Set Operation:**
```json
{"op": "set", "path": "key.name", "value": {"data": "value"}}
```

**Get Operation:**
```json
{"op": "get", "path": "key.name"}
```

**Append Operation:**
```json
{"op": "append", "path": "list.name", "value": "item"}
```

**Delete Operation:**
```json
{"op": "delete", "path": "key.name"}
```

**Full Context:**
```json
{"op": "full"}
```

### Recipe Schema

#### Recipe Structure

```yaml
test_meta:
  test_id: str                    # Unique identifier
  tester: str                     # Who created the recipe
  description: str                # Human-readable description

steps:
  - idx: int                      # Step index
    name: str                     # Step name (unique)
    type: "python" | "shell"      # Step type
    module: str                   # Python module (python only)
    function: str                 # Class name (python only)
    script: str                   # Script path (shell only)
    args: dict | list             # Parameters
    depends_on: list[str]         # Dependencies (runner_v2)
    cache_key: str               # Cache identifier (runner_v2)
    success:                     # Success criteria (future)
      ctx_has_keys: list[str]    # Required context keys
```

### Standard Output Formats

#### Success Response

```json
{
  "status": "ok",
  "outputs": ["context.key1", "context.key2"],
  "artifact": "path/to/generated/file.txt",
  "records_processed": 1000
}
```

#### Error Response

```json
{
  "status": "error",
  "reason": "Human readable error message",
  "error_type": "validation",
  "exit_code": 1,
  "step": "step_name",
  "timestamp": "2024-03-15T10:30:00Z",
  "details": {"additional": "context"}
}
```

### Environment Variables

#### Framework Configuration

```bash
ORCHESTRATOR_DEBUG=1             # Enable debug output
PYTHONPATH=/path/to/RemNB        # Python module path
```

#### Context Server Configuration

```bash
SHCTX_PORT=8765                  # Context server port
DASH_HOST=127.0.0.1             # Dashboard host
DASH_PORT=8765                  # Dashboard port
```

#### Application Configuration

```bash
SCRIPTLET_CONFIG=config.json     # Scriptlet configuration file
API_TOKEN=secret                 # External API authentication
DATABASE_URL=postgresql://...    # Database connection string
```

---

This comprehensive user manual provides detailed guidance for using, extending, and integrating the RemNB Automation Orchestrator Framework. The framework's modular design and standardized interfaces make it suitable for a wide range of automation, testing, and data processing applications in both development and production environments.
