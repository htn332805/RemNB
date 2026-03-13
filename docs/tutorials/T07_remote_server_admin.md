# T07 — Remote Server Adventures: Let AI Be Your Robot Helper 🤖🖥️

**Time:** This tutorial has 3 sessions. Do one per day or in order!  
&nbsp;&nbsp;&nbsp;&nbsp;• Session 1 (Phases 0–2): 45–60 min — "Hello, Remote Server!"  
&nbsp;&nbsp;&nbsp;&nbsp;• Session 2 (Phases 3–5): 45–60 min — "AI Writes Your Scripts"  
&nbsp;&nbsp;&nbsp;&nbsp;• Session 3 (Phases 6–8): 45–60 min — "Logs, Charts, and Reports"  

**Difficulty:** ⭐⭐⭐ (everything is copy-paste! No coding ever needed)  
**Builds on:** [T00 — Hello AIOS](T00_hello_aios.md)  
**You need:** Terminal open, AIOS running (`curl http://localhost:8080/health`), SSH access to a remote Linux server

---

## The Big Idea

A **remote server** is a computer somewhere else — could be in another room, another building, or even another country. You cannot touch its keyboard. Instead, you connect over the network using SSH (like a secret tunnel) and type commands that run on that faraway computer.

Normally, system administrators (the people who look after servers) have to:
- Type many commands by hand
- Remember lots of rules
- Write programs to automate repeated tasks

**But you will NOT do any of that.** Instead, you will:
1. **Describe** what you want in plain English
2. **Ask the AI** to write the code for you
3. **Send it** to the remote server using AIOS
4. **Collect the results** and turn them into a nice Excel report

The AI is your robot programmer. You are the boss who gives the instructions. 🎉

---

## Words You Will Learn

| Word | Meaning |
|---|---|
| **SSH** | A secure tunnel for sending commands to a remote computer |
| **Shell script** | A list of commands saved in a file, run in order |
| **GNU screen** | A tool that keeps your programs running even if you disconnect |
| **tmux** | Like screen but more powerful — can split into windows and panels |
| **Session** | A named "room" inside screen or tmux where commands run |
| **Cron** | A scheduler that automatically runs commands at set times |
| **System log** | A diary the computer keeps, recording everything that happens |
| **Excel / XLSX** | A spreadsheet file with tables and charts |

---

# 🌅 SESSION 1 — "Hello, Remote Server!"

*Goal: Register your server, connect to it, and collect its basic health information.*

---

## Phase 0 — Register Your Remote Server (10 min)

Before AIOS can talk to your remote server, you need to write its address in a config file — like adding a new contact to your phone.

### Step 1 — Open the server list

```bash
cat /home/tester/aios/configs/server_registry.yml
```

You will see a file like:
```yaml
servers:
  - name: server-01
    host: 192.168.1.101
    protocol: ssh
    username: testuser
    password: null
    port: 22
```

### Step 2 — Add YOUR server

Ask a grown-up for the server's **IP address**, **username**, and **password**, then run:

```bash
# Fill in YOUR values before running this!
SERVER_NAME="my-server"
SERVER_HOST="192.168.1.50"      # ← change this
SERVER_USER="ubuntu"             # ← change this
SERVER_PASSWORD="MyPassword123"  # ← change this

cat >> /home/tester/aios/configs/server_registry.yml << ENDOFSERVER

  - name: $SERVER_NAME
    host: $SERVER_HOST
    protocol: ssh
    username: $SERVER_USER
    password: $SERVER_PASSWORD
    port: 22
ENDOFSERVER

echo "✅ Server registered!"
cat /home/tester/aios/configs/server_registry.yml
```

### Step 3 — Test the connection (Dry Run first — safe!)

A **dry run** means "check if this WOULD work, but don't actually do it yet." Always test dry run first!

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{
    "username": "tester",
    "command": "echo Hello from the remote server!",
    "dry_run": true
  }' | python3 -m json.tool
