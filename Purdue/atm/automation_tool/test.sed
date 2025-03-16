sed -n 's/^Hardware alarm_bits/\n/g; s/[^\n]*\n//; s/Software-Alarm:[^\n]*//g; s/\n.*//; P' $1
