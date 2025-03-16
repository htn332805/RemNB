#!/run/current-system/sw/bin/bash
#!/bin/bash

session_name=$1
Starting_Port=$2
# Function to create the tmux session and split it into 13 panes
create_tmux_session() {
	tmux new-session -d -s $session_name
	tmux split-window -h
	tmux split-window -v
	tmux split-window -v
	tmux select-pane -t 0
	tmux split-window -v
	tmux split-window -v
	tmux select-pane -t 0
	tmux split-window -h
	tmux select-pane -t 2
	tmux split-window -h
	tmux select-pane -t 4
	tmux split-window -h
	tmux select-pane -t 6
	tmux split-window -h
	tmux select-pane -t 8
	tmux split-window -h
	tmux select-pane -t 10
	tmux split-window -h
	tmux select-layout tiled
	tmux rename-window -t $session_name:0 'custom_window'
}

# Function to loop through panes and send commands
configure_panes() {
	local Prompt=$Starting_Port
	for i in {0..12}; do
		port=Prompt+i
		#tmux send-keys -t $session_name:0.$i "/home/nixos/test_tool/connsat/connsat -A -W '120' -S /run/current-system/sw/bin/bash  -p $Prompt" C-m
		tmux send-keys -t $session_name:0.$i "/home/nixos/automation_tool/connsat/connsat -A -W '120' -S /run/current-system/sw/bin/bash  -p $Prompt" C-m
		if [ $i != 0 ]; then
			tmux send-keys -t $session_name:0.$i "export PS1='$session_name:$Prompt#] '" C-m
		else

			tmux send-keys -t $session_name:0.$i "export PS1='$session_name:$Starting_Port#] '" C-m
		fi
		tmux send-keys -t $session_name:0.$i "clear" C-m
		((Prompt++))
	done
}

# Check if the session already exists
tmux has-session -t $session_name 2>/dev/null

# If the session doesn't exist, create it and configure panes
if [ $? != 0 ]; then
	create_tmux_session
	configure_panes
fi

# Attach to the session
tmux attach-session -t $session_name
