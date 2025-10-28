/*

*/
#include "rt_config.h"
#include "mgmt/be_internal.h"
#include "hdev_ctrl.h"
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif

CH_FREQ_MAP CH_HZ_ID_MAP[] = {
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
	{14, 2484},

	/*  UNII */
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
	{50, 5250},
	{52, 5260},
	{54, 5270},
	{56, 5280},
	{58, 5290},
	{60, 5300},
	{62, 5310},
	{64, 5320},
	{149, 5745},
	{151, 5755},
	{153, 5765},
	{155, 5775},
	{157, 5785},
	{159, 5795},
	{161, 5805},
	{165, 5825},
	{167, 5835},
	{169, 5845},
	{171, 5855},
	{173, 5865},
	{175, 5875},
	{177, 5885},

	/* HiperLAN2 */
	{100, 5500},
	{102, 5510},
	{104, 5520},
	{106, 5530},
	{108, 5540},
	{110, 5550},
	{112, 5560},
	{114, 5570},
	{116, 5580},
	{118, 5590},
	{120, 5600},
	{122, 5610},
	{124, 5620},
	{126, 5630},
	{128, 5640},
	{132, 5660},
	{134, 5670},
	{136, 5680},
	{138, 5690},
	{140, 5700},
	{142, 5710},
	{144, 5720},

	/* Japan MMAC */
	{34, 5170},
	{38, 5190},
	{42, 5210},
	{46, 5230},

	/*  Japan */
	{184, 4920},
	{188, 4940},
	{192, 4960},
	{196, 4980},

	{208, 5040},	/* Japan, means J08 */
	{212, 5060},	/* Japan, means J12 */
	{216, 5080},	/* Japan, means J16 */
};

INT	CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP) / sizeof(CH_FREQ_MAP));