```

You should see `"status": "dry_run"` — that means the command would have worked!

### Step 4 — Run it for real

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{
    "username": "tester",
    "command": "echo Hello from the remote server!",
    "dry_run": false
  }' | python3 -m json.tool
```

Look for `"stdout": "Hello from the remote server!\n"` — the remote computer said hello! 🎉

---

## Phase 1 — Collect Remote Server Health Info (20 min)

Now you will ask the remote server lots of questions about itself. Think of it like a doctor's check-up!

### Step 1 — What computer is this? (server identity)

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "uname -a && hostname && uptime", "dry_run": false}' \
  | python3 -m json.tool
```

Look at the `"stdout"` field. It will tell you:
- The operating system name and version
- The computer's hostname (its name on the network)
- How long it has been running without a restart

### Step 2 — How much memory (RAM)?

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "free -h", "dry_run": false}' \
  | python3 -m json.tool
```

`free -h` shows:
- **total** — how much RAM the computer has
- **used** — how much is being used right now
- **free** — how much is available
- The `-h` flag makes it show gigabytes (GB) instead of raw bytes

### Step 3 — How many CPU cores?

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "nproc && lscpu | grep -E \"CPU|Model name\"", "dry_run": false}' \
  | python3 -m json.tool
```

### Step 4 — How much disk space is left?

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "df -h", "dry_run": false}' \
  | python3 -m json.tool
```

`df -h` shows every "drive" on the computer with:
- **Size** — total capacity of that drive
- **Used** — how much space is already taken
- **Avail** — how much is free
- **Use%** — the percentage full (warning: 90%+ is getting worrying!)

### Step 5 — What programs are running?

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "ps aux --no-header | sort -rk3 | head -10", "dry_run": false}' \
  | python3 -m json.tool
```

This shows the **top 10 busiest programs** on the remote computer right now.

### Step 6 — Save all the health info to a file

```bash
mkdir -p /home/tester/aios/workspace/my_documents

for CMD in "uname -a" "hostname" "uptime" "free -h" "df -h" "nproc"; do
  echo "=== $CMD ===" >> /home/tester/aios/workspace/my_documents/server_health.txt
  
  curl -s -X POST http://localhost:8080/agent/remote/run-command \
    -H "Content-Type: application/json" \
    -d "{\"username\": \"tester\", \"command\": \"$CMD\", \"dry_run\": false}" \
    | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout','ERROR: '+d[0].get('stderr','')))" \
    >> /home/tester/aios/workspace/my_documents/server_health.txt
  
  echo "" >> /home/tester/aios/workspace/my_documents/server_health.txt
done

echo "✅ Saved! Here is the combined report:"
cat /home/tester/aios/workspace/my_documents/server_health.txt
```

> 🎉 You just did a full system health check on a remote server — without touching it!

---

## Phase 2 — Ask AI to Explain What You Found (15 min)

The health info is saved. Now ask the AI to analyse it:

```bash
aios-vault analyze /home/tester/aios/workspace/my_documents/server_health.txt 2>/dev/null \
  || python3 - << 'PYEOF'
import requests, json

health_data = open("/home/tester/aios/workspace/my_documents/server_health.txt").read()

prompt = f"""You are a helpful teacher explaining server health to a 10-year-old.
Read this server health report and explain:
1. Is the server healthy? (yes/no and why)
2. Is there any disk space problem?
3. Is the memory usage normal?
4. What should someone watch out for?

Server report:
{health_data[:2000]}

Reply in simple language. Use bullet points."""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=60
)
result = resp.json()
print("=== AI Analysis of Your Server Health ===")
print(result.get("response", "No response from AI"))
PYEOF
```

> 🧠 The AI reads the numbers and explains them in plain English for you!

---

# ⚙️ SESSION 2 — "AI Writes Your Scripts"

*Goal: Use the AI to generate real scripts, then upload and run them remotely.*

---

## Phase 3 — Ask AI to Write a Monitoring Script (20 min)

Here is the big magic: you will **describe** what you want and the AI will **write the code**.

### Step 1 — Ask AI to write a disk space monitor script

```bash
python3 << 'PYEOF'
import requests

