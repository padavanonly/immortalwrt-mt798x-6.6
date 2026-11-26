#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"
source "${SCRIPT_DIR}/modem_debug.sh"
source "${SCRIPT_DIR}/modem_scan.sh"

# Modem scan task
modem_scan_task()
{
    sleep 8s # Wait for mobile network to appear after boot
	while true; do
        enable_dial=$(uci -q get modem.@global[0].enable_dial)
        if [ "$enable_dial" = "1" ]; then
            # Scan modem
            debug "Starting modem scan task"
            modem_scan
            debug "Finished modem scan task"
        fi
        sleep 10s
    done
}

modem_scan_task
