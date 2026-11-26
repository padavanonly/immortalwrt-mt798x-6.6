#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

#Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"

#Preset
tdtech_presets()
{
    #Disable unsolicited modem reports
	at_command='AT^CURC=0'
	sh "${SCRIPT_DIR}/modem_at.sh" "$at_port" "$at_command"

    #Enable 5G SA/NSA access
	at_command='AT^C5GOPTION=1,3,3'
	sh "${SCRIPT_DIR}/modem_at.sh" "$at_port" "$at_command"
}

#Get DNS
# $1:ATSerial port
# $2:Connection definition
tdtech_get_dns()
{
    local at_port="$1"
    local define_connect="$2"

    [ -z "$define_connect" ] && {
        define_connect="1"
    }

    local public_dns1_ipv4="223.5.5.5"
    local public_dns2_ipv4="119.29.29.29"
    local public_dns1_ipv6="2400:3200::1" #BII: 240C::6666, Ali: 2400:3200::1, Tencent: 2402:4e00::
    local public_dns2_ipv6="2402:4e00::"

    #Get DNS address (IPv4)
    at_command="AT^DHCP=${define_connect}"
    local response=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "\^DHCP:" | sed -n '1p')

    local ipv4_dns1=$(echo "${response}" | awk -F',' '{print $5}')
    if [ -z "$ipv4_dns1" ]; then
        ipv4_dns1="${public_dns1_ipv4}"
    else
        #Split hex by byte and convert to decimal representation
        ipv4_dns1=$(echo "$ipv4_dns1" | awk '{
            for (i = length; i >= 1; i -= 2) {
                printf "%d.", "0x" substr($0, i-1, 2)
            }
        }')
        ipv4_dns1="${ipv4_dns1%?}"
    fi

    local ipv4_dns2=$(echo "${response}" | awk -F',' '{print $6}')
    if [ -z "$ipv4_dns2" ]; then
        ipv4_dns2="${public_dns1_ipv4}"
    else
        #Split hex by byte and convert to decimal representation
        ipv4_dns2=$(echo "$ipv4_dns2" | awk '{
            for (i = length; i >= 1; i -= 2) {
                printf "%d.", "0x" substr($0, i-1, 2)
            }
        }')
        ipv4_dns2="${ipv4_dns2%?}"
    fi

    #Get DNS address (IPv6)
    at_command="AT^DHCPV6=${define_connect}"
    local response=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "\^DHCPV6:" | sed -n '1p')

    local ipv6_dns1=$(echo "${response}" | awk -F',' '{print $5}')
    [ -z "$ipv6_dns1" ] && {
        ipv6_dns1="${public_dns1_ipv6}"
    }

    local ipv6_dns2=$(echo "${response}" | awk -F',' '{print $6}')
    [ -z "$ipv6_dns2" ] && {
        ipv6_dns2="${public_dns2_ipv6}"
    }

    dns="{
        \"dns\":{
            \"ipv4_dns1\":\"$ipv4_dns1\",
            \"ipv4_dns2\":\"$ipv4_dns2\",
            \"ipv6_dns1\":\"$ipv6_dns1\",
            \"ipv6_dns2\":\"$ipv6_dns2\"
        }
    }"

    echo "$dns"
}

#Get dial mode
# $1:ATSerial port
# $2:Platform
tdtech_get_mode()
{
    local at_port="$1"
    local platform="$2"

    at_command="AT^SETMODE?"
    local mode_num=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | sed -n '2p' | sed 's/\r//g')

    if [ -z "$mode_num" ]; then
        echo "unknown"
        return
    fi

    #Get chipset platform
	if [ -z "$platform" ]; then
		local modem_number=$(uci -q get modem.@global[0].modem_number)
        for i in $(seq 0 $((modem_number-1))); do
            local at_port_tmp=$(uci -q get modem.modem$i.at_port)
            if [ "$at_port" = "$at_port_tmp" ]; then
                platform=$(uci -q get modem.modem$i.platform)
                break
            fi
        done
	fi

    local mode
    case "$platform" in
        "hisilicon")
            case "$mode_num" in
                "0"|"2") mode="ecm" ;;
                "1"|"3"|"4"|"5") mode="ncm" ;;
                "6") mode="rndis" ;;
                "7") mode="mbim" ;;
                "8") mode="ppp" ;;
                *) mode="$mode_num" ;;
            esac
        ;;
        *)
            mode="$mode_num"
        ;;
    esac
    echo "${mode}"
}

