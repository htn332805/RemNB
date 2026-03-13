# T01 — Feed Your AI a Document 📄🤖

**Time:** 30–40 minutes  
**Difficulty:** ⭐⭐ (easy, a little more typing)  
**Builds on:** [T00 — Hello AIOS](T00_hello_aios.md)  
**What you need:** A terminal, AIOS running (health check: `curl http://localhost:8080/health`)

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ How to create a simple text document on your computer
- ✅ How to ask the AI to analyse that document
- ✅ What a "summary" and "insights" look like
- ✅ How to save the AI's answer to a file you can read later

---

## The Big Picture

Imagine you have a long, boring report about **ocean animals** (or any topic you like). Reading it takes an hour. But if you give it to your AI helper, in a few seconds it will:

1. **Read** the whole document
2. **Pull out the main ideas** (the really important bits)
3. **Write a short summary** you can read in 2 minutes
4. **List key facts** you should remember

That is what this tutorial is all about!

---

## Step 1 — Create Your First Document

We are going to make a short practice document about ocean animals. You can use this topic for all the tutorials, or pick your own topic!

In your terminal, type this to go to the right folder:
```bash
cd /home/tester/aios
```

Now create a folder for your documents:
```bash
mkdir -p workspace/my_documents
```

Now create the document. Copy this whole block and paste it into your terminal, then press **Enter**:

```bash
cat > workspace/my_documents/ocean_animals.txt << 'ENDOFFILE'
# Ocean Animals — Amazing Facts

The ocean covers more than 70 percent of the Earth's surface.
It is home to an incredible variety of animals.

## Blue Whale
The blue whale is the largest animal that has ever lived on Earth.
It can grow up to 30 metres long and weigh 200 tonnes.
Blue whales eat tiny shrimp called krill. They eat 4 tonnes of krill every single day.
They talk to each other using very low sounds that can travel thousands of kilometres.

## Octopus
An octopus has 8 arms and 3 hearts. Two hearts pump blood to the gills
and one heart pumps blood to the rest of the body.
Octopuses are extremely intelligent. They can solve puzzles and open jars.
They can also change colour and texture in less than one second to hide from predators.

## Clownfish
Clownfish live inside sea anemones. The anemone stings other fish but not
the clownfish, because the clownfish has a special protective coat.
All clownfish are born male. If the female of a group disappears,
the biggest male changes its sex and becomes female.

## Deep Sea Anglerfish
The anglerfish lives in the deepest, darkest parts of the ocean.
It has a glowing lure on top of its head that it uses to attract prey.
In the pitch black deep sea, no sunlight reaches at all.
ENDOFFILE
```

### Check the file was created:
```bash
cat workspace/my_documents/ocean_animals.txt
```

You should see your document printed back to you.

---

## Step 2 — Ask the AI to Analyse the File

Now we use the `aios-vault` tool to analyse this file. This tool talks to the AI and the vault at the same time.

```bash
aios-vault analyze workspace/my_documents/ocean_animals.txt
```

> ⏳ **Wait a moment** — the AI is reading the whole document and thinking!

### ✅ What you should see:
The AI will print something like:
```
=== Analysis: ocean_animals.txt ===

Summary:
  A document about four ocean animals: blue whale, octopus, clownfish,
  and anglerfish. Covers size, diet, behaviour, and unique biological traits.

Key Topics:
  - Blue whale size and diet
  - Octopus intelligence and camouflage
  - Clownfish sex-change biology
  - Deep-sea anglerfish bioluminescence

Suggested Vault Notes:
  - Blue Whale
  - Octopus Intelligence
  - Clownfish Life Cycle
  - Deep Sea Ecosystems
  - Ocean Animal Adaptations

Confidence: 0.91
```

### What does it all mean?
| Section | What it tells you |
|---|---|
| **Summary** | The 2-sentence version of the whole document |
| **Key Topics** | The most important subjects inside the document |
| **Suggested Vault Notes** | Note titles the AI recommends creating in your vault |
| **Confidence** | How sure the AI is about its analysis (0-1, higher = more sure) |

---

## Step 3 — Save the Analysis to a File

Let's save the AI's output so we can read it again later:

```bash
aios-vault analyze workspace/my_documents/ocean_animals.txt > workspace/my_documents/ocean_animals_analysis.txt
```

The `>` symbol means "take everything printed on screen and save it to this file instead."

Read it back:
```bash
cat workspace/my_documents/ocean_animals_analysis.txt
```

---

## Step 4 — Try the Summarise Command on a Folder

The `summarize` command works on an **entire folder** — great when you have many documents:

```bash
aios-vault summarize workspace/my_documents
```

This shows a combined overview of all files in the folder.

---

## Step 5 — Look at What the AI Learned (Lessons)

Every time you give the AI a document, it tries to learn from it. Let's see:

```bash
aios-vault lessons
```

This shows lessons the AI has extracted from everything it has processed so far.

---

## 🏆 Mission Complete!

| Step | You Did | Why It Matters |
|---|---|---|
| Step 1 | Created a text document | Documents are the AI's "food" |
| Step 2 | Ran `aios-vault analyze` | The AI read and summarised for you |
| Step 3 | Saved the output to a file | You now have a permanent record |
| Step 4 | Ran `aios-vault summarize` on a folder | Works on many documents at once |
| Step 5 | Ran `aios-vault lessons` | Saw what the AI remembered |

---

## 🔭 Try It Yourself!

1. Create a new file called `workspace/my_documents/space_facts.txt`
2. Write at least 3 paragraphs about space (planets, stars, rockets — anything!)
3. Run `aios-vault analyze` on it
4. See if you agree with the AI's suggested vault note titles

---

## 💡 Bonus Challenge

Can you make a document about a topic you are studying at school right now? Analyse it and write down the **Key Topics** you see. Do they match what your teacher says is important?

---

## ➡️ Next Tutorial

[T02 — Save Notes to Your Vault →](T02_save_notes_to_vault.md)

Now that you have a summary, you will learn how to break it into small, smart notes and store them in your Obsidian knowledge vault — your very own AI brain!
