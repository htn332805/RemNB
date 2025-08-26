# Orchestrator Automation Framework Learning Curriculum

Welcome! This curriculum is designed to take you from zero to expert in using, extending, and mastering the orchestrator automation framework in this workspace. Each exercise builds on the previous, with clear explanations, hints, and code samples. By the end, you’ll be able to automate complex tasks, create reusable scriptlets, debug and optimize workflows, and build interactive dashboards—even if you have no programming background.

---

## **Module 1: Getting Started**

### 1. **Workspace Tour**
- **Objective:** Understand the folder and file structure.
- **Exercise:** Open the workspace and explore each folder. Write down what you think each folder is for.
- **Hint:** Refer to `README.md` and `UserGuide.md`.

### 2. **Install Requirements**
- **Objective:** Install all dependencies.
- **Exercise:** Run `pip install -r requirements.txt` in your terminal.
- **Hint:** If you get an error, check your Python version (`python --version`).

### 3. **Run Your First Recipe**
- **Objective:** Execute a sample recipe.
- **Exercise:** Run `python runner.py --recipe recipes/demo_screen_dash_ctx.yaml`.
- **Hint:** Watch the console output and check `Logs/` for logs.

### 4. **Inspect Data, Logs, and Assets**
- **Objective:** Find where outputs are stored.
- **Exercise:** After running a recipe, open `Data/`, `Logs/`, and `Assets/graph.png`.
- **Hint:** Use your file explorer or `ls` command.

---

## **Module 2: Understanding Recipes**

### 5. **Read a Recipe File**
- **Objective:** Understand the YAML recipe format.
- **Exercise:** Open `recipes/demo_screen_dash_ctx.yaml` and read each step.
- **Hint:** Comments in the YAML explain each step.

### 6. **Identify Step Types**
- **Objective:** Learn the difference between Python and shell steps.
- **Exercise:** In the recipe, list which steps are `type: python` and which are `type: shell`.

### 7. **Trace Step Execution**
- **Objective:** Follow the flow of steps.
- **Exercise:** Draw a flowchart of the steps and their dependencies.
- **Hint:** Use the `idx` and `depends_on` fields.

### 8. **Modify Step Order**
- **Objective:** Change the order of execution.
- **Exercise:** Swap two steps’ `idx` values and rerun the recipe.
- **Hint:** Observe how the execution order changes.

### 9. **Add a New Step**
- **Objective:** Insert a new step into a recipe.
- **Exercise:** Add a step that prints “Hello, Automation!” using a shell scriptlet.
- **Hint:** Use `echo "Hello, Automation!"` as the command.

### 10. **Skip and Only Steps**
- **Objective:** Use CLI filters.
- **Exercise:** Run the recipe with `--skip` and `--only` flags.
- **Hint:** Try `python runner.py --only create_screen_with_log`.

---

## **Module 3: Shared Context (ctx)**

### 11. **What is ctx?**
- **Objective:** Understand the shared in-memory context.
- **Exercise:** Read `context.py` and write a summary of what ctx does.

### 12. **View ctx Changes**
- **Objective:** See how ctx changes during execution.
- **Exercise:** Run a recipe in debug mode and inspect `Logs/ctx_debug.log`.

### 13. **Access ctx in Python**
- **Objective:** Read and write ctx in a Python scriptlet.
- **Exercise:** Modify a Python scriptlet to print the current ctx.
- **Hint:** Use `print(ctx)`.

### 14. **Access ctx in Shell**
- **Objective:** Pass ctx values to a shell scriptlet.
- **Exercise:** Use `expose_for_shell` to export a ctx key as an environment variable.
- **Hint:** See `context.py` for how to use `expose_for_shell`.

### 15. **Add a New ctx Key**
- **Objective:** Initialize a new key in ctx.
- **Exercise:** Use `ctx_init.py` to add a key called `my_data`.
- **Hint:** `python scriptlets/python/steps/ctx_init.py --keys my_data`

---

## **Module 4: Creating and Modifying Scriptlets**