CH_DESC Country_Region0_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_2GHZ[] = {
	{1, 13, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_2GHZ[] = {
	{10, 2, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_2GHZ[] = {
	{10, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_2GHZ[] = {
	{14, 1, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_2GHZ[] = {
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_2GHZ[] = {
	{3, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_2GHZ[] = {
	{5, 9, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region31_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 3, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region32_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 2, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region33_ChDesc_2GHZ[] = {
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[] = {
	{REGION_0_BG_BAND, Country_Region0_ChDesc_2GHZ},
	{REGION_1_BG_BAND, Country_Region1_ChDesc_2GHZ},
	{REGION_2_BG_BAND, Country_Region2_ChDesc_2GHZ},
	{REGION_3_BG_BAND, Country_Region3_ChDesc_2GHZ},
	{REGION_4_BG_BAND, Country_Region4_ChDesc_2GHZ},
	{REGION_5_BG_BAND, Country_Region5_ChDesc_2GHZ},
	{REGION_6_BG_BAND, Country_Region6_ChDesc_2GHZ},
	{REGION_7_BG_BAND, Country_Region7_ChDesc_2GHZ},
	{REGION_31_BG_BAND, Country_Region31_ChDesc_2GHZ},
	{REGION_32_BG_BAND, Country_Region32_ChDesc_2GHZ},
	{REGION_33_BG_BAND, Country_Region33_ChDesc_2GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_2GHZ = sizeof(Country_Region_ChDesc_2GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_5GHZ[] = {
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
CH_DESC Country_Region5_ChDesc_5GHZ[] = {
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	/* {100, 14, CHANNEL_DEFAULT_PROP}, */
	/* {149, 7, CHANNEL_DEFAULT_PROP}, */
	{}
};

CH_DESC Country_Region8_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region9_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region10_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region11_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 6, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

/* for FCC capable of using 144 , mapping of Country_Region1 */
CH_DESC Country_Region12_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{}
};
/* for FCC capable of using 144 , mapping of Country_Region7 */
CH_DESC Country_Region13_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
/* for FCC capable of using 144 , mapping of Country_Region9 */
CH_DESC Country_Region14_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region15_ChDesc_5GHZ[] = {
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region16_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region17_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region18_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region19_ChDesc_5GHZ[] = {
	{56, 3, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region20_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 7, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region21_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region22_ChDesc_5GHZ[] = {
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

/* for JP capable of using 144 , mapping of Country_Region18 */
CH_DESC Country_Region23_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 4, CHANNEL_DEFAULT_PROP},
	{}
};

/* for JP capable of using 144 , mapping of Country_Region22 */
CH_DESC Country_Region24_ChDesc_5GHZ[] = {
	{100, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region25_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 8, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region26_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{149, 8, CHANNEL_DEFAULT_PROP},
	{}
};

/* for US 5g high band , mapping of Country_Region27 */
CH_DESC Country_Region27_ChDesc_5GHZ[] = {
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};


COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[] = {
	{REGION_0_A_BAND, Country_Region0_ChDesc_5GHZ},
	{REGION_1_A_BAND, Country_Region1_ChDesc_5GHZ},
	{REGION_2_A_BAND, Country_Region2_ChDesc_5GHZ},
	{REGION_3_A_BAND, Country_Region3_ChDesc_5GHZ},
	{REGION_4_A_BAND, Country_Region4_ChDesc_5GHZ},
	{REGION_5_A_BAND, Country_Region5_ChDesc_5GHZ},
	{REGION_6_A_BAND, Country_Region6_ChDesc_5GHZ},
	{REGION_7_A_BAND, Country_Region7_ChDesc_5GHZ},
	{REGION_8_A_BAND, Country_Region8_ChDesc_5GHZ},
	{REGION_9_A_BAND, Country_Region9_ChDesc_5GHZ},
	{REGION_10_A_BAND, Country_Region10_ChDesc_5GHZ},
	{REGION_11_A_BAND, Country_Region11_ChDesc_5GHZ},
	{REGION_12_A_BAND, Country_Region12_ChDesc_5GHZ},
	{REGION_13_A_BAND, Country_Region13_ChDesc_5GHZ},
	{REGION_14_A_BAND, Country_Region14_ChDesc_5GHZ},
	{REGION_15_A_BAND, Country_Region15_ChDesc_5GHZ},
	{REGION_16_A_BAND, Country_Region16_ChDesc_5GHZ},
	{REGION_17_A_BAND, Country_Region17_ChDesc_5GHZ},
	{REGION_18_A_BAND, Country_Region18_ChDesc_5GHZ},
	{REGION_19_A_BAND, Country_Region19_ChDesc_5GHZ},
	{REGION_20_A_BAND, Country_Region20_ChDesc_5GHZ},
	{REGION_21_A_BAND, Country_Region21_ChDesc_5GHZ},
	{REGION_22_A_BAND, Country_Region22_ChDesc_5GHZ},
	{REGION_23_A_BAND, Country_Region23_ChDesc_5GHZ},
	{REGION_24_A_BAND, Country_Region24_ChDesc_5GHZ},
	{REGION_25_A_BAND, Country_Region25_ChDesc_5GHZ},
	{REGION_26_A_BAND, Country_Region26_ChDesc_5GHZ},
	{REGION_27_A_BAND, Country_Region27_ChDesc_5GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_5GHZ = sizeof(Country_Region_ChDesc_5GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_6GHZ[] = {
	{1, 24, CHANNEL_DEFAULT_PROP},
	{97, 5, CHANNEL_DEFAULT_PROP},
	{117, 18, CHANNEL_DEFAULT_PROP},
	{189, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_6GHZ[] = {
	{1, 24, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_6GHZ[] = {
	{97, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_6GHZ[] = {
	{117, 18, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_6GHZ[] = {
	{189, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_6GHZ[] = {
	{1, 24, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_6GHZ[] = {
	{1, 24, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_6GHZ[] = {
	{1, 24, CHANNEL_DEFAULT_PROP},
	{97, 5, CHANNEL_DEFAULT_PROP},
	{}
};


COUNTRY_REGION_CH_DESC Country_Region_ChDesc_6GHZ[] = {
	{REGION_0_A_BAND_6GHZ, Country_Region0_ChDesc_6GHZ},
	{REGION_1_A_BAND_6GHZ, Country_Region1_ChDesc_6GHZ},
	{REGION_2_A_BAND_6GHZ, Country_Region2_ChDesc_6GHZ},
	{REGION_3_A_BAND_6GHZ, Country_Region3_ChDesc_6GHZ},
	{REGION_4_A_BAND_6GHZ, Country_Region4_ChDesc_6GHZ},
	{REGION_5_A_BAND_6GHZ, Country_Region5_ChDesc_6GHZ},
	{REGION_6_A_BAND_6GHZ, Country_Region6_ChDesc_6GHZ},
	{REGION_7_A_BAND_6GHZ, Country_Region7_ChDesc_6GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_6GHZ = sizeof(Country_Region_ChDesc_6GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_GROUP_DESC Channel_GRP[] = {
	{36, 4},
	{52, 4},
	{100, 12},
	{149, 5},
	{}
};

UCHAR const Channel_GRP_Num = sizeof(Channel_GRP) / sizeof(CH_GROUP_DESC);

UINT16 TotalChNum(PCH_DESC pChDesc)
{
	UINT16 TotalChNum = 0;

	while (pChDesc->FirstChannel) {
		TotalChNum += pChDesc->NumOfCh;
		pChDesc++;
	}

	return TotalChNum;
}

UCHAR GetChannel_5GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index * 4;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannel_2GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannelFlag(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->ChannelProp;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}


/*Albania*/
CH_DESP Country_AL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Algeria*/
CH_DESP Country_DZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Argentina*/
CH_DESP Country_AR_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Armenia*/
CH_DESP Country_AM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Aruba*/
CH_DESP Country_AW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Australia*/
CH_DESP Country_AU_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Austria*/
CH_DESP Country_AT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Azerbaijan*/
CH_DESP Country_AZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bahamas*/
CH_DESP Country_BS_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 17, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bahrain*/
CH_DESP Country_BH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 33, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bangladesh*/
CH_DESP Country_BD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 33, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Barbados*/
CH_DESP Country_BB_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 28, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belarus*/
CH_DESP Country_BY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belgium*/
CH_DESP Country_BE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belize*/
CH_DESP Country_BZ_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Benin*/
CH_DESP Country_BJ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bhutan*/
CH_DESP Country_BT_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bolivia*/
CH_DESP Country_BO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bosnia and Herzegovina*/
CH_DESP Country_BA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Botswana*/
CH_DESP Country_BW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Brazil*/
CH_DESP Country_BR_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Brunei Darussalam*/
CH_DESP Country_BN_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bulgaria*/
CH_DESP Country_BG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Burkina Faso*/
CH_DESP Country_BF_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Burundi*/
CH_DESP Country_BI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cambodia*/
CH_DESP Country_KH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cameroon*/
CH_DESP Country_CM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Canada*/
CH_DESP Country_CA_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 24, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Central African Republic*/
CH_DESP Country_CF_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Chad*/
CH_DESP Country_TD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Chile*/
CH_DESP Country_CL_ChDesp[] = {
	{ 1,   13, 21, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 21, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 21, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 21, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*China*/
CH_DESP Country_CN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 33, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Colombia*/
CH_DESP Country_CO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Comoros*/
CH_DESP Country_KM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Costa Rica*/
CH_DESP Country_CR_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Croatia*/
CH_DESP Country_HR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cyprus*/
CH_DESP Country_CY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Czech Republic*/
CH_DESP Country_CZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Denmark*/
CH_DESP Country_DK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Dominica*/
CH_DESP Country_DM_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Dominican Republic*/
CH_DESP Country_DO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ecuador*/
CH_DESP Country_EC_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Egypt*/
CH_DESP Country_EG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*El Salvador*/
CH_DESP Country_SV_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Equatorial Guinea*/
CH_DESP Country_GQ_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Eritrea*/
CH_DESP Country_ER_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Estonia*/
CH_DESP Country_EE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ethiopia*/
CH_DESP Country_ET_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Fiji*/
CH_DESP Country_FJ_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Finland*/
CH_DESP Country_FI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Faroe Islands (Denmark)*/
CH_DESP Country_FO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*France*/
CH_DESP Country_FR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Gabon*/
CH_DESP Country_GA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Gambia*/
CH_DESP Country_GM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Georgia*/
CH_DESP Country_GE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Germany*/
CH_DESP Country_DE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ghana*/
CH_DESP Country_GH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Greece*/
CH_DESP Country_GR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Greenland*/
CH_DESP Country_GL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 20 */
	{ 0},			/* end*/
};
/*Grenada*/
CH_DESP Country_GD_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guam*/
CH_DESP Country_GU_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guatemala*/
CH_DESP Country_GT_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guyana*/
CH_DESP Country_GY_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Haiti*/
CH_DESP Country_HT_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2462MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,	5, 23, BOTH, TRUE}, 	/*5490~5600MHz, Ch 100~116, Max BW: 40 */
	{ 132,	4, 23, BOTH, TRUE}, 	/*5650~5710MHz, Ch 132~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Honduras*/
CH_DESP Country_HN_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Hong Kong*/
CH_DESP Country_HK_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Hungary*/
CH_DESP Country_HU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,  Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Iceland*/
CH_DESP Country_IS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*India*/
CH_DESP Country_IN_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 144,  1, 23, BOTH, TRUE},	/*5710~5730MHz, Ch 144~144, Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Indonesia*/
CH_DESP Country_ID_ChDesp[] = {
	{ 1,   13, 26, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Iran, Islamic Republic of*/
CH_DESP Country_IR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ireland*/
CH_DESP Country_IE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Israel*/
CH_DESP Country_IL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Italy*/
CH_DESP Country_IT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Jamaica*/
CH_DESP Country_JM_ChDesp[] = {
	{ 1,   11, 20, BOTH, FALSE},	/*2402~2462MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  5, 28, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Japan*/
CH_DESP Country_JP_ChDesp[] = {
	{ 1,    14, 23, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 20 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 0},			/* end*/
};
/*Jordan*/
CH_DESP Country_JO_ChDesp[] = {
	{ 1,  13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,  4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kazakhstan*/
CH_DESP Country_KZ_ChDesp[] = {
	{ 1,  13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},			/* end*/
};
/*Kenya*/
CH_DESP Country_KE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Korea, Democratic People's Republic of*/
CH_DESP Country_KP_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, TRUE},	/*5160~5250MHz, Ch 36~48, Max BW: 40 */
	{ 36,   8, 20, BOTH, FALSE},	/*5170~5330MHz, Ch 36~64, Max BW: 40 */
	{ 100,  7, 30, BOTH, TRUE},	/*5490~5630MHz, Ch 100~124, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Korea, Republic of*/
CH_DESP Country_KR_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 20 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kuwait*/
CH_DESP Country_KW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kyrgyzstan*/
CH_DESP Country_KG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Latvia*/
CH_DESP Country_LV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lebanon*/
CH_DESP Country_LB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lesotho*/
CH_DESP Country_LS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Liberia*/
CH_DESP Country_LR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Libya*/
CH_DESP Country_LY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Liechtenstein*/
CH_DESP Country_LI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lithuania*/
CH_DESP Country_LT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Luxembourg*/
CH_DESP Country_LU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Macao*/
CH_DESP Country_MO_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Macedonia, Republic of*/
CH_DESP Country_MK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Madagascar*/
CH_DESP Country_MG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malawi*/
CH_DESP Country_MW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malaysia*/
CH_DESP Country_MY_ChDesp[] = {
	{ 1,   13, 26, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 30, BOTH, TRUE},	/*5490~5650MHz, Ch 100~128, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Maldives*/
CH_DESP Country_MV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 20, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mali*/
CH_DESP Country_ML_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malta*/
CH_DESP Country_MT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Marshall Islands*/
CH_DESP Country_MH_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mauritania*/
CH_DESP Country_MR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mauritius*/
CH_DESP Country_MU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mexico*/
CH_DESP Country_MX_ChDesp[] = {
	{ 1,   11, 33, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  5, 23, BOTH, TRUE},	/*5490~5590MHz, Ch 100~116, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Monaco*/
CH_DESP Country_MC_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Montenegro*/
CH_DESP Country_ME_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Morocco*/
CH_DESP Country_MA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mozambique*/
CH_DESP Country_MZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nauru*/
CH_DESP Country_NR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nepal*/
CH_DESP Country_NP_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Netherlands*/
CH_DESP Country_NL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Netherlands Antilles*/
CH_DESP Country_AN_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*New Zealand*/
CH_DESP Country_NZ_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nicaragua*/
CH_DESP Country_NI_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Niger*/
CH_DESP Country_NE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nigeria*/
CH_DESP Country_NG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Norway*/
CH_DESP Country_NO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Oman*/
CH_DESP Country_OM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 33, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Pakistan*/
CH_DESP Country_PK_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};

/*Palau*/
CH_DESP Country_PW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Panama*/
CH_DESP Country_PA_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Papua New Guinea*/
CH_DESP Country_PG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Paraguay*/
CH_DESP Country_PY_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Peru*/
CH_DESP Country_PE_ChDesp[] = {
	{ 1,   11, 26, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 20, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Philippines*/
CH_DESP Country_PH_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Poland*/
CH_DESP Country_PL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Portuga*/
CH_DESP Country_PT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Puerto Rico*/
CH_DESP Country_PR_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Qatar*/
CH_DESP Country_QA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Republic of the Congo*/
CH_DESP Country_CG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Romania*/
CH_DESP Country_RO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Russian Federation*/
CH_DESP Country_RU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 132,  4, 30, BOTH, TRUE},	/*5650~5710MHz, Ch 132~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},			/* end*/
};
/*Rwanda*/
CH_DESP Country_RW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Barth'elemy*/
CH_DESP Country_BL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Kitts and Nevis*/
CH_DESP Country_KN_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Lucia*/
CH_DESP Country_LC_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Vincent and the Grenadines*/
CH_DESP Country_VC_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Samoa*/
CH_DESP Country_WS_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*San Marino*/
CH_DESP Country_SM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saudi Arabia*/
CH_DESP Country_SA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 13, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Senegal*/
CH_DESP Country_SN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Serbia*/
CH_DESP Country_RS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Seychelles*/
CH_DESP Country_SC_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sierra Leone*/
CH_DESP Country_SL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Singapore*/
CH_DESP Country_SG_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Slovakia*/
CH_DESP Country_SK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Slovenia*/
CH_DESP Country_SI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Solomon Islands*/
CH_DESP Country_SB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Somalia*/
CH_DESP Country_SO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*South Africa*/
CH_DESP Country_ZA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 13, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*South Sudan*/
CH_DESP Country_SS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Spain*/
CH_DESP Country_ES_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sri Lanka*/
CH_DESP Country_LK_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sudan*/
CH_DESP Country_SD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Swaziland*/
CH_DESP Country_SZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sweden*/
CH_DESP Country_SE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Switzerland*/
CH_DESP Country_CH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Syrian Arab Republic*/
CH_DESP Country_SY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Taiwan*/
CH_DESP Country_TW_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 24, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 24, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tajikistan*/
CH_DESP Country_TJ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Thailand*/
CH_DESP Country_TH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Togo*/
CH_DESP Country_TG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tonga*/
CH_DESP Country_TO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Trinidad and Tobago*/
CH_DESP Country_TT_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tunisia*/
CH_DESP Country_TN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Turkey*/
CH_DESP Country_TR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Turkmenistan*/
CH_DESP Country_TM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tuvalu*/
CH_DESP Country_TV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uganda*/
CH_DESP Country_UG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ukraine*/
CH_DESP Country_UA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*United Arab Emirates*/
CH_DESP Country_AE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*United Kingdom*/
CH_DESP Country_GB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*United States*/
CH_DESP Country_US_ChDesp[] = {
	{ 1,   11, 36, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 36, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  8, 30, BOTH, FALSE},	/*5735~5885MHz, Ch 149~177, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uruguay*/
CH_DESP Country_UY_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uzbekistan*/
CH_DESP Country_UZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Vanuatu*/
CH_DESP Country_VU_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Vatican City*/
CH_DESP Country_VA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Venezuela*/
CH_DESP Country_VE_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Viet Nam*/
CH_DESP Country_VN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Western Sahara*/
CH_DESP Country_EH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Yemen*/
CH_DESP Country_YE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Zambia*/
CH_DESP Country_ZM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Zimbabwe*/
CH_DESP Country_ZW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};

/* Group Region */
/*Europe*/
CH_DESP Country_EU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13 */
	{ 36,   4, 17, BOTH, FALSE},	/* 5G band 1, ch 36~48*/
	{ 0},			/* end*/
};
/*North America*/
CH_DESP Country_NA_ChDesp[] = {
	{ 1,   11,	20, BOTH, FALSE},	/* 2.4 G, ch 1~11*/
	{ 36,   4,	16, IDOR, FALSE},	/* 5G band 1, ch 36~48*/
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165*/
	{ 0},			/* end*/
};
/*World Wide*/
CH_DESP Country_WO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13*/
	{ 36,   4, 30, BOTH, FALSE},	/* 5G band 1, ch 36~48 */
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165 */
	{ 0},			/* end*/
};

CH_REGION ChRegion[] = {
	{"AL", CE, TRUE, Country_AL_ChDesp}, /* Albania */
	{"DZ", CE, TRUE, Country_DZ_ChDesp}, /* Algeria */
	{"AR", CE, TRUE, Country_AR_ChDesp}, /* Argentina */
	{"AM", CE, TRUE, Country_AM_ChDesp}, /* Armenia */
	{"AW", CE, TRUE, Country_AW_ChDesp}, /* Aruba */
	{"AU", CE, FALSE, Country_AU_ChDesp}, /* Australia */
	{"AT", CE, TRUE, Country_AT_ChDesp}, /* Austria */
	{"AZ", CE, TRUE, Country_AZ_ChDesp}, /* Azerbaijan */
	{"BS", CE, TRUE, Country_BS_ChDesp}, /* Bahamas */
	{"BH", CE, TRUE, Country_BH_ChDesp}, /* Bahrain */
	{"BD", CE, TRUE, Country_BD_ChDesp}, /* Bangladesh */
	{"BB", CE, TRUE, Country_BB_ChDesp}, /* Barbados */
	{"BY", CE, TRUE, Country_BY_ChDesp}, /* Belarus */
	{"BE", CE, TRUE, Country_BE_ChDesp}, /* Belgium */
	{"BZ", CE, TRUE, Country_BZ_ChDesp}, /* Belize */
	{"BJ", CE, TRUE, Country_BJ_ChDesp}, /* Benin */
	{"BT", CE, TRUE, Country_BT_ChDesp}, /* Bhutan */
	{"BO", CE, TRUE, Country_BO_ChDesp}, /* Bolivia */
	{"BA", CE, TRUE, Country_BA_ChDesp}, /* Bosnia and Herzegovina */
	{"BW", CE, TRUE, Country_BW_ChDesp}, /* Botswana */
	{"BR", CE, FALSE, Country_BR_ChDesp}, /* Brazil */
	{"BN", CE, TRUE, Country_BN_ChDesp}, /* Brunei Darussalam */
	{"BG", CE, TRUE, Country_BG_ChDesp}, /* Bulgaria */
	{"BF", CE, TRUE, Country_BF_ChDesp}, /* Burkina Faso */
	{"BI", CE, TRUE, Country_BI_ChDesp}, /* Burundi */
	{"KH", CE, TRUE, Country_KH_ChDesp}, /* Cambodia */
	{"CM", CE, TRUE, Country_CM_ChDesp}, /* Cameroon */
	{"CA", FCC, FALSE, Country_CA_ChDesp}, /* Canada */
	{"CF", FCC, FALSE, Country_CF_ChDesp}, /* Central African Republic */
	{"TD", FCC, FALSE, Country_TD_ChDesp}, /* Chad */
	{"CL", CE, TRUE, Country_CL_ChDesp}, /* Chile */
	{"CN", CHN, FALSE, Country_CN_ChDesp}, /* China */
	{"CO", CE, TRUE, Country_CO_ChDesp}, /* Colombia */
	{"KM", CE, TRUE, Country_KM_ChDesp}, /* Comoros */
	{"CR", CE, TRUE, Country_CR_ChDesp}, /* Costa Rica */
	{"HR", CE, TRUE, Country_HR_ChDesp}, /* Croatia */
	{"CY", CE, TRUE, Country_CY_ChDesp}, /* Cyprus */
	{"CZ", CE, TRUE, Country_CZ_ChDesp}, /* Czech Republic */
	{"DK", CE, TRUE, Country_DK_ChDesp}, /* Denmark */
	{"DM", CE, TRUE, Country_DM_ChDesp}, /* Dominica */
	{"DO", CE, TRUE, Country_DO_ChDesp}, /* Dominican Republic */
	{"EC", CE, TRUE, Country_EC_ChDesp}, /* Ecuador */
	{"EG", CE, TRUE, Country_EG_ChDesp}, /* Egypt */
	{"SV", CE, TRUE, Country_SV_ChDesp}, /* El Salvador */
	{"GQ", CE, TRUE, Country_GQ_ChDesp}, /* Equatorial Guinea */
	{"ER", CE, TRUE, Country_ER_ChDesp}, /* Eritrea */
	{"EE", CE, TRUE, Country_EE_ChDesp}, /* Estonia */
	{"ET", CE, TRUE, Country_ET_ChDesp}, /* Ethiopia */
	{"FJ", CE, TRUE, Country_FJ_ChDesp}, /* Fiji */
	{"FI", CE, TRUE, Country_FI_ChDesp}, /* Finland */
	{"FO", CE, TRUE, Country_FO_ChDesp}, /* Faroe Islands (Denmark) */
	{"FR", CE, TRUE, Country_FR_ChDesp}, /* France */
	{"GA", CE, TRUE, Country_GA_ChDesp}, /* Gabon */
	{"GM", CE, TRUE, Country_GM_ChDesp}, /* Gambia */
	{"GE", CE, TRUE, Country_GE_ChDesp}, /* Georgia */
	{"DE", CE, TRUE, Country_DE_ChDesp}, /* Germany */
	{"GH", CE, TRUE, Country_GH_ChDesp}, /* Ghana */
	{"GR", CE, TRUE, Country_GR_ChDesp}, /* Greece */
	{"GL", CE, TRUE, Country_GL_ChDesp}, /* Greenland */
	{"GD", CE, TRUE, Country_GD_ChDesp}, /* Grenada */
	{"GU", CE, TRUE, Country_GU_ChDesp}, /* Guam */
	{"GT", CE, TRUE, Country_GT_ChDesp}, /* Guatemala */
	{"GY", CE, TRUE, Country_GY_ChDesp}, /* Guyana */
	{"HT", CE, TRUE, Country_HT_ChDesp}, /* Haiti */
	{"HN", CE, TRUE, Country_HN_ChDesp}, /* Honduras */
	{"HK", CE, TRUE, Country_HK_ChDesp}, /* Hong Kong */
	{"HU", CE, TRUE, Country_HU_ChDesp}, /* Hungary */
	{"IS", CE, TRUE, Country_IS_ChDesp}, /* Iceland */
	{"IN", CE, TRUE, Country_IN_ChDesp}, /* India */
	{"ID", CE, TRUE, Country_ID_ChDesp}, /* Indonesia */
	{"IR", CE, TRUE, Country_IR_ChDesp}, /* Iran, Islamic Republic of */
	{"IE", CE, TRUE, Country_IE_ChDesp}, /* Ireland */
	{"IL", CE, FALSE, Country_IL_ChDesp}, /* Israel */
	{"IT", CE, TRUE, Country_IT_ChDesp}, /* Italy */
	{"JM", CE, TRUE, Country_JM_ChDesp}, /* Jamaica */
	{"JP", JAP, TRUE, Country_JP_ChDesp}, /* Japan */
	{"JO", CE, TRUE, Country_JO_ChDesp}, /* Jordan */
	{"KZ", CE, TRUE, Country_KZ_ChDesp}, /* Kazakhstan */
	{"KE", CE, TRUE, Country_KE_ChDesp}, /* Kenya */
	{"KP", CE, TRUE, Country_KP_ChDesp}, /* Korea, Democratic People's Republic of */
	{"KR", CE, FALSE, Country_KR_ChDesp}, /* Korea, Republic of */
	{"KW", CE, TRUE, Country_KW_ChDesp}, /* Kuwait */
	{"KG", CE, TRUE, Country_KG_ChDesp}, /* Kyrgyzstan */
	{"LV", CE, TRUE, Country_LV_ChDesp}, /* Latvia */
	{"LB", CE, TRUE, Country_LB_ChDesp}, /* Lebanon */
	{"LS", CE, TRUE, Country_LS_ChDesp}, /* Lesotho */
	{"LR", CE, TRUE, Country_LR_ChDesp}, /* Liberia */
	{"LY", CE, TRUE, Country_LY_ChDesp}, /* Libya */
	{"LI", CE, TRUE, Country_LI_ChDesp}, /* Liechtenstein */
	{"LT", CE, TRUE, Country_LT_ChDesp}, /* Lithuania */
	{"LU", CE, TRUE, Country_LU_ChDesp}, /* Luxembourg */
	{"MO", CE, TRUE, Country_MO_ChDesp}, /* Macao */
	{"MK", CE, TRUE, Country_MK_ChDesp}, /* Macedonia, Republic of */
	{"MG", CE, TRUE, Country_MG_ChDesp}, /* Madagascar */
	{"MW", CE, TRUE, Country_MW_ChDesp}, /* Malawi */
	{"MY", CE, TRUE, Country_MY_ChDesp}, /* Malaysia */
	{"MV", CE, TRUE, Country_MV_ChDesp}, /* Maldives */
	{"ML", CE, TRUE, Country_ML_ChDesp}, /* Mali */
	{"MT", CE, TRUE, Country_MT_ChDesp}, /* Malta */
	{"MH", CE, TRUE, Country_MH_ChDesp}, /* Marshall Islands */
	{"MR", CE, TRUE, Country_MR_ChDesp}, /* Mauritania */
	{"MU", CE, TRUE, Country_MU_ChDesp}, /* Mauritius */
	{"MX", CE, FALSE, Country_MX_ChDesp}, /* Mexico */
	{"MC", CE, TRUE, Country_MC_ChDesp}, /* Monaco */
	{"ME", CE, TRUE, Country_ME_ChDesp}, /* Montenegro */
	{"MA", CE, TRUE, Country_MA_ChDesp}, /* Morocco */
	{"MZ", CE, TRUE, Country_MZ_ChDesp}, /* Mozambique */
	{"NR", CE, TRUE, Country_NR_ChDesp}, /* Nauru */
	{"NP", CE, TRUE, Country_NP_ChDesp}, /* Nepal */
	{"NL", CE, TRUE, Country_NL_ChDesp}, /* Netherlands */
	{"AN", CE, TRUE, Country_AN_ChDesp}, /* Netherlands Antilles */
	{"NZ", CE, TRUE, Country_NZ_ChDesp}, /* New Zealand */
	{"NI", CE, TRUE, Country_NI_ChDesp}, /* Nicaragua */
	{"NE", CE, TRUE, Country_NE_ChDesp}, /* Niger */
	{"NG", CE, TRUE, Country_NG_ChDesp}, /* Nigeria */
	{"NO", CE, TRUE, Country_NO_ChDesp}, /* Norway */
	{"OM", CE, TRUE, Country_OM_ChDesp}, /* Oman */
	{"PK", CE, TRUE, Country_PK_ChDesp}, /* Pakistan */
	{"PW", CE, TRUE, Country_PW_ChDesp}, /* Palau */
	{"PA", CE, TRUE, Country_PA_ChDesp}, /* Panama */
	{"PG", CE, TRUE, Country_PG_ChDesp}, /* Papua New Guinea */
	{"PY", CE, TRUE, Country_PY_ChDesp}, /* Paraguay */
	{"PE", CE, TRUE, Country_PE_ChDesp}, /* Peru */
	{"PH", CE, TRUE, Country_PH_ChDesp}, /* Philippines */
	{"PL", CE, TRUE, Country_PL_ChDesp}, /* Poland */
	{"PT", CE, TRUE, Country_PT_ChDesp}, /* Portuga l*/
	{"PR", CE, TRUE, Country_PR_ChDesp}, /* Puerto Rico */
	{"QA", CE, TRUE, Country_QA_ChDesp}, /* Qatar */
	{"CG", CE, TRUE, Country_CG_ChDesp}, /* Republic of the Congo */
	{"RO", CE, TRUE, Country_RO_ChDesp}, /* Romania */
	{"RU", CE, FALSE, Country_RU_ChDesp}, /* Russian Federation */
	{"RW", CE, TRUE, Country_RW_ChDesp}, /* Rwanda */
	{"BL", CE, TRUE, Country_BL_ChDesp}, /* Saint Barth'elemy */
	{"KN", CE, TRUE, Country_KN_ChDesp}, /* Saint Kitts and Nevis */
	{"LC", CE, TRUE, Country_LC_ChDesp}, /* Saint Lucia */
	{"VC", CE, TRUE, Country_VC_ChDesp}, /* Saint Vincent and the Grenadines */
	{"WS", CE, TRUE, Country_WS_ChDesp}, /* Samoa */
	{"SM", CE, TRUE, Country_SM_ChDesp}, /* San Marino */
	{"SA", CE, TRUE, Country_SA_ChDesp}, /* Saudi Arabia */
	{"SN", CE, TRUE, Country_SN_ChDesp}, /* Senegal */
	{"RS", CE, TRUE, Country_RS_ChDesp}, /* Serbia */
	{"SC", CE, TRUE, Country_SC_ChDesp}, /* Seychelles */
	{"SL", CE, TRUE, Country_SL_ChDesp}, /* Sierra Leone */
	{"SG", CE, TRUE, Country_SG_ChDesp}, /* Singapore */
	{"SK", CE, TRUE, Country_SK_ChDesp}, /* Slovakia */
	{"SI", CE, TRUE, Country_SI_ChDesp}, /* Slovenia */
	{"SB", CE, TRUE, Country_SB_ChDesp}, /* Solomon Islands */
	{"SO", CE, TRUE, Country_SO_ChDesp}, /* Somalia */
	{"ZA", CE, FALSE, Country_ZA_ChDesp}, /* South Africa */
	{"SS", CE, TRUE, Country_SS_ChDesp}, /* South Sudan */
	{"ES", CE, TRUE, Country_ES_ChDesp}, /* Spain */
	{"LK", CE, TRUE, Country_LK_ChDesp}, /* Sri Lanka */
	{"SD", CE, TRUE, Country_SD_ChDesp}, /* Sudan */
	{"SZ", CE, TRUE, Country_SZ_ChDesp}, /* Swaziland */
	{"SE", CE, TRUE, Country_SE_ChDesp}, /* Sweden */
	{"CH", CE, TRUE, Country_CH_ChDesp}, /* Switzerland */
	{"SY", CE, TRUE, Country_SY_ChDesp}, /* Syrian Arab Republic */
	{"TW", FCC, FALSE, Country_TW_ChDesp}, /* Taiwan */
	{"TJ", CE, TRUE, Country_TJ_ChDesp}, /* Tajikistan */
	{"TH", CE, FALSE, Country_TH_ChDesp}, /* Thailand */
	{"TG", CE, TRUE, Country_TG_ChDesp}, /* Togo */
	{"TO", CE, TRUE, Country_TO_ChDesp}, /* Tonga */
	{"TT", CE, TRUE, Country_TT_ChDesp}, /* Trinidad and Tobago */
	{"TN", CE, TRUE, Country_TN_ChDesp}, /* Tunisia */
	{"TR", CE, TRUE, Country_TR_ChDesp}, /* Turkey */
	{"TM", CE, TRUE, Country_TM_ChDesp}, /* Turkmenistan */
	{"TV", CE, TRUE, Country_TV_ChDesp}, /* Tuvalu */
	{"UG", CE, TRUE, Country_UG_ChDesp}, /* Uganda */
	{"UA", CE, TRUE, Country_UA_ChDesp}, /* Ukraine */
	{"AE", CE, TRUE, Country_AE_ChDesp}, /* United Arab Emirates */
	{"GB", CE, TRUE, Country_GB_ChDesp}, /* United Kingdom */
	{"US", FCC, FALSE, Country_US_ChDesp}, /* United States */
	{"UY", CE, TRUE, Country_UY_ChDesp}, /* Uruguay */
	{"UZ", CE, TRUE, Country_UZ_ChDesp}, /* Uzbekistan */
	{"VU", CE, TRUE, Country_VU_ChDesp}, /* Vanuatu */
	{"VA", CE, TRUE, Country_VA_ChDesp}, /* Vatican City */
	{"VE", CE, TRUE, Country_VE_ChDesp}, /* Venezuela */
	{"VN", CE, TRUE, Country_VN_ChDesp}, /* Viet Nam */
	{"EH", CE, TRUE, Country_EH_ChDesp}, /* Western Sahara */
	{"YE", CE, TRUE, Country_YE_ChDesp}, /* Yemen */
	{"ZM", CE, TRUE, Country_ZM_ChDesp}, /* Zambia */
	{"ZW", CE, TRUE, Country_ZW_ChDesp}, /* Zimbabwe */
	{"EU", CE, TRUE, Country_EU_ChDesp}, /* Europe */
	{"NA", FCC, FALSE, Country_NA_ChDesp}, /* North America */
	{"WO", CE, FALSE, Country_WO_ChDesp}, /* World Wide */
	{"", 0, FALSE, NULL}, /* End */
};

COUNTRY_CODE_TO_COUNTRY_REGION allCountry[] = {
	/* {Country Number, ISO Name, Country Name, Support 11A, 11A Country Region,
	6G Country Region, Support 11G, 11G Country Region} */
	{0,		"DB",	"Debug",				TRUE,	REGION_7_A_BAND,	A_BAND_6G_REGION_0,	TRUE,	G_BAND_REGION_5},
	{8,		"AL",	"ALBANIA",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1,	TRUE,	G_BAND_REGION_1},
	{12,	"DZ",	"ALGERIA",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1,	TRUE,	G_BAND_REGION_1},
	{32,	"AR",	"ARGENTINA",			TRUE,	REGION_3_A_BAND,	A_BAND_6G_REGION_1,	TRUE,	G_BAND_REGION_1},
	{51,	"AM",	"ARMENIA",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{36,	"AU",	"AUSTRALIA",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{40,	"AT",	"AUSTRIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{31,	"AZ",	"AZERBAIJAN",			TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{48,	"BH",	"BAHRAIN",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{112,	"BY",	"BELARUS",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{56,	"BE",	"BELGIUM",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{84,	"BZ",	"BELIZE",				TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{68,	"BO",	"BOLIVIA",				TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{76,	"BR",	"BRAZIL",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{96,	"BN",	"BRUNEI DARUSSALAM",	TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{100,	"BG",	"BULGARIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{124,	"CA",	"CANADA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{152,	"CL",	"CHILE",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{156,	"CN",	"CHINA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{170,	"CO",	"COLOMBIA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{188,	"CR",	"COSTA RICA",			FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{191,	"HR",	"CROATIA",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{196,	"CY",	"CYPRUS",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{203,	"CZ",	"CZECH REPUBLIC",		TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{208,	"DK",	"DENMARK",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{214,	"DO",	"DOMINICAN REPUBLIC",	TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{218,	"EC",	"ECUADOR",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{818,	"EG",	"EGYPT",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{222,	"SV",	"EL SALVADOR",			FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{233,	"EE",	"ESTONIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{246,	"FI",	"FINLAND",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{250,	"FR",	"FRANCE",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{268,	"GE",	"GEORGIA",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{276,	"DE",	"GERMANY",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{300,	"GR",	"GREECE",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{320,	"GT",	"GUATEMALA",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{340,	"HN",	"HONDURAS",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{344,	"HK",	"HONG KONG",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{348,	"HU",	"HUNGARY",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{352,	"IS",	"ICELAND",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{356,	"IN",	"INDIA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{360,	"ID",	"INDONESIA",			TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{364,	"IR",	"IRAN",					TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{372,	"IE",	"IRELAND",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{376,	"IL",	"ISRAEL",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{380,	"IT",	"ITALY",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{392,	"JP",	"JAPAN",				TRUE,	REGION_9_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{400,	"JO",	"JORDAN",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{398,	"KZ",	"KAZAKHSTAN",			FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{408,	"KP",	"KOREA DEMOCRATIC PEOPLE'S REPUBLIC OF", TRUE,	REGION_5_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{410,	"KR",	"KOREA REPUBLIC OF",	TRUE,	REGION_5_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{414,	"KW",	"KUWAIT",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{428,	"LV",	"LATVIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{422,	"LB",	"LEBANON",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{438,	"LI",	"LIECHTENSTEIN",		TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{440,	"LT",	"LITHUANIA",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{442,	"LU",	"LUXEMBOURG",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{446,	"MO",	"MACAU",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{807,	"MK",	"MACEDONIA",			FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{458,	"MY",	"MALAYSIA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{484,	"MX",	"MEXICO",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{492,	"MC",	"MONACO",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{504,	"MA",	"MOROCCO",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{528,	"NL",	"NETHERLANDS",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{554,	"NZ",	"NEW ZEALAND",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{578,	"NO",	"NORWAY",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_0},
	{512,	"OM",	"OMAN",					TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{586,	"PK",	"PAKISTAN",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{591,	"PA",	"PANAMA",				TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_0},
	{604,	"PE",	"PERU",					TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{608,	"PH",	"PHILIPPINES",			TRUE,	REGION_4_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{616,	"PL",	"POLAND",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{620,	"PT",	"PORTUGAL",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{630,	"PR",	"PUERTO RICO",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{634,	"QA",	"QATAR",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{642,	"RO",	"ROMANIA",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{643,	"RU",	"RUSSIA FEDERATION",	FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{682,	"SA",	"SAUDI ARABIA",			FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{702,	"SG",	"SINGAPORE",			TRUE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{703,	"SK",	"SLOVAKIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{705,	"SI",	"SLOVENIA",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{710,	"ZA",	"SOUTH AFRICA",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{724,	"ES",	"SPAIN",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{752,	"SE",	"SWEDEN",				TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{756,	"CH",	"SWITZERLAND",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{760,	"SY",	"SYRIAN ARAB REPUBLIC",	FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{158,	"TW",	"TAIWAN",				TRUE,	REGION_3_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_0},
	{764,	"TH",	"THAILAND",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{780,	"TT",	"TRINIDAD AND TOBAGO",	TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{788,	"TN",	"TUNISIA",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{792,	"TR",	"TURKEY",				TRUE,	REGION_2_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{804,	"UA",	"UKRAINE",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{784,	"AE",	"UNITED ARAB EMIRATES",	FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{826,	"GB",	"UNITED KINGDOM",		TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_1},
	{840,	"US",	"UNITED STATES",		TRUE,	REGION_26_A_BAND,	A_BAND_6G_REGION_0, TRUE,	G_BAND_REGION_0},
	{858,	"UY",	"URUGUAY",				TRUE,	REGION_5_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{860,	"UZ",	"UZBEKISTAN",			TRUE,	REGION_1_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_0},
	{862,	"VE",	"VENEZUELA",			TRUE,	REGION_5_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{704,	"VN",	"VIET NAM",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{887,	"YE",	"YEMEN",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{716,	"ZW",	"ZIMBABWE",				FALSE,	REGION_0_A_BAND,	A_BAND_6G_REGION_1, TRUE,	G_BAND_REGION_1},
	{999,	"",	"",	0,	0,	0,	0,	0}
};

PCH_REGION GetChRegion(
	IN PUCHAR CountryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
		if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, (RTMP_STRING *)CountryCode, 2) == 0) {
			pChRegion = &ChRegion[loop];
			break;
		}

		loop++;
	}

	/* Default: use WO*/
	if (pChRegion == NULL)
		pChRegion = GetChRegion("WO");

	return pChRegion;
}

#ifdef EXT_BUILD_CHANNEL_LIST
static VOID ChBandCheck(
	IN UCHAR rfic,
	OUT PUCHAR pChType)
{
	*pChType = 0;

	if (rfic & RFIC_5GHZ)
		*pChType |= BAND_5G;

	if (rfic & RFIC_24GHZ)
		*pChType |= BAND_24G;

	if (rfic & RFIC_6GHZ)
		*pChType |= BAND_6G;

	if (*pChType == 0)
		*pChType = BAND_24G;
}

static UCHAR FillChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_DESP pChDesp,
	IN UCHAR Offset,
	IN UCHAR increment,
	IN UCHAR regulatoryDomain,
	struct wifi_dev *wdev)
{
	INT i, j;/* sachin - TODO, l; */
	UCHAR channel;
	USHORT PhyMode = wdev->PhyMode;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
#if defined(CONFIG_AP_SUPPORT) || defined(RT_CFG80211_SUPPORT)
	struct freq_oper oper;
	UCHAR bw = BW_20;
	if (hc_radio_query_by_wdev(wdev, &oper) == HC_STATUS_OK)
		bw = oper.bw;
#endif
	j = Offset;

	for (i = 0; i < pChDesp->NumOfCh; i++) {
		channel = pChDesp->FirstChannel + i * increment;

		if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "JP", 2)) {
			/* for JP, ch14 can only be used when PhyMode is "B only" */
			if ((channel == 14) &&
				(!WMODE_EQUAL(PhyMode, WMODE_B))) {
				pChDesp->NumOfCh--;
				break;
			}
		}

		/*New FCC spec restrict the used channel under DFS */
#ifdef CONFIG_AP_SUPPORT

		if ((pAd->CommonCfg.bIEEE80211H == 1) &&
			(pAd->CommonCfg.RDDurRegion == FCC) &&
			(pAd->Dot11_H.bDFSIndoor == 1))
			 {
			if (RESTRICTION_BAND_1(pAd, channel, bw))
				continue;
		} else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
				   (pAd->CommonCfg.RDDurRegion == FCC) &&
					(pAd->Dot11_H.bDFSIndoor == 0))
			{
				if ((channel >= 100) && (channel <= 140))
					continue;
			}

#endif /* CONFIG_AP_SUPPORT */
		/* sachin - TODO */
		pChCtrl->ChList[j].Channel = pChDesp->FirstChannel + i * increment;
		pChCtrl->ChList[j].MaxTxPwr = pChDesp->MaxTxPwr;
		pChCtrl->ChList[j].DfsReq = pChDesp->DfsReq;
		pChCtrl->ChList[j].RegulatoryDomain = regulatoryDomain;
#ifdef DOT11_N_SUPPORT

		if (N_ChannelGroupCheck(pAd, pChCtrl->ChList[j].Channel, wdev))
			pChCtrl->ChList[j].Flags |= CHANNEL_40M_CAP;

#ifdef DOT11_VHT_AC

		if (vht80_channel_group(pAd, pChCtrl->ChList[j].Channel, wdev))
			pChCtrl->ChList[j].Flags |= CHANNEL_80M_CAP;

		if (vht160_channel_group(pAd, pChCtrl->ChList[j].Channel, wdev))
			pChCtrl->ChList[j].Flags |= CHANNEL_160M_CAP;

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		CFG80211OS_ChanInfoInit(
			pAd->pCfg80211_CB,
			j,
			pChCtrl->ChList[j].Channel,
			pChCtrl->ChList[j].MaxTxPwr,
			WMODE_CAP_N(PhyMode),
			(bw == BW_20));
#endif /* RT_CFG80211_SUPPORT */
		j++;
	}
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE);
	pChCtrl->ChListNum = j;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "\x1b[1;33m [FillChList] Test - pChCtrl->ChListNum = %d \x1b[m \n", pChCtrl->ChListNum);

	return j;
}


static UCHAR CeateChListByRf(RTMP_ADAPTER *pAd, UCHAR RfIC, PCH_REGION pChRegion, UCHAR Geography, UCHAR offset, struct wifi_dev *wdev)
{
	UCHAR i;
	PCH_DESP pChDesp;
	UCHAR ChType;
	UCHAR increment;
	UCHAR regulatoryDomain;
	BOOLEAN IsRfSupport = HcIsRfSupport(pAd, RfIC);

	if (IsRfSupport) {
		ChBandCheck(RfIC, &ChType);

		if (pAd->CommonCfg.pChDesp != NULL)
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
		else
			pChDesp = pChRegion->pChDesp;

		for (i = 0; pChDesp[i].FirstChannel != 0; i++) {
			if (pChDesp[i].FirstChannel == 0)
				break;

			if (ChType == BAND_5G) {
				if (pChDesp[i].FirstChannel <= 14)
					continue;
			} else if (ChType == BAND_24G) {
				if (pChDesp[i].FirstChannel > 14)
					continue;
			}

			if ((pChDesp[i].Geography == BOTH)
				|| (Geography == BOTH)
				|| (pChDesp[i].Geography == Geography)) {
				if (pChDesp[i].FirstChannel > 14)
					increment = 4;
				else
					increment = 1;

				if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
					regulatoryDomain = pAd->CommonCfg.DfsType;
				else
					regulatoryDomain = pChRegion->op_class_region;

				offset = FillChList(pAd, &pChDesp[i], offset, increment, regulatoryDomain, wdev);
			}
		}
#ifdef RT_CFG80211_SUPPORT
		if (ChType == BAND_5G) {
			if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB,
									NULL, pChDesp, NULL) != 0)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
							"Update RegRule 5G failed!\n");
		} else if (ChType == BAND_24G) {
			if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB,
									pChDesp, NULL, NULL) != 0)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
							"Update RegRule 2.4G failed!\n");
		} else if (ChType == BAND_6G) {
			if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB,
									NULL, NULL, pChDesp) != 0)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
							"Update RegRule 6G failed!\n");
		}
#endif /*RT_CFG80211_SUPPORT*/
	}

	return offset;
}


static inline VOID CreateChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_REGION pChRegion,
	IN UCHAR Geography,
	struct wifi_dev *wdev)
{
	UCHAR offset = 0;
	/* INT i,PhyIdx; */
	/* PCH_DESP pChDesp; */
	/* UCHAR ChType; */
	/* UCHAR increment; */
	/* UCHAR regulatoryDomain; */

	UCHAR BandIdx = HcGetBandByWdev(wdev);
	USHORT PhyMode = wdev->PhyMode;
	/* Get channel ctrl address */
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChRegion == NULL)
		return;

	/* Check state of channel list */
	if (hc_check_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"BandIdx %d, channel list is already DONE\n", BandIdx);
		return;
	}

        /* Initialize channel list*/
	os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
	pChCtrl->ChListNum = 0;

        if (WMODE_CAP_2G(PhyMode))
	      offset = CeateChListByRf(pAd, RFIC_24GHZ, pChRegion, Geography, offset, wdev);
        if(WMODE_CAP_5G(PhyMode))
	      offset = CeateChListByRf(pAd, RFIC_5GHZ, pChRegion, Geography, offset, wdev);
}


VOID BuildChannelListEx(
	IN PRTMP_ADAPTER pAd,
        IN struct wifi_dev *wdev)
{
	PCH_REGION pChReg;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);
	CreateChList(pAd, pChReg, pAd->CommonCfg.Geography, wdev);
}

VOID BuildBeaconChList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen)
{
	INT i;
	ULONG TmpLen;
	PCH_REGION pChRegion;
	PCH_DESP pChDesp;
	UCHAR ChType;

	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (pChRegion == NULL)
		return;

	ChBandCheck(wmode_2_rfic(wdev->PhyMode), &ChType);
	*pBufLen = 0;


	if (pAd->CommonCfg.pChDesp != NULL)
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
	else
		pChDesp = pChRegion->pChDesp;

	if (WMODE_CAP_6G(wdev->PhyMode)) {
		UINT i = 0;
		UCHAR OpExtIdentifier = 0xFE;
		UCHAR CoverageClass = 0;
		UCHAR reg_class_value[5] = {0};

		get_reg_class_list_for_6g(pAd, wdev->PhyMode, reg_class_value);

		if (reg_class_value[0] == 0) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"reg_class is NULL !!!\n");
			os_free_mem(pBuf);
			return;
		}

		for (i = 0; reg_class_value[i] != 0; i++) {
			MakeOutgoingFrame(pBuf + *pBufLen,
					&TmpLen,
					1,
					&OpExtIdentifier,
					1,
					&reg_class_value[i],
					1,
					&CoverageClass,
					END_OF_ARGS);
			*pBufLen += TmpLen;
			if (i == 4)
				break;
		}
		return;
	}

	for (i = 0; pChRegion->pChDesp[i].FirstChannel != 0; i++) {
		if (pChDesp[i].FirstChannel == 0)
			break;

		if (ChType == BAND_5G) {
			if (pChDesp[i].FirstChannel <= 14)
				continue;
		} else if (ChType == BAND_24G) {
			if (pChDesp[i].FirstChannel > 14)
				continue;
		}

		if ((pChDesp[i].Geography == BOTH) ||
			(pChDesp[i].Geography == pAd->CommonCfg.Geography)) {
			MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
							  1,					&pChDesp[i].FirstChannel,
							  1,					&pChDesp[i].NumOfCh,
							  1,					&pChDesp[i].MaxTxPwr,
							  END_OF_ARGS);
			*pBufLen += TmpLen;
		}
	}
}
#endif /* EXT_BUILD_CHANNEL_LIST */

BOOLEAN GetEDCCASupport(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN ret = FALSE;
	PCH_REGION pChReg;

	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);

	if ((pChReg->op_class_region != FCC) && (pChReg->edcca_on == TRUE)) {
		/* actually need to check PM's table in CE country */
		ret = TRUE;
	}

	return ret;
}

UINT GetEDCCAStd(PRTMP_ADAPTER pAd, IN PUCHAR CountryCode, IN USHORT radioPhy)
{
	struct physical_device *ph_dev = pAd->physical_dev;
	UINT   u1EDCCAStd = EDCCA_REGION_DEFAULT;

	if (ph_dev->edcca_region == EDCCA_REGION_NONE) {
		/* compatible when edcca_region as NONE */
		if (strncmp("JP", (RTMP_STRING *)CountryCode, 2) == 0)
			u1EDCCAStd = EDCCA_REGION_JAPAN;
		else if (WMODE_CAP_6G(radioPhy)) {
			if ((strncmp("US", (RTMP_STRING *)CountryCode, 2) == 0) ||
				(strncmp("KR", (RTMP_STRING *)CountryCode, 2) == 0))
				u1EDCCAStd = EDCCA_REGION_FCC;
			else if (strncmp("EU", (RTMP_STRING *)CountryCode, 2) == 0)
				u1EDCCAStd = EDCCA_REGION_ETSI_2017;
		}
	} else {
		/* follow profile config */
		u1EDCCAStd = ph_dev->edcca_region;
	}

	return u1EDCCAStd;
}

UCHAR GetCountryRegionFromCountryCode(
	IN UCHAR *country_code)
{
	UCHAR ret = FCC;
	PCH_REGION pChReg;

	pChReg = GetChRegion(country_code);
	if (pChReg)
		ret = pChReg->op_class_region;
	return ret;
}

#ifdef DOT11_N_SUPPORT
BOOLEAN IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	INT i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel && pChCtrl->ChList[i].Priority != 0
			&& (pChCtrl->ChList[i].NonOccupancy == 0) && (pChCtrl->ChList[i].NOPSaveForClear == 0))
			break;
	}

	if (i == pChCtrl->ChListNum)
		return FALSE;
	else
		return TRUE;
}

UCHAR GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}

BOOLEAN ExtChCheck(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR Channel,
    IN UCHAR Direction,
	IN struct wifi_dev *wdev)
{
	UCHAR ExtCh;

	/* Get extension channel by current direction */
	ExtCh = GetExtCh(Channel, Direction);

	/* Check whether current extension channel is in channel list or not */
	if (IsValidChannel(pAd, ExtCh, wdev))
		return TRUE;
	else
		return FALSE;
}

BOOLEAN N_ChannelGroupCheck(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN struct wifi_dev *wdev)
{
	BOOLEAN	RetVal = FALSE;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);

	switch (ch_band) {
	case CMD_CH_BAND_24G:
		do {
			UCHAR ExtCh;

			if (Channel == 14) {
				RetVal = FALSE;
				break;
			}

			ExtCh = GetExtCh(Channel, EXTCHA_ABOVE);

			if (IsValidChannel(pAd, ExtCh, wdev))
				RetVal = TRUE;
			else {
				ExtCh = GetExtCh(Channel, EXTCHA_BELOW);

				if (IsValidChannel(pAd, ExtCh, wdev))
					RetVal = TRUE;
			}
		} while (FALSE);
		break;

	case CMD_CH_BAND_5G:
	case CMD_CH_BAND_6G:
		RetVal = vht40_channel_group(pAd, Channel, wdev);
		break;

	default:
		break;
	}

	return RetVal;
}

static const UCHAR wfa_ht_ch_ext[] = {
	36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
	44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
	52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
	60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
	100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
	108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
	116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
	124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
	132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
	140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
	149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
	157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
	165, EXTCHA_ABOVE, 169, EXTCHA_BELOW,
	173, EXTCHA_ABOVE, 177, EXTCHA_BELOW,
	0, 0
};

static const UCHAR wfa_ht_ch_ext_6G[] = {
	1, EXTCHA_ABOVE, 5, EXTCHA_BELOW,
	9, EXTCHA_ABOVE, 13, EXTCHA_BELOW,
	17, EXTCHA_ABOVE, 21, EXTCHA_BELOW,
	25, EXTCHA_ABOVE, 29, EXTCHA_BELOW,
	33, EXTCHA_ABOVE, 37, EXTCHA_BELOW,
	41, EXTCHA_ABOVE, 45, EXTCHA_BELOW,
	49, EXTCHA_ABOVE, 53, EXTCHA_BELOW,
	57, EXTCHA_ABOVE, 61, EXTCHA_BELOW,
	65, EXTCHA_ABOVE, 69, EXTCHA_BELOW,
	73, EXTCHA_ABOVE, 77, EXTCHA_BELOW,
	81, EXTCHA_ABOVE, 85, EXTCHA_BELOW,
	89, EXTCHA_ABOVE, 93, EXTCHA_BELOW,
	97, EXTCHA_ABOVE, 101, EXTCHA_BELOW,
	105, EXTCHA_ABOVE, 109, EXTCHA_BELOW,
	113, EXTCHA_ABOVE, 117, EXTCHA_BELOW,
	121, EXTCHA_ABOVE, 125, EXTCHA_BELOW,
	129, EXTCHA_ABOVE, 133, EXTCHA_BELOW,
	137, EXTCHA_ABOVE, 141, EXTCHA_BELOW,
	145, EXTCHA_ABOVE, 149, EXTCHA_BELOW,
	153, EXTCHA_ABOVE, 157, EXTCHA_BELOW,
	161, EXTCHA_ABOVE, 165, EXTCHA_BELOW,
	169, EXTCHA_ABOVE, 173, EXTCHA_BELOW,
	177, EXTCHA_ABOVE, 181, EXTCHA_BELOW,
	185, EXTCHA_ABOVE, 189, EXTCHA_BELOW,
	193, EXTCHA_ABOVE, 197, EXTCHA_BELOW,
	201, EXTCHA_ABOVE, 205, EXTCHA_BELOW,
	209, EXTCHA_ABOVE, 213, EXTCHA_BELOW,
	217, EXTCHA_ABOVE, 221, EXTCHA_BELOW,
	225, EXTCHA_ABOVE, 229, EXTCHA_BELOW,
	0, 0
};

VOID ht_ext_cha_adjust(RTMP_ADAPTER *pAd, UCHAR prim_ch, UCHAR *ht_bw, UCHAR *ext_cha, struct wifi_dev *wdev)
{
	INT idx;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	const UCHAR *ch_ext = NULL;

	if (ch_band == CMD_CH_BAND_5G)
		ch_ext = wfa_ht_ch_ext;
	else if (ch_band == CMD_CH_BAND_6G)
		ch_ext = wfa_ht_ch_ext_6G;

	if (*ht_bw == HT_BW_40) {
		if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
			idx = 0;

			while (ch_ext[idx] != 0) {
				if (ch_ext[idx] == prim_ch &&
					IsValidChannel(pAd, GetExtCh(prim_ch, ch_ext[idx + 1]), wdev)) {
					*ext_cha = ch_ext[idx + 1];
					break;
				}

				idx += 2;
			};

			if (ch_ext[idx] == 0) {
				*ht_bw = HT_BW_20;
				*ext_cha = EXTCHA_NONE;
			}
		} else {
			do {
				UCHAR ExtCh;
				UCHAR Dir = *ext_cha;

				if (Dir == EXTCHA_NONE)
					Dir = EXTCHA_ABOVE;

				ExtCh = GetExtCh(prim_ch, Dir);

				if (IsValidChannel(pAd, ExtCh, wdev)) {
					*ext_cha = Dir;
					break;
				}

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = GetExtCh(prim_ch, Dir);

				if (IsValidChannel(pAd, ExtCh, wdev)) {
					*ext_cha = Dir;
					break;
				}

				*ht_bw = HT_BW_20;
			} while (FALSE);

			if (prim_ch == 14)
				*ht_bw = HT_BW_20;
		}
	} else if (*ht_bw == HT_BW_20)
		*ext_cha = EXTCHA_NONE;
}

#endif /* DOT11_N_SUPPORT */


UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN USHORT PhyMode,
	IN struct wifi_dev *wdev,
	IN UCHAR ht_bw)
{
	int i;
	UINT8 channel = wdev->channel;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
#if defined(SINGLE_SKU) || defined(ANTENNA_CONTROL_SUPPORT)
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#endif /* defined(SINGLE_SKU) || defined(ANTENNA_CONTROL_SUPPORT) */

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable)
			TxPath = pAd->TxStream;
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel)
			break;
	}

	if (i == pChCtrl->ChListNum)
		return 30;

#ifdef SINGLE_SKU

	if (pAd->CommonCfg.bSKUMode == TRUE) {
		UINT deltaTxStreamPwr = 0;
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(PhyMode) && (TxPath == 2))
			deltaTxStreamPwr = 3; /* If 2Tx case, antenna gain will increase 3dBm*/

#endif /* DOT11_N_SUPPORT */

		if (pChCtrl->ChList[i].RegulatoryDomain == FCC) {
			/* FCC should maintain 20/40 Bandwidth, and without antenna gain */
#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(PhyMode) &&
				(ht_bw == BW_40) &&
				(channel == 1 || channel == 11))
				return pChCtrl->ChList[i].MaxTxPwr - pAd->CommonCfg.BandedgeDelta - deltaTxStreamPwr;
			else
#endif /* DOT11_N_SUPPORT */
				return pChCtrl->ChList[i].MaxTxPwr - deltaTxStreamPwr;
		} else if (pChCtrl->ChList[i].RegulatoryDomain == CE)
			return pChCtrl->ChList[i].MaxTxPwr - pAd->CommonCfg.AntGain - deltaTxStreamPwr;
		else
			return 30;
	} else
#endif /* SINGLE_SKU */
		return pChCtrl->ChList[i].MaxTxPwr;
}

/* for OS_ABL */
VOID RTMP_MapChannelID2KHZ(
	IN UCHAR Ch,
	OUT UINT32 * pFreq)
{
	int chIdx;

	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++) {
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel) {
			(*pFreq) = CH_HZ_ID_MAP[chIdx].freqKHz * 1000;
			break;
		}
	}

	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pFreq) = 2412000;
}

/* for OS_ABL */
VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh)
{
	int chIdx;

	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++) {
		if ((Freq) == CH_HZ_ID_MAP[chIdx].freqKHz) {
			(*pCh) = CH_HZ_ID_MAP[chIdx].channel;
			break;
		}
	}

	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pCh) = 1;
}

INT32 ChannelFreqToGroup(UINT32 ChannelFreq)
{
	INT32 GroupIndex = 0;

	if (ChannelFreq <= 2484) /* 2G CH14 = 2484 */
		GroupIndex = 0;
	else if ((ChannelFreq >= 4850) && (ChannelFreq <= 5140))
		GroupIndex = 1;
	else if ((ChannelFreq >= 5145) && (ChannelFreq <= 5250))
		GroupIndex = 2;
	else if ((ChannelFreq >= 5255) && (ChannelFreq <= 5360))
		GroupIndex = 3;
	else if ((ChannelFreq >= 5365) && (ChannelFreq <= 5470))
		GroupIndex = 4;
	else if ((ChannelFreq >= 5475) && (ChannelFreq <= 5580))
		GroupIndex = 5;
	else if ((ChannelFreq >= 5585) && (ChannelFreq <= 5690))
		GroupIndex = 6;
	else if ((ChannelFreq >= 5695) && (ChannelFreq <= 5800))
		GroupIndex = 7;
	else if ((ChannelFreq >= 5805) && (ChannelFreq <= 5950))
		GroupIndex = 8;
	else {
		GroupIndex = -1;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "Can't find group for [%d].\n", ChannelFreq);
	}

	return GroupIndex;
}


BOOLEAN MTChGrpValid(IN CHANNEL_CTRL *pChCtrl)
{
	return (pChCtrl->ChGrpABandChNum > 0 &&
		pChCtrl->ChGrpABandChNum < MAX_NUM_OF_CHANNELS &&
		pChCtrl->ChGrpABandEn != 0);
}

void MTSetChGrp(RTMP_ADAPTER *pAd, RTMP_STRING *buf)
{
	UCHAR ChGrpIdx, ChIdx, len_macptr2, BandIdx, TotalChNum;
	UCHAR *macptr, *macptr2, *buf_ChGrp;
	BOOLEAN bEnable;
	PCH_GROUP_DESC pChDescGrp;
	CHANNEL_CTRL *pChCtrl;
	struct wifi_dev *pwdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pwdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pwdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
	}
#endif

	if (buf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"(buf == NULL) Not enough memory for dynamic allocating\n");
		return;
	}

	/* Find channel group of each HW band */
	for (BandIdx = 0, macptr2 = rstrtok(buf, ";"); macptr2; macptr2 = rstrtok(buf, ";"), BandIdx++) {
		/*Get channel control*/
		/* 2.4G + 5G */
		/* l1profile: (2Gprofile;5Gprofile) */
		/* If DEFAULT_5G_PROFILE is NOT enabled, (2Gprofile;5Gprofile) iNIC_ap_2g.dat (if 2g.dat is 2G) is set to HW band 0 */
		/* If DEFAULT_5G_PROFILE is enabled, (5Gprofile;2Gprofile) iNIC_ap_5g.dat is set to HW band 1 */
		/* ChannelGrp is only for 5G channel list */

		/* 5G + 5G */
		/* l1profile: (B0_5G_profile;B1_5G_profile) */
		/* If DEFAULT_5G_PROFILE is NOT enabled, (B0_5G_profile;B1_5G_profile) B0_5G_profile is set to HW band 0 */
		/* If DEFAULT_5G_PROFILE is enabled, (B1_5G_profile;B0_5G_profile) B1_5G_profile is set to HW band 0 */
		if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
			/* 5G + 5G */
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"ENUM_DBDC_5G5G: BandIdx = %d\n", BandIdx);
		} else {
			if (pwdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"Main Wdev is NULL!\n");
				return;
			}
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		}

		/*Initialize channel group*/
		NdisZeroMemory(pChCtrl->ChGrpABandChList, (MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
		TotalChNum = 0;
		pChCtrl->ChGrpABandChNum = 0;
		pChCtrl->ChGrpABandEn = 0;

		/*Copy channel group of current HW band for checking bit map*/
		len_macptr2 = strlen(macptr2) + 1;
		os_alloc_mem(NULL, (UCHAR **)&buf_ChGrp, len_macptr2);
		if (buf_ChGrp == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"(buf_ChGrp == NULL) Not enough memory for dynamic allocating\n");
			return;
		}
		strncpy(buf_ChGrp, macptr2, len_macptr2);
		buf_ChGrp[len_macptr2 - 1] = '\0';

		/*Check bit map of channel group of current HW band*/
		for (ChGrpIdx = 0, macptr = rstrtok(buf_ChGrp, ":"); (macptr && ChGrpIdx < Channel_GRP_Num); macptr = rstrtok(NULL, ":"), ChGrpIdx++) {
			bEnable = 0;
			bEnable = (UCHAR) simple_strtol(macptr, 0, 10);
			pChDescGrp = &(Channel_GRP[ChGrpIdx]);
			if (bEnable == 1) {
				if ((TotalChNum + pChDescGrp->NumOfCh) < MAX_NUM_OF_CHANNELS) {
					pChCtrl->ChGrpABandEn |= (1 << ChGrpIdx);
					for (ChIdx = 0; ChIdx < pChDescGrp->NumOfCh; ChIdx++) {
						if ((TotalChNum + ChIdx) < MAX_NUM_OF_CHANNELS) {
							pChCtrl->ChGrpABandChList[TotalChNum + ChIdx] = pChDescGrp->FirstChannel + (ChIdx * 4);
							MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
							"ChCtrl[%d]->ChGrpABandChList[%d]=%d\n",
							BandIdx, TotalChNum + ChIdx,
							pChDescGrp->FirstChannel + (ChIdx * 4));
						}
					}
					TotalChNum += pChDescGrp->NumOfCh;
				}
			}
		}
		pChCtrl->ChGrpABandChNum = TotalChNum;

		/*Shift to the next channel group of HW band*/
		buf = buf + len_macptr2;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"BandIdx =%d, pChCtrl->ChGrpABandChNum=%d, ChGrp=%d/%d/%d/%d\n",
			BandIdx, pChCtrl->ChGrpABandChNum,
			((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND0)) ? 1 : 0),
			((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND1)) ? 1 : 0),
			((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND2)) ? 1 : 0),
			((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND3)) ? 1 : 0));

		os_free_mem(buf_ChGrp);
	}
}

BOOLEAN MTChGrpChannelChk(
	IN CHANNEL_CTRL *pChCtrl,
	IN UCHAR ch)
{
	UCHAR ChGrpIdx;
	BOOLEAN result = FALSE;

	for (ChGrpIdx = 0; ChGrpIdx < pChCtrl->ChGrpABandChNum; ChGrpIdx++) {
		if (ch == pChCtrl->ChGrpABandChList[ChGrpIdx]) {
			result = TRUE;
			break;
		}
	}
	return result;
}

#define NUM_OF_COUNTRIES	(sizeof(allCountry)/sizeof(COUNTRY_CODE_TO_COUNTRY_REGION))

/*
    ==========================================================================
    Description:
	Set Country String.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryString_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT   index = 0;
	INT   success = TRUE;
	RTMP_STRING name_buffer[40] = {0};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	BOOLEAN IsSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
	int ret;
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) <= 38) {
		if (strlen(arg) < 4) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					 "Parameter of CountryString are too short!\n");
			return FALSE;
		}

		for (index = 0; index < strlen(arg); index++) {
			if ((arg[index] >= 'a') && (arg[index] <= 'z'))
				arg[index] = toupper(arg[index]);
		}

		for (index = 0; index < NUM_OF_COUNTRIES; index++) {
			NdisZeroMemory(name_buffer, sizeof(name_buffer));
			ret = snprintf(name_buffer, sizeof(name_buffer), "\"%s\"", (RTMP_STRING *) allCountry[index].pCountryName);
			if (os_snprintf_error(sizeof(name_buffer), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return FALSE;
			}

			if (strncmp((RTMP_STRING *) allCountry[index].pCountryName, arg, strlen(arg)) == 0)
				break;
			else if (strncmp(name_buffer, arg, strlen(arg)) == 0)
				break;
		}

		if (index == NUM_OF_COUNTRIES)
			success = FALSE;
	} else
		success = FALSE;

	if (success == TRUE) {
		if (pAd->CommonCfg.CountryRegion & 0x80) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					 "parameter of CountryRegion in eeprom is programmed\n");
			success = FALSE;
		} else {
			success = FALSE;

			if (IsSupport2G) {
				if (allCountry[index].SupportGBand == TRUE) {
					pAd->CommonCfg.CountryRegion = (UCHAR) allCountry[index].RegDomainNum11G;
					success = TRUE;
				} else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"The Country are not Support G Band Channel\n");
			}

			if (IsSupport5G) {
				if (allCountry[index].SupportABand == TRUE) {
					pAd->CommonCfg.CountryRegionForABand = (UCHAR) allCountry[index].RegDomainNum11A;
					success = TRUE;
				} else
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"The Country are not Support A Band Channel\n");
			}
		}
	}

	if (success == TRUE) {
		os_zero_mem(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		os_move_mem(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
		/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
		/* it's no longer necessary since APStartUp will rebuild channel again. */
		/*BuildChannelList(pAd); */
		pAd->CommonCfg.bCountryFlag = TRUE;
		/* if set country string, driver needs to be reset */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"CountryString=%s CountryRegin=%d CountryCode=%s\n",
			allCountry[index].pCountryName,
			pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryCode);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Parameters out of range\n");

	return success;
}

/*
    ==========================================================================
    Description:
	Set Country Code.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"RF lock down! Cannot config CountryCode status!\n");
		return TRUE;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	/* reset temp table status */
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode,
					   sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "bCountryFlag=%d, CountryCode=%s\n",
			 pAd->CommonCfg.bCountryFlag, pAd->CommonCfg.CountryCode);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Country Region.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;
	UCHAR IfIdx;
	CHANNEL_CTRL *pChCtrl;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	IfIdx = pObj->ioctl_if;

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type);
		return FALSE;
	}

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"RF lock down!! Cannot config CountryRegion status!!\n");
		return TRUE;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */
	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_24G);

	if (retval == FALSE)
		return FALSE;

	/* If country region is set, driver needs to be reset*/
	/* Change channel state to NONE */
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
	BuildChannelList(pAd, wdev);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "CountryRegion=%d\n",
			 pAd->CommonCfg.CountryRegion);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Country Region for A band.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;
	UCHAR BandIdx, IfIdx;
	CHANNEL_CTRL *pChCtrl;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif

	IfIdx = pObj->ioctl_if;

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type);
		return FALSE;
	}

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"RF lock down!! Cannot config CountryRegion status!!\n");
		return TRUE;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */
	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_5G);

	if (retval == FALSE)
		return FALSE;

	/* If Country Region is set, channel list needs to be rebuilt*/
	/* Change channel state to NONE */
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef MT_DFS_SUPPORT
	pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
	BuildChannelList(pAd, wdev);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "CountryRegion=%d\n",
			 pAd->CommonCfg.CountryRegionForABand);
	return TRUE;
}

INT update_ht_extchan_cfg(RTMP_ADAPTER *pAd, UCHAR ext_chan, BOOLEAN *b_chg)
{
	struct wifi_dev *tdev = NULL;
	UCHAR i = 0;

	*b_chg = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"ext_chan=%d\n", ext_chan);

	if ((ext_chan != EXTCHA_ABOVE) && (ext_chan != EXTCHA_BELOW)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
		"invalid ext_chan, return\n");
		return -EINVAL;
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev) {
			struct wlan_config *cfg = (struct wlan_config *)tdev->wpf_cfg;

			if (cfg) {
				if ((cfg->ht_conf.ext_cha != ext_chan) || wlan_operate_get_ext_cha(tdev) != ext_chan) {
					cfg->ht_conf.ext_cha = ext_chan;
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"ext_cha=%d\n", cfg->ht_conf.ext_cha);
					*b_chg = TRUE;
				}
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "End.\n");
	return 0;
}

/*
	==========================================================================
	Description:
	Apply ht extension channel configuration
	Return:
	0 if all parameters are OK, negative value otherwise
	==========================================================================
*/
INT apply_ht_extchan_cfg(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *tdev = NULL;
	UCHAR i = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev)
			SetCommonHtVht(pAd, tdev);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "End.\n");
	return 0;
}

#ifdef DOT11_EHT_BE
BOOLEAN prepare_mlo_csa(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	IN UINT8 *buf = NULL;
	IN UINT16 buf_len = 0;

	if (bss_mngr_is_wdev_in_mlo_group(wdev)) {
		os_alloc_mem(pAd, (UCHAR **)&buf, CHANNEL_SWITCH_IE_BUF_SIZE);
		if (!buf) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Alloc memory failed.\n");
			return FALSE;
		}

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "\n");
		os_zero_mem(buf, CHANNEL_SWITCH_IE_BUF_SIZE);
		build_channel_switch_relatd_ie(pAd, wdev, buf, &buf_len);
		if (buf_len > CHANNEL_SWITCH_IE_BUF_SIZE)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"Buffer overflow!(Reserved:%d, Occupied:%d)\n", CHANNEL_SWITCH_IE_BUF_SIZE, buf_len);

		bss_mngr_mld_add_sta_profile(wdev, 0, 0, BMGR_MLD_APPL_ELEM_STA_PROFILE, buf, buf_len);
		os_free_mem(buf);

		return TRUE;
	}

	return FALSE;
}

void remove_mlo_csa(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "\n");

	if (bss_mngr_is_wdev_in_mlo_group(wdev)) {
		if (bss_mngr_mld_remove_sta_profile(wdev,
			BMGR_MLD_APPL_ELEM_STA_PROFILE, NULL, 0))
			bss_mngr_sync_bcn_update(wdev);
	}
}
#endif

#ifdef RT_CFG80211_SUPPORT

enum nl80211_chan_width phy_bw_2_nl_bw(int width)
{
	switch (width) {
	case BW_20:
		return NL80211_CHAN_WIDTH_20;
	case BW_40:
		return NL80211_CHAN_WIDTH_40;
	case BW_80:
		return NL80211_CHAN_WIDTH_80;
	case BW_8080:
		return NL80211_CHAN_WIDTH_80P80;
	case BW_160:
		return NL80211_CHAN_WIDTH_160;
#ifdef IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ
	case BW_320:
		return NL80211_CHAN_WIDTH_320;
#endif /* IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ */
	default:
		return NL80211_CHAN_WIDTH_20_NOHT;
	}
}

/*
    ==========================================================================
    Description:
	Set country code from mwctl vendor cmd
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
INT mtk_cfg80211_vndr_cmd_set_country_code(RTMP_ADAPTER *pAd, UCHAR *code)
{
	struct wifi_dev *wdev = NULL;
	UINT_8 idx = 0;
	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"RF lock down! Cannot config CountryCode status!\n");
		return -EBUSY;
	}
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			break;
		}
	}
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"No any wdev!!\n");
		return -EINVAL;
	}
#ifdef EXT_BUILD_CHANNEL_LIST
	/* reset temp table status */
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(code) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, code, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"bCountryFlag=%d, CountryCode=%s!\n",
			pAd->CommonCfg.bCountryFlag,
			pAd->CommonCfg.CountryCode);
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Invalid Country Code!\n");
		return -EINVAL;
	}
	return 0;
}

