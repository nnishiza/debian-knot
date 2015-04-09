/*!
* \file errcode.h
*
* \author Lubos Slovak <lubos.slovak@nic.cz>
* \author Marek Vavrusa <marek.vavrusa@nic.cz>
*
* \brief Error codes and function for getting error message.
*
* \addtogroup common_lib
* @{
*/
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

#pragma once

#include <errno.h>
#include "libknot/internal/errcode.h"

/*!
 * \brief Returns error message for the given error code.
 *
 * \param code Error code.
 *
 * \return String containing the error message.
 */
const char *knot_strerror(int code);

/*!
 * \brief Map POSIX errno to Knot error code.
 *
 * KNOT_ERROR is used as a fallback error, the list is terminated implicitly.
 */
int knot_map_errno(int arg0, ...);

/*! @} */