#Set dial mode
# $1:ATSerial port
# $2:Dial mode config
tdtech_set_mode()
{
    local at_port="$1"
    local mode_config="$2"

    #Get chipset platform
    local platform
    local modem_number=$(uci -q get modem.@global[0].modem_number)
    for i in $(seq 0 $((modem_number-1))); do
        local at_port_tmp=$(uci -q get modem.modem$i.at_port)
        if [ "$at_port" = "$at_port_tmp" ]; then
            platform=$(uci -q get modem.modem$i.platform)
            break
        fi
    done

    #Get dial mode config
    local mode_num
    case "$platform" in
        "hisilicon")
            case "$mode_config" in
                "ecm") mode_num="0" ;;
                "ncm") mode_num="4" ;;
                *) mode_num="0" ;;
            esac
        ;;
        *)
            mode_num="0"
        ;;
    esac

    #Configure modem
    at_command="AT^SETMODE=${mode_num}"
    sh ${SCRIPT_DIR}/modem_at.sh ${at_port} "${at_command}"
}

#Get bit
# $1:Band name
tdtech_get_bit()
{
    local band_name="$1"

    local bit
    case "$band_name" in
        "DCS_1800") bit="8" ;;
        "E-GSM_900"|"E_GSM_900") bit="9" ;;
        "P-GSM_900"|"P_GSM_900") bit="10" ;;
        "GSM_450") bit="17" ;;
        "GSM_480") bit="18" ;;
        "GSM_750") bit="19" ;;
        "GSM_850") bit="20" ;;
        "R-GSM_900"|"R_GSM_900") bit="21" ;;
        "PCS_1900") bit="22" ;;
    esac

    echo "${bit}"
}

#Get band information
# $1:Band binary number
# $2:Supported bands
# $3:Band type (2G, 3G, 4G, 5G)
tdtech_get_band_info()
{
    local band_bin="$1"
    local support_band="$2"
    local band_type="$3"

    local band_info=""
    local support_band=$(echo "$support_band" | sed 's/,/ /g')
    if [ "$band_type" = "2G" ]; then

        for band in $support_band; do
            #Getbitbit
            local bit=$(tdtech_get_bit ${band})
            #Get value
            local enable=$(awk -v s="$band_bin" -v b="$bit" 'BEGIN { n = length(s); print (b >= 1 && n >= b) ? substr(s, n - b + 1, 1) : "0" }')
            [ -z "$enable" ] && enable="0"
            #Set band information
            # band_info=$(echo ${band_info} | jq '. += [{"'$band'":'$enable'}]')
            band_info="${band_info},{\"$band\":$enable}"
        done
    else
        #Band start, Prefix position
        local start_bit
        local band_prefix
        case "$band_type" in
            "3G")
                start_bit="23"
                band_prefix="WCDMA_B"
            ;;
            "4G")
                start_bit="1"
                band_prefix="LTE_BC"
            ;;
            "5G")
                start_bit="1"
                band_prefix="NR5G_N"
            ;;
        esac

        for band in $support_band; do
            #Get value (starting from start_bit)
            local bit=$((band + start_bit - 1))
            local enable=$(awk -v s="$band_bin" -v b="$bit" 'BEGIN { n = length(s); print (b >= 1 && n >= b) ? substr(s, n - b + 1, 1) : "0" }')
            [ -z "$enable" ] && enable="0"
            #Set band information
            # band_info=$(echo ${band_info} | jq '. += [{'$band_prefix$band':'$enable'}]')
            band_info="${band_info},{\"$band_prefix$band\":$enable}"
        done
    fi
    #Remove first comma
    band_info="[${band_info#,}]"
    # band_info="[${band_info%?}]"

    echo "${band_info}"
}

