#!/usr/bin/expect -f
set timeout 10
#spawn telnet 172.25.27.10 5024
#spawn telnet 10.193.71.35 5024
spawn telnet 172.25.27.202 5024

sleep 3

expect "34972A>" {send "*IDN?\r"}
expect "34972A>" {send "*RST\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" {send "*CLS\r"}
expect "34972A>" { set timevar [exec date +%H,%M,%S]
send "SYSTem:TIME $timevar\r"}
expect "34972A>" { set datevar [exec date +%Y,%m,%d]
send ":SYSTem:DATE $datevar\r"}

expect "34972A>" {send ":Conf:Temp TC,T,(@101:112);:Unit:Temp C,(@101:112)\r"}
expect "34972A>" {send ":SENSe:TEMPerature:NPLC 10,(@101:112)\r"}
#expect  "34972A>" {send ":Format:Reading:Channel On\r"}
#expect  "34972A>" {send ":Format:Reading:Time On\r"}
expect "34972A>" {send ":SENSe:TEMPerature:APER 1, (@101:112)\r"}
#expect  "34972A>" {send ":Conf:volt:DC Auto,(@119:104);:Volt:DC:NPLC 10,(@119:120)\r"}
#expect  "34972A>" {send ":SENSe:VOLT:DC:APER 1, (@119:120)\r"}

expect "34972A>" {send ":DIAG:SEC FTP, ON\r"}
expect "34972A>" {send ":Format:Reading:Channel On\r"}
expect "34972A>" {send ":Format:Reading:Time On\r"}
expect "34972A>" {send ":FORMat:READing:TIME:TYPE ABSolute\r"}
expect "34972A>" {send ":FORMat:READing:ALARm On\r"}
expect "34972A>" {send ":MMEMory:LOG ON\r"}
expect "34972A>" {send ":MMEMory:FORMat:READing:RLIMit ON\r"}
expect "34972A>" {send ":TRIG:SOUR TIMER\r"}
#expect  "34972A>" {send ":TRIG:SOUR IMM\r"}
expect "34972A>" {send ":TRIG:TIMER 5\r"}
expect "34972A>" {send ":TRIG:COUNT INF\r"}
#expect  "34972A>" {send ":Trigger:Count INF\r"}

expect "34972A>" {send ":Route:Scan (@101:112)\r"}

expect "34972A>" {send "INITiate\r"}
#sleep 5
#log_user 0
#while {1} {
#    expect  "34972A>" { send ":DATA:REMove? 1\r";
#    set a [string range $expect_out(buffer) [expr {[string first "\n" $expect_out(buffer)] + 1}] end ];
#    set b [split $a , ];
#    puts "[lindex $b 2]/[lindex $b 3]/[lindex $b 1] [lindex $b 4]:[lindex $b 5]:[lindex $b 6],[lindex $b 0],[lindex $b 7],[lindex $b 8]";
#					}
#	sleep 05
#	}

interact

#!/bin/sh
# The next line is executed by /bin/sh, but not tcl \
#exec tclsh "$0" ${1+"$@"}
#package require Expect
