/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

*/
#include "warp.h"

static struct wo_entry wo;

void wed_wo_ops_register(struct wo_ops *ops)
{
	struct wo_ops *obj = &wo.ops;

	obj->mid_mod_schedule = ops->mid_mod_schedule;

	return;
}
EXPORT_SYMBOL(wed_wo_ops_register);

void wed_wo_init(struct wo_entry **_wo_entry)
{
	*_wo_entry = &wo;
}