#Get network preference
# $1:ATSerial port
# $2:Data interface
# $3:Modem name
tdtech_get_network_prefer()
{
    local at_port="$1"
    local data_interface="$2"
    local modem_name="$3"

    at_command="AT^SYSCFGEX?"
    local response=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "\^SYSCFGEX:" | sed 's/\^SYSCFGEX://g')
    local network_type_num=$(echo "$response" | awk -F'"' '{print $2}')

    #Get network type
    local network_prefer_2g="0";
    local network_prefer_3g="0";
    local network_prefer_4g="0";
    local network_prefer_5g="0";

    #Match different network types
    local auto=$(echo "${network_type_num}" | grep "00")
    if [ -n "$auto" ]; then
        network_prefer_2g="1"
        network_prefer_3g="1"
        network_prefer_4g="1"
        network_prefer_5g="1"
    else
        local gsm=$(echo "${network_type_num}" | grep "01")
        local wcdma=$(echo "${network_type_num}" | grep "02")
        local lte=$(echo "${network_type_num}" | grep "03")
        local nr=$(echo "${network_type_num}" | grep "08")
        if [ -n "$gsm" ]; then
            network_prefer_2g="1"
        fi 
        if [ -n "$wcdma" ]; then
            network_prefer_3g="1"
        fi
        if [ -n "$lte" ]; then
            network_prefer_4g="1"
        fi
        if [ -n "$nr" ]; then
            network_prefer_5g="1"
        fi
    fi

	#Get modem info
    local data_if_lc=$(echo "$data_interface" | tr 'A-Z' 'a-z')
    local modem_name_lc=$(echo "$modem_name" | tr 'A-Z' 'a-z')
    local modem_info=$(jq '.modem_support["'"$data_if_lc"'"]["'"$modem_name_lc"'"]' ${SCRIPT_DIR}/modem_support.json)

    #Get supported modem bands
    local support_2g_band=$(echo "$modem_info" | jq -r '.band_2g')
    local support_3g_band=$(echo "$modem_info" | jq -r '.band_3g')
    local support_4g_band=$(echo "$modem_info" | jq -r '.band_4g')
    local support_5g_band=$(echo "$modem_info" | jq -r '.band_5g')

    #Get band information
    local band_hex_2g_3g=$(echo "$response" | awk -F',' '{print $2}')
    #Hex to binary conversion
    local bin_2g_3g=$(echo "obase=2; ibase=16; $band_hex_2g_3g" | bc)
    local band_2g_info=$(tdtech_get_band_info "${bin_2g_3g}" "${support_2g_band}" "2G")
    local band_3g_info=$(tdtech_get_band_info "${bin_2g_3g}" "${support_3g_band}" "3G")

    local band_hex_4g_5g=$(echo "$response" | awk -F',' '{print $5}' | sed 's/\r//g')
    #Hex to binary conversion
    local bin_4g_5g=$(echo "obase=2; ibase=16; $band_hex_4g_5g" | bc)
    local band_4g_info=$(tdtech_get_band_info "${bin_4g_5g}" "${support_4g_band}" "4G")
    local band_5g_info=$(tdtech_get_band_info "${bin_4g_5g}" "${support_5g_band}" "5G")

    #Generate network preference
    local network_prefer="{
        \"network_prefer\":[
            {\"2G\":{
                \"enable\":$network_prefer_2g,
                \"band\":$band_2g_info
            }},
            {\"3G\":{
                \"enable\":$network_prefer_3g,
                \"band\":$band_3g_info
            }},
            {\"4G\":{
                \"enable\":$network_prefer_4g,
                \"band\":$band_4g_info
            }},
            {\"5G\":{
                \"enable\":$network_prefer_5g,
                \"band\":$band_5g_info
            }}
        ]
    }"
    echo "${network_prefer}"
}

#Set network preference
# $1:ATSerial port
# $2:Network preference config
tdtech_set_network_prefer()
{
    local at_port="$1"
    local network_prefer="$2"

    #Get enabled network preference
    local enable_5g=$(echo "$network_prefer" | jq -r '.["5G"].enable')
    local enable_4g=$(echo "$network_prefer" | jq -r '.["4G"].enable')
    local enable_3g=$(echo "$network_prefer" | jq -r '.["3G"].enable')
    local enable_2g=$(echo "$network_prefer" | jq -r '.["2G"].enable')

    #Get network preference config
    local network_prefer_config
    [ "$enable_5g" = "1" ] && network_prefer_config="${network_prefer_config}08"
    [ "$enable_4g" = "1" ] && network_prefer_config="${network_prefer_config}03"
    [ "$enable_3g" = "1" ] && network_prefer_config="${network_prefer_config}02"
    [ "$enable_2g" = "1" ] && network_prefer_config="${network_prefer_config}01"

    [ -z "$network_prefer_config" ] && network_prefer_config="99"

    #Configure modem
    at_command='AT^SYSCFGEX="'${network_prefer_config}'",40000000,1,2,40000000,,'
    sh ${SCRIPT_DIR}/modem_at.sh "${at_port}" "${at_command}"
}