prompt = """Write a bash shell script that:
1. Checks disk space on all mounted filesystems using 'df -h'
2. If ANY filesystem is more than 80 percent full, print a WARNING message
3. Shows CPU usage using 'top -bn1 | grep "Cpu(s)"'
4. Shows memory usage using 'free -h'
5. Shows the 5 processes using the most CPU using 'ps aux'
6. Saves all output to a file called /tmp/health_report.txt
7. Adds a timestamp at the top of the report
8. Prints "HEALTH CHECK COMPLETE" when done

Make the script simple. Add comments explaining each step.
Only output the bash script, nothing else. Start with #!/bin/bash"""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=120
)
script = resp.json().get("response", "")
print(script)

# Save the script to a file
with open("/home/tester/aios/workspace/my_documents/health_monitor.sh", "w") as f:
    f.write(script)
print("\n✅ Script saved to workspace/my_documents/health_monitor.sh")
PYEOF
```

### Step 2 — Read the script the AI wrote

```bash
cat /home/tester/aios/workspace/my_documents/health_monitor.sh
```

You do not need to understand every line! Just check it starts with `#!/bin/bash` and looks like computer instructions.

### Step 3 — Upload the script to the remote server

AIOS can copy the script to the remote server using `cat` + SSH:

```bash
SCRIPT_CONTENT=$(cat /home/tester/aios/workspace/my_documents/health_monitor.sh)

# First, send the script content to the remote server
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d "{\"username\": \"tester\", \"command\": \"cat > /tmp/health_monitor.sh << 'ENDSCRIPT'\n${SCRIPT_CONTENT}\nENDSCRIPT\", \"dry_run\": false}" \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print('Status:', d[0].get('status'))"
```

A simpler method — paste the script in one line using heredoc via SSH:

```bash
python3 << 'PYEOF'
import requests, json

# Read what the AI wrote
with open("/home/tester/aios/workspace/my_documents/health_monitor.sh") as f:
    script = f.read()

# Escape it for JSON sending
escaped = script.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")

# Upload using printf on remote (safe for special characters)
upload_cmd = f'printf "%s" "{escaped}" > /tmp/health_monitor.sh && chmod +x /tmp/health_monitor.sh && echo "Script uploaded OK"'

resp = requests.post(
    "http://localhost:8080/agent/remote/run-command",
    json={"username": "tester", "command": upload_cmd, "dry_run": False}
)
results = resp.json()
print("Upload result:", results[0].get("stdout", results[0].get("stderr")))
PYEOF
```

### Step 4 — Run the script on the remote server

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "bash /tmp/health_monitor.sh && cat /tmp/health_report.txt", "dry_run": false}' \
  | python3 -c "
import sys, json
data = json.load(sys.stdin)
for srv in data:
    print(f'=== {srv[\"server_name\"]} ===')
    print(srv.get('stdout', srv.get('stderr', 'no output')))
"
```

---

## Phase 4 — Ask AI to Write a User Account Script (20 min)

System administrators often need to create new user accounts on servers. Let's ask the AI to write that script for us!

### Step 1 — Ask AI for a user creation script

```bash
python3 << 'PYEOF'
import requests

prompt = """Write a bash script that:
1. Creates a new Linux user account called 'student01' with password 'Learn123!'
2. Creates a home directory for that user at /home/student01
3. Adds the user to the 'sudo' group so they have admin rights
4. Creates a welcome file at /home/student01/welcome.txt with a message
5. Shows a success message when done
6. Handles the case where the user already exists (don't error, just say 'user exists')
7. At the end, list all users to confirm the new user was created

Use 'useradd' and 'passwd' commands. Add error handling with if/else.
Only output the bash script. Start with #!/bin/bash"""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=120
)
script = resp.json().get("response", "")
print(script)

with open("/home/tester/aios/workspace/my_documents/create_user.sh", "w") as f:
    f.write(script)
