include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=Airpi-gpio-fan
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/Airpi-gpio-fan
  SUBMENU:=Other modules
  TITLE:=GPIO PWM Fan Control Driver
  FILES:=$(PKG_BUILD_DIR)/Airpi-gpio-fan.ko
  AUTOLOAD:=$(call AutoLoad,90,Airpi-gpio-fan)
  KCONFIG:=
endef

define KernelPackage/Airpi-gpio-fan/description
  Kernel module for PWM fan control using GPIO
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(Build/Patch)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KDIR="$(LINUX_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,Airpi-gpio-fan))
