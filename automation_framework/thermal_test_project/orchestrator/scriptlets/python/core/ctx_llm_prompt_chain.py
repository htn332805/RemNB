"""
ctx_llm_prompt_chain.py
-----------------------
Chains multiple LLM prompts, passing outputs as inputs, and stores results in ctx.
Chain multiple LLM prompts, passing outputs as inputs, and store all results in ctx.

Usage:
    run(ctx, {
        "chain": [
            {"prompt": "Summarize this data.", "input_key": "raw_data", "out": "summary"},
            {"prompt": "Generate 3 questions about this summary.", "input_key": "summary", "out": "questions"}
        ],
        "model": "gpt-3.5-turbo"
    })

Description:
    - Each step uses the previous output as input.
    - All outputs are stored in ctx.
"""
import openai

def run(ctx, params):
    chain = params["chain"]
    model = params.get("model", "gpt-3.5-turbo")
    for step in chain:
        input_data = ctx[step["input_key"]]
        response = openai.ChatCompletion.create(
            model=model,
            messages=[
                {"role": "system", "content": step["prompt"]},
                {"role": "user", "content": str(input_data)}
            ]
        )
        ctx[step["out"]] = response.choices[0].message["content"]
    return {"outputs": [step["out"] for step in chain]}