print("\n✅ User creation script saved!")
PYEOF
```

### Step 2 — Review the script first (dry run!)

Always review before running scripts that change user accounts:

```bash
echo "=== Script the AI wrote ==="
cat /home/tester/aios/workspace/my_documents/create_user.sh
echo ""
echo "=== Does this look sensible? Review before running! ==="
```

### Step 3 — Run it with dry_run FIRST

```bash
# Check what AIOS thinks of the command before running
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "bash /tmp/create_user.sh", "dry_run": true}' \
  | python3 -m json.tool
```

### Step 4 — Upload and run for real

```bash
python3 << 'PYEOF'
import requests, json

with open("/home/tester/aios/workspace/my_documents/create_user.sh") as f:
    script = f.read()
escaped = script.replace('"', '\\"').replace("\n", "\\n")

# Upload
upload = requests.post(
    "http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": f'printf "%s" "{escaped}" > /tmp/create_user.sh && chmod +x /tmp/create_user.sh && echo UPLOADED',
          "dry_run": False}
).json()
print("Upload:", upload[0].get("stdout", "").strip())

# Run
run = requests.post(
    "http://localhost:8080/agent/remote/run-command",
    json={"username": "tester", "command": "sudo bash /tmp/create_user.sh", "dry_run": False}
).json()
print("\n=== Script Output ===")
print(run[0].get("stdout", run[0].get("stderr", "no output")))
PYEOF
```

### Step 5 — Verify the user was created

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "id student01 && cat /home/student01/welcome.txt", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout',''))"
```

---

## Phase 5 — GNU screen and tmux Sessions (20 min)

Sometimes you want to run a long program on the remote server, but you need to disconnect your computer (go home, turn off your laptop). **screen** and **tmux** keep the program running even after you leave.

Think of it like watching a TV show on your TV instead of your phone — the show keeps playing even if you walk away.

### Part A — GNU screen

#### Create a named screen session and run a script inside it

```bash
# Start a screen session called "monitoring" in detached mode (-dm)
# and immediately run our health monitor script inside it
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "screen -dmS monitoring bash -c \"while true; do bash /tmp/health_monitor.sh; sleep 60; done\"", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print('Status:', d[0].get('status'))"
```

What this does:
- `screen -dmS monitoring` — creates a screen session named "monitoring" and starts it **detached** (running in the background)
- `bash -c "while true; do ...sleep 60; done"` — runs the health check every 60 seconds, forever

#### Check if the screen session is running

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "screen -ls", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout',''))"
```

You should see something like:
```
There is a screen on:
    12345.monitoring    (Detached)
1 Socket in /run/screen/S-ubuntu.
```

#### Send a command to the running screen session

```bash
# Ask the monitoring session to immediately run a check
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "screen -S monitoring -X stuff \"date\\n\"", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print('Sent command. Status:', d[0].get('status'))"
```

#### Stop the screen session

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "screen -S monitoring -X quit && echo Stopped", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout',''))"
```

-----

### Part B — tmux (the more powerful version)

tmux is like screen but it can split into multiple panels and has better logging.

#### Create a tmux session

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "tmux new-session -d -s server_watch && echo \"Session created\"", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout', d[0].get('stderr','')))"
```

#### List all tmux sessions

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "tmux list-sessions", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout','No sessions found'))"
```

#### Send a command to the tmux session

```bash
# Run the health script inside the tmux session
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "tmux send-keys -t server_watch \"bash /tmp/health_monitor.sh\" Enter", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print('Sent! Status:', d[0].get('status'))"
```

#### Turn on tmux logging (captures everything to a file)

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "tmux pipe-pane -t server_watch -o \"cat >> /tmp/tmux_session_log.txt\" && echo Logging started", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout',''))"
```

#### Wait a moment, then read what was logged

```bash
sleep 5
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "cat /tmp/tmux_session_log.txt", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout','No log yet'))"
```

#### Delete the tmux session when done

```bash
curl -s -X POST http://localhost:8080/agent/remote/run-command \
  -H "Content-Type: application/json" \
  -d '{"username": "tester", "command": "tmux kill-session -t server_watch && echo Session deleted", "dry_run": false}' \
  | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0].get('stdout',''))"
