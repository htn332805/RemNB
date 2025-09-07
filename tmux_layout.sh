#!/bin/bash

# --- Defaults ---
windows=1
panes=1
declare -a window_names
declare -a pane_names
session_name="session_$(date +%s)"

# --- Help Message ---
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -w  <number>       Number of windows (default: 1)"
    echo "  -wn <names>        Comma-separated window names (optional)"
    echo "  -p  <number>       Number of panes per window (default: 1)"
    echo "  -pn <names>        Comma-separated pane names (optional)"
    echo "  -sn <name>         Custom tmux session name"
    echo "  -h                 Show this help message"
    exit 1
}

# --- Parse arguments ---
while [[ $# -gt 0 ]]; do
    case $1 in
        -w)
            windows="$2"
            shift 2
            ;;
        -wn)
            IFS=',' read -ra window_names <<< "$2"
            shift 2
            ;;
        -p)
            panes="$2"
            shift 2
            ;;
        -pn)
            IFS=',' read -ra pane_names <<< "$2"
            shift 2
            ;;
        -sn|--session-name)
            session_name="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# --- Create logs directory ---
mkdir -p Logs

# --- Start tmux session (detached) ---
first_win_name="${window_names[0]:-win0}"
tmux new-session -d -s "$session_name" -n "$first_win_name"

# --- Enable mouse and border styling ---
tmux set-option -t "$session_name" -g mouse on
tmux set-option -t "$session_name" -g pane-border-style 'fg=grey'
tmux set-option -t "$session_name" -g pane-active-border-style 'fg=brightred'
tmux set-option -t "$session_name" -g status-left "#[bold]#S"

# --- Create windows and panes ---
for ((w=0; w<windows; w++)); do
    win_name="${window_names[w]:-win$w}"

    if [[ $w -ne 0 ]]; then
        tmux new-window -t "$session_name" -n "$win_name"
    else
        tmux rename-window -t "$session_name:$w" "$win_name"
    fi

    # Split panes horizontally
    for ((p=1; p<panes; p++)); do
        tmux split-window -t "$session_name:$w" -h
        tmux select-layout -t "$session_name:$w" tiled
    done

    # Label and log each pane
    pane_indexes=$(tmux list-panes -t "$session_name:$w" -F "#{pane_index}")
    for p_index in $pane_indexes; do
        pane_label="${pane_names[p_index]:-pane$p_index}"
        full_title="${session_name}_${win_name}_${p_index}_${pane_label}"

        # Set terminal title using ANSI escape sequence
        tmux send-keys -t "$session_name:$w.$p_index" "printf '\\033]2;%s\\033\\\\' '$full_title'" C-m

        # Start logging
        log_file="Logs/${full_title}.log"
        tmux pipe-pane -o -t "$session_name:$w.$p_index" "cat > \"$log_file\""
    done
done

# --- Attach to the session ---
tmux select-window -t "$session_name:0"
tmux attach-session -t "$session_name"