/*
    ==========================================================================
    Description:
	Set country region from mwctl vendor cmd
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
INT mtk_cfg80211_vndr_cmd_set_country_region(RTMP_ADAPTER *pAd, UINT32 region)
{
	CHANNEL_CTRL *pChCtrl;
	UCHAR ch_band;
	UCHAR *pCountryRegion;
	struct wifi_dev *wdev = NULL;
	struct wifi_dev *tdev = NULL;
	UINT_8 idx = 0;
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "region=%d\n",
			 region);
	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"RF lock down!! Cannot config CountryRegion status!!\n");
		return -EBUSY;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			break;
		}
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "No any wdev!!\n");
		return -EINVAL;
	}

	ch_band = wlan_config_get_ch_band(wdev);
	if (ch_band == CMD_CH_BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;

	/*
		   1. If this value is set before interface up, do not reject this value.
		   2. Country can be set only when EEPROM not programmed
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS) && (*pCountryRegion & EEPROM_IS_PROGRAMMED)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"CountryRegion in eeprom was programmed\n");
		return -EBUSY;
	}

	if (((ch_band == CMD_CH_BAND_24G) && ((region <= REGION_MAXIMUM_BG_BAND)
			|| (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND)
			|| (region == REGION_33_BG_BAND)))
		|| ((ch_band == CMD_CH_BAND_5G)
			&& (region <= REGION_MAXIMUM_A_BAND))
		|| ((ch_band == CMD_CH_BAND_6G)
			&& (region <= REGION_MAXIMUM_A_BAND_6GHZ)))
		*pCountryRegion = (UCHAR) region;
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"region(%d) out of range!\n", region);
		return -EINVAL;
	}

	/*before change region, do disconnetion*/
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		tdev = pAd->wdev_list[idx];
		if (tdev && HcIsRadioAcq(tdev)
			&& (tdev->wdev_type == WDEV_TYPE_AP))
			MacTableResetNonMapWdev(pAd, tdev);
	}

	/* If country region is set, driver needs to be reset*/
	/* Change channel state to NONE */
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef MT_DFS_SUPPORT
	pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
	BuildChannelList(pAd, wdev);
	if (ch_band == CMD_CH_BAND_24G)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "CountryRegion=%d\n",
					 pAd->CommonCfg.CountryRegion);
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"CountryRegionForABand=%d\n",
			pAd->CommonCfg.CountryRegionForABand);

	/*check current ch's sanity for new region*/
	if (!IsValidChannel(pAd, wdev->channel, wdev)) {
		UCHAR new_ch = FirstChannel(pAd, wdev);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"set new_ch = %d\n", new_ch);
		/*To do set channel, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"%s: TakeChannelOpCharge fail for SET channel!!\n", __func__);
			return -EBUSY;
		}

		/*set channel*/
		wdev->channel = new_ch;
		wlan_operate_set_prim_ch(wdev, new_ch);

		/*if channel setting is DONE, release ChannelOpCharge here*/
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);
	}

	return 0;
}

