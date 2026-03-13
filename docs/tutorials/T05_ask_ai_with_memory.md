# T05 — Ask the AI with Your Notes as Hints 💬🧩

**Time:** 45–60 minutes  
**Difficulty:** ⭐⭐⭐ (medium — you are now combining everything you learned!)  
**Builds on:** [T04 — Find What You Learned](T04_find_what_you_learned.md)  
**What you need:** A terminal, AIOS running, Qdrant running, vault notes from T02 and T03

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ What "context" means and why it makes AI answers MUCH better
- ✅ How to find relevant vault notes for a question
- ✅ How to send those notes to the AI along with your question
- ✅ How to compare answers: "AI without context" vs "AI with context"
- ✅ How Qdrant automatically finds the best matching notes for you

---

## The Big Picture

Think about asking a friend: "What do you think about that?"

If your friend knows nothing about the topic, they give a vague answer.

But if your friend just read your 5 notes about ocean animals, they can give a **specific, detailed, thoughtful** answer.

That is **context**. When you give the AI your notes before asking a question, it uses those notes to make its answer smarter, more personal, and more accurate.

This is the whole point of building a vault — **it becomes the AI's context**.

```
Your Question + Your Vault Notes = Smart, Personalised Answer
```

---

## Part A — Compare: Same Question, No Context vs With Context

### Step 1 — Ask the AI a Question WITHOUT Context

```bash
curl -s -X POST http://localhost:8080/agent/shell/run \
  -H "Content-Type: application/json" \
  -d '{
    "command": "echo How do ocean animals survive without eating much food?",
    "username": "tester",
    "dry_run": false
  }'
```

This runs a plain echo — the AI just repeats your question as output. (In real use, this would go to the LLM query endpoint — we use shell echo here to see the raw pipeline working.)

For a real AI answer, classify the intent first:

```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "How do ocean animals survive without eating much food?"}'
```

You should see `"intent": "search_knowledge"` — the AI wants to search first!

---

### Step 2 — Find Relevant Notes MANUALLY (Your Context)

Let's find the best notes in your vault that are relevant to "ocean animals surviving without much food":

```bash
# Search for "food" in the vault
grep -rli "food\|eat\|diet\|krill\|survive" /home/tester/aios/obsidian/vault/

# Also check which notes are tagged with relevant tags
grep -rli "tags:.*food\|tags:.*diet\|tags:.*ocean" /home/tester/aios/obsidian/vault/
```

You should find:
- `Blue_Whale.md` — talks about eating krill
- `Ocean_Food_Chains.md` — explains food chains
- `Ocean_Animals_Overview.md` — links to all animal notes

---

### Step 3 — Build a Context Package

Now combine those notes into a single "context file":

```bash
cat /home/tester/aios/obsidian/vault/Blue_Whale.md \
    /home/tester/aios/obsidian/vault/Ocean_Food_Chains.md \
    /home/tester/aios/obsidian/vault/Ocean_Animals_Overview.md \
    > /tmp/my_context.txt

echo "=== Context package size ==="
wc -l /tmp/my_context.txt
echo "lines"
```

---

### Step 4 — Ask the AI WITH Context

Now ask the same question, but include the context notes:

```bash
QUESTION="How do ocean animals survive without eating much food?"
CONTEXT=$(cat /tmp/my_context.txt)

curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d "{\"text\": \"Using these notes as context:\\n$CONTEXT\\n\\nQuestion: $QUESTION\"}"
```

Notice the `intent` and `confidence`. The AI now has much more information to work with.

> 🧠 **What just happened?** You took your vault notes and "handed them" to the AI inside the same message as your question. This is the manual version of what Qdrant does automatically!

---

## Part B — Let Qdrant Find the Context Automatically

### Step 5 — Use the Vault's Automatic Context System

Instead of manually finding notes, let AIOS do it for you. The `aios-vault` diff command can show you what's been added recently — useful context for a question about recent learning:

```bash
aios-vault diff --since 2026-03-01
```

More importantly, after running `aios-vault review`, the semantic search in Qdrant means AIOS automatically pulls the **most relevant** notes for any question you ask through the search_knowledge endpoint.

Let's verify your notes are indexed in Qdrant:

```bash
curl -s http://localhost:6333/collections/knowledge/points/count | python3 -m json.tool
```