### 16. **Run a Scriptlet as CLI**
- **Objective:** Execute a scriptlet directly.
- **Exercise:** Run `python scriptlets/python/steps/ctx_row_updater.py --key test --n 2 --interval_sec 1`.

### 17. **Write a Simple Python Scriptlet**
- **Objective:** Create a scriptlet that adds two numbers and stores the result in ctx.
- **Exercise:** Write `add_numbers.py` in `scriptlets/python/steps/`.
- **Hint:** Use `def run(ctx, params):`.

### 18. **Write a Simple Shell Scriptlet**
- **Objective:** Create a shell scriptlet that prints the current date.
- **Exercise:** Write `print_date.sh` in `scriptlets/shell/`.

### 19. **Document Your Scriptlet**
- **Objective:** Add a docstring with usage, description, and limitations.
- **Exercise:** Add a docstring to your `add_numbers.py`.

### 20. **Test Your Scriptlet**
- **Objective:** Write a test for your scriptlet.
- **Exercise:** Add a test in `tests/` that imports and runs your scriptlet.

---

## **Module 5: Debugging and Troubleshooting**

### 21. **Read Error Logs**
- **Objective:** Find and interpret errors.
- **Exercise:** Cause a step to fail and read `Logs/error.log`.

### 22. **Use Debug Mode**
- **Objective:** Enable verbose logging.
- **Exercise:** Create a file named `runner.debug` and rerun a recipe.

### 23. **Trace ctx Modifications**
- **Objective:** See who/what/when modified ctx.
- **Exercise:** Read `Logs/ctx_debug.log` and match changes to steps.

### 24. **Resume from Failure**
- **Objective:** Resume a recipe after a failure.
- **Exercise:** Use the `--resume-from` flag.

### 25. **Check Resource Usage**
- **Objective:** Monitor memory, CPU, and disk usage.
- **Exercise:** Observe the resource report printed after each step.

---

## **Module 6: Advanced Recipes and Automation**

### 26. **Create a Multi-Step Recipe**
- **Objective:** Build a recipe with at least 5 steps.
- **Exercise:** Design a recipe that processes data, computes stats, and generates a report.

### 27. **Add Dependencies**
- **Objective:** Use `depends_on` to enforce step order.
- **Exercise:** Make one step depend on the completion of another.

### 28. **Use Background Steps**
- **Objective:** Run a step in the background.
- **Exercise:** Mark a step with `background: true` in your recipe.

### 29. **Use Parallel Steps**
- **Objective:** Run steps in parallel.
- **Exercise:** Mark two steps with `parallel: true` and observe execution.

### 30. **Capture Step Output**
- **Objective:** Store step output in ctx.
- **Exercise:** Use `stdout_to_ctx` in a shell step.

---

## **Module 7: Building and Customizing Dash Dashboards**

### 31. **Run the Dash App**
- **Objective:** Launch the Dash app and view it in your browser.
- **Exercise:** Run `python scriptlets/python/apps/hello_dash.py --port 8050 --ctx_key new_row`.

### 32. **Understand Dash Layout**
- **Objective:** Read the layout code in `hello_dash.py`.
- **Exercise:** Identify each component in the layout.

### 33. **Add a New Graph**
- **Objective:** Add a second graph to the Dash app.
- **Exercise:** Modify the layout and callbacks to include another plot.

