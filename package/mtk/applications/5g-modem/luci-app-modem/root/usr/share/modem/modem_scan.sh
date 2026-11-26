#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"
source "${SCRIPT_DIR}/modem_util.sh"

# Get device physical address
# $1: Network device or serial port
get_device_physical_path()
{
    local device_name="$(basename "$1")"
    local device_path="$(find /sys/class/ -name $device_name)"
    local device_physical_path="$(readlink -f $device_path/device/)"
    echo "$device_physical_path"
}

# Configure modem
# $1: Network device
# $2: Network device sysfs path
set_modem_config()
{
    local network="$(basename $sys_network)"
    local network_path="$(readlink -f $sys_network)"

    # Only handle top-level network device
    local count=$(echo "${network_path}" | grep -o "/net" | wc -l)
    [ "$count" -ge "2" ] && return

    # Check if path contains usb (exclude other eth network devices)
    if [[ "$network" = *"eth"* ]] && [[ "$network_path" != *"usb"* ]]; then
        return
    fi

    # Get physical path
    local device_physical_path=$(m_get_device_physical_path ${network_path})
    # Set physical device
    m_set_physical_device "scan" "${network}" "${device_physical_path}"

    # Enable dialing
    enable_dial "${network}"
}

# Set system network device
# $1: System network device list
set_sys_network_config()
{
    local sys_networks="$1"

    for sys_network in $sys_networks; do
        local network="$(basename $sys_network)"
        set_modem_config "${network}" "${sys_network}"
    done
}

# Set modem info
modem_scan()
{
    # Initialize modem configuration
    sh "${SCRIPT_DIR}/modem_init.sh"

    # Emit modem add event
    #USB
    local sys_network
    sys_network="$(find /sys/class/net -name usb*)" #ECM RNDIS NCM
    set_sys_network_config "$sys_network"
    sys_network="$(find /sys/class/net -name wwan*)" #QMI MBIM
    set_sys_network_config "$sys_network"
    sys_network="$(find /sys/class/net -name eth*)" #RNDIS
    set_sys_network_config "$sys_network"

    #PCIE
    sys_network="$(find /sys/class/net -name mhi_hwip*)" # (Generic mhi driver)
    set_sys_network_config "$sys_network"
    sys_network="$(find /sys/class/net -name rmnet_mhi*)" # (Vendor mhi driver)
    set_sys_network_config "$sys_network"

    echo "modem scan complete"
}

# Enable during testing
modem_scan
