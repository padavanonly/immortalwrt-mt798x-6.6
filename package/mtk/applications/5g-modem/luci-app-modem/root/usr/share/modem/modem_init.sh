#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"
source "${SCRIPT_DIR}/modem_util.sh"

# Initialize modem configuration
modem_init()
{
    m_log "info" "Clearing all modem configurations"
    # Clear modem configuration
    local modem_count=$(uci -q get modem.@global[0].modem_number)
    for i in $(seq 0 $((modem_count-1))); do
        # Delete configuration for this modem
        uci -q del modem.modem${i}
    done
    uci set modem.@global[0].modem_number=0
    uci commit modem
    m_log "info" "All module configurations cleared"
}

modem_init