ARCH:=aarch64
SUBTARGET:=filogic_a73
BOARDNAME:=Filogic 8x0 (MT798x - A73)
CPU_TYPE:=generic
CPU_SUBTYPE:=cortex-a73
ARCH_PACKAGES:=aarch64_cortex-a53
CFLAGS:=-Os -pipe -march=armv8-a+crypto+crc -mtune=cortex-a73
DEFAULT_PACKAGES += fitblk kmod-phy-aquantia kmod-crypto-hw-safexcel wpad-openssl uboot-envtools kmod-mt798x-2p5g-phy mtkhqos_util
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic MT7988 boards with Cortex-A73 tuning.
endef
