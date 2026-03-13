# T02 — Save Notes to Your Knowledge Vault 🗃️✨

**Time:** 30–40 minutes  
**Difficulty:** ⭐⭐ (easy — mostly copy-paste)  
**Builds on:** [T01 — Feed AI a Document](T01_feed_ai_a_document.md)  
**What you need:** A terminal, AIOS running, the `ocean_animals_analysis.txt` file from T01

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ What a "vault" is and why it is like a smart notebook
- ✅ How to create notes in the Obsidian vault
- ✅ How the AI breaks a big topic into smaller topic notes
- ✅ How to add **tags** and **metadata** to a note so you can find it later

---

## The Big Picture

In T01 you gave the AI a document about ocean animals and it gave you a summary.

Now imagine if every time you learned something new, you wrote it in a **smart notebook** where:
- Each page is about **one specific topic**
- Pages can **link to each other** (like clicking a link on a website)
- You can **search** all your notes instantly
- The AI can **find related notes** when you ask a question

This smart notebook is called the **Obsidian vault**. In this tutorial, you will create your first notes!

---

## Step 1 — Find Your Vault

Your vault is a folder on the computer. Let's see where it is:

```bash
ls /home/tester/aios/obsidian/vault/
```

If it is empty or the folder does not exist, create it:
```bash
mkdir -p /home/tester/aios/obsidian/vault
```

> 📚 Think of the vault as a big library. Each markdown (`.md`) file is a book on the shelf. The library is searchable, and books can point to other books.

---

## Step 2 — Create Your First Vault Note

A vault note is just a text file that ends in `.md`. The special secret is that inside the text you can write `[[Note Name]]` to create a **link** to another note.

Let's create a note about Blue Whales:

```bash
cat > /home/tester/aios/obsidian/vault/Blue_Whale.md << 'ENDOFNOTE'
---
title: Blue Whale
tags: [ocean, mammals, animals, giants]
topic: marine biology
created: 2026-03-12
source: ocean_animals.txt
---

# Blue Whale

The blue whale is the **largest animal** that has ever lived on Earth.

## Key Facts
- Length: up to **30 metres**
- Weight: up to **200 tonnes**
- Diet: tiny shrimp called **krill**
- Daily food: **4 tonnes of krill per day**
- Communication: very low sounds travelling **thousands of kilometres**

## Why It Matters
The blue whale shows us that the biggest creatures on Earth eat some of
the smallest creatures. This is a key idea in [[Ocean Food Chains]].

## Related Notes
- [[Ocean Animals Overview]]
- [[Ocean Food Chains]]
- [[Whale Communication]]

ENDOFNOTE
```

### What are those dashes at the top?
The section between `---` and `---` is called **front matter** (or metadata). It is like a label on a library book telling you:
- `title` — the name of the note
- `tags` — keywords for searching
- `topic` — the broad subject area
- `created` — when you made it
- `source` — which document it came from

### What are the `[[...]]` links?
`[[Blue Whale]]` means "this note links to a note called Blue Whale." It is how notes become a **web of knowledge** instead of a pile of loose pages.

---

## Step 3 — Create Two More Notes

Now do the same for the octopus. Copy-paste this:

```bash
cat > /home/tester/aios/obsidian/vault/Octopus_Intelligence.md << 'ENDOFNOTE'
---
title: Octopus Intelligence
tags: [ocean, invertebrates, intelligence, camouflage]
topic: marine biology
created: 2026-03-12
source: ocean_animals.txt
---

# Octopus Intelligence

Octopuses are among the most intelligent animals in the ocean.

## Key Facts
- Arms: **8**
- Hearts: **3** (two for gills, one for body)
- Skill: can **solve puzzles** and **open jars**
- Defence: changes colour **and** texture in under **1 second**

## Why It Is Amazing
Most animals with no backbone (invertebrates) are not very smart.
The octopus is an exception. This links to the idea of [[Animal Intelligence]]
and how brain size alone does not predict smartness.

## Related Notes
- [[Ocean Animals Overview]]
- [[Animal Intelligence]]
- [[Camouflage in Nature]]

ENDOFNOTE
```

