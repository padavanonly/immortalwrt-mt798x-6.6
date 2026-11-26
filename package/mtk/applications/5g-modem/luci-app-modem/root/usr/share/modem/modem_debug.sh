#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"

. "${SCRIPT_DIR}/quectel.sh"
. "${SCRIPT_DIR}/fibocom.sh"
. "${SCRIPT_DIR}/meig.sh"
. "${SCRIPT_DIR}/huawei.sh"
. "${SCRIPT_DIR}/tdtech.sh"
# . "${SCRIPT_DIR}/simcom.sh"

# Debug switch
# 0: disabled
# 1: enabled
# 2: output to file
switch=0
out_file="/tmp/modem.log"	# Output file
# Log info
debug()
{
	time=$(date "+%Y-%m-%d %H:%M:%S")	# Get system time
	if [ $switch = 1 ]; then
		echo $time $1					# Print output
	elif [ $switch = 2 ]; then
		echo $time $1 >> $outfile		# Output to file
	fi
}

# Send AT command
# $1: AT serial port
# $2: AT command
at()
{
	local at_port="$1"
	local at_command="$2"

	#echo
	# echo -e "${at_command}"" > "${at_port}" 2>&1

	#sms_tool
	sms_tool -d "${at_port}" at "${at_command}" 2>&1
}

# Enable during testing
# debug $1
# at $1 $2

# Get quick commands
# $1: Quick option
# $2: Manufacturer
get_quick_commands()
{
	local quick_option="$1"
	local manufacturer="$2"

	local quick_commands
	case "$quick_option" in
		"auto") quick_commands=$(cat ${SCRIPT_DIR}/${manufacturer}_at_commands.json) ;;
		"custom") quick_commands=$(cat /etc/modem/custom_at_commands.json) ;;
		*) quick_commands=$(cat ${SCRIPT_DIR}/${manufacturer}_at_commands.json) ;;
	esac
	echo "$quick_commands"
}

# Dial log
# $1: AT command
# $2: Log path
dial_log()
{
	local at_command="$1"
	local path="$2"

	# Print log
    local update_time=$(date +"%Y-%m-%d %H:%M:%S")
    echo "[${update_time}] Send AT command ${at_command} to modem" >> "${path}"
}
