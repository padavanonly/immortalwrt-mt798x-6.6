#!/bin/sh
# Usage:
# - enable: ./mt76_sns_dump.sh &
# - disable: killall mt76_sns_dump.sh

do_cmd() {
    local cmd="$1"

    echo "[cmd] $cmd"
    eval $cmd
    echo ""
}

dump_board_info() {
    # get bootfile name
    part=$(cat /proc/mtd | grep "u-boot-env" | cut -d ":" -f 1)
    part_size=$(cat /proc/mtd | grep "u-boot-env" | cut -d " " -f 3)
    echo "/dev/${part} 0x0000 0x20000 0x${part_size}" > /etc/fw_env.config
    do_cmd "fw_printenv bootfile"

    do_cmd "uname -a"
    do_cmd "lspci"
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/fw_version"
    do_cmd "cat /etc/config/wireless"
    do_cmd "cat /etc/config/network"
    do_cmd "iw dev"
}

dump_connection_info() {
    iw dev | awk '
    $1 == "Interface" {iface=$2}
    $1 == "type" {print iface, $2}
    ' | while read iface type; do
    if [ "$type" = "AP" ]; then
        count=$(iw dev "$iface" station dump | grep Station | wc -l)
        echo -e "\033[1m$iface (AP): $count clients\033[0m"
        hostapd_cli -i "$iface" get_disconn_counter
    elif [ "$type" = "STA" ]; then
        echo -e "\033[1m$iface (STA):\033[0m"
        iw dev "$iface" link
    else
        echo "$iface ($type): Skip"
    fi
    done
}

dump_token_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/token"
}

dump_ple_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/ple_info"
}

dump_pse_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/pse_info"
}

dump_mib_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/*/mibinfo"
}

dump_drop_stats() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/tx_drop_stats"
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/rx_drop_stats"
}

dump_tr_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/tr_info"
}

dump_twt_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/twt_stats"
}

dump_ser_status() {
    do_cmd "echo 0 > /sys/kernel/debug/ieee80211/phy0/mt76/band0/sys_recovery"
}

dump_sta_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/sta_info"
}

dump_wm_info() {
    do_cmd "cat /sys/kernel/debug/ieee80211/phy0/mt76/fw_wm_info"
}

per_10_min_work() {
    dump_connection_info

    local i=0
    local max=3
    while [ $i -lt $max ]
    do
        dump_token_info
        dump_ple_info
        dump_pse_info
        dump_mib_info

        true $(( i++ ))
        sleep 1
    done
}

per_30_min_work() {
    dump_drop_stats
    dump_sta_info
    dump_twt_info
}

per_60_min_work() {
    dump_ser_status

    local i=0
    local max=2
    while [ $i -lt $max ]
    do
        dump_tr_info
        dump_wm_info

        true $(( i++ ))
        sleep 1
    done
}

counter=0

dump_board_info

while true; do
    echo "===== $(date '+%Y-%m-%d %H:%M:%S') ====="
    # works for every 10-minute
    per_10_min_work

    # works for every 30-minute
    if [ $(( $counter % 3 )) -eq "0" ]; then
        per_30_min_work
    fi

    # works for every 60-minute
    if [ $(( $counter % 6 )) -eq "0" ]; then
        per_60_min_work
    fi

    echo ""
    sleep 600

    counter=$(( (counter + 1) ))
done
