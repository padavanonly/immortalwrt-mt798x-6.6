// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2024 MediaTek Inc.
 */

#include "warp_muldf.h"


struct muldf_sub_category {
	char *name;
	u8 dump;
};

struct muldf_category {
	char *cat_name;
	struct muldf_sub_category **sub_cat;
};

static struct muldf_category module_warp[] = {
	{
		MULDF_CAT_MTWF_WARP,
		NULL
	},
	{	NULL,
		NULL
	}
};

void muldf_init(void)
{
	int i = 0, j = 0;

	/*add module*/
	muldf_add_module(MULDF_MODULE_NAME_WARP, MTK_MSG_INFO);

	/*add category/sub-category*/
	for (i = 0; module_warp[i].cat_name != NULL; i++) {
		muldf_add_cat(MULDF_MODULE_NAME_WARP, module_warp[i].cat_name);
		if (module_warp[i].sub_cat != NULL) {
			for (j = 0; module_warp[i].sub_cat[j] != NULL; j++) {
				muldf_add_sub_cat(MULDF_MODULE_NAME_WARP,
						     module_warp[i].cat_name,
						     module_warp[i].sub_cat[j]->name,
						     module_warp[i].sub_cat[j]->dump);
			}
		}
	}
}

