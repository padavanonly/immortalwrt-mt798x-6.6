#!/bin/bash
# SPDX-License-Identifier: <SPDX License Expression>

# translate coda for warp driver usage
################################################
# wed0.h [module: wed0, h_connac link]
################################################
filename='wed_hw.h'

# copy file
cp wed0.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(WED0_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/WED0_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ WED0_/ /g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# wdma0_v3.h [module: WDMA0_v3, h_connac link]
################################################
filename='wdma_hw.h'

# copy file
cp wdma0_v3.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(WDMA0_V3_BASE + 0x/0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/WDMA0_V3_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ WDMA0_V3_/ W/g' $filename
	sed -i 's/) \/\/ [[:xdigit:]]\+$//g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# pao0_gather_only.h [module: pao0_gather_only, h_connac link]
################################################
filename='pao_hw.h'

# copy file
cp pao0_gather_only.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(PAO0_GATHER_ONLY_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/PAO0_GATHER_ONLY_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ PAO0_GATHER_ONLY_/ /g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# wo0_ap2wo_mcu_ccif4.h [module: WO0_AP2WO_MCU_CCIF4, h_connac link]
################################################
filename='wox_ap2wo_mcu_ccif4.h'

# copy file
cp wo0_ap2wo_mcu_ccif4.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i -E 's/\(WO0_AP2WO_MCU_CCIF4_BASE.*0x([0-9a-fA-F]+)u.*/0x0000\1/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/WO0_AP2WO_MCU_CCIF4_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/WO0_AP2WO_MCU_CCIF4_/WOX_AP2WO_MCU_CCIF4_/g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# conn_mcu_cfg_hs.h [module: wo0_mcu_cfg_hs, h_connac link]
################################################
filename='wox_mcu_cfg_hs.h'

# copy file
cp conn_mcu_cfg_hs.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i -E 's/\(CONN_MCU_CFG_HS_BASE.*0x([0-9a-fA-F]+)u.*/0x0000\1/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/CONN_MCU_CFG_HS_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/CONN_MCU_CFG_/WOX_MCU_CFG_/g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

# remain base_addr
################################################
# conn_mcu_cfg_ls.h [module: wo0_mcu_cfg_ls, h_connac link]
################################################
filename='wox_mcu_cfg_ls.h'

# copy file
cp conn_mcu_cfg_ls.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(CONN_MCU_CFG_LS_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ CONN_MCU_CFG_/ WOX_MCU_CFG_/g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

# remain base_addr
################################################
# conn_mcu_cfg_on.h [module: wo0_mcu_cfg_on, h_connac link]
################################################
filename='wox_mcu_cfg_on.h'

# copy file
cp conn_mcu_cfg_on.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i -E 's/\(CONN_MCU_CFG_ON_BASE.*0x([0-9a-fA-F]+)u.*/0x0000\1/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/CONN_MCU_CFG_/WOX_MCU_CFG_/g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# pcie_mac_ireg.h [module: PCIE_MAC_IREG, h_connac link]
################################################
filename='pcie_mac.h'

# copy file
cp pcie_mac_ireg.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(PCIE_MAC_IREG_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/PCIE_MAC_IREG_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ PCIE_MAC_IREG_/ PCIE_MAC_/g' $filename

	# remove trailing 'u' in every number
	sed -i -E 's/([[:xdigit:]])u( |$)/\1\2/g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi
