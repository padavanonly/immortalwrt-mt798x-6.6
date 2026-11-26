#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"

# Runtime directory
MODEM_RUNDIR="/var/run/modem"

# Source utility helpers
source "${SCRIPT_DIR}/modem_debug.sh"

# Reset network interface
# $1: AT serial port
# $4: Connection definition
# $5: Modem index
reset_network_interface()
{
    local at_port="$1"
    local define_connect="$2"
    local modem_no="$3"

    local interface_name="wwan_5g_${modem_no}"
    local interface_name_ipv6="wwan6_5g_${modem_no}"

    # Get IPv4 address
    local at_command="AT+CGPADDR=${define_connect}"
    local ipv4=$(at ${at_port} ${at_command} | grep "+CGPADDR: " | sed -n '1p' | awk -F',' '{print $2}' | sed 's/"//g')
    # Output log
    # echo "[$(date +"%Y-%m-%d %H:%M:%S")] Get Modem new IPv4 address : ${ipv4}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

    # Get DNS address
    local dns=$(fibocom_get_dns ${at_port} ${define_connect})
    local ipv4_dns1=$(echo "${dns}" | jq -r '.dns.ipv4_dns1')
    local ipv4_dns2=$(echo "${dns}" | jq -r '.dns.ipv4_dns2')
    # Output log
    echo "[$(date +"%Y-%m-%d %H:%M:%S")] Get Modem IPv4 DNS1: ${ipv4_dns1}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    echo "[$(date +"%Y-%m-%d %H:%M:%S")] Get Modem IPv4 DNS2: ${ipv4_dns2}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    
    # Compare IPv4 address in network interface
    local ipv4_config=$(uci -q get network.${interface_name}.ipaddr)
    if [ "$ipv4_config" == "$ipv4" ]; then
        # Output log
        echo "[$(date +"%Y-%m-%d %H:%M:%S")] IPv4 address is the same as in the network interface, skip" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    else
        # Output log
        echo "[$(date +"%Y-%m-%d %H:%M:%S")] Reset network interface ${interface_name}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

        # Set static address
	uci set network.wan.metric='2'
	uci set network.wan6.metric='2'
        uci set network.${interface_name}.proto='static'
        uci set network.${interface_name}.ipaddr="${ipv4}"
        uci set network.${interface_name}.netmask='255.255.255.0'
        uci set network.${interface_name}.gateway="${ipv4%.*}.1"
        uci set network.${interface_name}.peerdns='0'
	uci set network.${interface_name}.metric='1'
	uci set network.${interface_name_ipv6}.metric='1'
        uci -q del network.${interface_name}.dns
        uci add_list network.${interface_name}.dns="${ipv4_dns1}"
        uci add_list network.${interface_name}.dns="${ipv4_dns2}"
        uci commit network
        service network reload

        # Output log
        echo "[$(date +"%Y-%m-%d %H:%M:%S")] Reset network interface successful" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    fi
}

# GobiNet dialing
# $1: AT serial port
# $2: Manufacturer
# $3: Connection definition
gobinet_dial()
{
    local at_port="$1"
    local manufacturer="$2"
    local define_connect="$3"

    # Activate
    local at_command="AT+CGACT=1,${define_connect}"
    # Print log
    dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

    at "${at_port}" "${at_command}"

    # Dial
    local at_command
    if [ "$manufacturer" = "quectel" ]; then
        # Quectel does not use this branch
        at_command='ATI'
    elif [ "$manufacturer" = "fibocom" ]; then
        at_command='AT$QCRMCALL=1,3'
    elif [ "$manufacturer" = "meig" ]; then
        at_command="AT$QCRMCALL=1,1,${define_connect},2,1"
    else
        at_command='ATI'
    fi

    # Print log
    dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

    at "${at_port}" "${at_command}"
}

