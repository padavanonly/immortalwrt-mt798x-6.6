/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#ifndef _AN8855_SWCONFIG_H_
#define _AN8855_SWCONFIG_H_

#ifdef CONFIG_SWCONFIG
#include <linux/switch.h>
#include "an8855.h"

int an8855_swconfig_init(struct gsw_an8855 *gsw);
void an8855_swconfig_destroy(struct gsw_an8855 *gsw);
#else
static inline int an8855_swconfig_init(struct gsw_an8855 *gsw)
{
	an8855_apply_vlan_config(gsw);

	return 0;
}

static inline void an8855_swconfig_destroy(struct gsw_an8855 *gsw)
{
}
#endif

#endif /* _AN8855_SWCONFIG_H_ */
