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
/*!
 * \file layer.h
 *
 * \author Marek Vavrusa <marek.vavrusa@nic.cz>
 *
 * \addtogroup query_processing
 * @{
 */

#pragma once

#include <stdint.h>

#include "libknot/consts.h"
#include "libknot/packet/pkt.h"
#include "libknot/rrtype/tsig.h"
#include "libknot/internal/lists.h"
#include "libknot/internal/mempattern.h"

/*! Layer processing states.
 *  Each state represents the state machine transition,
 *  and determines readiness for the next action.
 */
enum knot_layer_state {
	KNOT_STATE_NOOP    = 0,      /*!< N/A */
	KNOT_STATE_CONSUME = 1 << 0, /*!< Consume data. */
	KNOT_STATE_PRODUCE = 1 << 1, /*!< Produce data. */
	KNOT_STATE_DONE    = 1 << 2, /*!< Finished. */
	KNOT_STATE_FAIL    = 1 << 3  /*!< Error. */
};

/* Forward declarations. */
struct knot_layer_api;

/*! \brief Packet processing context. */
typedef struct knot_layer
{
	node_t node;
	uint16_t state;  /* Bitmap of enum knot_layer_state. */
	mm_ctx_t *mm;    /* Processing memory context. */
	void *data;      /* Module specific. */
	const struct knot_layer_api *api;
} knot_layer_t;

/*! \brief Packet processing module API. */
typedef struct knot_layer_api {
	int (*begin)(knot_layer_t *ctx, void *module_param);
	int (*reset)(knot_layer_t *ctx);
	int (*finish)(knot_layer_t *ctx);
	int (*consume)(knot_layer_t *ctx, knot_pkt_t *pkt);
	int (*produce)(knot_layer_t *ctx, knot_pkt_t *pkt);
	int (*fail)(knot_layer_t *ctx, knot_pkt_t *pkt);
	void *data;
} knot_layer_api_t;

/*!
 * \brief Initialize packet processing context.
 *
 * \param ctx   Layer context.
 * \param param Parameters for given module.
 * \param api   Layer API.
 * \return (module specific state)
 */
int knot_layer_begin(knot_layer_t *ctx, const knot_layer_api_t *api, void *param);

/*!
 * \brief Reset current packet processing context.
 * \param ctx   Layer context.
 * \return (module specific state)
 */
int knot_layer_reset(knot_layer_t *ctx);

/*!
 * \brief Finish and close packet processing context.
 *
 * \param ctx   Layer context.
 * \return (module specific state)
 */
int knot_layer_finish(knot_layer_t *ctx);

/*!
 * \brief Add more data to layer processing.
 *
 * \param ctx Layer context.
 * \param pkt Data packet.
 *
 * \return (module specific state)
 */
int knot_layer_consume(knot_layer_t *ctx, knot_pkt_t *pkt);

/*!
 * \brief Generate output from layer.
 *
 * \param ctx Layer context.
 * \param pkt Data packet.
 *
 * \return (module specific state)
 */
int knot_layer_produce(knot_layer_t *ctx, knot_pkt_t *pkt);

/*! @} */
