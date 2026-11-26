#!/bin/sh
#
# Copyright (C) 2023, hanwckf <hanwckf@vip.qq.com>
#

append DRIVERS "mtwifi"

is_wifi7_dev() {
	local chip
	chip="$(l1util get "${1}" INDEX 2>/dev/null)"
	case "$chip" in
		MT799*|mt799*) return 0 ;;
		MT798*|mt798*|MT791*|mt791*) return 1 ;;
		*) [ -d "/sys/module/mtk_hwifi" ] ;;
	esac
}

detect_mtwifi() {
	local idx ifname
	local band channel htmode htbsscoex ssid dbdc_main
	if [ -d "/sys/module/mt_wifi" ]; then
		dev_list="$(l1util list)"
		for dev in $dev_list; do
			channel=""
			config_get type ${dev} type
			[ "$type" = "mtwifi" ] || {
				ifname="$(l1util get ${dev} main_ifname)"

				idx="$(l1util get ${dev} subidx)"
				[ $idx -eq 1 ] && dbdc_main="1" || dbdc_main="0"

				band="$(l1util get ${dev} band)"
				if [ -z "$band" ] || [ "$band" = "nil" ]; then
					[ $idx -eq 1 ] && band="2g" || band="5g"
				fi

				if is_wifi7_dev "${dev}"; then
					if [ "$band" = "2g" ]; then
						htmode="EHT40"
						htbsscoex="1"
						ssid="ImmortalWrt-2.4G"
						channel="auto"
					elif [ "$band" = "5g" ]; then
						htmode="EHT160"
						htbsscoex="0"
						ssid="ImmortalWrt-5G"
						channel="36"
					elif [ "$band" = "6g" ]; then
						htmode="EHT160"
						htbsscoex="0"
						ssid="ImmortalWrt-6G"
						channel="auto"
					fi
				else
					if [ "$band" = "2g" ]; then
						htmode="HE40"
						htbsscoex="1"
						ssid="ImmortalWrt-2.4G"
						channel="auto"
					elif [ "$band" = "5g" ]; then
						htmode="HE160"
						htbsscoex="0"
						ssid="ImmortalWrt-5G"
						channel="36"
					elif [ "$band" = "6g" ]; then
						htmode="HE160"
						htbsscoex="0"
						ssid="ImmortalWrt-6G"
						channel="auto"
					fi
				fi

				uci -q batch <<-EOF
					set wireless.${dev}=wifi-device
					set wireless.${dev}.type=mtwifi
					set wireless.${dev}.phy=${ifname}
					set wireless.${dev}.band=${band}
					set wireless.${dev}.dbdc_main=${dbdc_main}
					set wireless.${dev}.channel=${channel}
					set wireless.${dev}.txpower=100
					set wireless.${dev}.htmode=${htmode}
					set wireless.${dev}.country=US
					set wireless.${dev}.mu_beamformer=1
					set wireless.${dev}.noscan=${htbsscoex}
					set wireless.${dev}.serialize=1
					
					set wireless.default_${dev}=wifi-iface
					set wireless.default_${dev}.device=${dev}
					set wireless.default_${dev}.network=lan
					set wireless.default_${dev}.mode=ap
					set wireless.default_${dev}.ssid=${ssid}
					set wireless.default_${dev}.encryption=$([ "${band}" = "6g" ] && echo "owe" || echo "none")
EOF
				uci -q commit wireless
			}
		done
	fi
}
