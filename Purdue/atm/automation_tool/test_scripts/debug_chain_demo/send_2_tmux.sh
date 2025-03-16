#!/bin/sh
# This script continuously sends the "date" command to tmux pane 0.1 every second.

TARGET_PANE="0.0"

while true; do
	# Send the command 'date' followed by the ENTER key to the specified pane
	date_string=$(date +'Timestamp: %m/%d/%y %H:%M:%S')
	echo $date_string
	tmux send-keys -t "$TARGET_PANE" "$date_string" Enter
	#	sleep 1
	tmux send-keys -t "$TARGET_PANE" "show environment temperature" Enter
	tmux send-keys -t "$TARGET_PANE" "show environment fan detail" Enter
	tmux send-keys -t "$TARGET_PANE" " " Enter
	tmux send-keys -t "$TARGET_PANE" "show environment power detail" Enter
	tmux send-keys -t "$TARGET_PANE" " " Enter
	tmux send-keys -t "$TARGET_PANE" "LEGEND: " Enter
	sleep 5
done
