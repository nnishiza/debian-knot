/*!
 * \file capture.h
 *
 * \author Marek Vavrusa <marek.vavrusa@nic.cz>
 *
 * \brief Simple packet capture processor.
 *
 * \addtogroup answer_processing
 * @{
 */
/*  Copyright (C) 2014 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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

#include "libknot/processing/layer.h"
#include "libknot/packet/pkt.h"

/* Processing module implementation. */
const knot_layer_api_t *capture_get_module(void);
#define LAYER_CAPTURE capture_get_module()
#define LAYER_CAPTURE_ID 3

/*!
 * \brief Processing module parameters.
 */
struct capture_param {
	knot_pkt_t *sink;        /*!< Container for captured response. */
};

/*! @} */