/*
    ==========================================================================
    Description:
	Set country string from mwctl vendor cmd
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
INT mtk_cfg80211_vndr_cmd_set_country_string(RTMP_ADAPTER *pAd, UCHAR *string)
{
	INT   index = 0;
	INT   success = TRUE;
	RTMP_STRING name_buffer[40] = {0};
	UCHAR ch_band;
	int ret;
	struct wifi_dev *wdev = NULL;
	UINT_8 idx = 0;
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */
	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"RF lock down! Cannot config CountryCode status!\n");
		return -EBUSY;
	}

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			break;
		}
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "No any wdev!!\n");
		return -EINVAL;
	}
	ch_band = wlan_config_get_ch_band(wdev);
	if (strlen(string) <= 38) {
		if (strlen(string) < 4) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					 "Parameter of CountryString are too short!\n");
			return -EINVAL;
		}

		for (index = 0; index < strlen(string); index++) {
			if ((string[index] >= 'a') && (string[index] <= 'z'))
				string[index] = toupper(string[index]);
		}

		for (index = 0; index < NUM_OF_COUNTRIES; index++) {
			NdisZeroMemory(name_buffer, sizeof(name_buffer));
			ret = snprintf(name_buffer, sizeof(name_buffer), "\"%s\"", (RTMP_STRING *) allCountry[index].pCountryName);
			if (os_snprintf_error(sizeof(name_buffer), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return -EINVAL;
			}

			if (strncmp((RTMP_STRING *) allCountry[index].pCountryName, string, strlen(string)) == 0)
				break;
			else if (strncmp(name_buffer, string, strlen(string)) == 0)
				break;
		}

		if (index == NUM_OF_COUNTRIES)
			success = FALSE;
	} else
		success = FALSE;

	if (success == TRUE) {
		os_zero_mem(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		os_move_mem(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
		/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
		/* it's no longer necessary since APStartUp will rebuild channel again. */
		/*BuildChannelList(pAd); */
		pAd->CommonCfg.bCountryFlag = TRUE;
		/* if set country string, driver needs to be reset */
		if (ch_band == CMD_CH_BAND_24G)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"CountryRegion=%d\n",
				pAd->CommonCfg.CountryRegion);
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"CountryRegionForABand=%d\n",
				pAd->CommonCfg.CountryRegionForABand);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				 "CountryString=%s CountryCode=%s\n",
				  allCountry[index].pCountryName,
				  pAd->CommonCfg.CountryCode);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Parameters out of range\n");

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			 "bCountryFlag=%d, CountryCode=%s\n",
			 pAd->CommonCfg.bCountryFlag, pAd->CommonCfg.CountryCode);
	return success ? 0 : -EFAULT;
}