#Setband
# $1:ATSerial port
# $2:Band preferenceConfig
tdtech_set_band_prefer()
{
    local at_port="$1"
    local network_prefer="$2"

    #Get selected count
    local count=$(echo "$network_prefer" | grep -o "1" | wc -l)
    #GetEach preference value
    local network_prefer_5g=$(echo "$network_prefer" | jq -r '.["5G"]')
    local network_prefer_4g=$(echo "$network_prefer" | jq -r '.["4G"]')
    local network_prefer_3g=$(echo "$network_prefer" | jq -r '.["3G"]')
    local network_prefer_2g=$(echo "$network_prefer" | jq -r '.["2G"]')

    #Get enabled network preference
    local enable_5g=$(echo "$network_prefer_5g" | jq -r '.enable')
    local enable_4g=$(echo "$network_prefer_4g" | jq -r '.enable')
    local enable_3g=$(echo "$network_prefer_3g" | jq -r '.enable')
    local enable_2g=$(echo "$network_prefer_2g" | jq -r '.enable')

    #Get network preference configand band preferenceConfig
    local network_prefer_config
    local band_hex_2g_3g=0
    local band_hex_4g_5g=0

    [ "$enable_5g" = "1" ] && {
        network_prefer_config="${network_prefer_config}08"
        local band_tmp=$(echo "$network_prefer_5g" | jq -r '.band[]')
        
        local i=0
        local bands=$(echo "$band_tmp" | jq -r 'to_entries | .[] | .key')
        #Iterateband value
        for band in $bands; do
            local value=$(echo "$network_prefer_5g" | jq -r '.band['"$i"']["'"$band"'"]')
            [ "$value" = "1" ] && {
                #Getbitbit
                local bit=$(echo "$band" | sed 's/NR5G_N//g')
                #Get value
                local result=$(echo "obase=16; ibase=10; 2^($bit-1)" | bc)
                band_hex_4g_5g=$(echo "obase=16; ibase=16; $band_hex_4g_5g + $result" | bc)
            }
            i=$((i+1))
        done
    }

    [ "$enable_4g" = "1" ] && {
        network_prefer_config="${network_prefer_config}03"
        local band_tmp=$(echo "$network_prefer_4g" | jq -r '.band[]')

        local i=0
        local bands=$(echo "$band_tmp" | jq -r 'to_entries | .[] | .key')
        #Iterateband value
        for band in $bands; do
            local value=$(echo "$network_prefer_4g" | jq -r '.band['"$i"']["'"$band"'"]')
            [ "$value" = "1" ] && {
                #Getbitbit
                local bit=$(echo "$band" | sed 's/LTE_BC//g')
                #Get value
                local result=$(echo "obase=16; ibase=10; 2^($bit-1)" | bc)
                band_hex_4g_5g=$(echo "obase=16; ibase=16; $band_hex_4g_5g + $result" | bc)
            }
            i=$((i+1))
        done
    }

    [ "$enable_3g" = "1" ] && {
        network_prefer_config="${network_prefer_config}02"
        local band_tmp=$(echo "$network_prefer_3g" | jq -r '.band[]')

        local i=0
        local bands=$(echo "$band_tmp" | jq -r 'to_entries | .[] | .key')
        #Iterateband value
        for band in $bands; do
            local value=$(echo "$network_prefer_3g" | jq -r '.band['"$i"']["'"$band"'"]')
            [ "$value" = "1" ] && {
                #Getbitbit
                local bit=$(echo "$band" | sed 's/WCDMA_B//g')
                #Get value
                local result=$(echo "obase=16; ibase=10; 2^($bit+22-1)" | bc)
                band_hex_2g_3g=$(echo "obase=16; ibase=16; $band_hex_2g_3g + $result" | bc)
            }
            i=$((i+1))
        done
    }

    [ "$enable_2g" = "1" ] && {
        network_prefer_config="${network_prefer_config}01"
        local band_tmp=$(echo "$network_prefer_2g" | jq -r '.band[]')

        local i=0
        local bands=$(echo "$band_tmp" | jq -r 'to_entries | .[] | .key')
        #Iterateband value
        for band in $bands; do
            # band_format=$(echo "$band" | sed 's/-/_/g')
            local value=$(echo "$network_prefer_2g" | jq -r '.band['"$i"']["'"$band"'"]')
            [ "$value" = "1" ] && {
                #Getbitbit
                local bit=$(tdtech_get_bit ${band})
                #Get value
                local result=$(echo "obase=16; ibase=10; 2^($bit-1)" | bc)
                band_hex_2g_3g=$(echo "obase=16; ibase=16; $band_hex_2g_3g + $result" | bc)
            }
            i=$((i+1))
        done
    }

    [ -z "$network_prefer_config" ] && network_prefer_config="99"

    #Configure modem
    at_command='AT^SYSCFGEX="'${network_prefer_config}'",'"${band_hex_2g_3g},1,2,${band_hex_4g_5g},,"
    sh ${SCRIPT_DIR}/modem_at.sh "${at_port}" "${at_command}"
}

#Get voltage
# $1:ATSerial port
tdtech_get_voltage()
{
    local at_port="$1"
    
    # #Voltage (Voltage)
    # at_command="AT+ADCREAD=0"
	# local voltage=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "+ADCREAD:" | awk -F' ' '{print $2}' | sed 's/\r//g')
    # voltage=$(awk "BEGIN{ printf \"%.2f\", $voltage / 1000000 }" | sed 's/\.*0*$//')
    # echo "${voltage}"
}

