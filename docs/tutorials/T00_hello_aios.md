# T00 — Hello AIOS! Wake Up Your AI Helper 👋

**Time:** 15–20 minutes  
**Difficulty:** ⭐ (super easy!)  
**What you need:** A computer with AIOS already installed, a terminal window

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ What AIOS is and what it can do for you
- ✅ How to open a terminal
- ✅ How to check that your AI helper is awake and working
- ✅ How to ask the AI what it thinks your question is about

---

## The Big Picture (Read This First!)

Imagine you have a very smart robot helper. This robot can:
1. **Read** long documents for you
2. **Summarise** them into short, easy notes
3. **Remember** everything it learns in a special treasure box called a **vault**
4. **Find** things it remembers when you ask questions

Today you are just going to **wake up the robot** and make sure it is listening.

The robot lives inside your computer and talks to you through a "door" at this address:
```
http://localhost:8080
```
- `localhost` means "this very computer, right here"
- `8080` is the door number

---

## Step 1 — Open a Terminal

A **terminal** is a text window where you type commands directly to the computer.

1. Look for an app called **Terminal**, **Console**, or **Bash** on your computer.
2. Click to open it. You will see a blinking cursor.

> 💡 **Tip:** The terminal can look scary, but you are just going to copy and paste text into it. Nothing bad can happen from the commands in these tutorials!

---

## Step 2 — Check That AIOS Is Running

Type this command (or copy-paste it) and press **Enter**:

```bash
curl http://localhost:8080/health
```

### What does that word "curl" mean?
`curl` is a tool that knocks on a door and shows you what is behind it. You are knocking on the AIOS health door to ask "are you awake?"

### ✅ Good answer — AIOS is awake!
If you see something like this, everything is working:
```json
{"status": "ok", "version": "0.1.0"}
```

### ❌ Bad answer — AIOS is asleep
If you see `Connection refused` or nothing at all, AIOS is not running. Ask a grown-up to start it:
```bash
# A grown-up runs this to wake up AIOS:
cd /home/tester/aios
python -m uvicorn agent.main:app --host 0.0.0.0 --port 8080
```
Then try the `curl` command again.

---

## Step 3 — Say Hello and Ask a Question

Let's ask AIOS to figure out what kind of question you are asking. Type this:

```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "Show me how much space is left on the computer"}'
```

### Breaking this down:
- `curl -s` — knocks on a door quietly (no extra messages)
- `-X POST` — you are **sending** information, not just reading
- `-H "Content-Type: application/json"` — tells the AI you are sending it in the JSON language
- `-d '{"text": "..."}'` — the actual question you are sending

### ✅ What you should see:
```json
{
  "intent": "shell_cmd",
  "confidence": 0.85,
  "method": "pattern_matcher"
}
```

### What does this mean?
| Word | Meaning |
|---|---|
| `intent` | What the AI thinks you want to do |
| `shell_cmd` | The AI thinks you want to run a computer command |
| `confidence` | How sure the AI is — 0.85 means 85% sure |
| `method` | Which part of the AI brain figured it out |

> 🎉 You just talked to an AI! It understood your question.

---

## Step 4 — Try Different Questions

Change the question and see what happens! Copy each one and run it:

**Question 1 — about searching notes:**
```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "Search my notes for information about elephants"}'
```
Expected `intent`: `search_knowledge`

**Question 2 — about making a report:**
```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "Create a report showing all the results"}'
```
Expected `intent`: `report_gen`

**Question 3 — about scheduling:**
```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d '{"text": "Run this every Monday morning automatically"}'
```
Expected `intent`: `schedule_task`

> 🧠 **Cool fact:** The AI reads your words and counts how many "clue words" match each category. Three or more clue words and it is very confident!

---

## Step 5 — Check the Memory Database

AIOS also has a helper called **Qdrant** — a memory database. Let's check it:

```bash
curl -s http://localhost:6333/collections | python3 -m json.tool
```

### ✅ Good answer:
You should see a list of collections (memory buckets) like:
```json
{
  "result": {
    "collections": [
      {"name": "knowledge"},
      {"name": "reports"},
      {"name": "logs"},
      {"name": "test_plans"},
      {"name": "cmd_history"},
      {"name": "scripts"}
    ]
  }
}
```

Think of each collection as a different folder in the AI's brain:
- `knowledge` — things it has learned from documents
- `reports` — summaries and reports it has made
- `logs` — a diary of everything it has done
- `cmd_history` — commands it has run before

---

## 🏆 Mission Complete!

You have finished Tutorial T00! Here is what you did:

| Step | What You Did | Why It Matters |
|---|---|---|
| Step 1 | Opened a terminal | Now you can talk to the computer |
| Step 2 | Checked AIOS health | Made sure the AI is awake |
| Step 3 | Classified an intent | The AI understood your question |
| Step 4 | Tried different questions | Saw 4 different types of intents |
| Step 5 | Checked memory database | Confirmed the memory bank is ready |

---

## 🔭 Try It Yourself!

Think of a question a student might ask. Try to guess which `intent` AIOS will pick before you run it. Were you right?

---

## ➡️ Next Tutorial

[T01 — Feed Your AI a Document →](T01_feed_ai_a_document.md)

You will paste a real document and watch the AI read it and give you a smart summary!
