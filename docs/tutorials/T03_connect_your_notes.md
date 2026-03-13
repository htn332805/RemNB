# T03 — Connect Your Notes 🕸️🔗

**Time:** 30–45 minutes  
**Difficulty:** ⭐⭐ (easy — thinking + typing)  
**Builds on:** [T02 — Save Notes to Your Vault](T02_save_notes_to_vault.md)  
**What you need:** A terminal, the 4 vault notes you made in T02

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ Why linking notes is so powerful (the "spider web" idea)
- ✅ How to check which notes link to which
- ✅ How to add a link from one note to another
- ✅ How to ask AIOS to suggest connections you might have missed

---

## The Big Picture

Right now your vault notes are like islands — they exist, but there are no bridges between them.

A vault with connected notes is like a **spider web**. Each note is a point on the web. A link is a thread. When you pull one thread, you find more threads. When you search for one idea, you find related ideas automatically.

The more connected your notes are, the **smarter** your vault becomes.

```
Blue Whale ──────────► Ocean Food Chains
     │                        │
     ▼                        ▼
Ocean Animals Overview ◄── Clownfish Life Cycle
     │
     ▼
Octopus Intelligence ──► Animal Intelligence
```

---

## Step 1 — See Which Notes Link to What

Let's see all the links in your vault right now:

```bash
grep -rh "\[\[" /home/tester/aios/obsidian/vault/ | sort | uniq
```

This looks through every note and finds every `[[link]]` inside them.

You should see a long list of links like:
```
- [[Animal Intelligence]]
- [[Blue Whale]]
- [[Camouflage in Nature]]
- [[Clownfish Life Cycle]]
- [[Ocean Animals Overview]]
- [[Ocean Food Chains]]
- [[Octopus Intelligence]]
- [[Sea Anemone Symbiosis]]
- [[Whale Communication]]
```

Some of these notes exist (like `Blue_Whale.md`). Some do not exist yet — they are **future notes waiting to be written**.

---

## Step 2 — Find "Empty Links" (Notes That Do Not Exist Yet)

In Obsidian, a link to a note that doesn't exist yet shows up as a hollow circle. Let's find them:

```bash
# Get the names of notes that exist
ls /home/tester/aios/obsidian/vault/ | sed 's/.md//' | sed 's/_/ /g' > /tmp/existing_notes.txt
echo "=== Notes that EXIST ==="
cat /tmp/existing_notes.txt
```

```bash
echo ""
echo "=== Notes LINKED but not yet created ==="
grep -rh "\[\[" /home/tester/aios/obsidian/vault/ \
  | grep -o '\[\[[^\]]*\]\]' \
  | sed 's/\[\[//;s/\]\]//' \
  | sort | uniq
```

Look at both lists and spot the notes that are **linked but not created yet**. These are like blank pages in your book — you know they should exist!

---

## Step 3 — Create a Missing Note: Ocean Food Chains

Several of your notes link to `[[Ocean Food Chains]]` but that note doesn't exist yet. Let's create it:

```bash
cat > /home/tester/aios/obsidian/vault/Ocean_Food_Chains.md << 'ENDOFNOTE'
---
title: Ocean Food Chains
tags: [ocean, ecology, food-chain, predator, prey]
topic: marine biology
created: 2026-03-12
---

# Ocean Food Chains

A food chain shows what eats what in a habitat.
Ocean food chains start with tiny living things and end with giant predators.

## The Basic Chain
```
Sunlight
   ↓
Phytoplankton (tiny plants)
   ↓
Zooplankton (tiny animals like krill)
   ↓
Small fish (like anchovies)
   ↓
Bigger fish (like tuna)
   ↓
Top predators (sharks, orcas, seabirds)
```

## How the Blue Whale Fits In
Despite being the largest animal on Earth, the [[Blue Whale]] sits in
the **middle** of the food chain — it eats krill (zooplankton), not fish.
This surprises many people!

## Why Food Chains Matter
If one part of the chain disappears, the whole chain is affected.
For example, if krill populations collapse, blue whales starve.

## Related Notes
- [[Blue Whale]]
- [[Ocean Animals Overview]]
- [[Deep Sea Ecosystems]]

ENDOFNOTE
```