#Get temperature
# $1:ATSerial port
tdtech_get_temperature()
{
    local at_port="$1"
    
    #Temperature (Temperature)
    at_command="AT^CHIPTEMP?"
	response=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "\^CHIPTEMP" | awk -F',' '{print $6}' | sed 's/\r//g')

    local temperature
	if [ -n "$response" ]; then
        response=$(awk "BEGIN{ printf \"%.2f\", $response / 10 }" | sed 's/\.*0*$//')
		temperature="${response}$(printf "\xc2\xb0")C"
    else
        temperature="NaN $(printf "\xc2\xb0")C"
	fi

    echo "${temperature}"
}

#Get connection status
# $1:ATSerial port
# $2:Connection definition
tdtech_get_connect_status()
{
    local at_port="$1"
    local define_connect="$2"

    #Default value is1
    [ -z "$define_connect" ] && {
        define_connect="1"
    }

    at_command="AT+CGPADDR=${define_connect}"
    local ipv4=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "+CGPADDR: " | awk -F'"' '{print $2}')
    local not_ip="0.0.0.0"

    #Set connection status
    local connect_status
    if [ -z "$ipv4" ] || [[ "$ipv4" = *"$not_ip"* ]]; then
        connect_status="disconnect"
    else
        connect_status="connect"
    fi

    echo "${connect_status}"
}

#Basic info
tdtech_base_info()
{
    debug "TDTech base info"

    #Name (Name)
    at_command="AT+CGMM"
    name=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | sed -n '2p' | sed 's/\r//g')
    #Manufacturer (Manufacturer)
    at_command="AT+CGMI"
    manufacturer=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | sed -n '2p' | sed 's/\r//g')
    #Revision (Firmware version)
    at_command="AT+CGMR"
    revision=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | sed -n '2p' | sed 's/\r//g')

    #Mode (Dial mode)
    mode=$(tdtech_get_mode ${at_port} ${platform} | tr 'a-z' 'A-Z')

    #Temperature (Temperature)
    temperature=$(tdtech_get_temperature ${at_port})
}

#GetSIMCard status
# $1:SIM status flag
tdtech_get_sim_status()
{
    local sim_status
    case $1 in
        "") sim_status="miss" ;;
        *"ERROR"*) sim_status="miss" ;;
        *"READY"*) sim_status="ready" ;;
        *"SIM PIN"*) sim_status="MT is waiting SIM PIN to be given" ;;
        *"SIM PUK"*) sim_status="MT is waiting SIM PUK to be given" ;;
        *"PH-FSIM PIN"*) sim_status="MT is waiting phone-to-SIM card password to be given" ;;
        *"PH-FSIM PIN"*) sim_status="MT is waiting phone-to-very first SIM card password to be given" ;;
        *"PH-FSIM PUK"*) sim_status="MT is waiting phone-to-very first SIM card unblocking password to be given" ;;
        *"SIM PIN2"*) sim_status="MT is waiting SIM PIN2 to be given" ;;
        *"SIM PUK2"*) sim_status="MT is waiting SIM PUK2 to be given" ;;
        *"PH-NET PIN"*) sim_status="MT is waiting network personalization password to be given" ;;
        *"PH-NET PUK"*) sim_status="MT is waiting network personalization unblocking password to be given" ;;
        *"PH-NETSUB PIN"*) sim_status="MT is waiting network subset personalization password to be given" ;;
        *"PH-NETSUB PUK"*) sim_status="MT is waiting network subset personalization unblocking password to be given" ;;
        *"PH-SP PIN"*) sim_status="MT is waiting service provider personalization password to be given" ;;
        *"PH-SP PUK"*) sim_status="MT is waiting service provider personalization unblocking password to be given" ;;
        *"PH-CORP PIN"*) sim_status="MT is waiting corporate personalization password to be given" ;;
        *"PH-CORP PUK"*) sim_status="MT is waiting corporate personalization unblocking password to be given" ;;
        *) sim_status="unknown" ;;
    esac
    echo "${sim_status}"
}