/*
    ==========================================================================
    Description:
	Apply new channel settings to phy
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
static INT apply_chan_change(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 chan)
{
	INT32 success = FALSE;
#ifdef TR181_SUPPORT
	UCHAR old_channel = wdev->channel;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
	INT ret = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

#ifdef WIFI_MD_COEX_SUPPORT
	if (!IsChannelSafe(pAd, chan)) {
		if (IsPwrChannelSafe(pAd, chan))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
				"caller:%pS. The channel %d is power backoff channel\n",
				OS_TRACE, chan);
		else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"caller:%pS. The channel %d is in unsafe channel list!!\n",
				OS_TRACE, chan);
			return -EINVAL;
		}
	}
#endif

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	pAd->CommonCfg.DfsParameter.ByPassCac = DfsZwBypassCac(pAd, wdev, wdev->channel, chan);
	if (!pAd->CommonCfg.DfsParameter.ByPassCac) {
		DfsDedicatedExamineSetNewCh(pAd, wdev, chan);
		DedicatedZeroWaitStop(pAd, TRUE);
	}
#endif

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE)
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_SET_CHANNEL;
#endif

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"TakeChannelOpCharge fail for SET channel!!\n");
		return -EBUSY;
	}


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"Channel(%d), Cert(%d), Quick(%d)\n",
		chan, pAd->CommonCfg.wifi_cert, wdev->quick_ch_change);

	pAd->ApCfg.iwpriv_event_flag = TRUE;
	RTMP_OS_REINIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);

#ifdef TR181_SUPPORT
	success = rtmp_set_channel(pAd, wdev, chan);
	if (success && (old_channel != chan)) {
		if (ctrl) {
			ctrl->rdev.pRadioCtrl->ManualChannelChangeCount++;
			ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"success:%d, Manual:%d, Total:%d\n",
				success, ctrl->rdev.pRadioCtrl->ManualChannelChangeCount,
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount);
		}
	}
#else
#ifdef DFS_CAC_R2
	if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
		success = rtmp_set_channel(pAd, wdev, chan);
		if (success == FALSE)
			wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, FALSE);
		/*return success; after set channel finished,then return iwpriv.*/
	} else
