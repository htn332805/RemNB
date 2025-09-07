#!/usr/bin/env bash  # Shebang line telling the system to run this script with bash
# CLI helper to manipulate the in-memory shared context via the local TCP JSON protocol.
# Each line below is explained so non-programmers can follow the logic easily.

# Usage examples (copy & run in a terminal):
#   ./update_state.sh full                 # Show the entire context
#   ./update_state.sh set data.example '{"hello": "world"}'  # Create/replace a value
#   ./update_state.sh append data.items '42'                  # Append number 42 to list at data.items
#   ./update_state.sh get data.example     # Retrieve a specific path
#   ./update_state.sh delete data.example  # Remove a key
#   ./update_state.sh pop data.items 0     # Remove first element from list
# NOTE: Values are treated as JSON; wrap plain strings in quotes (e.g. '"text"').

PORT=${SHCTX_PORT:-8765}  # Determine TCP port from environment or default to 8765
HOST=127.0.0.1            # Always connect to local loopback address for safety

send() {                  # Function that sends one JSON request and prints one-line JSON response
  local payload="$1";    # Capture first function argument as the JSON payload string
  # Primary method: use bash's /dev/tcp pseudo-device for raw TCP connectivity (lightweight, no dependencies)
  if exec 3<>/dev/tcp/$HOST/$PORT; then  # Open bidirectional file descriptor 3 to the server
    printf '%s\n' "$payload" >&3       # Write the JSON payload followed by newline to server
    IFS= read -r line <&3 || true        # Read exactly one line response (ignore read errors)
    echo "$line"                        # Echo the received response line to stdout
    exec 3>&- 3<&-                       # Close file descriptor 3 (both directions)
  elif command -v nc >/dev/null 2>&1; then                # Fallback method: check if netcat (nc) is installed
    echo "$payload" | nc $HOST $PORT -q 1 | head -n1      # Pipe payload into nc, wait briefly, read first line
  else                                                    # If neither /dev/tcp nor nc are available
    echo '{"ok":false,"error":"no tcp method"}'       # Output an error JSON object
    return 1                                             # Return non-zero status to signal failure
  fi
}

op=$1            # First CLI argument selects operation name (e.g. get, set, full)
shift || true    # Shift positional parameters so remaining args align with expected op-specific inputs
case "$op" in    # Begin pattern matching on requested operation
  full)          # If operation is 'full'
    send '{"op":"full"}' ;;  # Send simple JSON asking for entire context
  get)           # Fetch a single path
    path=$1; send "{\"op\":\"get\",\"path\":\"$path\"}" ;;  # Embed path into JSON payload safely
  set)           # Set/replace a value at a path
    path=$1; value=$2; send "{\"op\":\"set\",\"path\":\"$path\",\"value\":$value}" ;;  # Pass raw JSON value
  append)        # Append a value to a list at path
    path=$1; value=$2; send "{\"op\":\"append\",\"path\":\"$path\",\"value\":$value}" ;;  # Append payload
  delete)        # Delete a key or list element
    path=$1; send "{\"op\":\"delete\",\"path\":\"$path\"}" ;;  # Request deletion
  pop)           # Pop an element from a list
    path=$1; idx=${2:--1}; send "{\"op\":\"pop\",\"path\":\"$path\",\"index\":$idx}" ;;  # Provide index or default -1
  *)             # Unknown operation catch-all
    echo "Unknown op: $op"; exit 1 ;;  # Print error and exit with failure code
 esac  # End of case statement