#SIMCard info
tdtech_sim_info()
{
    debug "TDTech sim info"
    
    #SIM Slot (SIMSIM slot)
    # at_command="AT^SIMSLOT?"
	# response=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "\^SIMSLOT:" | awk -F': ' '{print $2}' | awk -F',' '{print $2}')

    # if [ "$response" != "0" ]; then
    #     sim_slot="1"
    # else
    #     sim_slot="2"
    # fi
    sim_slot="1"

    #IMEI (IMEI)
    at_command="AT+CGSN"
	imei=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | sed -n '2p' | sed 's/\r//g')

    #SIM Status (SIMStatus)
    at_command="AT+CPIN?"
	sim_status_flag=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "+CPIN: ")
    sim_status=$(tdtech_get_sim_status "$sim_status_flag")

    if [ "$sim_status" != "ready" ]; then
        return
    fi

    ISP_Read(){
            operatorCode=$(sendat 1 'AT^EONS=2' | awk -F ',' '{print $2}' | sed '/^$/d')
            case "$operatorCode" in
            "46000" | "46002" | "46004" | "46007" | "46008" | "46020")
                isp="China Mobile"
                ;;
            "46001" | "46006" | "46009")
                isp="China Unicom"
                ;;
            "46003" | "46005" | "46011")
                isp="China Telecom"
                ;;
            "46015")
                isp="China Broadcast"
                ;;
            *)
                isp="Unknown"
                ;;
            esac
    }
    ISP_Read 
    # if [ "$isp" = "CHN-CMCC" ] || [ "$isp" = "CMCC" ]|| [ "$isp" = "46000" ]; then
    #     isp="China Mobile"
    # elif [ "$isp" = "CHN-UNICOM" ] || [ "$isp" = "UNICOM" ] || [ "$isp" = "46001" ]; then
    #     isp="China Unicom"
    # elif [ "$isp" = "CHN-CT" ] || [ "$isp" = "CT" ] || [ "$isp" = "46011" ]; then
    #     isp="China Telecom"
    # fi

    #SIM Number (SIMCard number, Phone number)
    at_command="AT+CNUM"
	sim_number=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "+CNUM: " | awk -F'"' '{print $2}')
    [ -z "$sim_number" ] && {
        sim_number=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "+CNUM: " | awk -F'"' '{print $4}')
    }
	
    #IMSI (IMSI)
    at_command="AT+CIMI"
	imsi=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | sed -n '2p' | sed 's/\r//g')

    #ICCID (ICCID)
    at_command="AT+ICCID"
	iccid=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep -o "+ICCID:[ ]*[-0-9]\+" | grep -o "[-0-9]\{1,4\}")
}

#Get network type
# $1:Network typenumber
tdtech_get_rat()
{
    local rat
    case $1 in
		"0"|"1"|"3"|"8") rat="GSM" ;;
		"2"|"4"|"5"|"6"|"9"|"10") rat="WCDMA" ;;
        "7") rat="LTE" ;;
        "11"|"12") rat="NR" ;;
	esac
    echo "${rat}"
}

#GetRSSI (4G)
# $1:RSSI value
tdtech_get_rssi()
{
    local rssi
    case $1 in
		"99") rssi="unknown" ;;
		* )  rssi=$((2 * $1 - 113)) ;;
	esac
    echo "$rssi"
}

#Network info
tdtech_network_info()
{
    debug "TDTech network info"

    #Connect Status (Connection status)
    connect_status=$(tdtech_get_connect_status ${at_port} ${define_connect})
    if [ "$connect_status" != "connect" ]; then
        return
    fi

    #Network Type (Network type)
    at_command="AT^SYSINFOEX"
    network_type=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "\^SYSINFOEX:" | awk -F'"' '{print $4}')

    [ -z "$network_type" ] && {
        at_command='AT+COPS?'
        local rat_num=$(sh ${SCRIPT_DIR}/modem_at.sh ${at_port} ${at_command} | grep "+COPS:" | awk -F',' '{print $4}' | sed 's/\r//g')
        network_type=$(tdtech_get_rat ${rat_num})
    }

    #Set network type to5Gwhen, RSSI usesRSRPinstead
    # at_command="AT+GTCSQNREN=1"
    # sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command

    #CSQ (Signal strength)
    at_command="AT+CSQ"
    response=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "+CSQ:" | sed 's/+CSQ: //g' | sed 's/\r//g')

    #RSSI (4GRSSI)
    # rssi_num=$(echo $response | awk -F',' '{print $1}')
    # rssi=$(tdtech_get_rssi $rssi_num)
    #BER (4GChannel BER)
    # ber=$(echo $response | awk -F',' '{print $2}')

    # #PER (Signal strength)
    # if [ -n "$csq" ]; then
    #     per=$(($csq * 100/31))"%"
    # fi

    #AMBR (Max bit rate)
    at_command="AT^DSAMBR=${define_connect}"
    response=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "\^DSAMBR:" | sed 's/\^DSAMBR: //g' | sed 's/\r//g')
    ambr_ul_tmp=$(echo "$response" | awk -F',' '{print $2}')
    ambr_dl_tmp=$(echo "$response" | awk -F',' '{print $3}')

    #AMBR UL (Uplink AMBR, Unit, Mbps)
    ambr_ul=$(awk "BEGIN{ printf \"%.2f\", $ambr_dl_tmp / 1000 }" | sed 's/\.*0*$//')
    #AMBR DL (Downlink AMBR, Unit, Mbps)
    ambr_dl=$(awk "BEGIN{ printf \"%.2f\", $ambr_ul_tmp / 1000 }" | sed 's/\.*0*$//')

    # #Rate statistics
    # at_command='AT^DSFLOWQRY'
    # response=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "\^DSFLOWRPT:" | sed 's/\^DSFLOWRPT: //g' | sed 's/\r//g')

    # #Current upload rate (Bytes/s)
    # tx_rate=$(echo $response | awk -F',' '{print $1}')

    # #Current download rate (Bytes/s)
    # rx_rate=$(echo $response | awk -F',' '{print $2}')
}