```

> ⏸️ **Great stopping point!** End of Session 2. Take a break!

---

# 📊 SESSION 3 — "Logs, Charts, and Excel Reports"

*Goal: Harvest real logs from the remote server, analyse them with AI, and build a beautiful Excel report.*

---

## Phase 6 — Collect Log Data from Remote Server (15 min)

System logs are like the server's diary. Every event, error, and warning gets written down. Let's read them!

### Step 1 — Ask AI to write a log-harvesting script

```bash
python3 << 'PYEOF'
import requests

prompt = """Write a bash script that harvests useful log data from a Linux server:

1. From /var/log/syslog (or /var/log/messages), get the last 50 lines
2. Find all lines with ERROR, WARN, CRITICAL and count how many of each
3. From /var/log/auth.log, count how many failed SSH login attempts happened today
4. Show the top 5 most common error types
5. Save a clean summary to /tmp/log_harvest.txt in this format:
   TOTAL_ERRORS: <number>
   TOTAL_WARNINGS: <number>
   FAILED_LOGINS: <number>
   TOP_ERRORS: <list>
6. Also save the raw error lines to /tmp/log_errors_raw.txt

Use grep, awk, sort, uniq commands. Handle missing log files gracefully (use if -f to check if file exists before reading).
Only output the bash script. Start with #!/bin/bash"""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=120
)
script = resp.json().get("response", "")
print(script)
with open("/home/tester/aios/workspace/my_documents/log_harvester.sh", "w") as f:
    f.write(script)
print("\n✅ Log harvester script saved!")
PYEOF
```

### Step 2 — Upload and run the log harvester

```bash
python3 << 'PYEOF'
import requests

with open("/home/tester/aios/workspace/my_documents/log_harvester.sh") as f:
    script = f.read()
escaped = script.replace('"', '\\"').replace("\n", "\\n")

# Upload
requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": f'printf "%s" "{escaped}" > /tmp/log_harvester.sh && chmod +x /tmp/log_harvester.sh && echo UPLOADED',
          "dry_run": False})

# Run
run = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester", "command": "bash /tmp/log_harvester.sh", "dry_run": False}).json()
print("=== Log Harvest Output ===")
print(run[0].get("stdout", run[0].get("stderr", "no output")))

# Get the summary
summary = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester", "command": "cat /tmp/log_harvest.txt", "dry_run": False}).json()
print("\n=== Summary File ===")
print(summary[0].get("stdout", "No summary file found"))
PYEOF
```

### Step 3 — Collect disk statistics over time

Now let's collect disk usage numbers to put in a chart:

```bash
python3 << 'PYEOF'
import requests, json, time

# Run df -h and parse just the numbers
result = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": "df -h | awk 'NR>1 {print $1\",\"$2\",\"$3\",\"$4\",\"$5\",\"$6}'",
          "dry_run": False}).json()

print("=== Disk Usage CSV ===")
print("filesystem,size,used,available,use_pct,mountpoint")
print(result[0].get("stdout", "no data"))

# Save it
with open("/home/tester/aios/workspace/my_documents/disk_stats.csv", "w") as f:
    f.write("filesystem,size,used,available,use_pct,mountpoint\n")
    f.write(result[0].get("stdout", ""))
print("✅ Saved to disk_stats.csv")
PYEOF
```

---

## Phase 7 — Ask AI to Analyse the Logs (15 min)

```bash
python3 << 'PYEOF'
import requests

# Read what we collected
try:
    with open("/home/tester/aios/workspace/my_documents/disk_stats.csv") as f:
        disk_data = f.read()
except:
    disk_data = "No disk data collected yet."

log_result = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester", "command": "cat /tmp/log_harvest.txt 2>/dev/null || echo 'Log file not found'",
          "dry_run": False}).json()
log_summary = log_result[0].get("stdout", "No log data.")

