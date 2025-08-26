#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# screen_script.sh
# Description:
#   Manage GNU screen sessions: list, kill, create, recreate, send commands, and run with logging.
#
# Usage:
#   ./screen_script.sh -l
#       List all screen sessions (PID, Name) in two columns.
#   ./screen_script.sh -k screen_name
#       Kill the screen session with the given name.
#   ./screen_script.sh -c screen_name
#       Create a new detached screen session with the given name.
#   ./screen_script.sh -d screen_name
#       Destroy (kill) any existing session with the name, then create a new one.
#   ./screen_script.sh -s screen_name 'string to send'
#       Send a string of characters to the named screen session.
#   ./screen_script.sh -r screen_name
#       Create a screen session with logging enabled, flushing log every second.
#
# Limitations:
#   - Requires GNU screen to be installed and in PATH.
#   - Log flushing uses 'screen' built-in logging, which may not be millisecond-precise.
#   - Only supports session management by name, not by PID.
#
# Examples:
#   ./screen_script.sh -l
#   ./screen_script.sh -c mysession
#   ./screen_script.sh -k mysession
#   ./screen_script.sh -d mysession
#   ./screen_script.sh -s mysession "echo Hello World"
#   ./screen_script.sh -r mysession
# -----------------------------------------------------------------------------

set -euo pipefail

# Helper: print usage
usage() {
    grep '^#' "$0" | grep -v '^#!/' | sed 's/^# \{0,1\}//'
    exit 1
}

# Helper: check if screen is installed
if ! command -v screen >/dev/null 2>&1; then
    echo "ERROR: GNU screen is not installed or not in PATH." >&2
    exit 100
fi

# List all screen sessions: PID and Name in two columns
list_screens() {
    # screen -ls output: "    12345.session_name (Detached)"
    screen -ls | awk '/\t[0-9]+/{split($1,a,"."); printf "%-10s %s\n", a[1], a[2]}'
}

# Kill a screen session by name
kill_screen() {
    local name="$1"
    local pid
    pid=$(screen -ls | awk -v n="$name" '/\t[0-9]+/ {split($1,a,"."); if(a[2]==n) print a[1]}')
    if [[ -z "$pid" ]]; then
        echo "No screen session named '$name' found."
        return 1
    fi
    screen -S "$name" -X quit
    echo "Killed screen session '$name'."
}

# Create a new detached screen session by name
create_screen() {
    local name="$1"
    screen -dmS "$name"
    echo "Created new detached screen session '$name'."
}

# Destroy and recreate a screen session by name
destroy_and_create_screen() {
    local name="$1"
    kill_screen "$name" || true
    create_screen "$name"
}

# Send a string to a screen session
send_to_screen() {
    local name="$1"
    local str="$2"
    # Add a newline to simulate Enter key
    screen -S "$name" -X stuff "$str"$'\n'
    echo "Sent string to screen session '$name'."
}

# Create a screen session with logging, flush log every second
run_screen_with_log() {
    local name="$1"
    local logdir="Logs"
    mkdir -p "$logdir"
    local logfile="$logdir/${name}_screen.log"
    # Start screen with logging enabled
    screen -dmS "$name" bash -c "while true; do date; sleep 1; done"
    # Enable logging and set flush interval
    screen -S "$name" -X logfile "$logfile"
    screen -S "$name" -X log on
    screen -S "$name" -X logtstamp on
    screen -S "$name" -X logflush 1
    echo "Started screen session '$name' with logging to $logfile (flush every 1s)."
}

# Main CLI dispatch
if [[ $# -lt 1 ]]; then
    usage
fi

case "$1" in
    -l)
        list_screens
        ;;
    -k)
        [[ $# -eq 2 ]] || usage
        kill_screen "$2"
        ;;
    -c)
        [[ $# -eq 2 ]] || usage
        create_screen "$2"
        ;;
    -d)
        [[ $# -eq 2 ]] || usage
        destroy_and_create_screen "$2"
        ;;
    -s)
        [[ $# -eq 3 ]] || usage
        send_to_screen "$2" "$3"
        ;;
    -r)
        [[ $# -eq 2 ]] || usage
        run_screen_with_log "$2"
        ;;
    *)
        usage
        ;;
esac