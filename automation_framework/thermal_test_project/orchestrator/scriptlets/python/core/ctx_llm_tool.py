"""
ctx_llm_tool.py
---------------
Uses LLM to generate code, clean data, or transform ctx content.
Use LLM as a tool for code generation, data cleaning, or transformation.

Usage:
    run(ctx, {
        "key": "raw_data",
        "out": "cleaned_data",
        "instruction": "Clean and normalize this data as a list of dicts."
    })

Description:
    - LLM returns Python code or cleaned data, which is executed or parsed.
    - Use with caution: always validate LLM output!
"""
import openai

def run(ctx, params):
    key = params["key"]
    out = params["out"]
    instruction = params["instruction"]
    data = ctx[key]
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": instruction},
            {"role": "user", "content": str(data)}
        ]
    )
    # For safety, just store the LLM output as text
    ctx[out] = response.choices[0].message["content"]
    return {"llm_output": out}