Look for the count. Each note you have indexed is stored as a "vector" — a mathematical description of its meaning. When you ask a question, Qdrant compares the question's meaning to every stored note's meaning and picks the closest matches.

---

### Step 6 — The Full Contextual Search Workflow

Here is the official workflow you will use going forward:

```bash
# === STEP A: Run vault review to index any new notes ===
aios-vault review

# === STEP B: Search your vault for relevant notes ===
grep -rli "YOUR_TOPIC" /home/tester/aios/obsidian/vault/

# === STEP C: Get vault insights to see topic clusters ===
aios-vault insights

# === STEP D: Manually read the most relevant notes ===
cat /home/tester/aios/obsidian/vault/MOST_RELEVANT_NOTE.md

# === STEP E: Ask your question with that context ===
# (Paste or include the note content with your question)
```

---

### Step 7 — Practice: Full Context Question

Let's do a full practice. Your question is:

> **"Why is the octopus considered unusual compared to other invertebrates?"**

**Step A** — Find relevant notes:
```bash
grep -rli "octopus\|invertebraten\|intelligence\|unusual" /home/tester/aios/obsidian/vault/
```

**Step B** — Read the most relevant note:
```bash
cat /home/tester/aios/obsidian/vault/Octopus_Intelligence.md
```

**Step C** — Build context and classify:
```bash
NOTE=$(cat /home/tester/aios/obsidian/vault/Octopus_Intelligence.md)

curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d "{\"text\": \"Context from my notes:\\n$NOTE\\n\\nQuestion: Why is the octopus unusual compared to other invertebrates?\"}"
```

**Step D** — Notice the `confidence` score is high because the notes give clear context.

---

## Step 8 — Write a "Context Summary" Note

After you ask an AI question and get a good answer, it is a great habit to **save the question + key insights back to the vault**:

```bash
cat > /home/tester/aios/obsidian/vault/Why_Octopus_Is_Unusual.md << 'ENDOFNOTE'
---
title: Why Octopus Is Unusual
tags: [ocean, invertebrates, intelligence, question-answer]
topic: marine biology
created: 2026-03-12
question: "Why is the octopus unusual compared to other invertebrates?"
---

# Why Is the Octopus Unusual?

This note was created to answer my question about octopus intelligence.

## Key Answer
Most invertebrates (animals without backbones) have very small, simple nervous systems.
The octopus is unusual because it has a very large, complex brain for its body size.
Two-thirds of its neurons are actually in its arms, not its head!

## Why This Matters
It challenges the idea that only animals with backbones can be intelligent.
This connects to [[Animal Intelligence]] and the broader question of
what intelligence even means.

## Sources Used as Context
- [[Octopus_Intelligence]] — my primary vault note
- [[Ocean Animals Overview]] — overview of all ocean animal notes

ENDOFNOTE
```

---

## 🏆 Mission Complete!

| Step | You Did | Why It Matters |
|---|---|---|
| Step 1 | Asked AI without context | Baseline — generic answer |
| Step 2 | Found relevant vault notes manually | Pre-search: knowing what you know |
| Step 3 | Built a context package | Combined notes into one input |
| Step 4 | Asked AI WITH context | Richer, more specific answer |
| Step 5 | Verified Qdrant indexing | Confirmed automatic context works |
| Step 6 | Learned the full contextual workflow | Repeatable process for any topic |
| Step 7 | Practised a full context question | Put it all together |
| Step 8 | Saved the Q&A back to vault | The vault grows smarter every time |

---

## The Big Insight 🌟

> **The vault is not just a place to store notes. It is the AI's long-term memory.**
>
> Every note you add makes future AI answers better. Every question you ask (and save) makes your vault richer. Over time, the vault becomes a personalised knowledge system built around YOUR learning.

---

## 🔭 Try It Yourself!

1. Choose ONE of your "empty links" from T03 (like `Animal_Intelligence.md`)
2. Search your vault for any related notes (grep + tags)
3. Use those notes as context and ask AIOS a question about that topic
4. Save the AI's answer as a new vault note

---

## ➡️ Final Tutorial

[T06 — The Big Research Adventure →](T06_big_research_adventure.md)

The ultimate challenge: start with a brand new topic you know nothing about, and by the end you will have a fully linked vault topic with notes, insights, and AI-assisted summaries!
