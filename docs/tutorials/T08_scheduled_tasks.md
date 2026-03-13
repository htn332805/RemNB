# T08 — Set It and Forget It: Automatic Scheduled Tasks ⏰🤖

**Time:** 45–60 minutes  
**Difficulty:** ⭐⭐⭐ (medium — you are building automation that runs by itself!)  
**Builds on:** [T07 — Remote Server Admin](T07_remote_server_admin.md)  
**What you need:** AIOS running, a registered remote server from T07

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ What a "scheduled task" is and why it is useful
- ✅ How to use **cron expressions** to say "run every hour" or "run at 9am every weekday"
- ✅ How to create, list, pause, and delete scheduled tasks using AIOS
- ✅ How to schedule your health monitor from T07 to run automatically
- ✅ How to make a task that collects data AND saves a report — completely by itself

---

## The Big Picture

In T07 you learned to collect server health data by running commands manually. But what if you want the computer to check the server every hour — even while you are at school or asleep?

That is a **scheduled task**. You set it up once, and AIOS runs it automatically on whatever schedule you choose.

It is like setting an alarm clock — but instead of waking you up, it wakes up the computer to do a job for you.

```
You set up the schedule ONCE
           ↓
AIOS wakes up every hour (or every day, or every Monday...)
           ↓
Runs your chosen command automatically
           ↓
Saves the result in your workspace
           ↓
You can come back later and read the results
```

---

## Understanding Cron Expressions

A **cron expression** is a short code that describes WHEN to run something.
It has 5 parts, read left to right:

```
 ┌─── minute (0-59)
 │  ┌─── hour (0-23)
 │  │  ┌─── day of month (1-31)
 │  │  │  ┌─── month (1-12)
 │  │  │  │  ┌─── day of week (0=Sunday, 1=Monday ... 6=Saturday)
 │  │  │  │  │
 *  *  *  *  *
```

`*` means "every". So:

| Cron expression | When it runs |
|---|---|
| `* * * * *` | Every single minute |
| `0 * * * *` | Every hour, at :00 (top of the hour) |
| `0 9 * * *` | Every day at 9:00am |
| `0 9 * * 1` | Every Monday at 9:00am |
| `0 8,17 * * *` | Every day at 8am AND 5pm |
| `*/15 * * * *` | Every 15 minutes |
| `0 0 * * 0` | Every Sunday at midnight |

