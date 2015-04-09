/*!
 * \file ixfr.h
 *
 * \author Marek Vavrusa <marek.vavrusa@nic.cz> - IXFR/OUT
 * \author Jan Kadlec <marek.vavrusa@nic.cz> - IXFR/IN
 *
 * \brief IXFR processing.
 *
 * \addtogroup query_processing
 * @{
 */
/*  Copyright (C) 2013 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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

#include "libknot/packet/pkt.h"

struct query_data;
struct answer_data;

/*!
 * \brief IXFR query processing module.
 *
 * \retval PRODUCE if it has an answer, but not yet finished.
 * \retval FAIL if it encountered an error.
 * \retval DONE if finished.
 */
int ixfr_query(knot_pkt_t *pkt, struct query_data *qdata);

/*!
 * \brief IXFR response processing module.
 *
 * \retval CONSUME if more data are required.
 * \retval FAIL if it encountered an error, retry over AXFR will be done.
 * \retval DONE if finished.
 */
int ixfr_process_answer(knot_pkt_t *pkt, struct answer_data *adata);

/*! @} */
