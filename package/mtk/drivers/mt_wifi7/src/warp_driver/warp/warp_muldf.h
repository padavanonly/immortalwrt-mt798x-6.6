/* SPDX-License-Identifier: ISC */
/*
 * Copyright (C) 2024 MediaTek Inc.
 */


#ifndef _WARP_MULDF_H_
#define _WARP_MULDF_H_


#include <linux/soc/mediatek/muldf.h>


#ifndef MULDF_MODULE_NAME_WARP
#define MULDF_MODULE_NAME_WARP	"warp"
#endif

/* MULDF Debug Category for warp */
#define MULDF_CAT_MTWF_WARP		"mtwf_warp"

/* Sub-Category of  MULDF_CAT_MTWF_WARP */
/* NULL */


void muldf_init(void);


/*redefine dev_emerg*/

#undef dev_emerg
#define dev_emerg(dev, fmt, ...) \
	do {    \
		_dev_emerg(dev, dev_fmt(fmt), ##__VA_ARGS__);   \
		muldf_print(MTK_MSG_ERROR, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__); \
	} while (0)
/*redefine dev_crit*/
#undef dev_crit
#define dev_crit(dev, fmt, ...) \
	do {    \
		_dev_crit(dev, dev_fmt(fmt), ##__VA_ARGS__);    \
		muldf_print(MTK_MSG_ERROR, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__); \
	} while (0)
/*redefine dev_alert*/
#undef dev_alert
#define dev_alert(dev, fmt, ...) \
	do {    \
		_dev_alert(dev, dev_fmt(fmt), ##__VA_ARGS__);   \
		muldf_print(MTK_MSG_ERROR, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__); \
	} while (0)
/*redefine dev_err*/
#undef dev_err
#define dev_err(dev, fmt, ...) \
	do {    \
		_dev_err(dev, dev_fmt(fmt), ##__VA_ARGS__); \
		muldf_print(MTK_MSG_ERROR, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__); \
	} while (0)
/*redefine dev_warn*/
#undef dev_warn
#define dev_warn(dev, fmt, ...) \
	do {    \
		_dev_warn(dev, dev_fmt(fmt), ##__VA_ARGS__);    \
		muldf_print(MTK_MSG_WARNING, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__);   \
	} while (0)
/*redefine dev_notice*/
#undef dev_notice
#define dev_notice(dev, fmt, ...) \
	do {    \
		_dev_notice(dev, dev_fmt(fmt), ##__VA_ARGS__);  \
		muldf_print(MTK_MSG_WARNING, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__);   \
	} while (0)
/*redefine dev_info*/
#undef dev_info
#define dev_info(dev, fmt, ...) \
	do {    \
		_dev_info(dev, dev_fmt(fmt), ##__VA_ARGS__);    \
		muldf_print(MTK_MSG_INFO, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
			NULL, NULL, NULL, 0, dev_fmt(fmt), ##__VA_ARGS__);  \
	} while (0)


#endif /* _WARP_MULDF_H_ */
