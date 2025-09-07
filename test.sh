#!/usr/bin/bash
# Example: manipulate shared context

# Add a dictionary entry
bash ./update_state.sh set data.user '{"name": "Alice", "age": 30}'

# Append to a list
bash ./update_state.sh append data.numbers '100'
bash ./update_state.sh append data.numbers '200'

# Modify a value
bash ./update_state.sh set data.user '{"name": "Alice", "age": 31}'

# Read back values
echo "User:"
bash ./update_state.sh get data.user

echo "Numbers:"
bash ./update_state.sh get data.numbers

# Remove a key
bash ./update_state.sh delete data.user

# Show full context
bash ./update_state.sh full | jq .



bash ./simulate_cdu_poll.sh 1 | while IFS= read -r line; do
  # Skip empty separator lines
  [[ -z "$line" ]] && continue
  block+="$line"$'\n'
  if [[ "$line" == "}" ]]; then
    obj=$(printf "%s" "$block" | jq -c . 2>/dev/null) || { block=""; continue; }
    bash ./update_state.sh append data.sensors_data "$obj"
    block=""
  fi
done