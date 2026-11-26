ARCH:=aarch64
SUBTARGET:=filogic_mtwifi
BOARDNAME:=Filogic 8x0 (Vendor Wi-Fi 6 / mtwifi)
CPU_TYPE:=cortex-a53
ARCH_PACKAGES:=aarch64_cortex-a53
CFLAGS:=-Os -pipe -march=armv8-a+crypto+crc -mtune=cortex-a53 -mno-outline-atomics
DEFAULT_PACKAGES += fitblk kmod-phy-aquantia kmod-crypto-hw-safexcel kmod-mediatek_hnat wireless-regdb uboot-envtools mtkhqos_util
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic boards with Vendor Wi-Fi 6 (mtwifi) drivers.
endef
