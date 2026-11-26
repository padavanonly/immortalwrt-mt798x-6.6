/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define PATH_OF_MCU_BIN_IN "/bin/"
#define PATH_OF_MCU_BIN_OUT "/include/hw_ctrl/mcu_hdr/"
#define PATH_OF_EEPROM_IN "/bin/"
#define PATH_OF_EEPROM_OUT "/include/hw_ctrl/eeprom/"
#define PATH_OF_ROM_PATCH_IN "/bin/"
#define PATH_OF_ROM_PATCH_OUT "/include/hw_ctrl/mcu_hdr/"

static int snprintf_error(size_t size, int res)
{
	return res < 0 || (unsigned int) res >= size;
}

static size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;

		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

int bin2h_len(char *infpath, char *outfpath, char *infname, char *outfname,
	char *fw_name, const char *mode)
{
	char infname_temp[512], outfname_temp[512];
	int ret = 0;
	FILE *infile, *outfile;
	unsigned char c;
	int i = 0;
	int len = 0;
	int src_ret = 0;

	memset(infname_temp, 0, 512);
	memset(outfname_temp, 0, 512);
	strcpy(infname_temp, infpath);
	strcpy(outfname_temp, outfpath);
	strcat(infname_temp, infname);
	strcat(outfname_temp, outfname);

	infile = fopen(infname_temp, "r");

	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname_temp);
		return -1;
	}

	outfile = fopen(outfname_temp, mode);

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname_temp);
		fclose(infile);
		return -1;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "const char %s[] = {\n", fw_name);

	while (1) {
		char cc[3];
		int sRet = 0;

		c = getc(infile);

		if (feof(infile))
			break;

		memset(cc, 0, 2);

		if (i >= 16) {
			fputs("\n", outfile);
			i = 0;
		}

		if (i == 0)
			fputs("\t", outfile);
		else if (i < 16)
			fputs(" ", outfile);

		fputs("0x", outfile);
		sRet = snprintf(cc, sizeof(cc), "%02x", c);
		if (snprintf_error(sizeof(cc), sRet))
			printf("snprintf error!\n");
		fputs(cc, outfile);
		fputs(",", outfile);
		i++;
		len++;
	}

	fputs("\n};\n", outfile);

	fclose(infile);
	fclose(outfile);

	outfile = fopen(outfname_temp, "a");

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname_temp);
		return -1;
	}

	fputs("\n", outfile);

	fprintf(outfile, "#define %s_LEN %d\n", fw_name, len);

	return ret;
}



int bin2h(char *infname, char *outfname, char *fw_name, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile;
	unsigned char c;
	int i = 0;

	infile = fopen(infname, "r");

	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		return -1;
	}

	outfile = fopen(outfname, mode);

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);
		fclose(infile);
		return -1;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "const char %s[] = {\n", fw_name);

	while (1) {
		char cc[3];
		int sRet = 0;

		c = getc(infile);

		if (feof(infile))
			break;

		memset(cc, 0, 2);

		if (i >= 16) {
			fputs("\n", outfile);
			i = 0;
		}

		if (i == 0)
			fputs("\t", outfile);
		else if (i < 16)
			fputs(" ", outfile);

		fputs("0x", outfile);
		sRet = snprintf(cc, sizeof(cc), "%02x", c);
		if (snprintf_error(sizeof(cc), sRet))
			printf("snprintf error!\n");
		fputs(cc, outfile);
		fputs(",", outfile);
		i++;
	}

	fputs("\n};\n", outfile);
	fclose(infile);
	fclose(outfile);

	return ret;
}

int combin_header(char *outfname, char *fw_combin,
	char *fw_name1, char *fw_name2, char *fw_name3)
{
	int ret = 0;
	FILE *outfile;
	unsigned char c;
	int i = 0;


	outfile = fopen(outfname, "w");

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);
		return -1;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "const char *%s[3] = {%s, %s, %s};", fw_combin,
		fw_name1, fw_name2, fw_name3);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "const int %s_len[3] = {%s_LEN, %s_LEN, %s_LEN};", fw_combin,
		fw_name1, fw_name2, fw_name3);
	fputs("\n", outfile);
	fclose(outfile);

	return ret;
}

