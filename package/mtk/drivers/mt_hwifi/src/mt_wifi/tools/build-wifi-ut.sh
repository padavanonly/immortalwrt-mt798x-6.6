#!/bin/bash

TIME=`date +\%F-\%T`
echo
echo "######## ${TIME} ########"
echo

EmbeddedDir=`pwd`
WifiUtDir=${EmbeddedDir}/../wifi_ut/
OLD_PATH=${PATH}
export PATH=/mtkoss/gcc/9.4.0-ubuntu-14.04/bin:${PATH}
GCC_VERSION=`gcc -v`
echo "GCC_VERSION: ${GCC_VERSION}"

#KERNEL_HEADER_PREFIX_MTKSLT="/mtkoss/linux_headers/linux-headers-"
#KERNEL_HEADER_PREFIX_MTKSLT="/mfs/mtkslt1051/mtk30983/linux-headers__kasan/linux-headers-"
KERNEL_HEADER_PREFIX_MTKSLT="/mtkoss/linux_headers/linux_headers_uk/linux-headers-"
BUILD_LINUX_SRC=${KERNEL_HEADER_PREFIX_MTKSLT}5.0.0-37-generic
use_specified_kernel=1
use_specified_chip=$1
echo "use_specified_chip: ${use_specified_chip}"

# COMBINATION: specify wifi driver build parameter
# Please keep mt7990 as last one due to prelight check file procedure
#	 CHIP	  PLATFORM  WIFI_MODE	OS   HIF   UL_CTRL
declare -a COMBINATION=(
	"mt7990 	PC	AP	LINUX	pcie	CFG80211"
	"mt7992 	PC	AP	LINUX	pcie	CFG80211"
	)

if [ $use_specified_chip ]; then
	echo "build ${use_specified_chip}"
	# PATH: for checking build result .ko file
	lowcase_os=`echo linux`
	RESULT_PATH="os/${lowcase_os}/"
	# compose .ko file name
	driver_name=${use_specified_chip}\_AP
	driver_name=${driver_name,,} # cast the string to lower-case
	echo "driver_name: $driver_name"
	echo "RESULT_PATH: ${RESULT_PATH}${driver_name}.ko"
	# Logan clean
	make -f mk/Makefile clean
	# Logan build with debug symbol
	make -f mk/Makefile LINUX_SRC=$BUILD_LINUX_SRC CHIPSET=$use_specified_chip PLATFORM=PC WIFI_MODE=AP TARGET=LINUX WIFI_HIF=pcie UL_CTRL=CFG80211 CCACHE=1 CONFIG_HWIFI_SUPPORT=y CONFIG_UNI_CMD_SUPPORT=y CONFIG_DBG_CFG=n CONFIG_FUZZ_SUPPORT=y ADD_DBG_SYM=y -j8
	# check
	#   make return code
	#   check ${driver_name}.ko exist
	if [ -f "${RESULT_PATH}${driver_name}.ko" ]; then
		echo "${driver_name} built successfully."
	else
		echo "${driver_name} built FAILED (ko not found)!"
		exit 1
	fi
	cp -f ${RESULT_PATH}${driver_name}.ko ${EmbeddedDir}/.
	# Logan clean
	make -f mk/Makefile clean
	# Logan build
	make -f mk/Makefile LINUX_SRC=$BUILD_LINUX_SRC CHIPSET=$use_specified_chip PLATFORM=PC WIFI_MODE=AP TARGET=LINUX WIFI_HIF=pcie UL_CTRL=CFG80211 CCACHE=1 CONFIG_HWIFI_SUPPORT=y CONFIG_UNI_CMD_SUPPORT=y CONFIG_DBG_CFG=n CONFIG_FUZZ_SUPPORT=y -j8
	# check
	#   make return code
	#   check ${driver_name}.ko exist
	if [ -f "${RESULT_PATH}${driver_name}.ko" ]; then
		echo "${driver_name} built successfully."
	else
		echo "${driver_name} built FAILED (ko not found)!"
		exit 1
	fi
	#wifi_ut
	if [ -d $WifiUtDir ]; then
		echo "build wifi_ut now!"
		cd $WifiUtDir
		if [ $use_specified_kernel -eq 0 ]; then
			make clean
			make
		else
			make KERNEL_DIR=$BUILD_LINUX_SRC clean
			make KERNEL_DIR=$BUILD_LINUX_SRC
		fi

		if [ -f "${WifiUtDir}wifi_ut.ko" ]; then
			make release CHIPSET=$use_specified_chip
			echo "wifi_ut built successfully."
		else
			echo "wifi_ut built FAILED (ko not found)!"
			exit 1
		fi
	fi
else
	echo "build all"
	for combination in "${COMBINATION[@]}"
	do
		# parse build combination
		comb_arr=($combination)
		chip=${comb_arr[0]}
		platform=${comb_arr[1]}
		wifi_mode=${comb_arr[2]}
		os=${comb_arr[3]}
		hif=${comb_arr[4]}
		ul_ctrl=${comb_arr[5]}

		# compose .ko file name
		driver_name=$chip\_$wifi_mode
		driver_name=${driver_name,,} # cast the string to lower-case

		# PATH: for checking build result .ko file
		lowcase_os=`echo "$os" | awk '{ print tolower($1) }'`
		RESULT_PATH="os/${lowcase_os}/"

		echo "driver_name=$driver_name"

		# clean
		make -f mk/Makefile clean

		# make
		if [ $use_specified_kernel -eq 0 ]; then
			make -f mk/Makefile CHIPSET=$chip PLATFORM=$platform WIFI_MODE=$wifi_mode TARGET=$os WIFI_HIF=$hif UL_CTRL=$ul_ctrl CCACHE=1 CONFIG_HWIFI_SUPPORT=y CONFIG_UNI_CMD_SUPPORT=y CONFIG_DBG_CFG=n CONFIG_FUZZ_SUPPORT=y -j8
		else
			make -f mk/Makefile LINUX_SRC=$BUILD_LINUX_SRC CHIPSET=$chip PLATFORM=$platform WIFI_MODE=$wifi_mode TARGET=$os WIFI_HIF=$hif UL_CTRL=$ul_ctrl CCACHE=1 CONFIG_HWIFI_SUPPORT=y CONFIG_UNI_CMD_SUPPORT=y CONFIG_DBG_CFG=n CONFIG_FUZZ_SUPPORT=y -j8
		fi

		# check
		#   make return code
		#   check ${driver_name}.ko exist
		if [ -f "${RESULT_PATH}${driver_name}.ko" ]; then
			echo "${driver_name} built successfully."
		else
			echo "${driver_name} built FAILED (ko not found)!"
			exit 1
		fi
	done

	#wifi_ut
	if [ -d $WifiUtDir ]; then
		echo "build wifi_ut now!"
		cd $WifiUtDir
		if [ $use_specified_kernel -eq 0 ]; then
			make clean
			make
		else
			make KERNEL_DIR=$BUILD_LINUX_SRC clean
			make KERNEL_DIR=$BUILD_LINUX_SRC
		fi

		if [ -f "${WifiUtDir}wifi_ut.ko" ]; then
			echo "wifi_ut built successfully."
		else
			echo "wifi_ut built FAILED (ko not found)!"
			exit 1
		fi
	fi
fi

TIME=`date +\%F-\%T`
echo
echo "######## ${TIME} ########"
echo

