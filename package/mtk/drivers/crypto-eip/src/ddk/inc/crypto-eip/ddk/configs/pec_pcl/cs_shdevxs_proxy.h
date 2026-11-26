/* cs_shdevxs_proxy.h
 *
 * Configuration for user-mode proxy of Kernel Support Driver.
 */

/*****************************************************************************
* Copyright (c) 2012-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef CS_SHDEVXS_PROXY_H_
#define CS_SHDEVXS_PROXY_H_
#include "cs_shdevxs_proxy.h"

#define SHDEVXS_IRQ_COUNT 31

#define SHDEVXS_PROXY_MAX_DMA_BANKS 2

#define SHDEVXS_PROXY_IRQ_TIMEOUT_MS 1000

#define SHDEVXS_PROXY_REMOVE_IRQ_CLEAR
#define SHDEVXS_PROXY_REMOVE_RC
#define SHDEVXS_PROXY_REMOVE_TEST
#endif /* CS_SHDEVXS_PROXY_H_ */


/* end of file cs_shdevxs_proxy.h */
