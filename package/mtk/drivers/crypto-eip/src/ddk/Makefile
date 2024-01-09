# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2023 MediaTek Inc.

obj-$(CONFIG_RAMBUS_DDK) += crypto-eip-ddk.o

# slad
crypto-eip-ddk-y += ./slad/adapter_dmabuf.o
crypto-eip-ddk-y += ./slad/adapter_driver197_init.o
crypto-eip-ddk-y += ./slad/adapter_global_cs_init.o
crypto-eip-ddk-y += ./slad/adapter_global_drbg_init.o
crypto-eip-ddk-y += ./slad/adapter_global_eip207.o
crypto-eip-ddk-y += ./slad/adapter_global_eip74.o
crypto-eip-ddk-y += ./slad/adapter_global_eip97.o
crypto-eip-ddk-y += ./slad/adapter_global_init.o
crypto-eip-ddk-y += ./slad/adapter_lock_internal.o
crypto-eip-ddk-y += ./slad/adapter_pcl_generic.o
crypto-eip-ddk-y += ./slad/adapter_pcl_dtl.o
crypto-eip-ddk-y += ./slad/adapter_pec_dma.o
crypto-eip-ddk-y += ./slad/adapter_pec_pktbuf.o
crypto-eip-ddk-y += ./slad/adapter_ring_eip202.o
crypto-eip-ddk-y += ./slad/adapter_sglist.o
crypto-eip-ddk-y += ./slad/lkm/adapter_firmware.o
crypto-eip-ddk-y += ./slad/lkm/adapter_interrupts.o
crypto-eip-ddk-y += ./slad/lkm/adapter_lock.o
crypto-eip-ddk-y += ./slad/lkm/adapter_sleep.o
# reference FPGA_Zynq_ZC706
crypto-eip-ddk-y += ./slad/adapter_init.o

# device
crypto-eip-ddk-y += ./device/dmares_gen.o
crypto-eip-ddk-y += ./device/device_generic.o
crypto-eip-ddk-y += ./device/lkm/dmares_lkm.o
crypto-eip-ddk-y += ./device/lkm/of/device_lkm.o
crypto-eip-ddk-y += ./device/lkm/of/lkm.o

# sa builder
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder.o
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_basic.o
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_extended_basic.o
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_extended_ipsec.o
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_ipsec.o
# crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_srtp.o
crypto-eip-ddk-y += ./kit/builder/sa/sa_builder_ssltls.o

# token builder
crypto-eip-ddk-y += ./kit/builder/token/token_builder_context.o
crypto-eip-ddk-y += ./kit/builder/token/token_builder_core.o

# eip197
crypto-eip-ddk-y += ./kit/eip197/eip201.o
crypto-eip-ddk-y += ./kit/eip197/eip202_cd_format.o
crypto-eip-ddk-y += ./kit/eip197/eip202_cdr_dscr.o
crypto-eip-ddk-y += ./kit/eip197/eip202_cdr_event.o
crypto-eip-ddk-y += ./kit/eip197/eip202_cdr_fsm.o
crypto-eip-ddk-y += ./kit/eip197/eip202_cdr_init.o
crypto-eip-ddk-y += ./kit/eip197/eip202_global_init.o
crypto-eip-ddk-y += ./kit/eip197/eip202_rd_format.o
crypto-eip-ddk-y += ./kit/eip197/eip202_rdr_dscr.o
crypto-eip-ddk-y += ./kit/eip197/eip202_rdr_event.o
crypto-eip-ddk-y += ./kit/eip197/eip202_rdr_fsm.o
crypto-eip-ddk-y += ./kit/eip197/eip202_rdr_init.o
crypto-eip-ddk-y += ./kit/eip197/eip207_flow_dtl.o
crypto-eip-ddk-y += ./kit/eip197/eip207_flow_generic.o
crypto-eip-ddk-y += ./kit/eip197/eip207_flow_internal.o
crypto-eip-ddk-y += ./kit/eip197/eip207_flue.o
crypto-eip-ddk-y += ./kit/eip197/eip207_fluec.o
crypto-eip-ddk-y += ./kit/eip197/eip207_global_init.o
crypto-eip-ddk-y += ./kit/eip197/eip207_ice.o
crypto-eip-ddk-y += ./kit/eip197/eip207_oce.o
crypto-eip-ddk-y += ./kit/eip197/eip207_rc.o
crypto-eip-ddk-y += ./kit/eip197/eip207_rc_internal.o
crypto-eip-ddk-y += ./kit/eip197/eip207_support.o
crypto-eip-ddk-y += ./kit/eip197/eip74.o
crypto-eip-ddk-y += ./kit/eip197/eip97_global_event.o
crypto-eip-ddk-y += ./kit/eip197/eip97_global_fsm.o
crypto-eip-ddk-y += ./kit/eip197/eip97_global_init.o
crypto-eip-ddk-y += ./kit/eip197/eip97_global_prng.o

# log
crypto-eip-ddk-y += ./kit/log/log.o

# list
crypto-eip-ddk-y += ./kit/list/list.o

# iotoken
crypto-eip-ddk-y += ./kit/iotoken/iotoken.o

# ring
crypto-eip-ddk-y += ./kit/ring/ringhelper.o

# log
crypto-eip-ddk-y += ./kit/log/log.o
