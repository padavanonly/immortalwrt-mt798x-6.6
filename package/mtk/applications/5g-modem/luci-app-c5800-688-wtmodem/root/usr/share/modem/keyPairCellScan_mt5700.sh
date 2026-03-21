#!/bin/ash
#By leihuojian
PROGRAM="MT5700M_CELLSCAN"

lockfile=/tmp/cellscanlock

frequency_to_NR_ARFCN() {
    local band=$1
    local freq=$2
    local earfcn=""
    if ! [ "$freq" == "" ]; then
        case "$band" in
        "1" | "5" | "28" | "41")
            earfcn=$(($freq / 5))
            ;;
        "78" | "79")
            earfcn=$(($freq - 3000000))
            earfcn=$(($earfcn / 15))
            earfcn=$(($earfcn + 600000))
            ;;
        *)
            ;;
        esac
    fi
    if [ -z "$earfcn" ]; then
        earfcn="Unknown earfcn"
    fi
    echo "$earfcn"
}

frequency_to_LTE_ARFCN() {
    local band=$1
    local freq=$2
    local earfcn=""
    if ! [ "$freq" == "" ]; then
        case "$band" in
            "1")
                earfcn=$(($freq - 21100))
                ;;
            "3")
                earfcn=$(($freq + 1200))
                earfcn=$(($earfcn - 18050))
                ;;
            "5")
                earfcn=$(($freq + 2400))
                earfcn=$(($earfcn - 8690))                
                ;;
            "8")
                earfcn=$(($freq + 3450))
                earfcn=$(($earfcn - 9250))             
                ;;
            "34")
                earfcn=$(($freq + 36200))
                earfcn=$(($earfcn - 20100))                  
                ;;        
            "38")
                earfcn=$(($freq + 37750))
                earfcn=$(($earfcn - 25700))             
                ;;  
            "39")
                earfcn=$(($freq + 38250))
                earfcn=$(($earfcn - 18800))  
                ;;  
            "40")
                earfcn=$(($freq + 38650))
                earfcn=$(($earfcn - 23000))             
                ;;  
            "41")
                earfcn=$(($freq + 39650))
                earfcn=$(($earfcn - 24960))                 
                ;;  
            *) 
                ;;
        esac
    fi
    if [ -z "$earfcn" ]; then
        earfcn="Unknown earfcn"
    fi
    echo "$earfcn"
}


# 检查是否存在 /tmp/celltime 文件，以及文件中的时间戳
if [ -e /tmp/celltime ]; then
    celltime=$(cat /tmp/celltime)
    current_time=$(date +%s)
    time_difference=$((current_time - celltime))
    
    # 如果时间差小于20秒，则直接退出脚本
    if [ $time_difference -lt 20 ]; then
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
#调成默认参数让扫描可以获取更多数据
sendat 1 'AT^C5GOPTION=1,1,1'
sendat 1 'AT^LTEFREQLOCK=0'
sendat 1 'AT^NRFREQLOCK=0'
sendat 1 'AT^SYSCFGEX="0803",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,'
sleep 1
sendat 1 'AT+COPS=2'  #关闭网络准备扫描基站
sleep 1
if [ "$1" == "4" ]  
then
   echo -e 'AT^CELLSCAN=2\r\n' > /dev/ttyUSB1
else
   echo -e 'AT^CELLSCAN=3\r\n' > /dev/ttyUSB1
fi
timeout 60s cat /dev/ttyUSB1 | while read line; do
    case "$line" in "^CELLSCAN"*)
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
    case "$line" in "^CELLSCAN"*)
        operatorCode=$(echo $line | awk -F\" '{print $2}')
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
        rat=$(echo "$line" | awk -F ',' '{print $1}')
        freq=$(echo "$line" | awk -F ',' '{print $3}')
        band=$(echo "$line" | awk -F ',' '{print $5}')
        if ! [ "$band" == "" ]; then
            band=$((0x$band))
        fi
        if [ "$rat" == "^CELLSCAN: 2" ]; then
            rat="LTE"
            frequency_earfcn=$(frequency_to_LTE_ARFCN $band $freq)
            echo $line | awk -F ',' -v rat="$rat" -v operator="$operator" -v band="B$band" -v earfcn="$frequency_earfcn" '{printf("+QSCAN: \"%s\",%s,%s,%s,%s,%s,%s\n", rat, operator, band, earfcn, $4, $8, $15)}' >> /tmp/kpcellinfo
        elif [ "$rat" == "^CELLSCAN: 3" ]; then
            rat="NR5G"
            frequency_earfcn=$(frequency_to_NR_ARFCN $band $freq)
            echo $line | awk -F ',' -v rat="$rat" -v operator="$operator" -v band="N$band" -v earfcn="$frequency_earfcn" '{printf("+QSCAN: \"%s\",%s,%s,%s,%s,%s,%s\n", rat, operator, band, earfcn, $4, $12, $13)}' >> /tmp/kpcellinfo
        
        fi
        ;;
    esac
done < /tmp/tmpcellinfo
echo "<br>数据处理完成"
rm -f ${lockfile}
# 获取当前时间的时间戳
timestamp=$(date +%s)
echo $timestamp > /tmp/celltime
sendat 1 'AT+COPS=0'  #扫描完成，打开网络！
rm "/tmp/RF_Mode"
rm "/tmp/Band_LTE"
rm "/tmp/Band_SA"
sleep 3
/usr/share/modem/mt5700m.sh &
rm -f ${lockfile}