#endif
		success = rtmp_set_channel(pAd, wdev, chan);
#endif
	if (pAd->ApCfg.set_ch_async_flag == TRUE) {
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(
			&pAd->ApCfg.set_ch_aync_done, ((50*100*OS_HZ)/1000));/*Wait 5s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"wait channel setting success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"wait channel setting timeout.\n");
			pAd->ApCfg.set_ch_async_flag = FALSE;
		}
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success ? 0 : -EFAULT;
}


/*
	==========================================================================
	Description:
	Update bw configuration to DB
	Return:
	0 if all parameters are OK, negative value otherwise
	==========================================================================
*/
static INT update_bw_config(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 new_bw, BOOLEAN *bw_chg)
{
	struct wifi_dev *tdev = NULL;
	UCHAR i = 0;
	UCHAR vht_bw, ht_bw, eht_bw;
	struct wlan_config *cfg = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"new_bw=%d\n", new_bw);

	*bw_chg = FALSE;

	cfg = (struct wlan_config *)wdev->wpf_cfg;
	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"null wdev->wpf_cfg!\n");
		return -ENETDOWN;
	}

	if (new_bw > MWCTL_CHAN_WIDTH_40) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Not support VHT on such PhyMode(%d)!\n", wdev->PhyMode);
			return -EINVAL;
		}
	}

	if (new_bw > MWCTL_CHAN_WIDTH_160) {
		if ((wlan_config_get_ch_band(wdev) != CMD_CH_BAND_6G) || !WMODE_CAP_BE(wdev->PhyMode)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Not support 320M on such PhyMode(%d)!\n", wdev->PhyMode);
			return -EINVAL;
		}
	}

	ht_bw = cfg->ht_conf.ht_bw;
	vht_bw = cfg->vht_conf.vht_bw;
	eht_bw = cfg->eht_conf.bw;

	switch (new_bw) {
	case MWCTL_CHAN_WIDTH_20:
		ht_bw = HT_BW_20;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_20;
		break;
	case MWCTL_CHAN_WIDTH_40:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_2040;
		eht_bw = EHT_BW_2040;
		break;
	case MWCTL_CHAN_WIDTH_80:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_80;
		eht_bw = EHT_BW_80;
		break;
	case MWCTL_CHAN_WIDTH_160:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_160;
		eht_bw = EHT_BW_160;
		break;
	case MWCTL_CHAN_WIDTH_320:
		ht_bw = HT_BW_40;
		vht_bw = VHT_BW_160;
		eht_bw = EHT_BW_320;
		break;
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev) {
			struct wlan_config *cfg = (struct wlan_config *)tdev->wpf_cfg;

			if (cfg) {
				if (cfg->ht_conf.ht_bw != ht_bw) {
					wlan_config_set_ht_bw(tdev, ht_bw);
					*bw_chg = TRUE;
				}
				if (cfg->vht_conf.vht_bw != vht_bw) {
					wlan_config_set_vht_bw(tdev, vht_bw);
					*bw_chg = TRUE;
				}
				if (cfg->eht_conf.bw != eht_bw) {
					wlan_config_set_eht_bw(tdev, eht_bw);
					*bw_chg = TRUE;
				}
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"End. bw_chg=%d, ht_bw=%d, vht_bw=%d, eht_bw=%d\n",
		*bw_chg, ht_bw, vht_bw, eht_bw);
	return 0;
}

