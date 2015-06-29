/*  Copyright (C) 2011 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*!
 * \file msg.h
 *
 * \author Daniel Salzman <daniel.salzman@nic.cz>
 *
 * \brief Simple output formatting framework.
 *
 * \addtogroup knot_utils
 * @{
 */

#pragma once

#include <stdio.h>

#define ERROR_		"; Error: "
#define INFO_		"; Info: "
#define WARNING_	"; Warning: "
#define DEBUG_		"; Debug: "

#define ERR(msg, ...)	{ printf(ERROR_ msg, ##__VA_ARGS__); fflush(stdout); }
#define INFO(msg, ...)	{ printf(INFO_ msg, ##__VA_ARGS__); fflush(stdout); }
#define WARN(msg, ...)	{ printf(WARNING_ msg, ##__VA_ARGS__); fflush(stdout); }
#define DBG(msg, ...)	msg_debug(DEBUG_ msg, ##__VA_ARGS__)

/*! \brief Enable/disable debugging. */
int msg_enable_debug(int val);

/*! \brief Print debug message. */
int msg_debug(const char *fmt, ...);

/*! \brief Debug message for null input. */
#define DBG_NULL	DBG("%s: null parameter\n", __func__)

/*! @} */