# ECM dialing
# $1: AT serial port
# $2: Manufacturer
# $3: Connection definition
ecm_dial()
{
    local at_port="$1"
    local manufacturer="$2"
    local define_connect="$3"

    # Activate
    # local at_command="AT+CGACT=1,${define_connect}"
    # # Print log
    # dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

    # at "${at_port}" "${at_command}"

    # Dial
    local at_command
    if [ "$manufacturer" = "quectel" ]; then
        at_command="AT+QNETDEVCTL=${define_connect},3,1"
    elif [ "$manufacturer" = "fibocom" ]; then
        at_command="AT+GTRNDIS=1,${define_connect}"
    elif [ "$manufacturer" = "meig" ]; then
        at_command="AT^NDISDUP=${define_connect},1"
    elif [ "$manufacturer" = "huawei" ]; then
        at_command="AT^NDISDUP=${define_connect},1"
    elif [ "$manufacturer" = "tdtech" ]; then
        at_command="AT^NDISDUP=${define_connect},1"
    else
        at_command='ATI'
    fi

    # Print log
    dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

    at "${at_port}" "${at_command}"

    sleep 2s
}

# RNDIS dialing
# $1: AT serial port
# $2: Manufacturer
# $3: Platform
# $4: Connection definition
# $5: Modem index
rndis_dial()
{
    local at_port="$1"
    local manufacturer="$2"
    local platform="$3"
    local define_connect="$4"
    local modem_no="$5"

    # Manual dialing (Fibocom FM350-GL)
    if [ "$manufacturer" = "fibocom" ] && [ "$platform" = "mediatek" ]; then
        sleep 3s
	local at_command="AT+COPS?"
        isp=$(at ${at_port} ${at_command} | grep "+COPS" | awk -F'"' '{print $2}' | tr -d '\r\n' | xargs)
        if [[ "$isp" == "4E2D56FD8054901A" ]]; then
        isp="CHN-UNICOM"
        fi

	if [[ "$isp" == "CHN-CMCC" || "$isp" == "CMCC" || "$isp" == "46000" || "$isp" == "CHINA MOBILE" ]]; then
    	at_command="AT+CGDCONT=${define_connect},\"IPV4V6\",\"cbnet\""
    	at "$at_port" "$at_command"
	echo "matching ISP: [$isp]" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	elif [[ "$isp" == "CHN-UNICOM" || "$isp" == "UNICOM" || "$isp" == "46001" ]]; then
    	at_command="AT+CGDCONT=${define_connect},\"IPV4V6\",\"3gnet\""
    	at "$at_port" "$at_command"
	echo "matching ISP: [$isp]" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	elif [[ "$isp" == "CHN-CT" || "$isp" == "CT" || "$isp" == "46011"  || "$isp" == "CHINA TELECOM" ]]; then
    	at_command="AT+CGDCONT=${define_connect},\"IPV4V6\",\"ctnet\""
    	at "$at_port" "$at_command"
	echo "matching ISP: [$isp]" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	else
    	echo "No matching ISP: [$isp]" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	at_command="AT+CGDCONT=${define_connect},\"IPV4V6\",\"cbnet\""
        at "$at_port" "$at_command"
        echo "matching ISP: [$isp]" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	fi
	local at_command="AT+CGACT=1,${define_connect}"
        # Print log
        dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
        # Activate and dial
	sleep 3s
        at "${at_port}" "${at_command}"
    else
        # Dial
        ecm_dial "${at_port}" "${manufacturer}" "${define_connect}"
    fi
}

# Modem Manager dialing
# $1: Interface name
# $2: Connection definition
modemmanager_dial()
{
    local interface_name="$1"
    local define_connect="$2"

    # # Activate
    # local at_command="AT+CGACT=1,${define_connect}"
    # # Print log
    # dial_log "${at_command}" "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    # at "${at_port}" "${at_command}"

    # Bring up network interface
    ifup "${interface_name}";
}

