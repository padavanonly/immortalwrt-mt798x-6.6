/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PATH_OF_MCU_BIN_IN "./bin/"
#define PATH_OF_MCU_BIN_OUT "./mcu/"
#define STRCAT(out, in) strncat(out, in, strlen(in))

int bin2h(char *infname, char *outfname, char *fw_name, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile;
	unsigned char c;
	int i = 0;

	infile = fopen(infname, "r");

	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		ret = -1;
		goto err_out;
	}

	outfile = fopen(outfname, mode);

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);
		fclose(infile);
		ret = -1;
		goto err_out;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "u8 %s[] = {\n", fw_name);

	while (1) {
		char cc[3];

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
		sprintf(cc, "%02x", c);
		fputs(cc, outfile);
		fputs(",", outfile);
		i++;
	}

	fputs("\n};\n", outfile);
	fclose(infile);
	fclose(outfile);

err_out:
	return ret;
}

int main(int argc, char *argv[])
{
	char input_fw_path[512];
	char infname1[512], infname2[512], infname3[512];
	char outfname1[512], outfname2[512], outfname3[512];
	char chipsets[1024];
	char fw_name1[128], fw_name2[128], fw_name3[128];
	char *chipset;
	int is_bin2h_fw = 0;
	char *input_fw_dir = (char *)getenv("RT28xx_DIR");

	chipset = (char *)getenv("CHIPSET");

	if (!chipset) {
		printf("Environment value \"CHIPSET\" not export\n");
		return -1;
	}

	memset(chipsets, 0, sizeof(chipsets));
	memcpy(chipsets, chipset, strlen(chipset));
	chipsets[strlen(chipset)] = '\0';
	chipset = strtok(chipsets, "-, ");

	while (chipset != NULL) {
		printf("warp_bin2h: chipset = %s\n", chipset);
		memset(input_fw_path, 0, 512);
		memset(infname1, 0, 512);;
		memset(outfname1, 0, 512);
		memset(fw_name1, 0, 128);

		memset(infname2, 0, 512);;
		memset(outfname2, 0, 512);
		memset(fw_name2, 0, 128);

		memset(infname3, 0, 512);;
		memset(outfname3, 0, 512);
		memset(fw_name3, 0, 128);

		is_bin2h_fw = 0;
		if (input_fw_dir)
			STRCAT(infname1, input_fw_dir);
		else
			STRCAT(infname1, PATH_OF_MCU_BIN_IN);

		STRCAT(outfname1, PATH_OF_MCU_BIN_OUT);

		if (strncmp(chipset, "colgin", 7) == 0) {
			/* For bin input, select chipset folder inside /bin */
			STRCAT(infname1, chipset);
			STRCAT(infname1, "/");

			STRCAT(infname2, infname1);
			STRCAT(outfname2, outfname1);
			STRCAT(fw_name2, fw_name1);

			STRCAT(infname1, "colgin_WOCPU0_RAM_CODE_release.bin");
			STRCAT(outfname1, "WO0_firmware.h");
			STRCAT(fw_name1, "WO0_FirmwareImage");

			STRCAT(infname2, "colgin_WOCPU1_RAM_CODE_release.bin");
			STRCAT(outfname2, "WO1_firmware.h");
			STRCAT(fw_name2, "WO1_FirmwareImage");

			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "mt7986", 7) == 0) {
			STRCAT(infname2, infname1);
			STRCAT(outfname2, outfname1);
			STRCAT(infname1, "7986_WOCPU0_RAM_CODE_release.bin");
			STRCAT(outfname1, "WO0_firmware.h");
			STRCAT(fw_name1, "WO0_FirmwareImage");

			STRCAT(infname2, "7986_WOCPU1_RAM_CODE_release.bin");
			STRCAT(outfname2, "WO1_firmware.h");
			STRCAT(fw_name2, "WO1_FirmwareImage");

			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "mt7981", 7) == 0) {
                        STRCAT(infname2, infname1);
                        STRCAT(outfname2, outfname1);
                        STRCAT(infname1, "7981_WOCPU0_RAM_CODE_release.bin");
                        STRCAT(outfname1, "WO0_firmware.h");
                        STRCAT(fw_name1, "WO0_FirmwareImage");

			STRCAT(infname2, "7981_WOCPU1_RAM_CODE_release.bin");
                        STRCAT(outfname2, "WO1_firmware.h");
                        STRCAT(fw_name2, "WO1_FirmwareImage");

			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "mt6990", 10) == 0) {
			/* For bin input, select chipset folder inside /bin */
			STRCAT(infname1, chipset);
			STRCAT(infname1, "/");

			printf("1: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);

			STRCAT(infname2, infname1);
			STRCAT(outfname2, outfname1);
			STRCAT(fw_name2, fw_name1);
			STRCAT(infname3, infname1);
			STRCAT(outfname3, outfname1);
			STRCAT(fw_name3, fw_name1);

			printf("2: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);
			printf("2: outfname1=%s, outfname2=%s, outfname3=%s\n", outfname1, outfname2, outfname3);

			STRCAT(infname1, "bollinger_WOCPU0_RAM_CODE_release.bin");
			STRCAT(outfname1, "WO0_firmware.h");
			STRCAT(fw_name1, "WO0_FirmwareImage");

			STRCAT(infname2, "bollinger_WOCPU1_RAM_CODE_release.bin");
			STRCAT(outfname2, "WO1_firmware.h");
			STRCAT(fw_name2, "WO1_FirmwareImage");

			STRCAT(infname3, "bollinger_WOCPU2_RAM_CODE_release.bin");
			STRCAT(outfname3, "WO2_firmware.h");
			STRCAT(fw_name3, "WO2_FirmwareImage");

			printf("3: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);
			printf("3: outfname1=%s, outfname2=%s, outfname3=%s\n", outfname1, outfname2, outfname3);

			is_bin2h_fw = 1;
		}else if (strncmp(chipset, "mt7988", 10) == 0) {
			/* For bin input, select chipset folder inside /bin */
			STRCAT(infname1, chipset);
			STRCAT(infname1, "/");

			printf("1: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);

			STRCAT(infname2, infname1);
			STRCAT(outfname2, outfname1);
			STRCAT(fw_name2, fw_name1);
			STRCAT(infname3, infname1);
			STRCAT(outfname3, outfname1);
			STRCAT(fw_name3, fw_name1);

			printf("2: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);
			printf("2: outfname1=%s, outfname2=%s, outfname3=%s\n", outfname1, outfname2, outfname3);

			STRCAT(infname1, "7988_WOCPU0_RAM_CODE_release.bin");
			STRCAT(outfname1, "WO0_firmware.h");
			STRCAT(fw_name1, "WO0_FirmwareImage");

			STRCAT(infname2, "7988_WOCPU1_RAM_CODE_release.bin");
			STRCAT(outfname2, "WO1_firmware.h");
			STRCAT(fw_name2, "WO1_FirmwareImage");

			STRCAT(infname3, "7988_WOCPU2_RAM_CODE_release.bin");
			STRCAT(outfname3, "WO2_firmware.h");
			STRCAT(fw_name3, "WO2_FirmwareImage");

			printf("3: infname1=%s, infname2=%s, infname3=%s\n", infname1, infname2, infname3);
			printf("3: outfname1=%s, outfname2=%s, outfname3=%s\n", outfname1, outfname2, outfname3);

			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "mt6988", 10) == 0) {
			/* For bin input, select chipset folder inside /bin */
			STRCAT(infname1, chipset);
			STRCAT(infname1, "/");

			STRCAT(infname1, "t930_WOCPU0_RAM_CODE_release.bin");
			STRCAT(outfname1, "WO0_firmware.h");
			STRCAT(fw_name1, "WO0_FirmwareImage");

			is_bin2h_fw = 1;
		} else
			printf("unknown chipset = %s\n", chipset);

		if (is_bin2h_fw) {
			if (strncmp(infname1, input_fw_path, strlen(infname1)) != 0)
				bin2h(infname1, outfname1, fw_name1, "w");
			if (strncmp(infname2, input_fw_path, strlen(infname2)) != 0)
				bin2h(infname2, outfname2, fw_name2, "w");
			if (strncmp(infname3, input_fw_path, strlen(infname3)) != 0)
				bin2h(infname3, outfname3, fw_name3, "w");
		}

		chipset = strtok(NULL, " ");
	}

	exit(0);
}
