/* shdevxs_kernel_stub.c
 *
 * Kernel-side stub for Kernel-support driver (called from user space).
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */
#include "shdevxs_kernel.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_shdevxs.h"
#include "umdevxs_cmd.h"
#include "shdevxs_cmd.h"
#include "log.h"
#include "shdevxs_init.h"
#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
#include "shdevxs_rc.h"
#endif
#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
#include "shdevxs_dmapool.h"
#endif
#include "shdevxs_irq.h"
#ifdef SHDEVXS_ENABLE_PRNG_FUNCTIONS
#include "shdevxs_prng.h"
#endif
#include "shdevxs_kernel_internal.h"

#include <linux/uaccess.h>          // copy_to/from_user, access_ok

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * UMDevXS_KernelSupport_HandleCmd
 */
void
SHDevXS_HandleCmd(
    void * file_p,
    UMDevXS_CmdRsp_t * const CmdRsp)
{
    switch (CmdRsp->Opcode)
    {
    case UMDEVXS_OPCODE_SHDEVXS_GLOBAL_INIT:
        CmdRsp->Error = SHDevXS_Global_Init();
        break;
    case UMDEVXS_OPCODE_SHDEVXS_GLOBAL_UNINIT:
        CmdRsp->Error = SHDevXS_Global_UnInit();
        break;
    case UMDEVXS_OPCODE_SHDEVXS_TEST:
        CmdRsp->Error = SHDevXS_Test();
        break;
#ifdef SHDEVXS_ENABLE_DMAPOOL_FUNCTIONS
    case UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_INIT:
    {
        SHDevXS_DMAPool_t DMAPool_Info;
        CmdRsp->Error = SHDevXS_Internal_DMAPool_Init(file_p, &DMAPool_Info);
        CmdRsp->Handle = DMAPool_Info.Handle;
        if (CmdRsp->Error==0)
        {
            // Copy the RC_Info to user space.
            if (copy_to_user(
                    (void __user *)CmdRsp->ptr1,
                    &DMAPool_Info,
                    sizeof(DMAPool_Info)) != 0)
            {
                LOG_CRIT(
                    "SHDevXS Kernel Stub: "
                    "Failed on copy_to_user\n");
                CmdRsp->Error = -1;
            }
        }
        break;
    }
    case UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_UNINIT:
        SHDevXS_Internal_DMAPool_Uninit(file_p);
        CmdRsp->Error = 0;
        break;
    case UMDEVXS_OPCODE_SHDEVXS_DMAPOOL_GETBASE:
        CmdRsp->Error = SHDevXS_DMAPool_GetBase(&CmdRsp->ptr1);
        break;
#endif
#ifdef SHDEVXS_ENABLE_RC_FUNCTIONS
    case UMDEVXS_OPCODE_SHDEVXS_RC_TRC_INVALIDATE:
        CmdRsp->Error = SHDevXS_TRC_Record_Invalidate(CmdRsp->uint1);
        break;
    case UMDEVXS_OPCODE_SHDEVXS_RC_ARC4RC_INVALIDATE:
        CmdRsp->Error = SHDevXS_ARC4RC_Record_Invalidate(CmdRsp->uint1);
        break;
    case UMDEVXS_OPCODE_SHDEVXS_RC_LOCK:
#ifdef SHDEVXS_LOCK_SLEEPABLE
        CmdRsp->Error = SHDevXS_RC_Lock();
#else
        CmdRsp->Error = -1; // not allowed for non-sleepable locks
#endif // SHDEVXS_LOCK_SLEEPABLE
        break;
    case UMDEVXS_OPCODE_SHDEVXS_RC_FREE:
#ifdef SHDEVXS_LOCK_SLEEPABLE
        SHDevXS_RC_Free();
        CmdRsp->Error = 0;
#else
        CmdRsp->Error = -1; // not allowed for non-sleepable locks
#endif // SHDEVXS_LOCK_SLEEPABLE
        break;
#endif
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_INIT:
        CmdRsp->Error = SHDevXS_IRQ_Init();
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_UNINIT:
        CmdRsp->Error = SHDevXS_IRQ_UnInit();
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_ENABLE:
        CmdRsp->Error = SHDevXS_Internal_IRQ_Enable(file_p,
                                                    (int)CmdRsp->uint1,
                                                    CmdRsp->uint2);
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_DISABLE:
        CmdRsp->Error = SHDevXS_Internal_IRQ_Disable(file_p,
                                                     (int)CmdRsp->uint1,
                                                     CmdRsp->uint2);
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_CLEAR:
#ifdef SHDEVXS_ENABLE_IRQ_CLEAR
        CmdRsp->Error = SHDevXS_Internal_IRQ_Clear(file_p,
                                                     (int)CmdRsp->uint1,
                                                     CmdRsp->uint2);
#else
        CmdRsp->Error = -1;
#endif
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_CLEARANDENABLE:
#ifdef SHDEVXS_ENABLE_IRQ_CLEAR
        CmdRsp->Error = SHDevXS_Internal_IRQ_ClearAndEnable(file_p,
                                                     (int)CmdRsp->uint1,
                                                     CmdRsp->uint2);
#else
        CmdRsp->Error = -1;
#endif
        break;
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_SETHANDLER:
    {
        CmdRsp->Error = SHDevXS_Internal_IRQ_SetHandler(file_p,
                                             (int)CmdRsp->uint1,
                                             SHDevXS_Internal_IRQHandler);
        break;
    }
    case UMDEVXS_OPCODE_SHDEVXS_IRQ_WAIT:
    {
        int nIRQ = (int)CmdRsp->uint1;
        unsigned int timeout = CmdRsp->uint2;
        CmdRsp->Error = SHDevXS_Internal_WaitWithTimeout(file_p, nIRQ, timeout);
        break;
    }
#ifdef SHDEVXS_ENABLE_PRNG_FUNCTIONS
    case UMDEVXS_OPCODE_SHDEVXS_PRNG_RESEED:
    {
        SHDevXS_PRNG_Reseed_t Reseed;
        if (copy_from_user(&Reseed,
                           (void __user *)CmdRsp->ptr1,
                           sizeof(Reseed)))
        {
            LOG_INFO(
                "SHDevXS_HandleCmd: "
                "Failed on copy_from_user\n");
            CmdRsp->Error = -1;
        }
        CmdRsp->Error = SHDevXS_PRNG_Reseed(&Reseed);
        break;
    }
    case UMDEVXS_OPCODE_SHDEVXS_SUPPORTEDFUNCS_GET:
    {
        CmdRsp->uint1 = SHDevXS_SupportedFuncs_Get();
        CmdRsp->Error = 0;
        break;
    }
#endif
    default:
        LOG_CRIT("SHDevXS_HandleCmd: unsupported opcode %d\n",
                 CmdRsp->Opcode);

    }
}

/* shdevxs_kernel_stub.c */
