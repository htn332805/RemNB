#!/bin/bash

# Function to generate a random float with two decimal places
rand_float() {
    awk -v min="$1" -v max="$2" 'BEGIN { srand(); printf "%.2f", min + rand() * (max - min) }'
}

# Function to get current timestamp in specified format
get_timestamp() {
    date +"%m/%d/%Y %H:%M:%S"
}

# Simulate one sample of data
simulate_sample() {
    timestamp=$(get_timestamp)
    supply_pressure=$(rand_float 10 20)     # e.g., PSI
    return_pressure=$(rand_float 8 18)
    supply_temp=$(rand_float 18 25)         # e.g., Celsius
    return_temp=$(rand_float 20 28)

    # Output as JSON
    cat <<EOF
{
  "timestamp": "$timestamp",
  "supply_pressure": $supply_pressure,
  "return_pressure": $return_pressure,
  "supply_temp": $supply_temp,
  "return_temp": $return_temp
}
EOF
}

# Main loop for polling
while true; do
    simulate_sample
    echo ""  # newline for readability
    sleep $1  # adjust polling interval as needed
done