/*
	==========================================================================
	Description:
	Apply bw configuration
	Return:
	0 if all parameters are OK, negative value otherwise
	==========================================================================
*/
static INT apply_bw_cfg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *tdev = NULL;
	UCHAR i = 0;
	struct freq_cfg cfg;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

#ifdef MT_DFS_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	/* Reset ht coex result */
	pAd->CommonCfg.BssCoexScanLastResult.LastScanTime = 0;
	pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack = FALSE;
	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

#ifdef MT_DFS_SUPPORT
	pDot11h = wdev->pDot11_H;
	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
		RadarStateCheck(pAd, wdev);
	else if (pDot11h)
		pDot11h->ChannelMode = CHAN_NORMAL_MODE;
#endif

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

			os_zero_mem(&cfg, sizeof(cfg));
			phy_freq_get_cfg(tdev, &cfg);
			operate_loader_phy(tdev, &cfg);

			SetCommonHtVht(pAd, tdev);

			/* Update Beacon to Reflect BW Changes */
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
									BCN_BPCC_HEOP, TRUE);

#ifdef CONFIG_MAP_SUPPORT
			if (!IS_MAP_ENABLE(pAd))
#endif
			{
				if (tdev->wdev_type == WDEV_TYPE_AP)
					ap_send_broadcast_deauth(pAd, tdev);
				else if (tdev->wdev_type == WDEV_TYPE_STA) {
					pStaCfg = GetStaCfgByWdev(pAd, tdev);
					if (!pStaCfg || !INFRA_ON(pStaCfg))
						continue;
					cntl_disconnect_request(&pStaCfg->wdev,
										CNTL_DISASSOC,
										pStaCfg->Bssid,
										REASON_DISASSOC_STA_LEAVING);
				}
			}
		}
	}

	if (pAd->CommonCfg.bBssCoexEnable && HcIsRfSupport(pAd, RFIC_24GHZ)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Start 20/40 BSSCoex Channel Scan\n");
		APOverlappingBSSScan(pAd, wdev);
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "End.\n");
	return 0;
}

