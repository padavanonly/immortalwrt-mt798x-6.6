    #!/bin/sh
    #By Manper MT5700
    echo "SIM INIT..." >/tmp/simcardstat
    PROGRAM="MT5700M_MODEM"
    printMsg() {
        local msg="$1"
        logger -t "${PROGRAM}" "${msg}"
    } #日志输出调用API

    # 检查是否存在锁文件 @Icey
    lock_file="/tmp/rm520n.lock"

    if [ -e "$lock_file" ]; then
    # 锁文件存在，获取锁定的进程 ID，并终止它
    locked_pid=$(cat "$lock_file")
    if [ -n "$locked_pid" ]; then
        echo "Terminating existing rm520n.sh process (PID: $locked_pid)" 
        kill "$locked_pid"
        sleep 2  # 等待一段时间确保进程终止
    fi
    fi

    # 创建新的锁文件，记录当前进程 ID
    echo "$$" > "$lock_file"
    sleep 2 && /sbin/uci commit
    Modem_Enable=`uci -q get modem.@ndis[0].enable` || Modem_Enable=1
    #模块启动
    #模块开关
    if [ "$Modem_Enable" == 0 ]; then
        echo 0 >/sys/class/gpio/cpe-pwr/value
        printMsg "禁用模块，退出"
        rm $lock_file
        exit 0
    else
        printMsg "模块启用"
        echo 1 >/sys/class/gpio/cpe-pwr/value
        #预先开内置卡
        #echo 1 > /sys/class/gpio/cpe-sel0/value
    fi
    #先关闭数据！
    sleep 11

    Sim_Sel=`uci -q get modem.@ndis[0].simsel`|| Sim_Sel=0
    echo "simsel: $Sim_Sel" >> /tmp/moduleInit
    #SIM选择

    Enable_IMEI=`uci -q get modem.@ndis[0].enable_imei` || Enable_IMEI=0
    #IMEI修改开关

    RF_Mode=`uci -q get modem.@ndis[0].smode` || RF_Mode=0
    #网络制式 0: Auto, 1: 4G, 2: 5G
    NR_Mode=`uci -q get modem.@ndis[0].nrmode` || NR_Mode=0
    #0: Auto, 1: SA, 2: NSA
    Band_LTE=`uci -q get modem.@ndis[0].bandlist_lte` || Band_LTE=0
    Band_SA=`uci -q get modem.@ndis[0].bandlist_sa` || Band_SA=0
    Band_NSA=`uci -q get modem.@ndis[0].bandlist_nsa` || Band_NSA=0
    #Enable_PING=`uci -q get modem.@ndis[0].pingen` || Enable_PING=0
    #PING_Addr=`uci -q get modem.@ndis[0].pingaddr` || PING_Addr="119.29.29.29"
    #PING_Count=`uci -q get modem.@ndis[0].count` || PING_Count=10
    #FCN CI LOCK
    Earfcn=`uci -q get modem.@ndis[0].earfcn` || Earfcn=0
    Cellid=` uci -q get modem.@ndis[0].cellid` || Cellid=0
    Freqlock=` uci -q get modem.@ndis[0].freqlock` || Freqlock=0
    #
    switchNetwork=` uci -q get modem.@ndis[0].switchNetwork` || switchNetwork=0
    RF_Mode2=`uci -q get modem.@ndis[0].smode2` || RF_Mode2=0
    Autoswitchtime=` uci -q get modem.@ndis[0].Autoswitchtime` || Autoswitchtime="00-07"

     #-----------------SIM Card switch
     #attention！ims enable and autosel enable will make some card work under 4G network
    Sim_Sel=`uci -q get modem.@ndis[0].simsel`|| Sim_Sel=0
    echo "0" >> /tmp/sim_sel
    echo "simsel: $Sim_Sel" >> /tmp/moduleInit
    echo "simsel: $Sim_Sel"
    echo 0 > /sys/class/gpio/cpe-sel3/value
    echo 0 > /sys/class/gpio/cpe-sel2/value
    echo 0 > /sys/class/gpio/cpe-sel1/value
    echo 0 > /sys/class/gpio/cpe-sel0/value
    sleep 2
    case "$Sim_Sel" in
        0)
            printMsg "外置SIM卡1"
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 0 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 0 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=0,1'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "外置SIM卡1" >> /tmp/moduleInit
            echo "外置SIM卡1切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 0 > /tmp/sim_sel
        ;;
        1)
            printMsg "内置SIM卡1"
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 1 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 1 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=1,0'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "内置SIM卡1" >> /tmp/moduleInit
            echo "内置SIM卡1切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 1 > /tmp/sim_sel
        ;;
        2)
            printMsg "内置SIM卡2"
            echo 0 > /sys/class/gpio/cpe-sel3/value
            echo 1 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 1 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=1,0'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "内置SIM卡2" >> /tmp/moduleInit
            echo "内置SIM卡2切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 2 > /tmp/sim_sel
        ;;
        3)
            printMsg "外置SIM卡2"
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 1 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 0 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=0,1'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "外置SIM卡2" >> /tmp/moduleInit
            echo "外置SIM卡2切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 3 > /tmp/sim_sel
        ;;
        4)
            printMsg "外置SIM卡3"
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 1 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 1 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=0,1'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "外置SIM卡3" >> /tmp/moduleInit
            echo "外置SIM卡3切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 4 > /tmp/sim_sel
        ;;
        5)
            printMsg "外置SIM卡4"
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 1 > /sys/class/gpio/cpe-sel2/value
            echo 0 > /sys/class/gpio/cpe-sel1/value
            echo 1 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=0,1'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            echo "外置SIM卡4" >> /tmp/moduleInit
            echo "外置SIM卡4切换完成"
            echo "SIN卡UCI标识：$Sim_Sel"
            echo 5 > /tmp/sim_sel
        ;;
        *)
            echo 1 > /sys/class/gpio/cpe-sel3/value
            echo 0 > /sys/class/gpio/cpe-sel2/value
            echo 1 > /sys/class/gpio/cpe-sel1/value
            echo 0 > /sys/class/gpio/cpe-sel0/value
            sendat 1 'AT^SCICHG=0,1'
            sendat 1 'AT^HVSST=1,0'
            sleep 3
            sendat 1 'AT^HVSST=1,1'
            printMsg "卡槽错误状态"
            echo 6 > /tmp/sim_sel
            echo "卡槽未识别" >> /tmp/moduleInit
        ;;
        esac

    #MT5700-IMEI
    if [ ${Enable_IMEI} == 1 ];then
        IMEI_file="/tmp/IMEI"
        if [ -e "$IMEI_file" ]; then
            last_IMEI=$(cat "$IMEI_file")
        else
            last_IMEI=-1
        fi
        IMEI=`uci -q get modem.@ndis[0].modify_imei`
        if [ "$IMEI" != "$last_IMEI" ]; then
            sendat 1 "AT+CFUN=0"
            sleep 2
            sendat 1 "AT^PHYNUM=IMEI,$IMEI" >> /tmp/moduleInit
            sleep 3
            sendat 1 "AT+CFUN=1"
            printMsg "IMEI: ${IMEI}"
            echo "修改IMEI $IMEI" >> /tmp/moduleInit
            echo "修改IMEI $IMEI"
            echo "$IMEI" > "$IMEI_file"
        else
            echo "IMEI未变动, 不执行操作" >> /tmp/moduleInit
            echo "IMEI未变动, 不执行操作"
        fi
    fi

    # 网络模式选择
    #---------------------------------
 RF_Mode_file="/tmp/RF_Mode"
    if [ -e "$RF_Mode_file" ]; then
        last_RF_Mode=$(cat "$RF_Mode_file")
    else
        last_RF_Mode=-1
    fi
    #--
    if [ "$RF_Mode" != "$last_RF_Mode" ]; then
        if [ "$RF_Mode" == 0 ]; then
            sendat 1 'AT^C5GOPTION=1,1,1'
            echo "RF_Mode: $RF_Mode 自动网络" >> /tmp/moduleInit
            echo "切换自动网络：RF_Mode: $RF_Mode 自动网络"
            sendat 1 'AT^SYSCFGEX="080302",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,' >> /tmp/moduleInit
        elif [ "$RF_Mode" == 1 ]; then
            echo "RF_Mode: $RF_Mode 4G网络" >> /tmp/moduleInit
            echo "切换4G网络：RF_Mode: $RF_Mode 4G网络"
            sendat 1 'AT^SYSCFGEX="03",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,' >> /tmp/moduleInit
        elif [ "$RF_Mode" = 2 ]; then
            sendat 1 'AT^C5GOPTION=1,0,1'
            echo "RF_Mode: $RF_Mode 5G网络" >> /tmp/moduleInit
            echo "切换5G-SA：RF_Mode: $RF_Mode 5G网络"
            sendat 1 'AT^SYSCFGEX="08",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,' >> /tmp/moduleInit
        fi
        echo "$RF_Mode" > "$RF_Mode_file"
    else
        echo "RF_Mode未变动, 不执行操作" >> /tmp/moduleInit
        echo "网络制式选择RF_Mode未变动, 不执行操作"
    fi
    #-------------------------

    # LTE锁频
    #-------------------------
    if [ "$RF_Mode" == 1 ]; then

        Band_LTE_file="/tmp/Band_LTE"
        if [ -e "$Band_LTE_file" ]; then
            last_Band_LTE=$(cat "$Band_LTE_file")
        else
            last_Band_LTE=-1
        fi
        #--
        if [ "$Band_LTE" != "$last_Band_LTE" ]; then
            if [ "$Band_LTE" == 0 ]; then
                sendat 1 "AT+CFUN=0"
                sleep 3
                sendat_command='AT^SYSCFGEX="03",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,'
                sendat_result=$(sendat 1 "$sendat_command")
                echo "LTE自动: $sendat_result" >> /tmp/moduleInit
                echo "LTE锁频：自动: $sendat_result"
                sleep 3
                sendat 1 "AT+CFUN=1"
            else
                sendat 1 "AT+CFUN=0"
                sleep 3
                sendat_command="AT^LTEFREQLOCK=3,0,1,\"$Band_LTE\""
                sendat_result=$(sendat 1 "$sendat_command")
                echo "LTE锁频: $sendat_result" >> /tmp/moduleInit
                echo "LTE锁频: $sendat_result"
                sleep 3
                sendat 1 "AT+CFUN=1"
            fi
            echo "$Band_LTE" > "$Band_LTE_file"
        else
            echo "Band_LTE未变动, 不执行操作" >> /tmp/moduleInit
            echo "LTE锁频：Band_LTE未变动, 不执行操作"
        fi
    fi
    #----------------------


    # SA/NSA模式切换
    #----------------------
    if [ "$RF_Mode" == 2 ]; then
        #----------------------
        # SA锁频
        #----------------------
        band_sa_file="/tmp/Band_SA"
        if [ -e "$band_sa_file" ]; then
            last_Band_SA=$(cat "$band_sa_file")
        else
            last_Band_SA=-1
        fi
        #--
        if [ "$Band_SA" != "$last_Band_SA" ]; then
            if [ "$Band_SA" == 0 ]; then
                sendat 1 "AT+CFUN=0"
                sleep 3
                sendat_command='AT^SYSCFGEX="08",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,'
                sendat_result=$(sendat 1 "$sendat_command")
                echo "SA自动: $sendat_result" >> /tmp/moduleInit
                echo "5G-SA锁频：SA自动: $sendat_result"
                sleep 3
                sendat 1 "AT+CFUN=1"
            else
                sendat 1 "AT+CFUN=0"
                sleep 3
                sendat_command="AT^NRFREQLOCK=3,0,1,\"$Band_SA\""
                sendat_result=$(sendat 1 "$sendat_command")
                echo "SA锁频: $sendat_result" >> /tmp/moduleInit
                echo "5G-SA锁频: $sendat_result"
                sleep 3
                sendat 1 "AT+CFUN=1"
            fi
            echo "$Band_SA" > "$band_sa_file"
        else
            echo "5G-SA锁频：Band_SA未变动, 不执行操作" >> /tmp/moduleInit
            echo "5G-SA锁频：Band_SA未变动, 不执行操作"
        fi

    fi
    #-------------------

    #锁定频率
    #------------------------@Icey
    unlock() {
        printMsg "Unlock Band"
        echo "解锁4G锁"
        sendat 1 'AT^LTEFREQLOCK=0'
        sleep 1
        echo "解锁5G锁"
        sendat 1 'AT^NRFREQLOCK=0'
        return 0
    }

    band_lock() {
        printMsg "Start Band Lock"
        echo "开始锁小区..."
        if [ "$Freqlock" -eq 0 ]; then
        echo 'Freqlock" -eq 0 判断成功！'
            if [ ! -e "/tmp/freq.run" ]; then
            echo '/tmp/freq.run 判断成功！'
                if [ "$Band_SA" == 0 ] && [ "$Band_LTE" == 0 ]; then
                    printMsg "Restore band lock at boot"
                    echo "锁小区：Restore band lock at boot"
                    unlock
                fi
                echo '锁小区返回了!'
                return 0
            fi
            printMsg "Setting Will restore at next boot"
            echo "锁小区：Setting Will restore at next boot"
        fi 
            case "$RF_Mode" in
                0)
                    return 0
                    ;;
                1)
                    if [ "$Band_LTE" -ne 0 ] && [ "$Earfcn" -ne 0 ] && [ "$Cellid" -ne 0 ]; then
                        printMsg "BAND LOCK AT COMMON4G $Cellid,$Earfcn"
                        echo "4G锁小区： $Cellid,$Earfcn"
                        sendat 1 "AT+CFUN=0"
                        sleep 3
                        sendat 1 "AT^LTEFREQLOCK=2,0,1,\"$Band_LTE\",\"$Earfcn\",\"$Cellid\""
                        echo "4G锁小区命令：AT^LTEFREQLOCK=2,0,1,\"$Band_LTE\",\"$Earfcn\",\"$Cellid\""
                        sleep 2
                        sendat 1 "AT+CFUN=1"
                    else
                        if [ "$Band_LTE" == 0 ]; then
                            printMsg "Restore band lock at boot"
                            ehco "4G锁小区：Restore band lock at boot"
                            unlock
                        fi
                    fi
                    ;;
                2)
                    if [ "$NR_Mode" -ne 0 ] && [ "$Band_SA" -ne 0 -o "$Band_NSA" -ne 0 ] && [ "$Earfcn" -ne 0 ] && [ "$Cellid" -ne 0 ]; then
                            case "$Band_SA" in
                                1|2|3|5|7|8|12|20|25|28|66|71|75|76)
                                    scs=0
                                    ;;
                                38|40|41|48|77|78|79)
                                    scs=1
                                    ;;
                                257|258|260|261)
                                    scs=3
                                    ;;
                                *)
                                    printMsg "BANDLOCKFAILURE"
                                    ehco "5G锁小区错误!!!!!!!!!!!!!!"
                                    return 0
                                    ;;
                            esac
                        printMsg "BAND LOCK AT COMMON5G $Cellid,$Earfcn,$scs,$Band_SA"
                        echo "5G锁小区： $Cellid,$Earfcn,$scs,$Band_SA"
                        sendat 1 "AT+CFUN=0"
                        sleep 3
                        sendat 1 "AT^NRFREQLOCK=2,0,1,\"$Band_SA\",\"$Earfcn\",\"$scs\",\"$Cellid\""
                        echo "5G锁小区命令： AT^NRFREQLOCK=2,0,1,\"$Band_SA\",\"$Earfcn\",\"$scs\",\"$Cellid\""
                        sleep 2
                        sendat 1 "AT+CFUN=1"
                    else
                        if [ "$Band_SA" == 0 ]; then
                        printMsg "Restore band lock at boot"
                        echo "5G锁小区：Restore band lock at boot"
                        unlock
                        fi
                    fi
                    ;;
                *)
                    printMsg "BANDLOCKFAILURE"
                    echo "锁小区故障！"
                    return 0
                    ;;
            esac
    }
   
    #Check if SIM or esim exist
    chkSimExt() {
        sleep 5
        simStat=$(sendat 1 'AT^SIMSQ?' | awk '/^\^SIMSQ:/ {split($0, a, ","); print a[2]}'| tr -d '\r\n')
        echo "状态码：$simStat"
        sleep 5
        case $simStat in
            0)
                printMsg "SIM卡未插入."
                echo "SIM卡未插入" > /tmp/simcardstat
                echo "SIM卡未插入"
                printMsg "SIM卡未插入"
                ;;
            1)
                printMsg "SIM卡插入."
                echo "SIM卡已插入" > /tmp/simcardstat
                echo "SIM卡已插入"
                printMsg "SIM卡已插入"
                ;;
            2)
                echo "SIM卡被锁" > /tmp/simcardstat
                echo "SIM卡被锁"
                ;;
            3)
                echo "SIMLOCK 锁定(暂不支持上报)" > /tmp/simcardstat
                echo "SIMLOCK 锁定(暂不支持上报)"
                ;;
            10)
                echo "卡文件正在初始化 SIM Initializing" > /tmp/simcardstat
                ;;
            11)
                echo "卡初始化完成 （可接入网络）" > /tmp/simcardstat
                echo "卡初始化完成 （可接入网络）"
                printMsg "状态码11,卡初始化完成 （可接入网络）"
                ;;
            12)
                printMsg "SIM卡正常工作."
                echo "SIM卡正常工作" > /tmp/simcardstat
                echo "SIM卡正常工作"
                printMsg "状态码12,SIM卡正常工作"
                ;;
            98)
                echo "卡物理失效 （PUK锁死或者卡物理失效）" > /tmp/simcardstat
                ;;
            99)
                echo "卡移除 SIM removed" > /tmp/simcardstat
                ;;
            Note2)
                echo "不支持虚拟SIM卡" > /tmp/simcardstat
                ;;
            100)
                echo "卡错误（初始化过程中，卡失败）" > /tmp/simcardstat
                ;;
            *)
                echo "未知SIM卡状态" > /tmp/simcardstat
                echo "未知SIM卡状态 状态码：$simStat"
                printMsg "未知SIM卡状态 状态码：$simStat"
                ;;
        esac
    }

    #Check Module Hardware Set,pre check befroe everything
    moduleSetChk(){
        if [ -e "/tmp/CPEstartupSettings" ]; then
        macchk=0
        moduleSetChkMAX_RETRIES=10
        printMsg "Start Modem Hardware Check"
        sleep 1
        while [ $moduleSetChkMAX_RETRIES -gt 0 ]; do
        success = true

            #USB 端口形态配置,为linux系统下的NCM模式,Normal模式(产品默认模式)
            usb_port_configuration=$(sendat 1 'AT^SETMODE?' | sed -n '2p'  | grep '4')
            printMsg "usb_port_configuration Status: $usb_port_configuration"
            if ! echo "$usb_port_configuration" | grep -q "4"; then
                printMsg "usb_port_configuration Status check failed."
                sendat 1 'AT^SETMODE=4'
            fi

            #打开pcie 控制器
            pcieopen=$(sendat 1 ' AT^TDPMCFG?' | grep 'TDPMCFG' | tr -d '\r\n')
            printMsg "pcieopen Status: $pcieopen"
            if ! echo "$pcieopen" | grep -q "1,0,0,0"; then
                printMsg "pcieopen Status check failed."
                sendat 1 'AT^TDPMCFG=1,0,0,0'
            fi

            #PCIE 网卡phy,设置为RTL8125，支持2.5G速率
            pcie_phy=$(sendat 1 'AT^TDPCIELANCFG?' | grep 'TDPCIELANCFG' | tr -d '\r\n')
            printMsg "pcie_phy Status: $pcie_phy"
            if ! echo "$pcie_phy" | grep -q "2"; then
                printMsg "pcie_phy Status check failed."
                sendat 1 'AT^TDPCIELANCFG=2'
                #success=false
            fi

            #设置网卡数量
            network_cards=$(sendat 1 'AT^SETNETNUM?' | sed -n '2p'  | grep '1')
                printMsg "network_cards: $network_cards"
            if ! echo "$network_cards" | grep -q "1"; then
                printMsg "network_cards Status check failed."
                sendat 1 'AT^SETNETNUM=1'
            fi

            #关闭Fast Dormancy休眠 ，默认值为： ^FASTDORM: 1,5
            Fast_Dormancys=$(sendat 1 'AT^FASTDORM?' | grep 'FASTDORM')
                printMsg "Fast_Dormancys: $Fast_Dormancys"
            if ! echo "$Fast_Dormancys" | grep -q "FASTDORM: 0"; then
                printMsg "network_cards Status check failed."
                sendat 1 'AT^FASTDORM=0'
            fi

            #开启NR-CA
            nr_CA=$(sendat 1 'AT^NRRCCAPQRY=3' | grep 'NRRCCAPQRY' | tr -d '\r\n')
            printMsg "nr_ca Status: $nr_CA"
            if ! echo "$nr_CA" | grep -q "NRRCCAPQRY: 3,1"; then
                printMsg "nr_ca Status check failed."
                #配置开启NR-CA
                sendat 1 'AT^NRRCCAPCFG=3,1'
            fi

            #自动拨号
            sendat 1 'AT^SETAUTODIAL=1,2'


            if [ "$success" = false ]; then
                moduleSetChkMAX_RETRIES=$(($moduleSetChkMAX_RETRIES - 1))
                printMsg "Recheck Hardware Set...."                               
                sleep 2
                if [ $moduleSetChkMAX_RETRIES == 2 ]; then
                    #reboot
                    echo "1"
                fi                
            else
                printMsg "Hardware Check Complete."
                rm -rf "/tmp/CPEstartupSettings"
                return 0
            fi
        done

        fi
    }

    #Check if wan work
    check_and_activate_wan() {
    #sendat 1 'AT+CFUN=1'
    echo "开始检查网络连接是否异常..."
    sleep 1
    max_retries=60
    retry_interval=3
    retries=0
    while [ "$retries" -lt "$max_retries" ]; do
            ipv4_info=$(sendat 1 'AT+CGPADDR' | grep 'CGPADDR: 8'  | awk -F \" {'print $2'} | tr -d '\r\n')
            printMsg "Modem ip is now $ipv4_info"
            echo "模组现在的IPV4地址： ip is now $ipv4_info"
        if [ -z "$ipv4_info" ] || ! valid_ip "$ipv4_info" ; then
            printMsg "Retry $((retries + 1)): IPv4 address not obtained or invalid. Retrying in $retry_interval seconds..."
            ehco "重试 $((retries + 1)): IPv4 地址没有获取到或者正在获取. Retrying in $retry_interval seconds..."
            sleep "$retry_interval"
            retries=$((retries + 1))
        else           
            #/sbin/ifup wan
            sleep 2
            wantstcount=0
            while [ $wantstcount -lt 20 ]; do
                http_codeChk0=$(curl -o /dev/null -s -w %{http_code} http://connect.rom.miui.com/generate_204)
                http_codeChk1=$(curl -o /dev/null -s -w %{http_code} http://connectivitycheck.platform.hicloud.com/generate_204)

                if [ "$http_codeChk0" -eq 204 ] || [ "$http_codeChk1" -eq 204 ]; then
                    printMsg "Internet Connection Ready."
                    ehco "网络已连接！"
                    break
                else
                    printMsg "HTTP status code is not 204 for either URL. Retry $((count + 1))."
                    ehco "网络检查失败代码204，重试$((count + 1))."
                fi
                sleep 1
                wantstcount=$((wantstcount + 1))
                if [ "$wantstcount" -eq 20 ]; then
                    printMsg "Failed to Init Modem after $wantstcount"
                    ehco "Failed to Init Modem after $wantstcount"
                    exit 1
                fi
            done
            printMsg "IPV4 WAN UP!Ready to go Internet!"
            ehco "IPV4地址已完成获取，并成功接入互联网。"
            #/sbin/ifup wan6
            sleep 5
            #ehco "执行IPV6开关判断..."
            #/usr/share/modem/enableipv6v2.sh & 
        fi
    done

    if [ "$retries" -eq "$max_retries" ]; then
        printMsg "Failed to obtain valid IPv4 address after $max_retries retries. Exiting program." >>/tmp/moduleInit
        printMsg "Job is FAILURE"

        exit 1
    fi
    }

    valid_ip() {
    local ip=$1

    # 包含IP地址的正则表达式
    if echo "$ip" | grep -q -E '([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)'; then
        if echo "$ip" | grep -q "0\.0\.0\.0"; then
            return 1  # 返回 1 表示错误
        else
            return 0
        fi
        
    fi

    return 1  # 返回 1 表示错误
    }

    moduleStartCheckLine(){
        chkSimExt
        echo "chkSimExt $?" 
        moduleSetChk
        band_lock
        echo "moduleSetChk $?" 
        sleep 3
        smsen=`uci -q get modem.@ndis[0].smsen` || smsen=0
        if [ "$smsen" == 0 ]; then
            sendat 1 'AT+CGDCONT=5,"IPV4V6","","",0,0,0,0,1,1,1,,,,,,0,,0,0,0,0'
            echo "关闭短信中..." 
            sleep 1
            sendat 1 'AT+CEUS=1'
            sleep 1
            sendat 1 'AT^IMSSWITCH=0,0,0'
            sleep 1
            sendat 1 'AT+CFUN=0'
            sleep 2
            sendat 1 'AT+CFUN=1'
            echo "关闭短信完成" 
        else
            sendat 1 'AT+CGDCONT=5,"IPV4V6","ims","",0,0,0,0,1,1,1,,,,,,0,,0,0,0,0'
            echo "开启短信中..." 
            sleep 1
            sendat 1 'AT+CEUS=0'
            sleep 1
            sendat 1 'AT^IMSSWITCH=1,0,0'
            sleep 1
            sendat 1 'AT+CFUN=0'
            sleep 2
            sendat 1 'AT+CFUN=1'
            echo "开启短信完成" 
        fi

        sleep 5
        check_and_activate_wan
    }
    #start
    moduleStartCheckLine
    rm $lock_file
    exit



