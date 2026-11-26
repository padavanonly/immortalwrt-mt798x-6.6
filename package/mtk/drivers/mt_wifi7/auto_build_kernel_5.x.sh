if [ -d wifi_driver ]; then
    mkdir -p mt_wifi_ap
    mkdir -p mt_wifi_sta
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
    rm -rf mt_wifi
    cp -rf wifi_driver mt_wifi
    cd mt_wifi
    WIFI_MODE=AP
    TOP_DIR=./
    TOP_BIN_DIR=../
    export TOP_DIR CHIPSET WIFI_MODE TOP_BIN_DIR
    echo "harrier mt_wifi autobuild"
    CHIPSET=mt7915
    make -f mk/Makefile build_tools
    echo "Bellwether mt_wifi autobuild"
    CHIPSET=bellwether
    make -f mk/Makefile build_tools

    SKU_5MHZ_SHIFT=$2
    export SKU_5MHZ_SHIFT
    # run sinelg sku table transfomr to header file
    make -f mk/Makefile build_power_limit_tables
    echo "Eagle mt_wifi autobuild"
    CHIPSET=mt7990
    make -f mk/Makefile build_tools
    # let bin2h.c to get the env to generate correct firmware header
    MT7990_2ADIE_TRIBAND=y
    echo $MT7990_2ADIE_TRIBAND
    export MT7990_2ADIE_TRIBAND
    make -f mk/Makefile build_tools
    echo "Kite mt_wifi autobuild"
    CHIPSET=mt7992
    # let bin2h.c to get the env to generate correct firmware header
    SKU=$1
    FW_LOG_TYPE=$2
    echo $SKU
    echo $FW_LOG_TYPE
    export SKU FW_LOG_TYPE
    make -f mk/Makefile build_tools
    echo "Kite CCN34 mt_wifi autobuild"
    PROJECT=CCN34
    export PROJECT
    make -f mk/Makefile build_tools
    unset PROJECT
    echo "Griffin mt_wifi autobuild"
    CHIPSET=mt7993
    make -f mk/Makefile build_tools
    echo "Blackhawk mt_wifi autobuild"
    CHIPSET=blackhawk
    make -f mk/Makefile build_tools
else
    exit 1
fi