prompt = f"""You are a helpful system administrator assistant explaining findings to a beginner.

Here is disk usage data from the remote server:
{disk_data}

Here is the log summary from the remote server:
{log_summary}

Please:
1. Tell me if the disk space situation is concerning (is any disk over 80%?)
2. Explain what the error and warning counts mean
3. Are the failed login attempts normal? Should we be worried?
4. Give me a list of 3 things the server administrator should check or fix
5. Rate the overall server health from 1-10 and explain why

Speak simply, like you are explaining to a curious 12-year-old."""

resp = requests.post(
    "http://localhost:11434/api/generate",
    json={"model": "llama3.2:1b", "prompt": prompt, "stream": False},
    timeout=120
)
analysis = resp.json().get("response", "No response")
print("=== AI Analysis of Your Server ===")
print(analysis)

# Save the analysis
with open("/home/tester/aios/workspace/my_documents/server_analysis.txt", "w") as f:
    f.write(analysis)
print("\n✅ Analysis saved!")
PYEOF
```

---

## Phase 8 — Build an Excel Report with Charts! (20 min)

This is the grand finale! Everything you collected — disk stats, log counts, server info — gets turned into a beautiful Excel spreadsheet with charts.

### Step 1 — Collect all data for the report

```bash
python3 << 'PYEOF'
import requests, json

print("Collecting all data for the report...")

# 1. Disk usage
disk = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": "df -h | awk 'NR>1 && $6 != \"/dev\" && $6 != \"/sys\" {printf \"%s,%s,%s,%s\\n\",$6,$2,$3,$5}'",
          "dry_run": False}).json()
disk_rows = []
for line in disk[0].get("stdout", "").strip().split("\n"):
    if line and "," in line:
        parts = line.split(",")
        if len(parts) >= 4:
            pct = parts[3].replace("%", "")
            try:
                disk_rows.append([parts[0], parts[1], parts[2], int(pct)])
            except ValueError:
                disk_rows.append([parts[0], parts[1], parts[2], 0])
print(f"  ✅ Disk data: {len(disk_rows)} filesystems found")

# 2. Memory stats
mem = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": "free -m | awk 'NR==2{printf \"%s,%s,%s\\n\",$2,$3,$4}'",
          "dry_run": False}).json()
mem_line = mem[0].get("stdout", "8192,2048,6144").strip()
mem_parts = mem_line.split(",") if mem_line else ["8192","2048","6144"]
mem_rows = [
    ["Total", int(mem_parts[0]) if len(mem_parts) > 0 and mem_parts[0].isdigit() else 0],
    ["Used", int(mem_parts[1]) if len(mem_parts) > 1 and mem_parts[1].isdigit() else 0],
    ["Free", int(mem_parts[2]) if len(mem_parts) > 2 and mem_parts[2].isdigit() else 0],
]
print(f"  ✅ Memory data collected")

# 3. Top processes by CPU
procs = requests.post("http://localhost:8080/agent/remote/run-command",
    json={"username": "tester",
          "command": "ps aux --no-header --sort=-%cpu | head -8 | awk '{printf \"%s,%.1f,%.1f\\n\",$11,$3,$4}'",
          "dry_run": False}).json()
proc_rows = []
for line in procs[0].get("stdout", "").strip().split("\n"):
    if line and "," in line:
        parts = line.split(",")
        if len(parts) >= 3:
            try:
                proc_rows.append([parts[0], float(parts[1]), float(parts[2])])
            except ValueError:
                proc_rows.append([parts[0], 0.0, 0.0])
print(f"  ✅ Process data: {len(proc_rows)} processes found")

# Save all collected vals for next step
data = {"disk_rows": disk_rows, "mem_rows": mem_rows, "proc_rows": proc_rows}
with open("/home/tester/aios/workspace/my_documents/report_data.json", "w") as f:
    json.dump(data, f, indent=2)
print("\n✅ All data saved to report_data.json")
PYEOF
```

### Step 2 — Build the Excel report!

```bash
python3 << 'PYEOF'
import json
from pathlib import Path
import sys

