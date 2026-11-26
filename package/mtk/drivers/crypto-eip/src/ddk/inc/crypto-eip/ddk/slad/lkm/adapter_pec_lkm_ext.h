/* adapter_pec_lkm_ext.h
 *
 * PEC API implementation,
 * Linux kernel specific extensions
 *
 */

/*****************************************************************************
* Copyright (c) 2012-2022 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef ADAPTER_PEC_LKM_EXT_H_
#define ADAPTER_PEC_LKM_EXT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

// Linux Kernel API
#include <linux/init.h>     // module_init, module_exit
#include <linux/module.h>   // EXPORT_SYMBOL


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_pec.h"        // PEC API

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
#include "api_pec_sg.h"     // PEC SG API
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */

#include "iotoken.h"        // IOToken API

#ifdef ADAPTER_AUTO_TOKENBUILDER
#include "token_builder.h"
#include "sa_builder.h"
#include "sa_builder_ipsec.h"
#include "sa_builder_ssltls.h"
#include "sa_builder_basic.h"
// #include "sa_builder_srtp.h"
#endif

// PEC API
EXPORT_SYMBOL(PEC_Capabilities_Get);
EXPORT_SYMBOL(PEC_Init);
EXPORT_SYMBOL(PEC_UnInit);
EXPORT_SYMBOL(PEC_SA_Register);
EXPORT_SYMBOL(PEC_SA_UnRegister);
EXPORT_SYMBOL(PEC_Packet_Put);
EXPORT_SYMBOL(PEC_Packet_Get);
EXPORT_SYMBOL(PEC_Scatter_Preload);
EXPORT_SYMBOL(PEC_CommandNotify_Request);
EXPORT_SYMBOL(PEC_ResultNotify_Request);
EXPORT_SYMBOL(PEC_CD_Control_Write);
EXPORT_SYMBOL(PEC_RD_Status_Read);
EXPORT_SYMBOL(PEC_Put_Dump);
EXPORT_SYMBOL(PEC_Get_Dump);

#ifdef ADAPTER_PEC_ENABLE_SCATTERGATHER
EXPORT_SYMBOL(PEC_SGList_Create);
EXPORT_SYMBOL(PEC_SGList_Destroy);
EXPORT_SYMBOL(PEC_SGList_Write);
EXPORT_SYMBOL(PEC_SGList_Read);
EXPORT_SYMBOL(PEC_SGList_GetCapacity);
EXPORT_SYMBOL(PEC_Scatter_PreloadNotify_Request);
#endif /* ADAPTER_PEC_ENABLE_SCATTERGATHER */

// IOToken API
EXPORT_SYMBOL(IOToken_Create);
EXPORT_SYMBOL(IOToken_Parse);
EXPORT_SYMBOL(IOToken_InWordCount_Get);
EXPORT_SYMBOL(IOToken_OutWordCount_Get);
EXPORT_SYMBOL(IOToken_SAAddr_Update);
EXPORT_SYMBOL(IOToken_SAReuse_Update);
EXPORT_SYMBOL(IOToken_Mark_Set);
EXPORT_SYMBOL(IOToken_Mark_Check);
EXPORT_SYMBOL(IOToken_PacketLegth_Get);
EXPORT_SYMBOL(IOToken_BypassLegth_Get);
EXPORT_SYMBOL(IOToken_ErrorCode_Get);

#ifdef ADAPTER_AUTO_TOKENBUILDER
// SABuilder API
EXPORT_SYMBOL(SABuilder_GetSizes);
EXPORT_SYMBOL(SABuilder_BuildSA);
EXPORT_SYMBOL(SABuilder_Init_ESP);
EXPORT_SYMBOL(SABuilder_Init_SSLTLS);
EXPORT_SYMBOL(SABuilder_Init_Basic);
// EXPORT_SYMBOL(SABuilder_Init_SRTP);

// TokenBuilder API
EXPORT_SYMBOL(TokenBuilder_GetContextSize);
EXPORT_SYMBOL(TokenBuilder_BuildContext);
EXPORT_SYMBOL(TokenBuilder_GetSize);
EXPORT_SYMBOL(TokenBuilder_BuildToken);
#endif


#endif // ADAPTER_PEC_LKM_EXT_H_


/* end of file adapter_pec_lkm_ext.h */
