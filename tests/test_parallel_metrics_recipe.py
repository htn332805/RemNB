"""
Test for the parallel metrics computation recipe.
"""
import pytest
import pathlib
from orchestrator.runner_v2 import run_recipe_v2

def test_parallel_metrics_recipe():
    """Test that the parallel metrics recipe works correctly."""
    recipe_path = "orchestrator/recipes/parallel_metrics_recipe.yaml"
    
    # Run the recipe
    ctx = run_recipe_v2(recipe_path, set(), set(), None)
    
    # Verify all expected outputs are present
    assert ctx.get("numbers.raw_v1") is not None, "Raw numbers should be loaded"
    assert ctx.get("numbers.normalized_v1") is not None, "Numbers should be normalized"
    assert ctx.get("numbers.stats_v1") is not None, "Basic stats should be computed"
    
    # Verify parallel metrics computation
    assert ctx.get("metrics.mean_v1") is not None, "Mean should be computed"
    assert ctx.get("metrics.median_v1") is not None, "Median should be computed"
    assert ctx.get("metrics.std_v1") is not None, "Standard deviation should be computed"
    
    # Verify aggregation
    agg = ctx.get("metrics.aggregate_v1")
    assert agg is not None, "Metrics should be aggregated"
    assert "mean" in agg, "Aggregated metrics should contain mean"
    assert "median" in agg, "Aggregated metrics should contain median"
    assert "std" in agg, "Aggregated metrics should contain standard deviation"
    
    # Verify summary conversion
    summary = ctx.get("metrics.summary_v1")
    assert summary is not None, "Summary should be created"
    assert "count" in summary, "Summary should contain count"
    assert "mean" in summary, "Summary should contain mean"
    assert "min" in summary, "Summary should contain min"
    assert "max" in summary, "Summary should contain max"
    
    # Verify report generation
    assert ctx.get("report.path_v1") is not None, "Report should be generated"
    
    # Check that report file exists
    report_path = pathlib.Path("orchestrator/Data/report.md")
    assert report_path.exists(), "Report file should exist"
    
    # Verify report content
    report_content = report_path.read_text()
    assert "Count: 20" in report_content, "Report should show correct count"
    assert "Mean:" in report_content, "Report should show mean"
    assert "Min:" in report_content, "Report should show min"
    assert "Max:" in report_content, "Report should show max"

def test_parallel_scriptlet_flags():
    """Verify that the individual metric scriptlets are marked as parallelizable."""
    from orchestrator.scriptlets.python.steps.compute_mean import ComputeMean
    from orchestrator.scriptlets.python.steps.compute_median import ComputeMedian  
    from orchestrator.scriptlets.python.steps.compute_std import ComputeStd
    
    assert ComputeMean.parallelizable == True, "ComputeMean should be parallelizable"
    assert ComputeMedian.parallelizable == True, "ComputeMedian should be parallelizable"
    assert ComputeStd.parallelizable == True, "ComputeStd should be parallelizable"

if __name__ == "__main__":
    test_parallel_metrics_recipe()
    test_parallel_scriptlet_flags()
    print("All tests passed!")