# Load our collected data
with open("/home/tester/aios/workspace/my_documents/report_data.json") as f:
    data = json.load(f)

disk_rows  = data.get("disk_rows", [["/ (root)", "50G", "20G", 40]])
mem_rows   = data.get("mem_rows",  [["Total",8192],["Used",2048],["Free",6144]])
proc_rows  = data.get("proc_rows", [["python3", 2.5, 1.0]])

# Add fallbacks if empty
if not disk_rows:
    disk_rows = [["/ (root)", "50G", "20G", 40], ["/data", "100G", "60G", 60]]
if not proc_rows:
    proc_rows = [["systemd", 0.1, 0.5]]

# Build the ReportSpec (this is the AIOS format for Excel generation)
sys.path.insert(0, "/home/tester/aios")
from output_engine.models import ReportSpec, TableSpec, ChartSpec
from output_engine.xlsx_builder import write_excel_report
from datetime import datetime

report: ReportSpec = {
    "title": f"Server Health Report — {datetime.now().strftime('%Y-%m-%d %H:%M')}",
    "tables": [
        {
            "name": "Disk Usage",
            "columns": ["Mountpoint", "Total Size", "Used", "Used %"],
            "rows": disk_rows,
        },
        {
            "name": "Memory",
            "columns": ["Type", "MB"],
            "rows": mem_rows,
        },
        {
            "name": "Top Processes",
            "columns": ["Process", "CPU %", "Memory %"],
            "rows": proc_rows,
        },
    ],
    "charts": [
        {
            "title": "Disk Usage by Mountpoint (%)",
            "table_name": "Disk Usage",
            "x_column": "Mountpoint",
            "y_column": "Used %",
            "chart_type": "bar",
        },
        {
            "title": "Memory Breakdown (MB)",
            "table_name": "Memory",
            "x_column": "Type",
            "y_column": "MB",
            "chart_type": "bar",
        },
        {
            "title": "Top Processes CPU Usage",
            "table_name": "Top Processes",
            "x_column": "Process",
            "y_column": "CPU %",
            "chart_type": "bar",
        },
    ],
    "metadata": {
        "generated_by": "aios",
        "version": "1.0",
        "purpose": "Remote Server Health Report",
    },
}

out_path = Path("/home/tester/aios/workspace/my_documents/server_health_report.xlsx")
write_excel_report(report, out_path)
print(f"\n🎉 Excel report created!")
print(f"📁 Location: {out_path}")
print(f"📊 Sheets: {[t['name'] for t in report['tables']]}")
print(f"📈 Charts: {[c['title'] for c in report['charts']]}")
print("\nOpen it in LibreOffice Calc or Microsoft Excel to see your charts!")
PYEOF
```

### Step 3 — Open the report!

```bash
echo "Your report is at:"
ls -lh /home/tester/aios/workspace/my_documents/server_health_report.xlsx

# Try to open it (works if LibreOffice is installed)
xdg-open /home/tester/aios/workspace/my_documents/server_health_report.xlsx 2>/dev/null \
  || echo "Open the file manually in LibreOffice Calc or Excel"
```

### Step 4 — Add the AI's written analysis to the report

Let's also generate a Markdown report with the AI's written explanation alongside the data:

```bash
python3 << 'PYEOF'
import requests, json
from datetime import datetime

# Read all the data we have
try:
    disk = open("/home/tester/aios/workspace/my_documents/disk_stats.csv").read()
except: disk = "No disk data"

try:
    analysis = open("/home/tester/aios/workspace/my_documents/server_analysis.txt").read()
except: analysis = "Run Phase 7 to generate analysis."

try:
    health = open("/home/tester/aios/workspace/my_documents/server_health.txt").read()
except: health = "Run Phase 1 to collect health data."