---

## Step 4 — Add a Back-Link in Blue Whale

Right now, `Blue_Whale.md` already links TO `Ocean_Food_Chains.md`. But `Ocean_Food_Chains.md` now also links BACK to `Blue_Whale.md`. This makes the connection **two-way** — much stronger!

Let's check the back-link exists:
```bash
grep "Blue Whale" /home/tester/aios/obsidian/vault/Ocean_Food_Chains.md
```

You should see the `[[Blue Whale]]` link you just wrote. ✅

---

## Step 5 — Ask AIOS to Suggest More Connections

AIOS can look at your vault and suggest links you may have missed:

```bash
aios-vault reorganize
```

> This is a **preview only** — it shows suggestions but does NOT change anything until you add `--apply`.

You should see output like:
```
=== Reorganisation Suggestions ===

Possible groupings:
  Topic "marine biology": Blue_Whale, Clownfish_Life_Cycle, Ocean_Animals_Overview,
                          Octopus_Intelligence, Ocean_Food_Chains

Possible new links:
  Octopus_Intelligence  →  [[Clownfish_Life_Cycle]]  (both: ocean, biology)
  Clownfish_Life_Cycle  →  [[Blue_Whale]]            (both: ocean, marine biology)

No changes applied. Run with --apply to reorganise.
```

These are the AI's **guesses** at connections that make sense. You decide whether to add them!

---

## Step 6 — Manually Add a Suggested Link

Let's add one of the AI's suggestions. We will add a link from `Octopus_Intelligence` to `Clownfish_Life_Cycle`.

```bash
echo "" >> /home/tester/aios/obsidian/vault/Octopus_Intelligence.md
echo "- [[Clownfish_Life_Cycle]] — another ocean animal with unusual biology" >> /home/tester/aios/obsidian/vault/Octopus_Intelligence.md
```

Now verify it was added:
```bash
tail -5 /home/tester/aios/obsidian/vault/Octopus_Intelligence.md
```

---

## Step 7 — Count Your Vault Links

How many total links do you have now?

```bash
grep -roh "\[\[[^\]]*\]\]" /home/tester/aios/obsidian/vault/ | wc -l
```

The number shows how "connected" your vault is. More connections = smarter vault!

| Links | What it means |
|---|---|
| 0–5 | Baby vault — just getting started |
| 5–20 | Growing vault — taking shape |
| 20–50 | Healthy vault — starting to feel like a web |
| 50+ | Strong vault — truly a knowledge network |

---

## 🏆 Mission Complete!

| Step | You Did | Why It Matters |
|---|---|---|
| Step 1 | Listed all links in vault | Saw the full connection picture |
| Step 2 | Found "empty links" | Discovered notes waiting to be written |
| Step 3 | Created `Ocean_Food_Chains.md` | Filled in a gap in your knowledge web |
| Step 4 | Verified back-links | Two-way connections are stronger |
| Step 5 | Ran `aios-vault reorganize` | AI suggested missing connections |
| Step 6 | Added a suggested link manually | You decided what connections make sense |
| Step 7 | Counted total links | Measured your vault's connection strength |

---

## 🧠 Key Idea: The Knowledge Snowball

When you first start a vault, it feels small. But every new note **links to existing notes**, which makes existing notes more useful, which encourages you to create more notes, which makes the vault even smarter. It grows like a snowball rolling down a hill!

---

## 🔭 Try It Yourself!

1. Pick one of your other "empty links" (from Step 2 that you haven't created yet)
2. Create that note with: title, tags, 3 facts, and 2 links back to existing notes
3. Count your links again — did your number go up?

---

## ➡️ Next Tutorial

[T04 — Find What You Learned →](T04_find_what_you_learned.md)

Now that your vault is growing, you will learn how to **search** it — both with keywords AND with meaning — so you can always find what you need.
