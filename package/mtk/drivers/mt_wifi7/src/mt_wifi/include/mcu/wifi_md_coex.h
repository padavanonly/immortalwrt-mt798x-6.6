#ifndef __WIFI_MD_COEX_H__
#define __WIFI_MD_COEX_H__

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
#define WIFI_CH_MASK_IDX_NUM	4
#define WIFI_RAM_BAND_NUM 3
#define MAX_TG_NUM_LTE 3
#define MAX_TG_NUM_NR 3

struct COEX_IDC_INFO {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	/* IDC Enable / Disable */
	BOOLEAN idcEnable;
	/* MD operfreq: LTE/NR */
	UINT8 lte_oper_band[MAX_TG_NUM_LTE];
	UINT16 lte_dl_freq[MAX_TG_NUM_LTE];
	UINT16 lte_ul_freq[MAX_TG_NUM_LTE];
	UINT8 nr_oper_band[MAX_TG_NUM_NR];
	UINT32 nr_dl_freq[MAX_TG_NUM_NR];
	UINT32 nr_ul_freq[MAX_TG_NUM_NR];
	/* MD Unsafe channel (driver/ fw) */
	UINT32 u4SafeChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4TdmChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4PwrChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4FdmChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	/* IDC Solution (Free run / Power backoff / TDM / Unsafe FDM) */
	/*
	* 0: Free run
	* 1: Powerbackoff
	* 2: TDM
	* 3: Unsafe FDM
	*/
	UINT8 u4WiFiFinalSoluation[WIFI_RAM_BAND_NUM];
	UINT8	u4MDFinalSoluation[WIFI_RAM_BAND_NUM];
	/* MD Connected mode / Idle mode */
	UINT8 lteTxExist;
	UINT8 nrTxExist;
	/* MD Pwr flag */
	UINT16  u2LtePwrBackBmp;
	UINT16  u2NrPwrBackBmp;
	/* MD / Wi-Fi TDM time */
	UINT16 tdm_lte_window; /* LTE window(ms) */
	UINT16 tdm_lte_conn_window; /* Wi-Fi window(ms) */
	UINT16 tdm_nr_window; /* NR window(ms) */
	UINT16 tdm_conn_window; /* Wi-Fi window(ms) */
};

VOID Coex_IDC_Info_Handle(RTMP_ADAPTER *pAd, struct COEX_IDC_INFO *pEventIdcInfo);
INT Set_Idc_State(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Idc_TxPwrBackOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Idc_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID init_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
VOID deinit_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
#endif /* WIFI_MD_COEX_SUPPORT */
#endif /* __WIFI_MD_COEX_H__ */

