ARCH:=aarch64
SUBTARGET:=filogic
BOARDNAME:=Filogic 8x0 (MT798x)
CPU_TYPE:=cortex-a53
ARCH_PACKAGES:=aarch64_cortex-a53
CFLAGS:=-Os -pipe -mcpu=cortex-a53+crypto
DEFAULT_PACKAGES += fitblk kmod-phy-aquantia kmod-crypto-hw-safexcel wpad-openssl uboot-envtools kmod-mt798x-2p5g-phy mtkhqos_util
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic ARM based boards.
endef