> 💡 **Tip:** Use [crontab.guru](https://crontab.guru) to type a cron expression and see it in plain English.

---

## Phase 1 — Create Your First Scheduled Task (15 min)

Let's schedule the health monitoring command from T07 to run every hour.

### Step 1 — Create the schedule (with dry_run protection!)

The AIOS scheduler has a safety feature: a task must have `dry_run_verified: true` before it can actually run for real. This stops you from accidentally creating a dangerous automated task.

First, create it in **dry-run-only** mode to review it:

```bash
curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "name": "hourly-server-health",
    "username": "tester",
    "cron": "0 * * * *",
    "intent": "remote_exec",
    "payload": {
      "command": "df -h && free -h && uptime",
      "dry_run": true
    },
    "enabled": false
  }' | python3 -m json.tool
```

### What just happened?
- `"name"` — a friendly label for this task
- `"cron": "0 * * * *"` — run at the top of every hour
- `"intent": "remote_exec"` — this task runs a remote command
- `"payload"` — the actual command to run when the alarm goes off
- `"enabled": false` — the task exists but is paused (safe to review)

You will get back a response like:
```json
{
  "id": "a7b3c1d2",
  "name": "hourly-server-health",
  "username": "tester",
  "cron": "0 * * * *",
  "intent": "remote_exec",
  "dry_run_verified": false,
  "enabled": false
}
```

**Save the `id` value** — you will need it in the next steps!

```bash
# Run this to save the ID automatically
TASK_ID=$(curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "name": "hourly-disk-check",
    "username": "tester",
    "cron": "0 * * * *",
    "intent": "remote_exec",
    "payload": {"command": "df -h", "dry_run": false},
    "enabled": false
  }' | python3 -c "import sys,json; print(json.load(sys.stdin).get('id','ERROR'))")

echo "Task ID saved: $TASK_ID"
```

---

## Phase 2 — List, Inspect, and Manage Tasks (10 min)

### Step 1 — List all scheduled tasks

```bash
curl -s http://localhost:8080/agent/schedule | python3 -m json.tool
```

You should see a list showing all tasks. Notice some may have `"enabled": false` and `"dry_run_verified": false`.

### Step 2 — Get details of one specific task

```bash
# Replace TASK_ID with the actual ID you saved above
curl -s http://localhost:8080/agent/schedule/$TASK_ID | python3 -m json.tool
```

### Step 3 — Enable the task (turn it on)

Once you have reviewed the task and are happy with it, enable it:

```bash
curl -s -X PATCH http://localhost:8080/agent/schedule/$TASK_ID \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}' | python3 -m json.tool
```

You should now see `"enabled": true` in the response.

### Step 4 — Pause it again (turn it off temporarily)

```bash
curl -s -X PATCH http://localhost:8080/agent/schedule/$TASK_ID \
  -H "Content-Type: application/json" \
  -d '{"enabled": false}' | python3 -m json.tool
```

### Step 5 — Delete the task (clean up)

```bash
curl -s -X DELETE http://localhost:8080/agent/schedule/$TASK_ID | python3 -m json.tool
```

You should see: `{"deleted": "<task_id>"}` — the task is gone.

---

## Phase 3 — Schedule Real Useful Tasks (20 min)

Now let's schedule something genuinely useful. You will create three tasks:
1. An hourly disk check
2. A daily log harvest
3. A weekly Excel report

### Task 1 — Hourly disk alert

This task checks if any disk is over 80% full and writes to a log file if it is:

```bash
DISK_ALERT_ID=$(curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "name": "hourly-disk-alert",
    "username": "tester",
    "cron": "0 * * * *",
    "intent": "remote_exec",
    "payload": {
      "command": "df -h | awk '\''NR>1 && substr($5,1,length($5)-1)+0 > 80 {print \"DISK WARNING: \" $6 \" is \" $5 \" full\"}'\'' >> /tmp/disk_alerts.log && echo run_ok",
      "dry_run": false
    },
    "enabled": false
  }' | python3 -c "import sys,json; print(json.load(sys.stdin).get('id','ERROR'))")

echo "Disk alert task ID: $DISK_ALERT_ID"
```

Enable it when ready:
```bash
curl -s -X PATCH http://localhost:8080/agent/schedule/$DISK_ALERT_ID \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print('Status: enabled =', d.get('enabled'))"
```

### Task 2 — Daily system summary at 8am

This runs every morning and saves a health snapshot:

```bash
DAILY_ID=$(curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "name": "daily-health-snapshot",
    "username": "tester",
    "cron": "0 8 * * *",
    "intent": "remote_exec",
    "payload": {
      "command": "echo === $(date) === && df -h && free -h && uptime",
      "dry_run": false
    },
    "enabled": false
  }' | python3 -c "import sys,json; print(json.load(sys.stdin).get('id','ERROR'))")

echo "Daily snapshot task ID: $DAILY_ID"
```

### Task 3 — View all your scheduled tasks in one go

```bash
echo "=== All Scheduled Tasks ==="
curl -s http://localhost:8080/agent/schedule \
  | python3 -c "
import sys, json
tasks = json.load(sys.stdin)
print(f'Total tasks: {len(tasks)}')
for t in tasks:
    status = '✅ ON ' if t.get('enabled') else '⏸️  OFF'
    print(f'  {status} | {t[\"name\"]:30s} | cron: {t[\"cron\"]:15s} | id: {t[\"id\"]}')
"
```

---

## Phase 4 — Ask AI to Write a Cron Expression For You (10 min)

You do not need to memorise cron syntax! Just describe what you want to the AI:

```bash
python3 << 'PYEOF'
import requests

prompt = """Convert these task descriptions into cron expressions:

1. Run every 30 minutes
2. Run every day at 7am
3. Run every Monday and Friday at 6pm
4. Run on the 1st of every month at midnight
5. Run every 5 minutes between 9am and 5pm on weekdays

For each one, give me:
- The cron expression (5 fields)
- A plain English confirmation of when it runs

Format each as:
TASK: description
CRON: * * * * *
WHEN: plain English explanation"""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=60
)
print(resp.json().get("response", "No response"))
PYEOF
```

Now use the cron expressions the AI gave you to schedule tasks exactly when you need them!

---

## Phase 5 — Schedule a Task to Save Results to Your Vault (5 min)

The best way to remember what your server's health looked like over time is to **save snapshots to the vault**. Here is a task that appends today's disk usage to a vault note every day:

```bash
VAULT_TASK_ID=$(curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "name": "daily-vault-snapshot",
    "username": "tester",
    "cron": "0 20 * * *",
    "intent": "remote_exec",
    "payload": {
      "command": "echo \"## $(date +%Y-%m-%d) disk snapshot\" && df -h | grep -v tmpfs",
      "dry_run": false
    },
    "enabled": false
  }' | python3 -c "import sys,json; print(json.load(sys.stdin).get('id','ERROR'))")

echo "Vault snapshot task ID: $VAULT_TASK_ID"
```

After it runs (at 8pm every day), you would manually take the output and append it to a vault note called `Server_Daily_Health.md`. In T09 you will see how to make EVEN THIS part automatic!

---

## 🏆 Mission Complete!

| Phase | You Did | Why It Matters |
|---|---|---|
| 1 | Created your first scheduled task | Automation runs while you sleep |
| 2 | Listed, enabled, paused, and deleted tasks | Full task lifecycle management |
| 3 | Scheduled 3 real tasks (hourly / daily) | Your server is now self-monitoring |
| 4 | Asked AI to write cron expressions | Never memorise cron syntax again |
| 5 | Scheduled a vault snapshot task | Daily health history in your notes |

---

## Scheduled Task Cheat Sheet

```bash
# Create a task
curl -s -X POST http://localhost:8080/agent/schedule \
  -H "Content-Type: application/json" \
  -d '{"name":"NAME","username":"tester","cron":"0 * * * *","intent":"remote_exec","payload":{"command":"COMMAND","dry_run":false},"enabled":false}'

# List all tasks
curl -s http://localhost:8080/agent/schedule | python3 -m json.tool

# Get one task
curl -s http://localhost:8080/agent/schedule/TASK_ID | python3 -m json.tool

# Enable a task
curl -s -X PATCH http://localhost:8080/agent/schedule/TASK_ID \
  -H "Content-Type: application/json" -d '{"enabled": true}'

# Disable a task
curl -s -X PATCH http://localhost:8080/agent/schedule/TASK_ID \
  -H "Content-Type: application/json" -d '{"enabled": false}'

# Delete a task
curl -s -X DELETE http://localhost:8080/agent/schedule/TASK_ID
```

---

## 🔭 Try It Yourself!

1. Create a task that runs `uptime` every 15 minutes
2. Create a task that runs every morning at 7am and checks how many users are logged in with `who`
3. Ask the AI: "What is a good monitoring schedule for a small web server?" and use its suggestions to create tasks

---

## ➡️ Next Tutorial

[T09 — The Full Automation Pipeline →](T09_full_automation_pipeline.md)

Put everything together: a single automated pipeline that collects data from remote servers, analyses it with AI, builds an Excel report, and saves the insights to your vault — all triggered by a schedule, completely hands-free!
