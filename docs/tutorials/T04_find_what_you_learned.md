# T04 — Find What You Already Know 🔍🧠

**Time:** 30–45 minutes  
**Difficulty:** ⭐⭐ (easy — learning two types of searching)  
**Builds on:** [T03 — Connect Your Notes](T03_connect_your_notes.md)  
**What you need:** A terminal, AIOS running, your ocean animal vault notes

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ The difference between **keyword search** and **meaning search**
- ✅ How to search your vault with the `aios-vault` tool
- ✅ How to use Qdrant (the meaning memory) to find similar topics
- ✅ How to use search results as a list of "what I already know"

---

## The Big Picture

Imagine your vault has 500 notes about science. You want to find everything about **animals that use light** (like the anglerfish's glowing lure, or fireflies).

**Keyword search** looks for the exact words you type. If you type "bioluminescence" it finds only notes with that exact word.

**Meaning search** (also called semantic search) looks for notes that are *about the same idea*, even if they use different words. If you type "glowing animals" it finds notes about bioluminescence, fireflies, deep-sea fish — anything related in meaning.

AIOS uses **both** kinds of searching. Today you will learn to use each one.

---

## Step 1 — Keyword Search (Simple)

Let's find all vault notes that mention "whale":

```bash
grep -ri "whale" /home/tester/aios/obsidian/vault/ --include="*.md" -l
```

- `-r` means "search inside all folders"
- `-i` means "ignore upper/lowercase"
- `-l` means "just tell me the file names, not every line"

You should see:
```
/home/tester/aios/obsidian/vault/Blue_Whale.md
/home/tester/aios/obsidian/vault/Ocean_Animals_Overview.md
/home/tester/aios/obsidian/vault/Ocean_Food_Chains.md
```

Now search for a word that appears in only ONE note:
```bash
grep -ri "krill" /home/tester/aios/obsidian/vault/ --include="*.md" -l
```

---

## Step 2 — Tag Search (Super Useful!)

Remember the `tags:` line we added at the top of every note? Tags let you find ALL notes on a theme in one command:

```bash
grep -ri "tags:.*ocean" /home/tester/aios/obsidian/vault/ --include="*.md" -l
```

Try searching for different tags:
```bash
# Find all notes tagged with "intelligence"
grep -ri "tags:.*intelligence" /home/tester/aios/obsidian/vault/ --include="*.md" -l

# Find all notes tagged with "biology"
grep -ri "tags:.*biology" /home/tester/aios/obsidian/vault/ --include="*.md" -l

# Find all notes from your ocean_animals.txt source
grep -ri "source:.*ocean_animals" /home/tester/aios/obsidian/vault/ --include="*.md" -l
```

> 💡 This is why adding good tags to your notes is so important! Tags are like labels on jars — you can find the right jar instantly.

---

## Step 3 — The Vault Insights Command

`aios-vault insights` is a powerful tool that reads your whole vault and gives you a smart report:

```bash
aios-vault insights
```

> ⏳ Wait a few seconds — the AI is reading everything!

### What you should see:
```
=== Vault Insights ===

Total notes: 5
Topics found: marine biology

Most connected notes (by link count):
  1. Ocean_Animals_Overview  (links: 4)
  2. Blue_Whale               (links: 3)
  3. Octopus_Intelligence     (links: 3)

Most referenced notes (linked-to most often):
  1. Ocean_Animals_Overview
  2. Ocean_Food_Chains
  3. Blue_Whale

Suggested reading order for marine biology:
  Ocean_Animals_Overview → Blue_Whale → Ocean_Food_Chains →
  Octopus_Intelligence → Clownfish_Life_Cycle

Potential knowledge gaps (links to non-existent notes):
  - Animal Intelligence
  - Camouflage in Nature
  - Deep Sea Ecosystems
  - Sea Anemone Symbiosis
  - Whale Communication
```

This shows you:
- Which notes are the **most important** (hubs of the web)
- A logical **reading order**
- What topics you could **explore next** (the gaps)

---

## Step 4 — Meaning Search with Qdrant

This is the MOST powerful kind of search. Qdrant stores the **meaning** of your notes. When your notes are indexed (added to Qdrant), you can search by idea, not just by word.

First, let's check how many items are stored in the `knowledge` memory bucket:

```bash
curl -s http://localhost:6333/collections/knowledge | python3 -m json.tool
```

Look for `"vectors_count"` in the output. If it says `0`, your notes have not been indexed yet. Let's do that:

```bash
aios-vault review
```

This scans the vault and sends any new notes into Qdrant's memory.

Now check again — `vectors_count` should be greater than 0.

---

## Step 5 — Ask AIOS to Search by Meaning

Now let's use meaning search. We'll ask AIOS to find notes related to "animals that are surprisingly intelligent":

```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "Search my knowledge for animals that are surprisingly intelligent"}'
```

You should see `"intent": "search_knowledge"` — it recognised this as a knowledge search!

Now let's do the actual meaning search through the vault CLI:

```bash
aios-vault insights
```

Look for the section that mentions any of your notes related to intelligence. If you have `Octopus_Intelligence.md` and `Animal_Intelligence.md`, they should both appear near each other in the insights.

---

## Step 6 — Save a Search Result for Later

Let's save the insights report so you can keep it:

```bash
aios-vault insights > workspace/my_documents/vault_insights_report.txt
echo "=== Saved! ==="
cat workspace/my_documents/vault_insights_report.txt
```

---

## Step 7 — The "What Do I Know?" Workflow

Here is a workflow you can use any time you start studying a new topic:

```
1. Run: aios-vault insights
   → Find what you ALREADY have about this topic

2. Run: grep -ri "KEYWORD" /home/tester/aios/obsidian/vault/ -l
   → Find specific notes by word

3. Run: grep -ri "tags:.*TOPIC" /home/tester/aios/obsidian/vault/ -l
   → Find ALL notes with this tag

4. Open the notes you found and READ them
   → Refresh your memory before asking the AI a new question
```

This is called **pre-search** — checking what you know before you ask for new help.

---

## 🏆 Mission Complete!

| Step | You Did | Why It Matters |
|---|---|---|
| Step 1 | Keyword search with `grep` | Find notes by exact words |
| Step 2 | Tag search | Find all notes on a theme instantly |
| Step 3 | Ran `aios-vault insights` | Got a whole-vault intelligence report |
| Step 4 | Checked Qdrant memory | Confirmed meaning-search is ready |
| Step 5 | Searched by meaning | Found notes by idea, not just words |
| Step 6 | Saved a search report | Permanent record of what you know |
| Step 7 | Learned the "What do I know?" workflow | Repeatable process for any topic |

---

## Two Types of Search — Side by Side

| | Keyword Search | Meaning Search |
|---|---|---|
| **How it works** | Exact word match | Understands similar meanings |
| **Speed** | Instant | Fast (a second or two) |
| **Best for** | Specific terms, names | Ideas, concepts, themes |
| **Example** | Find "krill" | Find "animals that eat tiny things" |
| **Tool** | `grep` | Qdrant via `aios-vault` |

---

## 🔭 Try It Yourself!

1. Add a new note about any topic — for example, `Sharks.md`
2. Add tags like `[predator, ocean, teeth]`
3. Run `aios-vault review` to index the new note
4. Run `aios-vault insights` and see if your new note appears

---

## ➡️ Next Tutorial

[T05 — Ask the AI with Your Notes as Hints →](T05_ask_ai_with_memory.md)

You have a vault full of knowledge. Now you will learn how to **give that knowledge to the AI** when you ask it a question, so it gives you a much smarter, more personalised answer!