#GetNRSubcarrier spacing
# $1:NR subcarrier spacing value
tdtech_get_scs()
{
    local scs
	case $1 in
		"0") scs="15" ;;
		"1") scs="30" ;;
        "2") scs="60" ;;
        "3") scs="120" ;;
        "4") scs="240" ;;
        *) scs=$(awk "BEGIN{ print 2^$1 * 15 }") ;;
	esac
    echo "$scs"
}

#Get band
# $1:Network type
# $2:Band number
tdtech_get_band()
{
    local band
    case $1 in
        "GSM")
            case $2 in
                "0") band="850" ;;
                "1") band="900" ;;
                "2") band="1800" ;;
                "3") band="1900" ;;
            esac
        ;;
		"WCDMA") band="$2" ;;
		"LTE") band="$(($2-100))" ;;
        "NR") band="$2" band="${band#*50}" ;;
	esac
    echo "$band"
}

#Cell info
tdtech_cell_info()
{
    debug "TDTech cell info"

    at_command="AT^MONSC"
    response=$(sh ${SCRIPT_DIR}/modem_at.sh $at_port $at_command | grep "\^MONSC:" | sed 's/\^MONSC: //')
    
    local rat=$(echo "$response" | awk -F',' '{print $1}')

    case $rat in
        "NR")
            network_mode="NR5G-SA Mode"
            nr_mcc=$(echo "$response" | awk -F',' '{print $2}')
            nr_mnc=$(echo "$response" | awk -F',' '{print $3}')
            nr_arfcn=$(echo "$response" | awk -F',' '{print $4}')
            nr_scs_num=$(echo "$response" | awk -F',' '{print $5}')
            nr_scs=$(tdtech_get_scs ${nr_scs_num})
            nr_cell_id_hex=$(echo "$response" | awk -F',' '{print $6}')
            nr_cell_id=$(echo "ibase=16; $nr_cell_id_hex" | bc)
            nr_physical_cell_id_hex=$(echo "$response" | awk -F',' '{print $7}')
            nr_physical_cell_id=$(echo "ibase=16; $nr_physical_cell_id_hex" | bc)
            nr_tac=$(echo "$response" | awk -F',' '{print $8}')
            nr_rsrp=$(echo "$response" | awk -F',' '{print $9}')
            nr_rsrq=$(echo "$response" | awk -F',' '{print $10}')
            nr_sinr=$(echo "$response" | awk -F',' '{print $11}' | sed 's/\r//g')
        ;;
        "LTE-NR")
            network_mode="EN-DC Mode"
            #LTE
            endc_lte_mcc=$(echo "$response" | awk -F',' '{print $2}')
            endc_lte_mnc=$(echo "$response" | awk -F',' '{print $3}')
            endc_lte_earfcn=$(echo "$response" | awk -F',' '{print $4}')
            endc_lte_cell_id_hex=$(echo "$response" | awk -F',' '{print $5}')
            endc_lte_cell_id=$(echo "ibase=16; $endc_lte_cell_id_hex" | bc)
            endc_lte_physical_cell_id_hex=$(echo "$response" | awk -F',' '{print $6}')
            endc_lte_physical_cell_id=$(echo "ibase=16; $endc_lte_physical_cell_id_hex" | bc)
            endc_lte_tac=$(echo "$response" | awk -F',' '{print $7}')
            endc_lte_rsrp=$(echo "$response" | awk -F',' '{print $8}')
            endc_lte_rsrq=$(echo "$response" | awk -F',' '{print $9}')
            endc_lte_rxlev=$(echo "$response" | awk -F',' '{print $10}' | sed 's/\r//g')
            #NR5G-NSA
            endc_nr_mcc=$(echo "$response" | awk -F',' '{print $2}')
            endc_nr_mnc=$(echo "$response" | awk -F',' '{print $3}')
            endc_nr_arfcn=$(echo "$response" | awk -F',' '{print $4}')
            endc_nr_scs_num=$(echo "$response" | awk -F',' '{print $5}')
            endc_nr_scs=$(tdtech_get_scs ${nr_scs_num})
            endc_nr_cell_id_hex=$(echo "$response" | awk -F',' '{print $6}')
            endc_nr_cell_id=$(echo "ibase=16; $endc_nr_cell_id_hex" | bc)
            endc_nr_physical_cell_id_hex=$(echo "$response" | awk -F',' '{print $7}')
            endc_nr_physical_cell_id=$(echo "ibase=16; $endc_nr_physical_cell_id_hex" | bc)
            endc_nr_tac=$(echo "$response" | awk -F',' '{print $8}')
            endc_nr_rsrp=$(echo "$response" | awk -F',' '{print $9}')
            endc_nr_rsrq=$(echo "$response" | awk -F',' '{print $10}')
            endc_nr_sinr=$(echo "$response" | awk -F',' '{print $11}' | sed 's/\r//g')
        ;;
        "LTE"|"eMTC"|"NB-IoT")
            network_mode="LTE Mode"
            lte_mcc=$(echo "$response" | awk -F',' '{print $2}')
            lte_mnc=$(echo "$response" | awk -F',' '{print $3}')
            lte_earfcn=$(echo "$response" | awk -F',' '{print $4}')
            lte_cell_id_hex=$(echo "$response" | awk -F',' '{print $5}')
            lte_cell_id=$(echo "ibase=16; $lte_cell_id_hex" | bc)
            lte_physical_cell_id_hex=$(echo "$response" | awk -F',' '{print $6}')
            lte_physical_cell_id=$(echo "ibase=16; $lte_physical_cell_id_hex" | bc)
            lte_tac=$(echo "$response" | awk -F',' '{print $7}')
            lte_rsrp=$(echo "$response" | awk -F',' '{print $8}')
            lte_rsrq=$(echo "$response" | awk -F',' '{print $9}')
            lte_rxlev=$(echo "$response" | awk -F',' '{print $10}' | sed 's/\r//g')
        ;;
        "WCDMA"|"TD-SCDMA"|"UMTS")
            network_mode="WCDMA Mode"
            wcdma_mcc=$(echo "$response" | awk -F',' '{print $2}')
            wcdma_mnc=$(echo "$response" | awk -F',' '{print $3}')
            wcdma_arfcn=$(echo "$response" | awk -F',' '{print $4}')
            wcdma_psc=$(echo "$response" | awk -F',' '{print $5}')
            wcdma_cell_id_hex=$(echo "$response" | awk -F',' '{print $6}')
            wcdma_cell_id=$(echo "ibase=16; $wcdma_cell_id_hex" | bc)
            wcdma_lac=$(echo "$response" | awk -F',' '{print $7}')
            wcdma_rscp=$(echo "$response" | awk -F',' '{print $8}')
            wcdma_rxlev=$(echo "$response" | awk -F',' '{print $9}')
            wcdma_ecn0=$(echo "$response" | awk -F',' '{print $10}')
            wcdma_drx=$(echo "$response" | awk -F',' '{print $11}')
            wcdma_ura=$(echo "$response" | awk -F',' '{print $12}' | sed 's/\r//g')
        ;;
        "GSM")
            network_mode="GSM Mode"
            gsm_mcc=$(echo "$response" | awk -F',' '{print $2}')
            gsm_mnc=$(echo "$response" | awk -F',' '{print $3}')
            gsm_band_num=$(echo "$response" | awk -F',' '{print $4}')
            gsm_band=$(tdtech_get_band "GSM" ${gsm_band_num})
            gsm_arfcn=$(echo "$response" | awk -F',' '{print $5}')
            gsm_bsic=$(echo "$response" | awk -F',' '{print $6}')
            gsm_cell_id_hex=$(echo "$response" | awk -F',' '{print $7}')
            gsm_cell_id=$(echo "ibase=16; $gsm_cell_id_hex" | bc)
            gsm_lac=$(echo "$response" | awk -F',' '{print $8}')
            gsm_rxlev=$(echo "$response" | awk -F',' '{print $9}')
            gsm_rx_quality=$(echo "$response" | awk -F',' '{print $10}')
            gsm_ta=$(echo "$response" | awk -F',' '{print $11}' | sed 's/\r//g')
        ;;
    esac
}

#GetTD TechModem info
# $1:ATSerial port
# $2:Platform
# $3:Connection definition
get_tdtech_info()
{
    debug "get tdtech info"
    #SetATSerial port
    at_port="$1"
    platform="$2"
    define_connect="$3"

    #Basic info
    tdtech_base_info

	#SIMCard info
    tdtech_sim_info
    if [ "$sim_status" != "ready" ]; then
        return
    fi

    #Network info
    tdtech_network_info
    if [ "$connect_status" != "connect" ]; then
        return
    fi

    #Cell info
    tdtech_cell_info
}
