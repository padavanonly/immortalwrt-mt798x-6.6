#!/bin/sh

###############################################
# copy original file to processed file
###############################################
cp wed0_c_header.h wed_hw.h
cp WDMA0_v3_c_header.h wdma_hw.h
cp pao2_reg.h pao_hw.h
cp wo2_mcu_cfg_ls_reg.h wox_mcu_cfg_ls.h
cp wo2_mcu_cfg_hs_reg.h wox_mcu_cfg_hs.h
cp WO2_AP2WO_MCU_CCIF4_reg.h wox_ap2wo_mcu_ccif4.h

# translate coda for warp driver usage
################################################
# wed0_c_header.h [module: wed0, H link]
################################################
# add some def and macro
sed -i '8 a typedef unsigned int UINT32;' wed_hw.h
sed -i '12 a #define REG_FLD(width, shift) (shift)' wed_hw.h

# change offset
sed -i 's/INREG32(.*\ /0x0000/g' wed_hw.h

# remove some lines
sed -i '/wed0_BASE\|REG_FLD_GET\|REG_FLD_SET\|REG_FLD_VAL\|include/d' wed_hw.h

################################################
# WDMA0_v3_c_header.h [module: WDMA0_v3, H link]
################################################
# add some def and macro
sed -i '12 a #define REG_FLD(width, shift) (shift)' wdma_hw.h
sed -i '14 a typedef unsigned int UINT32;' wdma_hw.h

# change offset
sed -i 's/INREG32(.*\ /0x000000/g' wdma_hw.h

# change name
sed -i 's/define\ DMA/define \ WDMA/g' wdma_hw.h

# remove some lines
sed -i '/WDMA0_v3_BASE\|REG_FLD_GET\|REG_FLD_SET\|REG_FLD_VAL\|include/d' wdma_hw.h

# Search for any hex value
hex_val_arr=($(grep -Eow "0x[0-9a-zA-Z]+" wdma_hw.h))

# First hex value as starting address
start_value=0

# If hex value is found
if [[ ${#hex_val_arr[@]} -ge 1 ]]; then
	start_value=${hex_val_arr[i]}
else
	printf "No hex value found\n"
	exit 1
fi

# Subtraction by starting address
for (( i=0; i<${#hex_val_arr[@]}; i++ ));
do
	offset=$(( ${hex_val_arr[i]} - $start_value ))
	cmd=$(printf "sed -i 's/%s/0x%010X/g' wdma_hw.h" ${hex_val_arr[i]} $offset)
	eval "$cmd"
done

################################################
# pao2_reg.h [module: pao0, sysc_reg link]
################################################
sed -i 's/PAO2_//g' pao_hw.h
sed -i 's/(.*\ 5/0x00001/g' pao_hw.h

# add some def and macro
sed -i '10 a typedef unsigned int uint32_t;' pao_hw.h

# remove some line
sed -i '/include/d' pao_hw.h

################################################
# wo2_mcu_cfg_ls_reg.h [module: wo0_mcu_cfg_ls, sysc_reg link]
################################################
sed -i 's/WO2_MCU/WOX_MCU/g' wox_mcu_cfg_ls.h
sed -i 's/WO0_CPU/WOX_CPU/g' wox_mcu_cfg_ls.h
sed -i 's/WO0_BOOT/WOX_BOOT/g' wox_mcu_cfg_ls.h

# remove some line
sed -i '/include\|pragma/d' wox_mcu_cfg_ls.h

# add some def and macro
sed -i '1 a #ifndef __WOX_MCU_CFG_LS_H__' wox_mcu_cfg_ls.h
sed -i '2 a #define __WOX_MCU_CFG_LS_H__' wox_mcu_cfg_ls.h
sed -i '8 a typedef unsigned int uint32_t;' wox_mcu_cfg_ls.h
sed -i '1713 a #endif' wox_mcu_cfg_ls.h

################################################
# wo2_mcu_cfg_hs_reg.h [module: wo0_mcu_cfg_hs, sysc_reg link]
################################################
sed -i 's/WO2_MCU/WOX_MCU/g' wox_mcu_cfg_hs.h
sed -i 's/WO0_APSRC/WOX_APSRC/g' wox_mcu_cfg_hs.h
sed -i 's/WO0_CPU/WOX_CPU/g' wox_mcu_cfg_hs.h

# remove some line
sed -i '/include\|pragma/d' wox_mcu_cfg_hs.h

# add some def and macro
sed -i '1 a #ifndef __WOX_MCU_CFG_HS_H__' wox_mcu_cfg_hs.h
sed -i '2 a #define __WOX_MCU_CFG_HS_H__' wox_mcu_cfg_hs.h
sed -i '8 a typedef unsigned int uint32_t;' wox_mcu_cfg_hs.h
sed -i '174 a #endif' wox_mcu_cfg_hs.h

################################################
# WO2_AP2WO_MCU_CCIF4_reg.h [module: WO0_WO2AP_MCU_CCIF4, sysc_reg link]
################################################
sed -i 's/WO2_AP2WO/WOX_AP2WO/g' wox_ap2wo_mcu_ccif4.h

# remove some line
sed -i '/include\|pragma/d' wox_ap2wo_mcu_ccif4.h

# add some def and macro
sed -i '1 a #ifndef __WOX_AP2WO_MCU_CCIF4_H__' wox_ap2wo_mcu_ccif4.h
sed -i '2 a #define __WOX_AP2WO_MCU_CCIF4_H__' wox_ap2wo_mcu_ccif4.h
sed -i '8 a typedef unsigned int uint32_t;' wox_ap2wo_mcu_ccif4.h
sed -i '498 a #endif' wox_ap2wo_mcu_ccif4.h
