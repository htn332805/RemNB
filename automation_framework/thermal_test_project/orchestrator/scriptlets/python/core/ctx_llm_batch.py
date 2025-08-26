"""
ctx_llm_batch.py
----------------
Sends multiple ctx data items to an LLM API and stores results.
Batch LLM calls for multiple ctx keys or data slices.

Usage:
    run(ctx, {
        "keys": ["foo", "bar"],
        "out": "summaries",
        "prompt": "Summarize this data."
    })

Description:
    - Stores a dict of summaries in ctx[out].
"""
import openai

def run(ctx, params):
    keys = params["keys"]
    out = params["out"]
    prompt = params["prompt"]
    summaries = {}
    for key in keys:
        data = ctx[key]
        response = openai.ChatCompletion.create(
            model="gpt-3.5-turbo",
            messages=[
                {"role": "system", "content": prompt},
                {"role": "user", "content": str(data)}
            ]
        )
        summaries[key] = response.choices[0].message["content"]
    ctx[out] = summaries
    return {"summarized": keys}