And now the clownfish:

```bash
cat > /home/tester/aios/obsidian/vault/Clownfish_Life_Cycle.md << 'ENDOFNOTE'
---
title: Clownfish Life Cycle
tags: [ocean, fish, biology, sex-change]
topic: marine biology
created: 2026-03-12
source: ocean_animals.txt
---

# Clownfish Life Cycle

Clownfish have one of the most unusual life cycles of any fish.

## Key Facts
- Home: **sea anemone** (it provides shelter and protection)
- Protection: special coating makes them **immune to anemone stings**
- Born: **all clownfish are born male**
- Sex change: if the group's female disappears, the **largest male becomes female**

## Interesting Connection
The clownfish proves that biology can be very flexible. Other animals
that can change sex include certain shrimp and wrasse fish.
See [[Sex Change in Animals]] for more examples.

## Related Notes
- [[Ocean Animals Overview]]
- [[Sea Anemone Symbiosis]]
- [[Sex Change in Animals]]

ENDOFNOTE
```

---

## Step 4 — Create the Overview Note That Links Them All

Now make an overview note that acts like a **table of contents** for your ocean animal notes:

```bash
cat > /home/tester/aios/obsidian/vault/Ocean_Animals_Overview.md << 'ENDOFNOTE'
---
title: Ocean Animals Overview
tags: [ocean, overview, marine biology]
topic: marine biology
created: 2026-03-12
source: ocean_animals.txt
---

# Ocean Animals Overview

The ocean is home to an enormous variety of animals.
This note is the **hub** for all ocean animal topic notes.

## Notes in This Topic
- [[Blue Whale]] — largest animal on Earth
- [[Octopus Intelligence]] — smartest invertebrate
- [[Clownfish Life Cycle]] — remarkable sex-change biology
- [[Deep Sea Anglerfish]] — bioluminescence in the dark deep

## Cross-Topic Connections
- [[Ocean Food Chains]] — how ocean animals eat each other
- [[Animal Intelligence]] — comparing smart animals
- [[Camouflage in Nature]] — hiding strategies across species

ENDOFNOTE
```

---

## Step 5 — Check All Your Notes Are There

```bash
ls /home/tester/aios/obsidian/vault/
```

You should see:
```
Blue_Whale.md
Clownfish_Life_Cycle.md
Ocean_Animals_Overview.md
Octopus_Intelligence.md
```

Let's also confirm the links look correct:
```bash
grep "\[\[" /home/tester/aios/obsidian/vault/*.md
```

This shows every link (`[[...]]`) in every note.

---

## Step 6 — Tell AIOS About Your New Notes

Now let's make AIOS aware of the notes so it can search them later:

```bash
aios-vault review
```

This command scans your vault and shows you a summary of everything in it.

---

## 🏆 Mission Complete!

| Step | You Did | Why It Matters |
|---|---|---|
| Step 1 | Found your vault folder | The vault is where all knowledge lives |
| Step 2 | Created `Blue_Whale.md` with metadata | Tags + front matter = searchable knowledge |
| Step 3 | Created two more topic notes | One note per specific idea |
| Step 4 | Created an overview note with links | Overview notes make navigation easy |
| Step 5 | Verified all files exist | Double-checked your work |
| Step 6 | Ran `aios-vault review` | AIOS knows your vault exists now |

---

## The Golden Rule of Vault Notes

> 🌟 **One note = one idea.** Short is better than long. If a note grows too big, split it into two notes and link them.

This is called the **Zettelkasten method** — used by famous scientists and writers.

---

## 🔭 Try It Yourself!

Create a note about the **Deep Sea Anglerfish** from the ocean animals document. Give it:
- A title
- At least 3 tags
- At least 3 key facts
- A link to `[[Ocean Animals Overview]]`

---

## ➡️ Next Tutorial

[T03 — Connect Your Notes →](T03_connect_your_notes.md)

Your notes exist but they do not know about each other yet! You will learn how to check links, create connections, and make your vault into a true knowledge web.
