obj-$(CONFIG_RAMBUS_DDK) += crypto-eip-ddk-ksupport.o

ccflags-y += -I$(src)/../../inc/crypto-eip/ddk/configs/ksupport
ccflags-y += -I$(src)/../../inc/crypto-eip/ddk/configs/ksupport/shdevxs
ccflags-y += -I$(src)/../../inc/crypto-eip/ddk/configs/ksupport/umdevxs

$(info not that bad!!!)
$(info ccflags: $(ccflags-y))

# Kernel Support
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_lkm.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_ofdev.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_chrdev.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_device.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_smbuf.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_bufadmin.o
crypto-eip-ddk-ksupport-y += ../../device/umdevxs/umdevxs_interrupt.o

crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_kernel_stub.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_kernel_lock.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_kernel_init.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_init.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_dmapool.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_irq.o
crypto-eip-ddk-ksupport-y += ../../device/shdevxs/shdevxs_prng.o

crypto-eip-ddk-ksupport-y += ../../slad/lkm/adapter_interrupts.o
crypto-eip-ddk-ksupport-y += ../../kit/log/log.o
crypto-eip-ddk-ksupport-y += ../../kit/eip197/eip201.o
crypto-eip-ddk-ksupport-y += ../../device/device_generic.o
crypto-eip-ddk-ksupport-y += ../../device/lkm/of/lkm.o
crypto-eip-ddk-ksupport-y += ../../device/lkm/of/device_lkm.o