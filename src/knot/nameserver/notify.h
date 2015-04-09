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
 * \file notify.h
 *
 * \author Lubos Slovak <lubos.slovak@nic.cz>
 *
 * \brief NOTIFY request/reply API.
 *
 * \addtogroup query_processing
 * @{
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include "libknot/packet/pkt.h"
#include "knot/zone/contents.h"

struct query_data;
struct answer_data;

#define NOTIFY_TIMEOUT 3 /*!< Interval between NOTIFY retries. */

/*!
 * \brief Answer IN class zone NOTIFY message (RFC1996).
 *
 * \retval FAIL if it encountered an error.
 * \retval DONE if finished.
 */
int notify_process_query(knot_pkt_t *pkt, struct query_data *qdata);

/*!
 * \brief Process an answer to the NOTIFY query.
 *
 * \retval FAIL if it encountered an error.
 * \retval DONE if finished.
 */
int notify_process_answer(knot_pkt_t *pkt, struct answer_data *data);

/*! @} */
