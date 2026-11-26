obj-$(CONFIG_RAMBUS_DDK) += crypto-eip-ddk-engine.o

ccflags-y += -I$(src)/../../inc/crypto-eip/ddk/configs/pec_pcl

# Driver PEC PCL
crypto-eip-ddk-engine-y += ../../slad/adapter_driver197_pec_pcl_init.o
crypto-eip-ddk-engine-y += ../../slad/adapter_init_pec.o
crypto-eip-ddk-engine-y += ../../slad/adapter_int_shdevxs.o
crypto-eip-ddk-engine-y += ../../slad/lkm/adapter_sleep.o
crypto-eip-ddk-engine-y += ../../slad/lkm/adapter_lock.o
crypto-eip-ddk-engine-y += ../../slad/adapter_lock_internal.o
crypto-eip-ddk-engine-y += ../../slad/adapter_pcl_generic.o
crypto-eip-ddk-engine-y += ../../slad/adapter_pcl_dtl.o
crypto-eip-ddk-engine-y += ../../slad/adapter_pec_dma.o
crypto-eip-ddk-engine-y += ../../slad/adapter_pec_pktbuf.o
crypto-eip-ddk-engine-y += ../../slad/adapter_ring_eip202.o
crypto-eip-ddk-engine-y += ../../slad/adapter_sglist.o
crypto-eip-ddk-engine-y += ../../slad/adapter_dmabuf.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip207_flow_internal.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip207_flow_generic.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip207_flow_dtl.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_cdr_init.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_cdr_dscr.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_cdr_event.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_cd_format.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_cdr_fsm.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_rdr_init.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_rdr_dscr.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_rdr_event.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_rd_format.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip202_rdr_fsm.o
crypto-eip-ddk-engine-y += ../../kit/eip197/eip201.o
crypto-eip-ddk-engine-y += ../../kit/iotoken/iotoken.o
crypto-eip-ddk-engine-y += ../../kit/ring/ringhelper.o
crypto-eip-ddk-engine-y += ../../kit/list/list.o
crypto-eip-ddk-engine-y += ../../device/lkm/of/device_lkm.o
crypto-eip-ddk-engine-y += ../../device/lkm/dmares_lkm.o
crypto-eip-ddk-engine-y += ../../device/dmares_gen.o
crypto-eip-ddk-engine-y += ../../device/device_generic.o

# sa builder
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_basic.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_extended_basic.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_extended_ipsec.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_ipsec.o
# crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_srtp.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_ssltls.o
crypto-eip-ddk-engine-y += ../../kit/builder/sa/sa_builder_extended_dtls.o

# token builder
crypto-eip-ddk-engine-y += ../../kit/builder/token/token_builder_context.o
crypto-eip-ddk-engine-y += ../../kit/builder/token/token_builder_core.o