report_md = f"""# Server Health Report
**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M')}
**Tool:** AIOS + AI Analysis

---

## Executive Summary (Written by AI)

{analysis}

---

## Raw Server Health Data

```
{health[:1500]}
```

---

## Disk Statistics (Raw CSV)

```
{disk}
```

---

## Files Referenced

| File | Description |
|---|---|
| server_health_report.xlsx | Excel report with charts |
| disk_stats.csv | Raw disk usage CSV |
| server_health.txt | Full server health output |
| server_analysis.txt | AI analysis |
| health_monitor.sh | AI-generated monitoring script |
| log_harvester.sh | AI-generated log harvester |

---

*This report was generated automatically using AIOS. No manual coding was done.*
"""

out = "/home/tester/aios/workspace/my_documents/server_health_FULL_REPORT.md"
with open(out, "w") as f:
    f.write(report_md)
print(f"✅ Full written report saved to:")
print(f"   {out}")
PYEOF
```

---

## 🏆 THE COMPLETE MISSION — DONE!

Here is everything you accomplished in Session 3 (and the whole tutorial):

### Complete Achievement List

| Phase | What You Did | Skills Used |
|---|---|---|
| 0 | Registered a remote server | Configuration |
| 1 | Collected server health info (disk, RAM, CPU) | Remote command execution |
| 2 | Asked AI to explain the health data | AI analysis |
| 3 | Asked AI to write a monitoring script | AI code generation |
| 4 | Asked AI to write a user creation script | AI code generation |
| 5A | Created/managed GNU screen sessions | Remote sessions |
| 5B | Created/logged/deleted tmux sessions | Remote sessions |
| 6 | Asked AI to write a log harvester | AI code generation |
| 7 | Asked AI to analyse the logs | AI analysis |
| 8 | Built an Excel report with 3 charts | Data reporting |

### Your Files

All your work is saved here:
```
workspace/my_documents/
├── server_health.txt             ← raw health data from Phase 1
├── server_analysis.txt           ← AI's written analysis
├── health_monitor.sh             ← AI-generated monitor script
├── create_user.sh                ← AI-generated user creation script
├── log_harvester.sh              ← AI-generated log harvester
├── disk_stats.csv                ← disk data as CSV
├── report_data.json              ← all data collected for the report
├── server_health_report.xlsx     ← ⭐ EXCEL REPORT WITH CHARTS ⭐
└── server_health_FULL_REPORT.md  ← ⭐ FULL WRITTEN REPORT ⭐
```

---

## The Master Workflow (for any future task!)

```
1. DESCRIBE what you want in plain English
         ↓
2. PASTE your description into the AI prompt template
         ↓
3. AI WRITES the script for you
         ↓
4. DRY RUN first (safety check!)
         ↓
5. UPLOAD the script to the remote server via AIOS
         ↓
6. RUN the script via AIOS /agent/remote/run-command
         ↓
7. COLLECT the output (stdout)
         ↓
8. ANALYSE with AI if the output is complex
         ↓
9. BUILD an Excel report with charts from the numbers
         ↓
10. SAVE vault notes about what you learned
```

---

## 🔭 Try It Yourself!

Here are ideas for your next automation project. For each one, ask the AI to write the script!

| Challenge | What to ask the AI |
|---|---|
| **Network health** | "Write a script that pings 5 websites and reports which ones are reachable" |
| **Log monitor** | "Write a script that watches /var/log/syslog and alerts if ERROR appears" |
| **Disk cleanup** | "Write a script that finds files larger than 100MB and lists them" |
| **Backup** | "Write a script that copies a folder to /tmp/backup with a timestamp" |
| **Service status** | "Write a script that checks if nginx, ssh, and mysql are running" |
| **Load test** | "Write a script that measures how long it takes to run a command 10 times" |

Remember: **You describe. AI writes. AIOS runs. You report.** That is the full loop!

---

## ➡️ Want to Go Further?

- Review your Excel reports in the vault: add a note `[[Server Health Reports]]` with what you found
- Schedule the monitoring script to run every hour automatically (see [T03](T03_connect_your_notes.md) for scheduling concepts, or check the AIOS scheduler docs)
- Try the log pipeline on multiple servers at once — AIOS will run the command on ALL registered servers simultaneously!