# Check modem network connection
# $1: Config ID
# $2: Modem index
# $3: Dial mode
modem_network_task()
{
    local config_id="$1"
    local modem_no="$2"
    local mode="$3"

    # Get AT port, manufacturer, platform, connection definition, interface name
    local at_port=$(uci -q get modem.modem${modem_no}.at_port)
    local manufacturer=$(uci -q get modem.modem${modem_no}.manufacturer)
    local platform=$(uci -q get modem.modem${modem_no}.platform)
    local define_connect=$(uci -q get modem.modem${modem_no}.define_connect)
    local interface_name="wwan_5g_${modem_no}"
    local interface_name_ipv6="wwan6_5g_${modem_no}"
    local interface_network=$(uci -q get modem.modem${modem_no}.network_interface)
    # Re-fetch AT port if not acquired (avoid dialing during recognition)
    while [ -z "$manufacturer" ] || [ "$manufacturer" = "unknown" ]; do
        at_port=$(uci -q get modem.modem${modem_no}.at_port)
        manufacturer=$(uci -q get modem.modem${modem_no}.manufacturer)
        platform=$(uci -q get modem.modem${modem_no}.platform)
        define_connect=$(uci -q get modem.modem${modem_no}.define_connect)
        sleep 1s
    done

    # Reload configuration (ensure AT command delivery)
    # service modem reload

    # IPv4 address cache
    local ipv4_cache

    # Output log
    echo "[$(date +"%Y-%m-%d %H:%M:%S")] Start network task" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
    while true; do
        # Global
        local enable_dial=$(uci -q get modem.@global[0].enable_dial)
        if [ "$enable_dial" != "1" ]; then
            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] The dialing configuration has been disabled, this network task quit" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            break
        fi
        # Per-modem
        enable=$(uci -q get modem.${config_id}.enable)
        if [ "$enable" != "1" ]; then
            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] The modem has disabled dialing, this network task quit" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            break
        fi

        # Network connection check
	if ! ping -c 2 -w 5 -I "$interface_network" 223.5.5.5 > /dev/null 2>&1; then
        echo "[$(date +"%Y-%m-%d %H:%M:%S")] ping failed" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
	local at_command="AT+CGPADDR=${define_connect}"
        local ipv4=$(at ${at_port} ${at_command} | grep "+CGPADDR: " | sed -n '1p' | awk -F',' '{print $2}' | sed 's/"//g')

        if [ -z "$ipv4" ]; then
            [ "$mode" = "modemmanager" ] && {
                # When dial tool is modemmanager, no need to reconfigure connection definition
                continue
            }
            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Unable to get IPv4 address" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Redefine connect to ${define_connect}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            service modem reload

            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Modem dial" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            # Dial (for modems returning empty IPv4)
            case "$mode" in
                "gobinet") gobinet_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
                "ecm") ecm_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
                "rndis") rndis_dial "${at_port}" "${manufacturer}" "${platform}" "${define_connect}" "${modem_no}" ;;
                "modemmanager") modemmanager_dial "${interface_name}" "${define_connect}" ;;
                *) ecm_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
            esac
        elif [[ "$ipv4" = *"0.0.0.0"* ]]; then

            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Modem${modem_no} current IPv4 address : ${ipv4}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Modem dial" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            # Dial
            case "$mode" in
                "gobinet") gobinet_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
                "ecm") ecm_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
                "rndis") rndis_dial "${at_port}" "${manufacturer}" "${platform}" "${define_connect}" "${modem_no}" ;;
                "modemmanager") modemmanager_dial "${interface_name}" "${define_connect}" ;;
                *) ecm_dial "${at_port}" "${manufacturer}" "${define_connect}" ;;
            esac
            
        elif [ "$ipv4" != "$ipv4_cache" ]; then

            # Suppress log when initial cached IP is empty
            [ -n "$ipv4_cache" ] && {
                # Output log
                echo "[$(date +"%Y-%m-%d %H:%M:%S")] Modem${modem_no} IPv4 address has changed" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"
            }

            # Output log
            echo "[$(date +"%Y-%m-%d %H:%M:%S")] Modem${modem_no} current IPv4 address : ${ipv4}" >> "${MODEM_RUNDIR}/modem${modem_no}_dial.cache"

            # Cache current IP
            ipv4_cache="${ipv4}"

            # Reconfigure network interface (Fibocom FM350-GL)
            if [ "$manufacturer" = "fibocom" ] && [ "$platform" = "mediatek" ]; then
                reset_network_interface "${at_port}" "${define_connect}" "${modem_no}"
                sleep 3s
            fi

            [ "$mode" != "modemmanager" ] && {
                # Restart network interface
                ifup "${interface_name}"
                ifup "${interface_name_ipv6}"
            }
	    /etc/init.d/firewall restart
        fi
	fi
	rdisc6 eth2 &
        ndisc6 fe80::1 eth2 &
        sleep 5s
    done
}

modem_network_task "$1" "$2" "$3"