int main(int argc, char *argv[])
{
	char input_fw_path[512];
	char infname[512], ine2pname[512], in_rom_patch[512], in_rom_patch_e2[512], in_rom_patch_e3[512], in_emi_ram_ilm[512], in_emi_ram_dlm[512];
	char infname1[512], infname_e2[512], infname_e3[512], infname_testmode[512];
	char outfname[512], oute2pname[512], out_rom_patch[512], out_rom_patch_e2[512], out_rom_patch_e3[512], out_emi_ram[512];
	char outfname1[512], outfname_e2[512], outfname_e3[512], outfname_testmode[512];
	char chipsets[1024];
	char fw_name[128], e2p_name[128], rom_patch_name[128], rom_patch_name_e2[128], rom_patch_name_e3[128], emi_ram_ilm_name[128], emi_ram_dlm_name[128];
	char fw_name_e2[128], fw_name_e3[128], fw_name_testmode[128];
	char fw_name1[128];
	char *top_dir, *top_bin_dir;
	char *chipset, *token;
	char *wow, *wifi_mode;
	char *fpga, *rx_cut_through;
	int is_bin2h_fw = 0, is_bin2h_wa_fw = 0, is_bin2h_rom_patch = 0, is_bin2h_e2p = 0, is_bin2h_rom_patch_e2 = 0, is_bin2h_rom_patch_e3 = 0, is_b2h_emi_ram = 0;
	char ine2pname2[512], ine2pname3[512], e2p_name2[128], e2p_name3[128];
	char infname_idx[512], outfname_idx[512];
	char *adie;
	char *sku;
	char *project;
	char *fw_log_type;
	char *mt7990_2adie_triband;

	top_dir = (char *)getenv("TOP_DIR");
	if (!top_dir) {
		printf("Environment value \"TOP_DIR\" not export\n");
		return -1;
	}

	top_bin_dir = (char *)getenv("TOP_BIN_DIR");
	if (!top_bin_dir) {
		printf("Environment value \"TOP_BIN_DIR\" not export\n");
		return -1;
	}

	chipset = (char *)getenv("CHIPSET");
	if (!chipset) {
		printf("Environment value \"CHIPSET\" not export\n");
		return -1;
	}

	adie = (char *)getenv("ADIE");
	sku = (char *)getenv("SKU");
	project = (char *)getenv("PROJECT");
	fw_log_type = (char *)getenv("FW_LOG_TYPE");
	if (!fw_log_type)
		fw_log_type = NULL;
	wow = (char *)getenv("HAS_WOW_SUPPORT");
	wifi_mode = (char *)getenv("WIFI_MODE");
	if (!wifi_mode) {
		printf("Environment value \"WIFI_MODE\" not export\n");
		return -1;
	}

	printf("Build %s %s\n", chipset, wifi_mode);

	mt7990_2adie_triband = (char *)getenv("MT7990_2ADIE_TRIBAND");
	if (!mt7990_2adie_triband)
		printf("Environment value \"MT7990_2ADIE_TRIBAND\" not export, and use default\n");

	memset(chipsets, 0, sizeof(chipsets));
	memcpy(chipsets, chipset, strlen(chipset));
	chipsets[strlen(chipset)] = '\0';

	if (strlen(top_dir) > (sizeof(infname) - 100)) {
		printf("Environment value \"TOP_DIR\" is too long!\n");
		return -1;
	}

	if (strlen(top_bin_dir) > (sizeof(infname) - 100)) {
		printf("Environment value \"TOP_BIN_DIR\" is too long!\n");
		return -1;
	}

	chipsets[strlen(chipset)] = '\0';
	chipset = strtok(chipsets, "-, ");

	while (chipset != NULL) {
		printf("chipset = %s\n", chipset);
		memset(input_fw_path, 0, 512);
		memset(infname, 0, 512);
		memset(infname_e2, 0, 512);
		memset(infname_e3, 0, 512);
		memset(infname_testmode, 0, 512);
		memset(infname1, 0, 512);
		memset(ine2pname, 0, 512);
		memset(ine2pname2, 0, 512);
		memset(ine2pname3, 0, 512);
		memset(infname_idx, 0, 512);
		memset(outfname, 0, 512);
		memset(outfname_e2, 0, 512);
		memset(outfname_e3, 0, 512);
		memset(outfname_testmode, 0, 512);
		memset(outfname1, 0, 512);
		memset(oute2pname, 0, 512);
		memset(outfname_idx, 0, 512);
		memset(fw_name, 0, 128);
		memset(fw_name_e2, 0, 128);
		memset(fw_name_e3, 0, 128);
		memset(fw_name_testmode, 0, 128);
		memset(fw_name1, 0, 128);
		memset(e2p_name, 0, 128);
		memset(e2p_name2, 0, 128);
		memset(e2p_name3, 0, 128);
		memset(in_rom_patch, 0, 512);
		memset(in_rom_patch_e2, 0, 512);
		memset(in_rom_patch_e3, 0, 512);
		memset(out_rom_patch, 0, 512);
		memset(out_rom_patch_e2, 0, 512);
		memset(out_rom_patch_e3, 0, 512);
		memset(rom_patch_name, 0, 128);
		memset(rom_patch_name_e2, 0, 128);
		memset(rom_patch_name_e3, 0, 128);
		memset(in_emi_ram_ilm, 0, 512);
		memset(in_emi_ram_dlm, 0, 512);
		memset(out_emi_ram, 0, 512);
		memset(emi_ram_ilm_name, 0, 128);
		memset(emi_ram_dlm_name, 0, 128);

		strcat(input_fw_path, top_bin_dir);
		strcat(input_fw_path, PATH_OF_MCU_BIN_IN);
		strcat(input_fw_path, chipset);
		strcat(input_fw_path, "/");

		strcat(infname, top_bin_dir);
		strcat(infname_e2, top_bin_dir);
		strcat(infname_e3, top_bin_dir);
		strcat(infname_testmode, top_bin_dir);
		strcat(infname1, top_bin_dir);
		strcat(ine2pname, top_bin_dir);
		strcat(infname_idx, top_bin_dir);
		strcat(in_rom_patch, top_bin_dir);
		strcat(in_rom_patch_e2, top_bin_dir);
		strcat(in_rom_patch_e3, top_bin_dir);
		strcat(outfname, top_dir);
		strcat(outfname_e2, top_dir);
		strcat(outfname_e3, top_dir);
		strcat(outfname_testmode, top_dir);
		strcat(outfname1, top_dir);
		strcat(oute2pname, top_dir);
		strcat(outfname_idx, top_dir);
		strcat(out_rom_patch, top_dir);
		strcat(out_rom_patch_e2, top_dir);
		strcat(out_rom_patch_e3, top_dir);
		strcat(in_emi_ram_ilm, top_bin_dir);
		strcat(in_emi_ram_dlm, top_bin_dir);
		strcat(out_emi_ram, top_dir);

		is_bin2h_fw = 0;
		is_bin2h_wa_fw = 0;
		is_bin2h_rom_patch = 0;
		is_bin2h_e2p = 0;
		is_b2h_emi_ram = 0;
		strcat(infname, PATH_OF_MCU_BIN_IN);
		strcat(infname_e2, PATH_OF_MCU_BIN_IN);
		strcat(infname_e3, PATH_OF_MCU_BIN_IN);
		strcat(infname_testmode, PATH_OF_MCU_BIN_IN);
		strcat(outfname, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e2, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e3, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_testmode, PATH_OF_MCU_BIN_OUT);
		strcat(ine2pname, PATH_OF_EEPROM_IN);
		strcat(oute2pname, PATH_OF_EEPROM_OUT);
		strcat(in_rom_patch, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e2, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e3, PATH_OF_ROM_PATCH_IN);
		strcat(out_rom_patch, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e2, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e3, PATH_OF_ROM_PATCH_OUT);
		strcat(infname1, PATH_OF_MCU_BIN_IN);
		strcat(outfname1, PATH_OF_MCU_BIN_OUT);
		strcat(infname_idx, PATH_OF_MCU_BIN_IN);
		strcat(outfname_idx, PATH_OF_MCU_BIN_OUT);
		strcat(in_emi_ram_ilm, PATH_OF_MCU_BIN_IN);
		strcat(in_emi_ram_dlm, PATH_OF_MCU_BIN_IN);
		strcat(out_emi_ram, PATH_OF_MCU_BIN_OUT);

		/* For bin input, select chipset folder inside /bin */
		strcat(infname, chipset);
		strcat(infname_e2, chipset);
		strcat(infname_e3, chipset);
		strcat(infname_testmode, chipset);
		strcat(infname1, chipset);
		strcat(infname_idx, chipset);
		strcat(ine2pname, chipset);
		strcat(in_rom_patch, chipset);
		strcat(in_rom_patch_e2, chipset);
		strcat(in_rom_patch_e3, chipset);
		strcat(in_emi_ram_ilm, chipset);
		strcat(in_emi_ram_dlm, chipset);

		strcat(infname, "/");
		strcat(infname_e2, "/");
		strcat(infname_e3, "/");
		strcat(infname_testmode, "/");
		strcat(infname1, "/");
		strcat(infname_idx, "/");
		strcat(ine2pname, "/");
		strcat(in_rom_patch, "/");
		strcat(in_rom_patch_e2, "/");
		strcat(in_rom_patch_e3, "/");
		strcat(in_emi_ram_ilm, "/");
		strcat(in_emi_ram_dlm, "/");

		if (adie != NULL && (strncmp(adie, "mt7976", 7) == 0) &&
				(strncmp(chipset, "mt7915", 7) == 0)) {
			printf("MT7976 A-Die!\n");
			strcat(in_rom_patch, "rebb/mt7915_6e_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7915_6e_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7915_6e_rom_patch_e1");
/*
			strcat(in_rom_patch_e2, "rebb/mt7915_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7915_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7915_rom_patch_e2");
*/
			strcat(infname, "rebb/WIFI_RAM_CODE_7915_6e.bin");
			strcat(outfname, "mt7915_6e_firmware.h");
			strcat(fw_name, "MT7915_6e_FirmwareImage_E1");
/*
			strcat(infname_e2, "rebb/WIFI_RAM_CODE_7915_6e_E2.bin");
			strcat(outfname_e2, "mt7915_6e_firmware_e2.h");
			strcat(fw_name_e2, "MT7915_6e_FirmwareImage_E2");
*/
			strcat(infname1, "rebb/7915_6e_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7915_6e_WA_firmware.h");
			strcat(fw_name1, "MT7915_6e_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_6e_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7915_MT7976_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_6e_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7915", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7915_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7915_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7915_rom_patch_e1");

			strcat(in_rom_patch_e2, "rebb/mt7915_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7915_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7915_rom_patch_e2");

			strcat(infname, "rebb/WIFI_RAM_CODE_7915.bin");
			strcat(outfname, "mt7915_firmware.h");
			strcat(fw_name, "MT7915_FirmwareImage_E1");

			strcat(infname_e2, "rebb/WIFI_RAM_CODE_7915_E2.bin");
			strcat(outfname_e2, "mt7915_firmware_e2.h");
			strcat(fw_name_e2, "MT7915_FirmwareImage_E2");

			strcat(infname1, "rebb/7915_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7915_WA_firmware.h");
			strcat(fw_name1, "MT7915_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7915_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7915_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7915_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strlcpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strlcpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7915_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7915_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		}  else if (strncmp(chipset, "bellwether", 10) == 0) {
			strcat(in_rom_patch, "rebb/BELLWETHER_patch_e1_hdr.bin");
			strcat(out_rom_patch, "bellwether_rom_patch_e1.h");
			strcat(rom_patch_name, "bellwether_rom_patch_e1");

			strcat(infname, "rebb/WIFI_RAM_CODE_BELLWETHER.bin");
			strcat(outfname, "bellwether_firmware.h");
			strcat(fw_name, "bellwether_FirmwareImage_E1");

			strcat(infname1, "rebb/BELLWETHER_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "bellwether_WA_firmware.h");
			strcat(fw_name1, "bellwether_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "BELLWETHER_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/BELLWETHER_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "bellwether_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "BELLWETHER_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/BELLWETHER_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "bellwether_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "BELLWETHER_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/BELLWETHER_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "bellwether_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strlcpy(ine2pname2, ine2pname, 512);
			strlcpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "BELLWETHER_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/BELLWETHER_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "bellwether_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7990", 10) == 0) {
			if (mt7990_2adie_triband != NULL && strncmp(mt7990_2adie_triband, "y", 10) == 0) {
				strcat(in_rom_patch,
						"rebb/2adie_triband/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin");
				strcat(out_rom_patch, "WIFI_MT7990_PATCH_MCU_1_1_hdr_2adie.h");

				/* Normal wm bin */
				strcat(infname, "rebb/2adie_triband/WIFI_RAM_CODE_MT7990_1_1.bin");
				strcat(outfname, "WIFI_RAM_CODE_MT7990_1_1_2adie.h");

				/* Testmode wm bin */
				strcat(infname_testmode,
						"rebb/2adie_triband/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin");
				strcat(outfname_testmode,
						"WIFI_RAM_CODE_MT7990_1_1_TESTMODE_2adie.h");

				strcat(infname1,
						"rebb/2adie_triband/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin");
				strcat(outfname1, "WIFI_MT7990_WACPU_RAM_CODE_1_1_2adie.h");
			} else {
				strcat(in_rom_patch, "rebb/WIFI_MT7990_PATCH_MCU_1_1_hdr.bin");
				strcat(out_rom_patch, "WIFI_MT7990_PATCH_MCU_1_1_hdr_.h");

				/* Normal wm bin */
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7990_1_1.bin");
				strcat(outfname, "WIFI_RAM_CODE_MT7990_1_1_.h");

				/* Testmode wm bin */
				strcat(infname_testmode,
						"rebb/WIFI_RAM_CODE_MT7990_1_1_TESTMODE.bin");
				strcat(outfname_testmode, "WIFI_RAM_CODE_MT7990_1_1_TESTMODE_.h");

				strcat(infname1, "rebb/WIFI_MT7990_WACPU_RAM_CODE_1_1.bin");
				strcat(outfname1, "WIFI_MT7990_WACPU_RAM_CODE_1_1_.h");
			}
			strcat(rom_patch_name, "mt7990_rom_patch_1_1");
			strcat(fw_name, "mt7990_FirmwareImage_1_1");
			strcat(fw_name_testmode, "mt7990_FirmwareImage_Testmode_1_1");
			strcat(fw_name1, "mt7990_WA_FirmwareImage_1_1");

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			/* default bin */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7990_E2PImage");
			strcat(ine2pname, "rebb/MT7990_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7990_e2p.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7990_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7990_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7990_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7990_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7990_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7990_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7990_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7990_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7990_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strlcpy(ine2pname2, ine2pname, 512);
			strlcpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7990_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7990_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7990_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7992", 10) == 0) {
			printf("chipset: %s, sku: %s\n", chipset, sku);

			/* rom patch bin */
			if (strncmp(sku, "BE5040", 6) == 0) {
				strcat(in_rom_patch, "rebb/WIFI_MT7992_PATCH_MCU_2_1_hdr.bin");
				strcat(out_rom_patch, "WIFI_MT7992_PATCH_MCU_2_1_hdr_.h");
			} else if (strncmp(sku, "BE6500", 6) == 0) {
				strcat(in_rom_patch, "rebb/WIFI_MT7992_PATCH_MCU_3_1_hdr.bin");
				strcat(out_rom_patch, "WIFI_MT7992_PATCH_MCU_3_1_hdr_.h");
			} else if (strncmp(sku, "BE3600SDB", 9) == 0) {
				if (project && strncmp(project, "CCN34", 5) == 0) {
					/* Generate 2G5G rom patch  */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/ccn34/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_4_1_hdr_ccn34.h",
						"mt7992_rom_patch_4_1", "w");
					/* Generate 2G6G rom patch  */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/ccn34/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_5_1_hdr_ccn34.h",
						"mt7992_rom_patch_5_1", "w");
					/* Generate 5G6G rom patch */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/ccn34/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_6_1_hdr_ccn34.h",
						"mt7992_rom_patch_6_1", "w");
					/* Combin rom patch */
					strcat(out_rom_patch,
						"WIFI_MT7992_PATCH_MCU_1_1_hdr_sdb_ccn34.h");
					combin_header(out_rom_patch, "mt7992_rom_patch_1_1",
						"mt7992_rom_patch_4_1", "mt7992_rom_patch_5_1",
						"mt7992_rom_patch_6_1");
				} else {
					/* Generate 2G5G rom patch  */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/WIFI_MT7992_PATCH_MCU_4_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_4_1_hdr_.h",
						"mt7992_rom_patch_4_1", "w");
					/* Generate 2G6G rom patch  */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/WIFI_MT7992_PATCH_MCU_5_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_5_1_hdr_.h",
						"mt7992_rom_patch_5_1", "w");
					/* Generate 5G6G rom patch */
					bin2h_len(in_rom_patch, out_rom_patch,
						"rebb/WIFI_MT7992_PATCH_MCU_6_1_hdr.bin",
						"WIFI_MT7992_PATCH_MCU_6_1_hdr_.h",
						"mt7992_rom_patch_6_1", "w");
					/* Combin rom patch */
					strcat(out_rom_patch,
						"WIFI_MT7992_PATCH_MCU_1_1_hdr_sdb.h");
					combin_header(out_rom_patch, "mt7992_rom_patch_1_1",
						"mt7992_rom_patch_4_1", "mt7992_rom_patch_5_1",
						"mt7992_rom_patch_6_1");
				}
			} else {
				strcat(in_rom_patch, "rebb/WIFI_MT7992_PATCH_MCU_1_1_hdr.bin");
				strcat(out_rom_patch, "WIFI_MT7992_PATCH_MCU_1_1_hdr_.h");
			}
			printf("rom_patch: %s\n", in_rom_patch);
			strcat(rom_patch_name, "mt7992_rom_patch_1_1");

			/* Normal wm bin and idx log wm bin */
			if (strncmp(sku, "BE5040", 6) == 0) {
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7992_2_1.bin");
				strcat(outfname, "WIFI_RAM_CODE_MT7992_2_1_.h");
				strcat(infname_idx, "rebb/WIFI_RAM_CODE_MT7992_idx_2_1.bin");
				strcat(outfname_idx, "WIFI_RAM_CODE_MT7992_idx_2_1_.h");
			} else if (strncmp(sku, "BE6500", 6) == 0) {
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7992_3_1.bin");
				strcat(outfname, "WIFI_RAM_CODE_MT7992_3_1_.h");
				strcat(infname_idx, "rebb/WIFI_RAM_CODE_MT7992_idx_3_1.bin");
				strcat(outfname_idx, "WIFI_RAM_CODE_MT7992_idx_3_1_.h");
			} else if (strncmp(sku, "BE3600SDB", 9) == 0) {
				if (project && strncmp(project, "CCN34", 5) == 0) {
					/* Generate 2G5G normal mode wm for ccn34 */
					bin2h_len(infname, outfname,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_4_1.bin",
						"WIFI_RAM_CODE_MT7992_4_1_ccn34.h",
						"mt7992_FirmwareImage_4_1", "w");
					/* Generate 2G6G normal mode wm for ccn34 */
					bin2h_len(infname, outfname,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_5_1.bin",
						"WIFI_RAM_CODE_MT7992_5_1_ccn34.h",
						"mt7992_FirmwareImage_5_1", "w");
					/* Generate 5G6G normal mode wm for ccn34 */
					bin2h_len(infname, outfname,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_6_1.bin",
						"WIFI_RAM_CODE_MT7992_6_1_ccn34.h",
						"mt7992_FirmwareImage_6_1", "w");

					//combin normal mode WM for ccn34
					strcat(outfname, "WIFI_RAM_CODE_MT7992_1_1_sdb_ccn34.h");
					combin_header(outfname, "mt7992_FirmwareImage_1_1",
						"mt7992_FirmwareImage_4_1",
						"mt7992_FirmwareImage_5_1",
						"mt7992_FirmwareImage_6_1");

					/* Generate 2G5G normal mode idx wm for ccn34 */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_idx_4_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_4_1_ccn34.h",
						"mt7992_FirmwareImage_4_1", "w");
					/* Generate 2G6G normal mode idx wm for ccn34 */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_idx_5_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_5_1_ccn34.h",
						"mt7992_FirmwareImage_5_1", "w");
					/* Generate 5G6G normal mode idx wm for ccn34 */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/ccn34/WIFI_RAM_CODE_MT7992_idx_6_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_6_1_ccn34.h",
						"mt7992_FirmwareImage_6_1", "w");

					//combin normal mode idx WM for ccn34
					strcat(outfname_idx,
						"WIFI_RAM_CODE_MT7992_idx_1_1_sdb_ccn34.h");
					combin_header(outfname_idx, "mt7992_FirmwareImage_1_1",
						"mt7992_FirmwareImage_4_1",
						"mt7992_FirmwareImage_5_1",
						"mt7992_FirmwareImage_6_1");
				} else {
					/* Generate 2G5G normal mode wm */
					bin2h_len(infname, outfname,
						"rebb/WIFI_RAM_CODE_MT7992_4_1.bin",
						"WIFI_RAM_CODE_MT7992_4_1_.h",
						"mt7992_FirmwareImage_4_1", "w");
					/* Generate 2G6G normal mode wm  */
					bin2h_len(infname, outfname,
						"rebb/WIFI_RAM_CODE_MT7992_5_1.bin",
						"WIFI_RAM_CODE_MT7992_5_1_.h",
						"mt7992_FirmwareImage_5_1", "w");
					/* Generate 5G6G normal mode wm */
					bin2h_len(infname, outfname,
						"rebb/WIFI_RAM_CODE_MT7992_6_1.bin",
						"WIFI_RAM_CODE_MT7992_6_1_.h",
						"mt7992_FirmwareImage_6_1", "w");

					//combin normal mode WM
					strcat(outfname, "WIFI_RAM_CODE_MT7992_1_1_sdb.h");
					combin_header(outfname, "mt7992_FirmwareImage_1_1",
						"mt7992_FirmwareImage_4_1",
						"mt7992_FirmwareImage_5_1",
						"mt7992_FirmwareImage_6_1");

					/* Generate 2G5G normal mode idx wm */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/WIFI_RAM_CODE_MT7992_idx_4_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_4_1_.h",
						"mt7992_FirmwareImage_4_1", "w");
					/* Generate 2G6G normal mode idx wm  */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/WIFI_RAM_CODE_MT7992_idx_5_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_5_1_.h",
						"mt7992_FirmwareImage_5_1", "w");
					/* Generate 5G6G normal mode idx wm */
					bin2h_len(infname_idx, outfname_idx,
						"rebb/WIFI_RAM_CODE_MT7992_idx_6_1.bin",
						"WIFI_RAM_CODE_MT7992_idx_6_1_.h",
						"mt7992_FirmwareImage_6_1", "w");

					//combin normal mode idx WM
					strcat(outfname_idx, "WIFI_RAM_CODE_MT7992_idx_1_1_sdb.h");
					combin_header(outfname_idx, "mt7992_FirmwareImage_1_1",
						"mt7992_FirmwareImage_4_1",
						"mt7992_FirmwareImage_5_1",
						"mt7992_FirmwareImage_6_1");
				}
			} else {
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7992_1_1.bin");
				strcat(outfname, "WIFI_RAM_CODE_MT7992_1_1_.h");
				strcat(infname_idx, "rebb/WIFI_RAM_CODE_MT7992_idx_1_1.bin");
				strcat(outfname_idx, "WIFI_RAM_CODE_MT7992_idx_1_1_.h");
			}
			printf("WM: %s\n", infname);
			printf("WM (idx_log): %s\n", infname_idx);
			strcat(fw_name, "mt7992_FirmwareImage_1_1");

			/* Testmode wm bin */
			if (strncmp(sku, "BE5040", 6) == 0) {
				strcat(infname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_2_1_TESTMODE.bin");
				strcat(outfname_testmode,
					"WIFI_RAM_CODE_MT7992_2_1_TESTMODE_.h");
			} else if (strncmp(sku, "BE6500", 6) == 0) {
				strcat(infname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_3_1_TESTMODE.bin");
				strcat(outfname_testmode,
					"WIFI_RAM_CODE_MT7992_3_1_TESTMODE_.h");
			} else if (strncmp(sku, "BE3600SDB", 9) == 0) {
				/* Generate 2G5G test mode wm */
				bin2h_len(infname_testmode, outfname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_4_1_TESTMODE.bin",
					"WIFI_RAM_CODE_MT7992_4_1_TESTMODE_.h",
					"mt7992_FirmwareImage_Testmode_4_1", "w");
				/* Generate 2G6G test mode wm  */
				bin2h_len(infname_testmode, outfname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_5_1_TESTMODE.bin",
					"WIFI_RAM_CODE_MT7992_5_1_TESTMODE_.h",
					"mt7992_FirmwareImage_Testmode_5_1", "w");
				/* Generate 5G6G test mode wm */
				bin2h_len(infname_testmode, outfname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_6_1_TESTMODE.bin",
					"WIFI_RAM_CODE_MT7992_6_1_TESTMODE_.h",
					"mt7992_FirmwareImage_Testmode_6_1", "w");
				//combin test mode WM
				strcat(outfname_testmode,
					"WIFI_RAM_CODE_MT7992_1_1_TESTMODE_sdb.h");
				combin_header(outfname_testmode,
					"mt7992_FirmwareImage_Testmode_1_1",
					"mt7992_FirmwareImage_Testmode_4_1",
					"mt7992_FirmwareImage_Testmode_5_1",
					"mt7992_FirmwareImage_Testmode_6_1");
			} else {
				strcat(infname_testmode,
					"rebb/WIFI_RAM_CODE_MT7992_1_1_TESTMODE.bin");
				strcat(outfname_testmode,
					"WIFI_RAM_CODE_MT7992_1_1_TESTMODE_.h");
			}
			printf("Testmode: %s\n", infname_testmode);
			strcat(fw_name_testmode, "mt7992_FirmwareImage_Testmode_1_1");

			/* wa bin */
			if (strncmp(sku, "BE5040", 6) == 0) {
				strcat(infname1, "rebb/WIFI_MT7992_WACPU_RAM_CODE_2_1.bin");
				strcat(outfname1, "WIFI_MT7992_WACPU_RAM_CODE_2_1_.h");
			} else if (strncmp(sku, "BE6500", 6) == 0) {
				strcat(infname1, "rebb/WIFI_MT7992_WACPU_RAM_CODE_3_1.bin");
				strcat(outfname1, "WIFI_MT7992_WACPU_RAM_CODE_3_1_.h");
			} else if (strncmp(sku, "BE3600SDB", 9) == 0) {
				if (project && strncmp(project, "CCN34", 5) == 0) {
					/* Generate 2G5G WA */
					bin2h_len(infname1, outfname1,
						"rebb/ccn34/WIFI_MT7992_WACPU_RAM_CODE_4_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_4_1_ccn34.h",
						"mt7992_WA_FirmwareImage_4_1", "w");
					/* Generate 2G6G WA  */
					bin2h_len(infname1, outfname1,
						"rebb/ccn34/WIFI_MT7992_WACPU_RAM_CODE_5_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_5_1_ccn34.h",
						"mt7992_WA_FirmwareImage_5_1", "w");
					/* Generate 5G6G WA */
					bin2h_len(infname1, outfname1,
						"rebb/ccn34/WIFI_MT7992_WACPU_RAM_CODE_6_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_6_1_ccn34.h",
						"mt7992_WA_FirmwareImage_6_1", "w");
					//combin WA
					strcat(outfname1,
						"WIFI_MT7992_WACPU_RAM_CODE_1_1_sdb_ccn34.h");
					combin_header(outfname1, "mt7992_WA_FirmwareImage_1_1",
						"mt7992_WA_FirmwareImage_4_1",
						"mt7992_WA_FirmwareImage_5_1",
						"mt7992_WA_FirmwareImage_6_1");
				} else {
					/* Generate 2G5G WA */
					bin2h_len(infname1, outfname1,
						"rebb/WIFI_MT7992_WACPU_RAM_CODE_4_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_4_1_.h",
						"mt7992_WA_FirmwareImage_4_1", "w");
					/* Generate 2G6G WA  */
					bin2h_len(infname1, outfname1,
						"rebb/WIFI_MT7992_WACPU_RAM_CODE_5_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_5_1_.h",
						"mt7992_WA_FirmwareImage_5_1", "w");
					/* Generate 5G6G WA */
					bin2h_len(infname1, outfname1,
						"rebb/WIFI_MT7992_WACPU_RAM_CODE_6_1.bin",
						"WIFI_MT7992_WACPU_RAM_CODE_6_1_.h",
						"mt7992_WA_FirmwareImage_6_1", "w");
					//combin WA
					strcat(outfname1, "WIFI_MT7992_WACPU_RAM_CODE_1_1_sdb.h");
					combin_header(outfname1, "mt7992_WA_FirmwareImage_1_1",
						"mt7992_WA_FirmwareImage_4_1",
						"mt7992_WA_FirmwareImage_5_1",
						"mt7992_WA_FirmwareImage_6_1");
				}
			} else {
				strcat(infname1, "rebb/WIFI_MT7992_WACPU_RAM_CODE_1_1.bin");
				strcat(outfname1, "WIFI_MT7992_WACPU_RAM_CODE_1_1_.h");
			}
			printf("WA: %s\n", infname1);
			strcat(fw_name1, "mt7992_WA_FirmwareImage_1_1");

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			if (strncmp(sku, "BE3600SDB", 9) == 0) {
				//disable bin2h generate
				is_bin2h_fw = 0;
				is_bin2h_wa_fw = 0;
				is_bin2h_rom_patch = 0;
				is_bin2h_rom_patch_e2 = 0;
			}

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7992_E2PImage_iPAiLNA");
			if (strncmp(sku, "BE5040", 6) == 0)
				strcat(ine2pname, "rebb/MT7991_MT7976_EEPROM_BE5040_ePAeLNA.bin");
			else if (strncmp(sku, "BE6500", 6) == 0)
				strcat(ine2pname,
					"rebb/MT7992_MT7978_MT7979_EEPROM_BE6500_iPAiLNA.bin");
			else if (strncmp(sku, "BE3600SDB", 9) == 0)
				strcat(ine2pname,
					"rebb/MT7992_SDB_EEPROM.bin");
			else
				strcat(ine2pname,
					"rebb/MT7992_MT7976_MT7977_EEPROM_BE7200_ePAeLNA_midFEM.bin");
			printf("default_e2p: %s\n", ine2pname);
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7992_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7992_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7992_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7992_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7992_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7992_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7992_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strlcpy(ine2pname2, ine2pname, 512);
			strlcpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7992_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7992_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7992_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7993", 10) == 0) {
			/* rom patch bin */
			strcat(in_rom_patch, "rebb/WIFI_MT7993_PATCH_MCU_1_1_hdr.bin");
			strcat(out_rom_patch, "WIFI_MT7993_PATCH_MCU_1_1_hdr.h");
			strcat(rom_patch_name, "mt7993_rom_patch_1_1");
			printf("rom_patch: %s\n", in_rom_patch);

			/* Normal wm bin */
			strcat(infname, "rebb/WIFI_RAM_CODE_MT7993_1_1.bin");
			strcat(outfname, "WIFI_RAM_CODE_MT7993_1_1.h");
			strcat(fw_name, "mt7993_FirmwareImage_1_1");
			printf("WM: %s\n", infname);

			/* Testmode wm bin */
			strcat(infname_testmode,
				"rebb/WIFI_RAM_CODE_MT7993_1_1_TESTMODE.bin");
			strcat(outfname_testmode,
				"WIFI_RAM_CODE_MT7993_1_1_TESTMODE.h");
			strcat(fw_name_testmode, "mt7993_FirmwareImage_Testmode_1_1");
			printf("Testmode: %s\n", infname_testmode);

			is_bin2h_fw = 1;
			is_bin2h_wa_fw = 0;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7993_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7993_MT7976_EEPROM_BE3600_iPAiLNA.bin");
			printf("default_e2p: %s\n", ine2pname);
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7993_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7993_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7993_MT7976_EEPROM_BE3600_iPAeLNA.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7993_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7993_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7993_MT7976_EEPROM_BE3600_ePAeLNA.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7993_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, top_bin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strlcpy(ine2pname2, ine2pname, 512);
			strlcpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7993_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7993_MT7976_EEPROM_BE3600_ePAiLNA.bin");
			strcat(oute2pname, top_dir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7993_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */ 
		} else
			printf("unknown chipset = %s\n", chipset);

		if (is_bin2h_fw) {
			if (strncmp(infname, input_fw_path, strlen(infname)) != 0)
				bin2h(infname, outfname, fw_name, "w");/* N9 E1 */

			if (strncmp(infname_e2, input_fw_path, strlen(infname_e2)) != 0)
				bin2h(infname_e2, outfname_e2, fw_name_e2, "w");/* N9 E2 */

			if (strncmp(infname_e3, input_fw_path, strlen(infname_e3)) != 0)
				bin2h(infname_e3, outfname_e3, fw_name_e3, "w");/* N9 E3 */

			/* kite idx log WM */
			if (strncmp(chipset, "mt7992", 10) == 0 &&
				strncmp(infname, input_fw_path, strlen(infname)) != 0) {
				bin2h(infname_idx, outfname_idx, fw_name, "w");
			}

			/* N9 Testmode E1 */
			if (strncmp(infname_testmode, input_fw_path, strlen(infname_testmode)) != 0)
				bin2h(infname_testmode, outfname_testmode, fw_name_testmode, "w");

			if (is_bin2h_wa_fw &&
				(strncmp(infname1, input_fw_path, strlen(infname1)) != 0))
				bin2h(infname1, outfname1, fw_name1, "w");/* CR4 */
		}

		if (is_bin2h_rom_patch)
			if (strncmp(in_rom_patch, input_fw_path, strlen(in_rom_patch)) != 0)
				bin2h(in_rom_patch, out_rom_patch, rom_patch_name, "w");

		if (is_bin2h_rom_patch_e2)
			if (strncmp(in_rom_patch_e2, input_fw_path, strlen(in_rom_patch_e2)) != 0)
				bin2h(in_rom_patch_e2, out_rom_patch_e2, rom_patch_name_e2, "w");

		if (is_bin2h_rom_patch_e3)
			if (strncmp(in_rom_patch_e3, input_fw_path, strlen(in_rom_patch_e3)) != 0)
				bin2h(in_rom_patch_e3, out_rom_patch_e3, rom_patch_name_e3, "w");

		if (is_b2h_emi_ram) {
			bin2h(in_emi_ram_ilm, out_emi_ram, emi_ram_ilm_name, "w");
			bin2h(in_emi_ram_dlm, out_emi_ram, emi_ram_dlm_name, "a");
		}

		if (is_bin2h_e2p) {
			bin2h(ine2pname, oute2pname, e2p_name, "w");

			if (e2p_name2[0])
				bin2h(ine2pname2, oute2pname, e2p_name2, "a");

			if (e2p_name3[0])
				bin2h(ine2pname3, oute2pname, e2p_name3, "a");
		}

		chipset = strtok(NULL, " ");
	}

	exit(0);
}
