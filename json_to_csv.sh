#!/bin/bash

first_line=true
keys=()

# Buffer for multi-line JSON objects
json_block=""

while IFS= read -r line || [ -n "$line" ]; do
    # Accumulate lines until we see a closing brace (assumes flat JSON)
    json_block+="$line"
    
    # Check if it's a complete JSON object (naively ends with a closing brace)
    if [[ "$line" == *"}" ]]; then

        if $first_line; then
            # Extract and save field names
            mapfile -t keys < <(echo "$json_block" | jq -r 'keys_unsorted[]')
            IFS=,; echo "${keys[*]}"
            first_line=false
        fi

        # Construct jq filter dynamically
        jq_filter="["
        for key in "${keys[@]}"; do
            jq_filter+=".${key},"
        done
        jq_filter="${jq_filter%,}] | @csv"

        # Output CSV row
        echo "$json_block" | jq -r "$jq_filter"

        # Reset buffer
        json_block=""
    fi
done