### 34. **Use Dash Bootstrap Components**
- **Objective:** Add a Bootstrap-styled button.
- **Exercise:** Install `dash-bootstrap-components` and add a button to the layout.
- **Hint:** See [dash-bootstrap-components docs](https://dash-bootstrap-components.opensource.faculty.ai/).

### 35. **Make a Responsive Layout**
- **Objective:** Use Bootstrap grid for layout.
- **Exercise:** Arrange components in two columns.

### 36. **Add Interactivity**
- **Objective:** Add a dropdown to select which ctx key to plot.
- **Exercise:** Update the callback to use the dropdown value.

### 37. **Display Data Table**
- **Objective:** Show ctx data in a table.
- **Exercise:** Use `dash_table.DataTable` to display rows.

### 38. **Add Alerts and Notifications**
- **Objective:** Show a Bootstrap alert when ctx changes.
- **Exercise:** Use `dbc.Alert` and update it in a callback.

### 39. **Add Modal Dialogs**
- **Objective:** Show a modal with details on button click.
- **Exercise:** Use `dbc.Modal` and callbacks.

### 40. **Theme Your Dashboard**
- **Objective:** Apply a Bootstrap theme.
- **Exercise:** Change the theme and observe the effect.

---

## **Module 8: Real-World Automation Challenges**

### 41. **Automate Data Cleaning**
- **Objective:** Write a scriptlet to clean a CSV file.
- **Exercise:** Use Python’s `csv` module to remove empty rows.

### 42. **Automate Data Analysis**
- **Objective:** Write a scriptlet to compute summary statistics.
- **Exercise:** Use `pandas` to compute mean and stddev.

### 43. **Automate Report Generation**
- **Objective:** Generate a Word report from ctx data.
- **Exercise:** Use `python-docx` and the template in `templates/`.

### 44. **Automate Email Notification**
- **Objective:** Send an email when a recipe completes.
- **Exercise:** Use Python’s `smtplib` in a scriptlet.

### 45. **Automate File Upload**
- **Objective:** Upload a file to a server after processing.
- **Exercise:** Use Python’s `requests` library.

### 46. **Secure Your Dashboard**
- **Objective:** Add password protection to the Dash app.
- **Exercise:** Use `dash-auth` or Flask login.

### 47. **Audit and Log All Actions**
- **Objective:** Ensure every ctx change and step is logged.
- **Exercise:** Review logs and add missing logging.

### 48. **Optimize for Performance**
- **Objective:** Profile and optimize a slow step.
- **Exercise:** Use `cProfile` and `psutil` to find bottlenecks.

### 49. **Handle Large Data**
- **Objective:** Modify a scriptlet to process data in batches.
- **Exercise:** Use generators or chunked reading.

### 50. **Build a Custom Dashboard with All Major Dash Bootstrap Components**
- **Objective:** Create a dashboard using:
    - Navbar
    - Sidebar
    - Cards
    - Alerts
    - Modals
    - Tabs
    - Accordions
    - Tooltips
    - Spinners
    - DataTable
    - Graphs
    - Forms
    - Pagination
    - Toasts
- **Exercise:** For each component, add it to your dashboard, connect it to ctx, and make it interactive.
- **Hint:** See [dash-bootstrap-components gallery](https://dash-bootstrap-components.opensource.faculty.ai/examples/simple-app/).

---

## **Capstone Project: Your Own Automation Pipeline**

### 51. **Design Your Own Recipe**
- **Objective:** Plan an automation workflow for a real-world task (e.g., data ETL, monitoring, reporting).
- **Exercise:** Write a recipe YAML with at least 8 steps, including Python and shell scriptlets, ctx sharing, and a Dash dashboard.

### 52. **Implement Custom Scriptlets**
- **Objective:** Write at least two new reusable scriptlets for your workflow.

### 53. **Test and Debug Your Pipeline**
- **Objective:** Run, debug, and optimize your pipeline using all the tools and techniques learned.

### 54. **Document and Share**
- **Objective:** Write documentation for your recipe and scriptlets, following the workspace standards.

---

**Congratulations!**  
You are now an expert in orchestrator automation, recipe authoring, scriptlet development, debugging, and dashboarding.  
Continue to explore, automate, and share your solutions!

---

# Orchestrator Framework: Advanced Learning & Real-World Recipe Design Curriculum

---

## **Module 9: Designing Your Own Automation Recipe**

### 55. **Define Your Automation Goal**
- **Objective:** Clearly state the problem you want to automate (e.g., data cleaning, monitoring, reporting).
- **Exercise:** Write a one-sentence goal for your automation.
- **Hint:** Example: “Automate daily sales data cleaning and dashboard reporting.”

### 56. **List Required Steps**
- **Objective:** Break your goal into atomic steps (e.g., fetch data, clean data, analyze, visualize, notify).
- **Exercise:** Write down each step as a short phrase.
- **Hint:** Use verbs: “Download CSV”, “Remove duplicates”, “Compute stats”, “Send email”.

### 57. **Choose Step Types**
- **Objective:** Decide which steps are Python, which are shell, and which need a dashboard.
- **Exercise:** Mark each step as `python`, `shell`, or `dash`.

### 58. **Draft Your Recipe YAML**
- **Objective:** Write a draft YAML recipe with `test_meta` and `steps`.
- **Exercise:** Use `recipes/demo_screen_dash_ctx.yaml` as a template.
- **Hint:** Start with 3-5 steps, add more as you go.

### 59. **Add Dependencies**
- **Objective:** Specify which steps depend on others.
- **Exercise:** Use the `depends_on` field to enforce order.

### 60. **Integrate Existing Scriptlets**
- **Objective:** Reuse scriptlets from `scriptlets/python/steps/` and `scriptlets/shell/`.
- **Exercise:** For each step, specify the `module` or `cmd` and `function` or `args`.

### 61. **Test Your Recipe**
- **Objective:** Run your recipe and observe the output.
- **Exercise:** Use `python runner.py --recipe your_recipe.yaml`.

### 62. **Debug and Refine**
- **Objective:** Fix errors, add missing steps, and adjust parameters.
- **Exercise:** Use debug mode and logs to troubleshoot.

### 63. **Document Your Recipe**
- **Objective:** Add comments and a description to your YAML.
- **Exercise:** Explain what each step does and why.

---

## **Module 10: Real-World Dash Bootstrap Interactive Charting**

### 64. **Install Dash Bootstrap Components**
- **Objective:** Add Bootstrap styling to your dashboards.
- **Exercise:** Run `pip install dash-bootstrap-components`.

### 65. **Create a Bootstrap-Styled Line Chart**
- **Objective:** Use `dbc.Container`, `dbc.Row`, and `dbc.Col` to layout a line chart.
- **Exercise:** Modify `hello_dash.py` to use Bootstrap layout.

### 66. **Add a Navbar**
- **Objective:** Add a `dbc.NavbarSimple` to your dashboard.
- **Exercise:** Place navigation links for “Home”, “Data”, “About”.

### 67. **Add a Sidebar for Controls**
- **Objective:** Use `dbc.Sidebar` for dropdowns and buttons.
- **Exercise:** Move column selection to the sidebar.

### 68. **Add Tabs for Multiple Charts**
- **Objective:** Use `dbc.Tabs` to switch between different chart types.
- **Exercise:** Add a tab for a bar chart and a tab for a scatter plot.

### 69. **Add a DataTable**
- **Objective:** Display ctx data in a `dash_table.DataTable`.
- **Exercise:** Add a table below your chart.

### 70. **Add Interactive Filters**
- **Objective:** Use `dbc.Input` and `dbc.Button` to filter data.
- **Exercise:** Add a date range filter for your chart.

### 71. **Add Alerts and Notifications**
- **Objective:** Show a `dbc.Alert` when new data arrives.
- **Exercise:** Trigger an alert in a callback when ctx changes.

### 72. **Add a Modal for Details**
- **Objective:** Use `dbc.Modal` to show detailed info on row click.
- **Exercise:** Add a callback to open the modal.

### 73. **Add Accordions for Collapsible Sections**
- **Objective:** Use `dbc.Accordion` for advanced options.
- **Exercise:** Place advanced chart settings in an accordion.

### 74. **Add Tooltips and Popovers**
- **Objective:** Use `dbc.Tooltip` to explain controls.
- **Exercise:** Add tooltips to all buttons and inputs.

### 75. **Add Spinners for Loading States**
- **Objective:** Use `dbc.Spinner` to indicate loading.
- **Exercise:** Show a spinner while data is loading.

### 76. **Add Toasts for Success/Failure**
- **Objective:** Use `dbc.Toast` for notifications.
- **Exercise:** Show a toast when a step completes.

### 77. **Add Pagination to DataTable**
- **Objective:** Handle large datasets with pagination.
- **Exercise:** Enable pagination in your DataTable.

### 78. **Add Download Button**
- **Objective:** Allow users to download chart data as CSV.
- **Exercise:** Use `dcc.Download` and a callback.

### 79. **Add Custom Themes**
- **Objective:** Apply a custom Bootstrap theme.
- **Exercise:** Try different themes and observe the effect.

### 80. **Deploy Your Dashboard**
- **Objective:** Deploy your Dash app to a server.
- **Exercise:** Use `gunicorn` or `heroku` for deployment.

---

## **Module 11: Advanced Debugging and Troubleshooting**

### 81. **Simulate a Step Failure**
- **Objective:** Cause a step to fail and observe error handling.
- **Exercise:** Intentionally break a scriptlet and run the recipe.

### 82. **Analyze Error Logs**
- **Objective:** Read and interpret `Logs/error.log`.
- **Exercise:** Find the cause of the failure.

### 83. **Use Debug Mode for ctx Tracking**
- **Objective:** Enable debug mode and watch `Logs/ctx_debug.log`.
- **Exercise:** Track ctx changes step by step.

### 84. **Resume from Failure**
- **Objective:** Use the `--resume-from` flag to continue after fixing an error.
- **Exercise:** Fix the error and resume the recipe.

### 85. **Profile Resource Usage**
- **Objective:** Use the resource report to find bottlenecks.
- **Exercise:** Identify which step uses the most memory or CPU.

### 86. **Trace Step Dependencies**
- **Objective:** Use the digraph to visualize step dependencies.
- **Exercise:** Open `Assets/graph.png` after each run.

### 87. **Add More Logging**
- **Objective:** Add custom logging to a scriptlet.
- **Exercise:** Use `print()` or the logging module.

### 88. **Test with Large Data**
- **Objective:** Run a recipe with a large dataset.
- **Exercise:** Observe performance and troubleshoot slow steps.

### 89. **Handle Unexpected ctx Content**
- **Objective:** Add type checks and error handling for ctx values.
- **Exercise:** Modify a scriptlet to check ctx types.

### 90. **Write Unit Tests for Scriptlets**
- **Objective:** Add tests for your custom scriptlets.
- **Exercise:** Use `pytest` and see `tests/` for examples.

---

## **Module 12: Solidifying Framework Concepts**

### 91. **Review the Framework Layout**
- **Objective:** Draw a diagram of the workspace structure.
- **Exercise:** Label where each type of file/scriptlet goes.

### 92. **Explain ctx Sharing**
- **Objective:** Write a paragraph explaining how ctx enables step communication.

### 93. **Explain Recipe Flow**
- **Objective:** Describe how runner.py executes a recipe step by step.

### 94. **Explain Scriptlet Reusability**
- **Objective:** List three ways to reuse a scriptlet in different recipes.

### 95. **Explain Debugging Workflow**
- **Objective:** Write out the steps you’d take to debug a failed recipe.

### 96. **Create a New Scriptlet**
- **Objective:** Write a scriptlet that computes the median of a list in ctx.

### 97. **Integrate Your Scriptlet into a Recipe**
- **Objective:** Add your new scriptlet to a recipe and run it.

### 98. **Document Your Scriptlet**
- **Objective:** Write a docstring and usage example for your scriptlet.

### 99. **Share Your Recipe and Scriptlet**
- **Objective:** Present your work to a peer or mentor for feedback.

### 100. **Reflect and Plan Next Steps**
- **Objective:** Write a short reflection on what you learned and what you want to automate next.

---

**By completing these modules, you will have hands-on experience with every aspect of the orchestrator framework, from recipe design to advanced Dash dashboards, debugging, and extensibility. You’ll be able to confidently create, debug, and optimize your own automation workflows and dashboards—even as a non-programmer!**