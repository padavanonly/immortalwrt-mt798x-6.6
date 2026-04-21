#!/bin/sh
. /usr/share/libubox/jshn.sh

_command_huawei_mode_convert() {
	local _mode _res
	_mode="$1"
	_res="Unknown"
	case "$_mode" in
		"1")
			_res="GSM"
			;;
		"2")
			_res="CDMA"
			;;
		"3")
			_res="WCDMA"
			;;
		"4")
			_res="TD-SCDMA"
			;;
		"6")
			_res="LTE"
			;;
		"7")
			_res="NR"
			;;
	esac
	echo "$_res"
}

_command_huawei_hfreqinfo() {
	local _hfreq _sysmode
	local nr_index=0
	local nr_buffer=""
	local basic_index=3
	local step=7
	_hfreq="$1"

	_cnt=$(echo "$_hfreq"|wc -l)
	json_init
	for _line in $_hfreq; do
		local loop=0
		
		nr_buffer=""
		_sysmode="$(echo "$_line"|cut -d, -f2)"
		_mode=$(_command_huawei_mode_convert "$_sysmode")
		
		if [ $nr_index -ge 1 -a "$_mode" == "NR" ];then
			nr_buffer="$nr_index"			
		fi

		while true;do
			local next_index=$((basic_index+step*loop))
			local band_index=$((next_index))
			local band="$(echo "$_line"|cut -d, -f$band_index)"

			[ -z "$band" ] && break
			local dlearfcn_index=$((next_index+1))
			local dlfreq_index=$((next_index+2))
			local dlbandwidth_index=$((next_index+3))
			local ulearfcn_index=$((next_index+4))
			local ulfreq_index=$((next_index+5))
			local ulbandwidth_index=$((next_index+6))

			if [ $loop -ge 1 ];then
				nr_index=$((nr_index+1))
				nr_buffer="$nr_index"
			fi

			json_add_object "${_mode}${nr_buffer}"
			json_add_string "BAND" "$band"
			json_add_int "EARFCN" "$(echo "$_line"|cut -d, -f$dlearfcn_index)"
			json_add_int "DL_FREQ" "$(echo "$_line"|cut -d, -f$dlfreq_index)"
			json_add_int "DL_BANDWIDTH" "$(echo "$_line"|cut -d, -f$dlbandwidth_index)"
			json_add_int "UL_FCN" "$(echo "$_line"|cut -d, -f$ulearfcn_index)"
			json_add_int "UL_FREQ" "$(echo "$_line"|cut -d, -f$ulfreq_index)"
			json_add_int "UL_BANDWIDTH" "$(echo "$_line"|cut -d, -f$ulbandwidth_index)"
			json_close_object
			loop=$((loop+1))
		done

		if [ "$_mode" == "NR" ];then
			nr_index=$((nr_index+1))
		fi
		
	done
    json_add_int nr_count $nr_index
	json_dump
	json_cleanup
}


get_freq_info() {
    local out_info=""
	_hfreq="$(sendat 1 "AT^hfreqinfo?" |grep 'HFREQINFO:'|awk -F: '{print $2}')"
    _hfreq="$(_command_huawei_hfreqinfo "$_hfreq")"
	if echo "$_hfreq"|jsonfilter -e '$["NR"]' > /dev/null; then
		nr_count="$(echo "$_hfreq"|jsonfilter -e '$["nr_count"]')"
		i=0
		while [ $i -lt $nr_count ];do			
			index=""
			if [ $i -gt 0 ];then
				index="$i"
			fi
			if echo "$_hfreq"|jsonfilter -e '$["NR'$index'"]' > /dev/null; then
                band="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['BAND']")"
                dlbw="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['DL_BANDWIDTH']")"
				EARFCN1="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['EARFCN']")"
				DL_FREQ1="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['DL_FREQ']")"
				UL_FCN1="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['UL_FCN']")"
				UL_FREQ1="$(echo "$_hfreq"|jsonfilter -e "\$['NR$index']['UL_FREQ']")"
                [ -n "$dlbw" ] && dlbw=$((dlbw/1000))
                if [ -z "$out_info" ];then
                    out_info="N$band ($dlbw MHz)-$EARFCN1\\$UL_FREQ1"
					echo "主波：$out_info" > /tmp/zbjhz.file
				else
                    out_info="$out_info + N$band ($dlbw MHz)-$EARFCN1\\$UL_FREQ1"
					echo "主从波：$out_info" > /tmp/zbjhc.file
                fi
			fi
			i=$((i+1))
		done
	fi
    echo "$out_info"
}

get_freq_info

