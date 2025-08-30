#!/bin/sh
#by manper 20250117
lockfile="/var/run/cooling_control.pid"

if [ -e "$lockfile" ]; then
    pid=$(cat "$lockfile")
    if kill -0 $pid > /dev/null 2>&1; then
        echo "Script is already running with PID $pid."
        exit 1
    else
        echo "Lock file found but no process running with PID $pid. Removing lock file."
        rm "$lockfile"
    fi
fi

echo $$ > "$lockfile"

trap "rm -f '$lockfile'; exit" INT TERM EXIT

echo disabled > /sys/class/thermal/thermal_zone0/mode
fanvall=$(cat /etc/fanvall)
    
if [ "$fanvall" = 3 ]; then
    echo 255 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
    echo 255 > /usr/bin/fanspeed.conf
    exit 1
elif [ "$fanvall" = 2 ]; then
    echo 192 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
    echo 192 > /usr/bin/fanspeed.conf
    exit 1
elif [ "$fanvall" = 1 ]; then
    echo 128 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
    echo 128 > /usr/bin/fanspeed.conf
    exit 1
elif [ "$fanvall" = 0 ]; then
    echo 64 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
    echo 64 > /usr/bin/fanspeed.conf
    exit 1
fi

while true; do
    fanvall_conf=$(cat /etc/fanvallv.conf)
    if [ "$fanvall_conf" = "模组温度" ]; then
        temp=$(sendat 1 'AT^CHIPTEMP?' | grep 'CHIPTEMP' | sed -n '1p' | cut -d, -f9 | sed '/^$/d')
        if [ -n "$temp" ]; then
            temp=$((temp * 100))
        else
            temp=$(cat /sys/class/thermal/thermal_zone0/temp)
        fi
    else
        temp=$(cat /sys/class/thermal/thermal_zone0/temp)
    fi

    if [ "$temp" -gt 85000 ]; then
        echo 255 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
	echo 255 > /usr/bin/fanspeed.conf
    elif [ "$temp" -gt 60000 ] && [ "$temp" -le 85000 ]; then
        echo 192 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
	echo 192 > /usr/bin/fanspeed.conf
    elif [ "$temp" -gt 50000 ] && [ "$temp" -le 60000 ]; then
        echo 128 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
	echo 128 > /usr/bin/fanspeed.conf
    elif [ "$temp" -gt 0 ] && [ "$temp" -le 50000 ]; then
        echo 64 > /sys/devices/platform/pwm-fan/hwmon/hwmon2/pwm1
	echo 64 > /usr/bin/fanspeed.conf
    fi
    sleep 8
done


