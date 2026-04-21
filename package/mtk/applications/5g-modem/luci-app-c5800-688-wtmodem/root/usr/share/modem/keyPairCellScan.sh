#!/bin/ash

PROGRAM="RM520N_CELLSCAN"

lockfile=/tmp/cellscanlock

earfcn_to_band() {
    local earfcn=$1
    local bands=""
    # 4G LTE
    if [ $earfcn -ge 0 ] && [ $earfcn -le 41589 ]; then
        if [ $earfcn -ge 0 ] && [ $earfcn -le 599 ]; then
            bands="Band 1"
        elif [ $earfcn -ge 1200 ] && [ $earfcn -le 1949 ]; then
            bands="Band 3"
        elif [ $earfcn -ge 2400 ] && [ $earfcn -le 2649 ]; then
            bands="Band 5"
        elif [ $earfcn -ge 3450 ] && [ $earfcn -le 3799 ]; then
            bands="Band 8"
        elif [ $earfcn -ge 36200 ] && [ $earfcn -le 36349 ]; then
            bands="Band 34"
        elif [ $earfcn -ge 37750 ] && [ $earfcn -le 38249 ]; then
            bands="Band 38"
        elif [ $earfcn -ge 38250 ] && [ $earfcn -le 38649 ]; then
            bands="Band 39"
        elif [ $earfcn -ge 38650 ] && [ $earfcn -le 39649 ]; then
            bands="Band 40"
        elif [ $earfcn -ge 39650 ] && [ $earfcn -le 41589 ]; then
            bands="Band 41"
        fi
    # 以下是5G NR的
    elif [ $earfcn -ge 422000 ] && [ $earfcn -le 434000 ]; then
        bands="n1"
    elif [ $earfcn -ge 361000 ] && [ $earfcn -le 376000 ]; then
        bands="n3"
    elif [ $earfcn -ge 173800 ] && [ $earfcn -le 178800 ]; then
        bands="n5"
    elif [ $earfcn -ge 185000 ] && [ $earfcn -le 192000 ]; then
        bands="n8"
    elif [ $earfcn -ge 499200 ] && [ $earfcn -le 537999 ]; then
        bands="n41"
    elif [ $earfcn -ge 620000 ] && [ $earfcn -le 680000 ]; then
        bands="n78/n77"
    elif [ $earfcn -ge 693334 ] && [ $earfcn -le 733333 ]; then
        bands="n79"
    # 5G NR重复频段检查
    elif [ $earfcn -ge 158200 ] && [ $earfcn -le 164200 ]; then
        bands="n20"
    fi
    if [ $earfcn -ge 151600 ] && [ $earfcn -le 160600 ]; then
        [ -n "$bands" ] && bands="${bands}/"
        bands="${bands}n28"
    fi
    
    if [ -z "$bands" ]; then
        bands="Unknown Band"
    fi
    
    echo "$bands"
}



# 检查是否存在 /tmp/celltime 文件，以及文件中的时间戳
if [ -e /tmp/celltime ]; then
    celltime=$(cat /tmp/celltime)
    current_time=$(date +%s)
    time_difference=$((current_time - celltime))
    
    # 如果时间差小于20秒，则直接退出脚本
    if [ $time_difference -lt 40 ]; then
        echo "时间间隔小于20秒，使用缓存结果"
        exit 0
    fi
fi

if [ -e ${lockfile} ]; then
    if kill -9 $(cat ${lockfile}); then
        echo "Cell scanning is already Kill it."
        rm -f ${lockfile}
    else
        echo "Removing stale lock file."
        rm -f ${lockfile}
    fi
fi

echo $$ >${lockfile}
pid=$(cat ${lockfile})
>/tmp/kpcellinfo
>/tmp/tmpcellinfo
echo "开始基站扫描..."
# 获取当前时间的时间戳
timestamp=$(date +%s)
echo $timestamp > /tmp/celltime
echo $timestamp > /tmp/cellscan_run_time
if [ "$1" == "5" ]  
then
   echo -e 'at+qscan=2,0\r\n' >/dev/ttyUSB2
elif [ "$1" == "4" ] 
then
   echo -e 'at+qscan=1,0\r\n' >/dev/ttyUSB2
else
   echo -e 'at+qscan=3,0\r\n' >/dev/ttyUSB2
fi
timeout 185s cat /dev/ttyUSB2 | while read line; do
    case "$line" in "+QSCAN"*)
        echo "$line" >> /tmp/tmpcellinfo
        ;;
    esac
    case "$line" in *"OK"*)
        echo "<br>基站扫描完成"
        # 格式化输出基站信息供用户选择
        # awk '{print NR, $0}' /tmp/kpcellinfo
        # rm -f ${lockfile}
        # kill -9 $pid
        exit 0
        ;;
    esac
done
while read line; do
    case "$line" in "+QSCAN"*)
        operatorCode=$(echo $line | awk -F ',' '{print $2$3}')
        case "$operatorCode" in
        "46000" | "46002" | "46004" | "46007" | "46008" | "46020")
            operator="中国移动"
            ;;
        "46001" | "46006" | "46009")
            operator="中国联通"
            ;;
        "46003" | "46005" | "46011")
            operator="中国电信"
            ;;
        "46015")
            operator="中国广电"
            ;;
        *)
            operator="未知运营商"
            ;;
        esac
        earfcn=$(echo "$line" | awk -F ',' '{print $4}')
        frequency_band=$(earfcn_to_band $earfcn)
        echo "$line" | awk -F ',' -v operator="$operator" -v frequency_band="$frequency_band" -v earfcn="$earfcn" '{printf("%s,%s,%s,%s,%s,%s,%s\n", $1, operator, frequency_band, earfcn, $5, $6, $7)}' >> /tmp/kpcellinfo
        ;;
    esac
done < /tmp/tmpcellinfo
echo "<br>数据处理完成"
rm -f ${lockfile}
# 获取当前时间的时间戳
timestamp=$(date +%s)
echo $timestamp > /tmp/celltime

rm -f ${lockfile}
