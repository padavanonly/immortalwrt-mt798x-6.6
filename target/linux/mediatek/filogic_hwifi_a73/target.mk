ARCH:=aarch64
SUBTARGET:=filogic_hwifi_a73
BOARDNAME:=Filogic 8x0 (Vendor Wi-Fi 7 / hwifi A73)
CPU_TYPE:=generic
CPU_SUBTYPE:=cortex-a73
ARCH_PACKAGES:=aarch64_cortex-a53
CFLAGS:=-O2 -pipe -march=armv8-a+crypto+crc -mtune=cortex-a73 --param=l1-cache-size=64 --param=l1-cache-line-size=64 --param=l2-cache-size=1024 -mno-outline-atomics -mno-fix-cortex-a53-835769 -mno-fix-cortex-a53-843419 -falign-functions=16:12 -falign-loops=8:4
DEFAULT_PACKAGES += fitblk kmod-crypto-hw-safexcel kmod-mediatek_hnat wireless-regdb uboot-envtools kmod-mt798x-2p5g-phy mtkhqos_util
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic Cortex-A73 boards with Vendor Wi-Fi 7 (hwifi) drivers (Cortex-A73 tuned).
endef
