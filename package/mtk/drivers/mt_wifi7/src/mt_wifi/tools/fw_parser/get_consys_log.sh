#!/bin/bash

function start_get_log() {
    mwctl ra0 set fwlogdir=/tmp/log
    if [[ "$2" == "" ]]; then
        mwctl ra0 set fwlog=0:96
    elif [[ "$2" == "storage" ]]; then
        mwctl ra0 set fwlog=0:40
    fi
    mwctl ra0 set time_sync=1
}

function stop_get_log() {
    mwctl ra0 set fwlog=0:0
    timestamp=$(date +"%y%m%d_%H%M%S")
    mkdir "/tmp/log/Log_${timestamp}"
    mv "/tmp/log/fw_log.bin" "/tmp/log/Log_${timestamp}/WIFI_FW_$timestamp.clog"
    dmesg > /tmp/log/Log_${timestamp}/WIFI_KERNEL_$timestamp.clog
	if [ -f /lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin ]; then
		echo $(tail -c 456 /lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin |
           head -c 52 )"-DUT.xml">> "/tmp/log/Log_${timestamp}/7992_fw_version.txt"
	fi
	if [ -f /lib/firmware/WIFI_RAM_CODE_MT7993_1_1.bin ]; then
		echo $(tail -c 456 /lib/firmware/WIFI_RAM_CODE_MT7993_1_1.bin |
           head -c 52 )"-DUT.xml">> "/tmp/log/Log_${timestamp}/7993_fw_version.txt"
	fi
}

if [[ "$1" == "start" ]]; then
    echo "Start Catching FW Log"
    start_get_log "$1" "$2"
elif [[ "$1" == "stop" ]]; then
    echo "Stop Catching FW Log"
    stop_get_log
fi
