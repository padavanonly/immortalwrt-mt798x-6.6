#!/bin/sh 
LOCKFILE="/tmp/zinfo57002.lock"
{
    flock -n 200 || {
        exit 1
    }
    trap cleanup INT TERM EXIT
    ATPORT=1
    Temperature(){
        OX=$(sendat 1 'AT^CHIPTEMP?'| grep 'CHIPTEMP' | sed -n '1p' | cut -d, -f9 | sed '/^$/d')
        CTEMP=$(($OX / 10))
    }
    ISP_Read(){
            operatorCode=$(sendat 1 'AT^EONS=2' | awk -F ',' '{print $2}' | sed '/^$/d')
            case "$operatorCode" in
            "46000" | "46002" | "46004" | "46007" | "46008" | "46020")
                ISP="中国移动"
                ;;
            "46001" | "46006" | "46009")
                ISP="中国联通"
                ;;
            "46003" | "46005" | "46011")
                ISP="中国电信"
                ;;
            "46015")
                ISP="中国广电"
                ;;
            *)
                ISP="未知运营商"
                ;;
            esac
    }

    TD_Tech_Ltd_SIMINFO()
    {
        # 获取IMEI
        IMEI=$( sendat $ATPORT "AT+CGSN"  | sed -n '2p'  )
        # 获取载波聚合数据
        zbjhox=$(5700_get_zbjh.sh)
        if [ "$zbjhox" == "" ] ; then
            Zbjh="Loss 无载波聚合"
        else
            Zbjh=$zbjhox
        fi
        # 获取IMSI
        IMSI=$( sendat $ATPORT "AT+CIMI"  | sed -n '2p'  )
        # 获取ICCID
        ICCID=$( sendat $ATPORT "AT^ICCID?" | sed -n '2p' )
        ICCID=$(echo $ICCID | cut -d ':' -f2)
        # 获取电话号码
        phone=$(sendat $ATPORT "AT+CNUM"  | awk -F ',' '{print $2}'| sed '/^$/d' )
        phone=${phone:1:-1}
        QCI1=$(sendat 1 'AT+CGEQOSRDP=8' | sed -n '2p')

        QCI2=$(echo $QCI1 | cut -d ',' -f2)
        if [[ "$QCI2" =~ "1" ]]; then
            QCI="QCI值：1-GBR优级2\\-Budget100ms"
        elif [[ "$QCI2" =~ "2" ]]; then
            QCI="QCI值：2-GBR优级4\\-Budget150ms"
        elif [[ "$QCI2" =~ "3" ]]; then
            QCI="QCI值：3-GBR优级3\\-Budget50ms"
        elif [[ "$QCI2" =~ "4" ]]; then
            QCI="QCI值：4-GBR优级5\\-Budget300ms"
        elif [[ "$QCI2" =~ "5" ]]; then
            QCI="QCI值：5-GBR优级1\\-Budget100ms"
        elif [[ "$QCI2" =~ "6" ]]; then
            QCI="QCI值：6-GBR优级6\\-Budget300ms"
        elif [[ "$QCI2" =~ "7" ]]; then
            QCI="QCI值：7-GBR优级7\\-Budget100ms"
        elif [[ "$QCI2" =~ "8" ]]; then
            QCI="QCI值：8-GBR优级8\\-Budget300ms"
        elif [[ "$QCI2" =~ "9" ]]; then
            QCI="QCI值：9-GBR优级9\\-延时预300ms"
        else
            QCI="锁频\锁小区\关闭短信等模式下均无法查看"
        fi
    }
    Read_signal_data(){
        OX=$(sendat 1 'AT^MONSC' | sed -n '2p' | grep 'MONSC') 
        if echo "$OX" | grep -q "NR"; then
            DTAT_5G
        elif echo "$OX" | grep -q "LTE"; then
            DTAT_4G
        fi
    }
    DTAT_5G(){
        OX=$(sendat 1 'AT^MONSC' | sed -n '2p' | grep 'MONSC') 
        MODE="NR-5G"
        RSCP=$(echo $OX | awk -F ',' '{print $9}')
        CSQ_RSSI=$RSCP
        if [ $RSCP -ge -85 ] ; then
        CSQ_PER="100%"
        elif [ $RSCP -le -85 -a $RSCP -ge -90 ]  ; then
        CSQ_PER="90%"
        elif [ $RSCP -le -90 -a $RSCP -ge -95 ]  ; then
        CSQ_PER="80%"
        elif [ $RSCP -le -95 -a $RSCP -ge -105 ]  ; then
        CSQ_PER="50%"
        elif [ $RSCP -le 105 -a $RSCP -ge -115 ]  ; then
        CSQ_PER="10%"        
        fi
        ECIO=$(echo $OX | awk -F ',' '{print $10}')
        CSQ_RSSI=$ECIO
        SINR=$(echo $OX | awk -F ',' '{print $11}')
        COPS_MCC=$( echo $OX | awk -F ',' '{print $2}')
        COPS_MNC=$( echo $OX | awk -F ',' '{print $3}')
        LAC=$( echo $OX | awk -F ',' '{print $8}' | sed '/^$/d')
        CID=$( echo $OX | awk -F ',' '{print $6}' | sed '/^$/d')
        CHANNEL=$( echo $OX | awk -F ',' '{print $4}')
        HFREQINFO=$(sendat 1 "AT^HFREQINFO?" | grep "HFREQINFO")
        band=$(echo $HFREQINFO | cut -d, -f3)   
        dl_bwN=$(echo $HFREQINFO | cut -d, -f6)
        ul_bwN=$(echo $HFREQINFO | cut -d, -f9)
        dl_bwN=$(($dl_bwN / 1000))
        ul_bwN=$(($ul_bwN / 1000))
        LBAND="N"$band" (Bandwidth $dl_bwN MHz Down | $ul_bwN MHz Up)"
        PCI=$( echo $OX | awk -F ',' '{print $7}' | sed '/^$/d')
        PCI=$((0x$PCI))
        zbjhox=$(sendat 1 'AT^CASCELLINFO?' | grep 'ERROR')
        if [ "$zbjhox" == "" ] ; then
            R2cc=$(sendat 1 'AT^CASCELLINFO?' | grep 'CASCELLINFO: 1')
            R3cc=$(sendat 1 'AT^CASCELLINFO?' | grep 'CASCELLINFO: 2')
        else
            R2cc="Loss-in"
            R3cc="Loss-in"
        fi
    }

    DTAT_4G(){
        OX=$(sendat 1 'AT^MONSC' | sed -n '2p' | grep 'MONSC') 
        MODE="LTE-4G"
        CSQ_RSSI=$(echo $OX | awk -F ',' '{print $10}')
        RSCP=$(echo $OX | awk -F ',' '{print $8}')
        if [ $RSCP -ge -85 ] ; then
        CSQ_PER="100%"
        elif [ $RSCP -le -85 -a $RSCP -ge -90 ]  ; then
        CSQ_PER="90%"
        elif [ $RSCP -le -90 -a $RSCP -ge -95 ]  ; then
        CSQ_PER="80%"
        elif [ $RSCP -le -95 -a $RSCP -ge -105 ]  ; then
        CSQ_PER="50%"
        elif [ $RSCP -le 105 -a $RSCP -ge -115 ]  ; then
        CSQ_PER="10%"        
        fi
        ECIO=$(echo $OX | awk -F ',' '{print $9}')
        SINR="\\"
        COPS_MCC=$( echo $OX | awk -F ',' '{print $2}')
        COPS_MNC=$( echo $OX | awk -F ',' '{print $3}')
        LAC=$( echo $OX | awk -F ',' '{print $7}' | sed '/^$/d')
        CID=$( echo $OX | awk -F ',' '{print $5}' | sed '/^$/d')
        CHANNEL=$( echo $OX | awk -F ',' '{print $4}')
        HFREQINFO=$(sendat 1 "AT^HFREQINFO?" | grep "HFREQINFO")
        band=$(echo $HFREQINFO | cut -d, -f3)   
        dl_bwN=$(echo $HFREQINFO | cut -d, -f6)
        ul_bwN=$(echo $HFREQINFO | cut -d, -f9)
        dl_bwN=$(($dl_bwN / 1000))
        ul_bwN=$(($ul_bwN / 1000))
        LBAND="B"$band"-(Bandwidth $dl_bwN MHz Down | $ul_bwN MHz Up)"
        PCI=$( echo $OX | awk -F ',' '{print $6}' | sed '/^$/d')
        PCI=$((0x$PCI))
        zbjhox=$(sendat 1 'AT^CASCELLINFO?' | grep 'ERROR')
        if [ "$zbjhox" == "" ] ; then
            R2cc=$(sendat 1 'AT^CASCELLINFO?' | grep 'CASCELLINFO: 1')
            R3cc=$(sendat 1 'AT^CASCELLINFO?' | grep 'CASCELLINFO: 2')
        else
            R2cc="Loss-in"
            R3cc="Loss-in"
        fi
    }
    AMBR(){
        rm -rf "$LOCK_FILE"
        AMB=$(sendat 1 'AT^DSAMBR=8' | grep 'DSAMBR')
        apn=$(echo $AMB | cut -d, -f4) 
        apn=${apn:1:-1}
        DOWNspeed=$(echo $AMB | cut -d, -f2)
        DOWNspeed=`expr $DOWNspeed / 1000`
        UPspeed=$(echo $AMB | cut -d, -f3)
        UPspeed=`expr $UPspeed / 1000`
    }
    Read_module_data_AT(){
        ModuleName=$(sendat 1 "ATI" | sed -n '2p'|sed 's/\r$//') #'TD Tech Ltd'
        ModuleType=$(sendat 1 "ATI" | sed -n '3p'|sed 's/\r$//') #'MT5700M-CN'
        Moduleversion=$(sendat 1 "ATI" | sed -n '4p' | cut -d ':' -f2 | tr -d ' '|sed 's/\r$//') #'V200R001C20B008'
        Temperature
        ISP_Read
        TD_Tech_Ltd_SIMINFO
        Read_signal_data
        AMBR
    }

    neighbor(){
    list=$(sendat 1 'AT^MONNC')
    for file in $list
    do  
        if echo "$file" | grep -q "NR" || echo "$file" | grep -q "LTE"; then
            pci_10=$((0x$(echo $file | awk -F ',' '{print $3}' | tr -d '\r\n')))
            echo $file | awk -F ',' -v pci="$pci_10" '{printf("Mode:%s earfcn:%s pci:%s RSRP:%s\n", $1, $2, pci, $4, $5, $6)}'
        fi  
    done
    }

    InitData(){
        Date=''
        CHANNEL="-" 
        ECIO="-"
        RSCP="-"
        ECIO1=" "
        RSCP1=" "
        NETMODE="-"
        LBAND="-"
        PCI="-"
        CTEMP="-"
        MODE="-"
        SINR="-"
        IMEI='-'
        IMSI='-'
        ICCID='-'
        phone='-'
        conntype=''
        Model=''
    }

    sim_sel=$(cat /tmp/sim_sel)
    SIMCard=""
    case $sim_sel in
        0)
            SIMCard="外置SIM卡1"
            ;;
        1)
            SIMCard="内置SIM1"
            ;;
        2)
            SIMCard="内置SIM2"
            ;;
        3)
            SIMCard="外置SIM2"
            ;;
        4)
            SIMCard="外置SIM3"
            ;;
        5)
            SIMCard="外置SIM4"
            ;;
        *)
            SIMCard="SIM未读取"
            ;;
    esac
    OutData(){
        {
        echo "USB串口1模式" 
        echo "$ModuleName" #'Quectel'
        echo "$ModuleType" #'RM520N-CN'
        echo "$Moduleversion" #'RM520NCNAAR03A03M4G
        echo "$CTEMP" # 设备温度 41°C
        echo `date "+%Y-%m-%d %H:%M:%S"` # 时间
        #----------------------------------
        echo "$SIMCard" # 卡槽
        echo "$ISP" #运营商
        echo "$IMEI" #imei
        echo "$IMSI" #imsi
        echo "$ICCID" #iccid
        echo "$phone" #phone
        #-----------------------------------
        echo "$MODE" #蜂窝网络类型 NR5G-SA "TDD"
        echo "$CSQ_PER" #CSQ_PER 信号质量
        echo "$CSQ_RSSI" #信号强度 RSSI 信号强度
        echo "$ECIO dB" #接收质量 RSRQ 
        echo "$RSCP dBm" #接收功率 RSRP
        echo "$SINR" #信噪比 SINR  rv["sinr"]
        #-----------------------------------
        echo "$COPS_MCC /$COPS_MNC" #MCC / MNC
        echo "$LAC"  #位置区编码
        echo "$CID"  #小区基站编码
        echo "$LBAND" # 频段 频宽
        echo "$CHANNEL" # 频点
        echo "$PCI" #物理小区标识   
        echo "$apn" 
        echo "$DOWNspeed""mbps"
        echo "$UPspeed""mbps"
        echo "$QCI"
        echo "$Zbjh" #载波聚合
        echo "$R2cc"
        echo "$R3cc"
        neighbor
        } > /tmp/cpe_cell.file
    }
    InitData
    Read_module_data_AT
    OutData
    cleanup() {
    rm -rf "$LOCKFILE"
   }
} 200>"$LOCKFILE"


