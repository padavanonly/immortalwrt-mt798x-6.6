#!/bin/sh
# SPDX-License-Identifier: GPL-2.0

sdb_index=$1
test_mode=$2
mt7992_mod_option="/etc/modules.d/66-mt7992"
sdb_fw_offset=$(($sdb_index + 3))

in_wifi_module(){
	read -r mt7992_module < "$mt7992_mod_option"
	insmod mt_wifi_cmn
	insmod mt_wifi
	insmod mtk_hwifi
	insmod connac_if
	insmod mtk_pci
	insmod mtk_wed
	insmod ${mt7992_module}
}

rm_wifi_module(){
	rmmod mt7992
	rmmod mtk_wed
	rmmod mtk_pci
	rmmod connac_if
	rmmod mtk_hwifi
	rmmod mt_wifi
	rmmod mt_wifi_cmn
}

set_sdb_band_sel(){
	if grep -qF "sdb_band_sel" ${mt7992_mod_option}; then
		sed -i "s/sdb_band_sel=[0-9]/sdb_band_sel=$1/" ${mt7992_mod_option}
	else
		sed -i "$ s/$/ sdb_band_sel=$1/" ${mt7992_mod_option}
	fi
}

if [ "$sdb_index" -ge 1 ] && [ "$sdb_index" -le 3 ]; then
	wifi down
	rm_wifi_module

	if [ $test_mode == "1" ]; then
		sed -i "s/TestModeEn=[0-9]/TestModeEn=1/" /etc/wireless/mediatek/mt7992.1_${sdb_index}.dat
		#echo "Test 1 /etc/wireless/mediatek/mt7992.1_${sdb_index}.dat"
	else
		#echo "Test 0 /etc/wireless/mediatek/mt7992.1_${sdb_index}.dat"
		sed -i "s/TestModeEn=[0-9]/TestModeEn=0/" /etc/wireless/mediatek/mt7992.1_${sdb_index}.dat
	fi

	set_sdb_band_sel $sdb_index
	ln -sf /etc/wireless/l1profile_${sdb_index}.dat /etc/wireless/l1profile.dat
	ln -sf /lib/firmware/WIFI_MT7992_PATCH_MCU_${sdb_fw_offset}_1_hdr.bin /lib/firmware/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin
	ln -sf /lib/firmware/WIFI_RAM_CODE_MT7992_${sdb_fw_offset}_1.bin /lib/firmware/WIFI_RAM_CODE_MT7992_1_1.bin
	ln -sf /lib/firmware/WIFI_RAM_CODE_MT7992_${sdb_fw_offset}_1_TESTMODE.bin /lib/firmware/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin
	sync
	wifi detect
	in_wifi_module
	wifi up
fi
