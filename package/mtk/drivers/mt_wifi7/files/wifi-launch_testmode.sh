#!/bin/sh
# SPDX-License-Identifier: GPL-2.0

dev=/dev/console

if [ -f /tmp/.testmode ]; then
        echo -e "\033[1;36m\n\nWiFi Normal Mode Init Done, Enter Into Test Mode\n\n\033[1;31m" > ${dev}
        cat /etc/banner.testmode > ${dev}
        echo -e "        $(cat /sys/kernel/debug/mtk_hwifi/chips/chip0 | grep TEST | cut -d ":" -f 2) \n\n\033[0m" > ${dev}
        sleep 1
        sh /tmp/.testmode
fi
