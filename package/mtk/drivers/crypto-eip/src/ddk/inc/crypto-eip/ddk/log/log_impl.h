/* log_impl.h
 *
 * Log Module, implementation for Linux Kernel Mode
 */

/*****************************************************************************
* Copyright (c) 2008-2020 by Rambus, Inc. and/or its subsidiaries.
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

#ifndef INCLUDE_GUARD_LOG_IMPL_H
#define INCLUDE_GUARD_LOG_IMPL_H

#include <linux/kernel.h>   // printk

#define Log_Message           printk
#define Log_FormattedMessage  printk

// backwards compatible implementation
#define Log_FormattedMessageINFO  Log_FormattedMessage
#define Log_FormattedMessageWARN  Log_FormattedMessage
#define Log_FormattedMessageCRIT  Log_FormattedMessage

#endif /* Include Guard */

/* end of file log_impl.h */
