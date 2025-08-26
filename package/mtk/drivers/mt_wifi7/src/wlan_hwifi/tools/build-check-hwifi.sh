
#!/bin/bash 

TIME=`date +\%F-\%T`
echo 
echo "######## ${TIME} ########"
echo

OLD_PATH=${PATH}
export PATH=/mtkoss/gcc/5.4.0-ubuntu-14.04/bin:${PATH}
GCC_VERSION=`gcc -v`
echo "GCC_VERSION: ${GCC_VERSION}"

KERNEL_HEADER_PREFIX_MTKSLT="/mtkoss/linux_headers/linux-headers-"
BUILD_LINUX_SRC=${KERNEL_HEADER_PREFIX_MTKSLT}5.0.0-37-generic

	make -f Makefile.6 KERNEL_DIR=$BUILD_LINUX_SRC clean

	# make
	#make LINUX_SRC=$BUILD_LINUX_SRC LINUX_SRC_MODULE=$BUILD_LINUX_SRC_MODULE -j8
	make -f Makefile.6 WM_RAM=rebb DRIVER_REPO=logan CONFIG_HWIFI_IF_MAC80211=n KERNEL_DIR=$BUILD_LINUX_SRC -j8

TIME=`date +\%F-\%T`
echo 
echo "######## ${TIME} ########"
echo 
