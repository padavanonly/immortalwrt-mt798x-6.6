export MODULE_FLAGS

KBUILD_EXTRA_SYMBOLS+= $(TOP_DIR)/os/linux/Module.symvers
$(info PLUG-IN $(TOP_DIR))

ifdef CONFIG_WHNAT_SUPPORT
ifeq ($(CONFIG_WARP_V2), y)
#obj-$(CONFIG_WHNAT_SUPPORT)+= warp_proxy/
else
#obj-$(CONFIG_WHNAT_SUPPORT)+= whnat/
endif
endif

ifdef CONFIG_DEBUGGER_SUPPORT
obj-$(CONFIG_DEBUGGER_SUPPORT)+= debugger/
endif

clean:
	find . -name "*.o" | xargs rm -rf
	find . -name "*.ko" | xargs rm -rf
	find . -name "modules.order" | xargs rm -rf
	find . -name "Module.symvers" | xargs rm -rf
install:
