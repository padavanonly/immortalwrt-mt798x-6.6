#!/bin/sh

LOG_TAG="wapp"

log_i() {
    logger -t "$LOG_TAG" "INFO: $*"
}

log_e() {
    logger -t "$LOG_TAG" "ERROR: $*"
}

run_cmd() {
    log_i "exec: $*"
    "$@"
    local rc=$?
    if [ "$rc" -eq 0 ]; then
        log_i "ok($rc): $*"
    else
        log_e "fail($rc): $*"
    fi
    return "$rc"
}

run_bg() {
    log_i "exec(bg): $*"
    "$@" >/dev/null 2>&1 &
    log_i "started pid=$!: $*"
}

log_i "startwapp.sh start"

run_cmd sh -c "killall bs20 2>/dev/null || true"
run_cmd sh -c "killall wapp 2>/dev/null || true"
br0_mac=$(cat /sys/class/net/br-lan/address)
ctrlr_al_mac=$br0_mac
agent_al_mac=$br0_mac
ra0=0
rax0=0

log_i "bridge mac: ${br0_mac}"

uci_get_default() {
    local value
    value="$(uci -q get "$1" 2>/dev/null)"
    if [ -n "$value" ]; then
        printf '%s' "$value"
    else
        printf '%s' "$2"
    fi
}

wapp_enabled=0
for dev in $(uci -q show wireless | sed -n 's/^wireless\.\([^.]*\)=wifi-device$/\1/p'); do
    dev_type="$(uci_get_default wireless.${dev}.type "")"
    dev_wapp="$(uci_get_default wireless.${dev}.wapp 0)"
    log_i "check device=${dev} type=${dev_type} wapp=${dev_wapp}"
    [ "$dev_type" = "mtwifi" ] || continue
    if [ "$(uci_get_default wireless.${dev}.wapp 0)" -eq "1" ]; then
        wapp_enabled=1
        break
    fi
done

if [ "$wapp_enabled" -ne "1" ]; then
    log_i "wapp switch disabled, exit"
    exit 0
fi

log_i "wapp switch enabled, continue"

sleep 2

run_cmd sed -i "s/map_controller_alid=.*/map_controller_alid=${ctrlr_al_mac}/g" /etc/map/1905d.cfg
run_cmd sed -i "s/map_agent_alid=.*/map_agent_alid=${agent_al_mac}/g" /etc/map/1905d.cfg

    ra0_7981="$(uci_get_default wireless.MT7981_1_1.bandsteering 0)"
    ra0_7986="$(uci_get_default wireless.MT7986_1_1.bandsteering 0)"
    if [ "$ra0_7981" -eq "1" ] || [ "$ra0_7986" -eq "1" ]; then
    ra0=1
    rax0=1
    fi
    
    ra0_7981="$(uci_get_default wireless.MT7981_1_1.ieee80211r 0)"
    ra0_7986="$(uci_get_default wireless.MT7986_1_1.ieee80211r 0)"
    if [ "$ra0_7981" -eq "1" ] || [ "$ra0_7986" -eq "1" ]; then
    ra0=1
    fi
    
    ra0_7981="$(uci_get_default wireless.default_MT7981_1_2.steeringthresold 0)"
    ra0_7986="$(uci_get_default wireless.default_MT7986_1_2.steeringthresold 0)"
    if [ "$ra0_7981" -lt "0" ] || [ "$ra0_7986" -lt "0" ]; then
    ra0=1
    fi
        
    ra0_7981="$(uci_get_default wireless.default_MT7981_1_1.disabled 0)"
    ra0_7986="$(uci_get_default wireless.default_MT7986_1_1.disabled 0)"
    if [ "$ra0_7981" -eq "1" ] || [ "$ra0_7986" -eq "1" ]; then
    ra0=0
    rax0=0
    fi
    
    rax0_7981="$(uci_get_default wireless.MT7981_1_2.ieee80211r 0)"
    rax0_7986="$(uci_get_default wireless.MT7986_1_2.ieee80211r 0)"
    if [ "$rax0_7981" -eq "1" ] || [ "$rax0_7986" -eq "1" ]; then
    rax0=1
    fi
   
    rax0_7981="$(uci_get_default wireless.default_MT7981_1_2.steeringthresold 0)"
    rax0_7986="$(uci_get_default wireless.default_MT7986_1_2.steeringthresold 0)"
    if [ "$rax0_7981" -lt "0" ] || [ "$rax0_7986" -lt "0" ]; then
    rax0=1
    fi
    
    rax0_7981="$(uci_get_default wireless.default_MT7981_1_2.disabled 0)"
    rax0_7986="$(uci_get_default wireless.default_MT7986_1_2.disabled 0)"
    if [ "$rax0_7981" -eq "1" ] || [ "$rax0_7986" -eq "1" ]; then
    rax0=0
    fi

    log_i "decision flags: ra0=${ra0}, rax0=${rax0}"
     
    if [ "$rax0" -eq "1" ] && [ "$ra0" -eq "1" ]  ; then
    run_bg wapp -d1 -v2 -cra0 -crax0
    elif [ "$ra0" -eq "1" ] && [ "$rax0" -eq "0" ] ; then
    run_bg wapp -d1 -v2 -cra0
    elif [ "$rax0" -eq "1" ] && [ "$ra0" -eq "0" ] ; then
    run_bg wapp -d1 -v2 -crax0
    else
    log_i "no interface requires wapp start"
    fi
sleep 1
if [ "$rax0" -eq "1" ] || [ "$ra0" -eq "1" ]  ; then
run_cmd iwpriv ra0 set mapR2Enable=0
run_cmd iwpriv ra0 set mapTSEnable=0
run_cmd iwpriv ra0 set mapR3Enable=0
run_cmd iwpriv ra0 set DppEnable=0
run_cmd iwpriv rax0 set mapR2Enable=0
run_cmd iwpriv rax0 set mapTSEnable=0
run_cmd iwpriv rax0 set mapR3Enable=0
run_cmd iwpriv rax0 set DppEnable=0
run_cmd iwpriv ra0 set mapEnable=2
run_cmd iwpriv rax0 set mapEnable=2
run_bg bs20
run_cmd wappctrl rax0 mbo reset_default
run_cmd wappctrl ra0 mbo reset_default
rax0_7981="$(uci -q get wireless.default_MT7981_1_2.steeringbssid 2>/dev/null)"
rax0_7986="$(uci -q get wireless.default_MT7986_1_2.steeringbssid 2>/dev/null)"
if [ -n "$rax0_7981" ]; then
	run_cmd /sbin/setbssid rax0 "$rax0_7981"
fi

if [ -n "$rax0_7986" ]; then
	run_cmd /sbin/setbssid rax0 "$rax0_7986"
fi

ra0_7981="$(uci -q get wireless.default_MT7981_1_1.steeringbssid 2>/dev/null)"
ra0_7986="$(uci -q get wireless.default_MT7986_1_1.steeringbssid 2>/dev/null)"
if [ -n "$ra0_7981" ]; then
	run_cmd /sbin/setbssid ra0 "$ra0_7981"
fi

if [ -n "$ra0_7986" ]; then
	run_cmd /sbin/setbssid ra0 "$ra0_7986"
fi

fi

log_i "startwapp.sh done"