/*
	==========================================================================
	Description:
	Apply ht coex configuration
	Return:
	0 if all parameters are OK, negative value otherwise
	==========================================================================
*/
static INT apply_ht_coex_cfg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *tdev = NULL;
	UCHAR i = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	if (!HcIsRfSupport(pAd, RFIC_24GHZ)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"Not support coex!\n");
		return -RTMP_IO_EOPNOTSUPP;
	}

	if (pAd->CommonCfg.bBssCoexEnable) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Start 20/40 BSSCoex Channel Scan\n");
		APOverlappingBSSScan(pAd, wdev);
	} else if (pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack) {
		/* switch back 20/40 */
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Set bBssCoexEnable:  Switch back 20/40.\n");
		pAd->CommonCfg.BssCoexScanLastResult.LastScanTime = 0;
		pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack = FALSE;
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

		if (wlan_config_get_ht_bw(wdev) == BW_40)
			wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_config_get_ext_cha(wdev));
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev)
			UpdateBeaconHandler(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG));
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "End.\n");
	return 0;
}


/*
    ==========================================================================
    Description:
	Set Channel attributes (channel, bw, ht_coex) from mwctl vendor cmd
    Return:
	0 if all parameters are OK, negative value otherwise
    ==========================================================================
*/
INT mtk_cfg80211_vndr_cmd_set_channel_attributes(
	RTMP_ADAPTER *pAd, UINT32 set_flag, UCHAR chan, UINT32 bw, UCHAR ext_chan, UCHAR ht_coex)
{
	BOOLEAN b_chn_change = FALSE, b_bw_change = FALSE, b_coex_change = FALSE;
	BOOLEAN b_extchan_change = FALSE;
	struct wifi_dev *wdev = NULL;
	UCHAR new_ch, new_bw, old_ch, old_bw;
	INT err_code = 0, i;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"End. set_flag=0x%x, set_flag=%d, bw=%d, ht_coex=%d\n",
		set_flag, set_flag, bw, ht_coex);

	/* Find an active wdev on the phy */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] && HcIsRadioAcq(pAd->wdev_list[i])) {
			wdev = pAd->wdev_list[i];
			break;
		}
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"No any active wdev on the phy!\n");
		return -EINVAL;
	}

	new_ch = wdev->channel;
	new_bw = (bw == 0) ? wlan_operate_get_bw(wdev) : bw;
	old_ch = wdev->channel;
	old_bw = wlan_operate_get_bw(wdev);
	if (set_flag & MTK_CFG80211_CHAN_SET_FLAG_CHAN) {
		if (chan != wdev->channel) {
			new_ch = chan;
			b_chn_change = TRUE;
		}
	}

	if (set_flag & MTK_CFG80211_CHAN_SET_FLAG_EXT_CHAN) {
		ext_chan = ext_chan ? EXTCHA_ABOVE : EXTCHA_BELOW;
		err_code = update_ht_extchan_cfg(pAd, ext_chan, &b_extchan_change);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"update ht_extchan_cfg failed!\n");
			return err_code;
		}
	}

	if (set_flag & MTK_CFG80211_CHAN_SET_FLAG_BW) {
		err_code = update_bw_config(pAd, wdev, bw, &b_bw_change);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"update bw failed!\n");
			return err_code;
		}
	}

	if (set_flag & MTK_CFG80211_CHAN_SET_FLAG_COEX) {
		if (pAd->CommonCfg.bBssCoexEnable != ht_coex) {
			pAd->CommonCfg.bBssCoexEnable = ht_coex;
			b_coex_change = TRUE;
		}
	}

#ifdef MT_DFS_SUPPORT
	if (pAd->CommonCfg.BandSelBand == BAND_SELECT_BAND_5G) {
		update_cac_ctrl_status(pAd, wdev, old_ch, old_bw, new_ch, new_bw);
	}
#endif /*MT_DFS_SUPPORT*/

	if (b_chn_change) {
#ifdef CONFIG_MAP_SUPPORT
		if (!IS_MAP_ENABLE(pAd) && b_bw_change)
#else
		if (b_bw_change)
#endif
			pAd->Dot11_H.disconn_after_ch_switch = TRUE;

		err_code = apply_chan_change(pAd, wdev, chan);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Apply channel failed!\n");
			return err_code;
		}
	} else if (b_bw_change) {
		err_code = apply_bw_cfg(pAd, wdev);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Apply bw failed!\n");
			return err_code;
		}
	} else if (b_coex_change) {
		err_code = apply_ht_coex_cfg(pAd, wdev);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Apply coex failed!\n");
			return err_code;
		}
	}

	if (b_extchan_change) {
		err_code = apply_ht_extchan_cfg(pAd);
		if (err_code) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Apply extchan failed!\n");
			return err_code;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "End.\n");
	return err_code;
}

#endif

/**
* init_ch_chg_info - Init channel change info.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID init_ch_chg_info(IN PRTMP_ADAPTER pAd)
{
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "\n");

	pAd->ch_chg_info.old_channel = 0;
	pAd->ch_chg_info.old_bw = 0xFF;
	pAd->ch_chg_info.ch_chg_cnt = 0;
}
