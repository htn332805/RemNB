"""
ctx_ai_llm.py
-------------
Sends ctx data to an LLM API (e.g., OpenAI) and stores the summary.

Usage:
    run(ctx, {"key": "foo", "out": "foo_summary", "prompt": "Summarize this data."})

Description:
    - Requires openai package and API key.
    - Stores the LLM response in ctx[out].
"""
import openai

def run(ctx, params):
    key = params["key"]
    out = params["out"]
    prompt = params["prompt"]
    data = ctx[key]
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": prompt},
            {"role": "user", "content": str(data)}
        ]
    )
    ctx[out] = response.choices[0].message["content"]
    return {"summary": out}