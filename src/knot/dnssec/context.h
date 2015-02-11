/*  Copyright (C) 2015 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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

#include <time.h>

#include <dnssec/kasp.h>
#include <dnssec/keystore.h>

/*!
 * \brief DNSSEC signing context.
 */
struct kdnssec_ctx {
	time_t now;

	dnssec_kasp_t *kasp;
	dnssec_kasp_zone_t *zone;
	dnssec_kasp_policy_t *policy;
	dnssec_keystore_t *keystore;

	uint32_t old_serial;
	uint32_t new_serial;
	bool rrsig_drop_existing;
};

typedef struct kdnssec_ctx kdnssec_ctx_t;

/*!
 * \brief Initialize DNSSEC signing context.
 *
 * \param ctx        Signing context to be initialized.
 * \param kasp_path  Path to KASP store.
 * \param zone_name  Name of the zone.
 */
int kdnssec_ctx_init(kdnssec_ctx_t *ctx, const char *kasp_path,
                     const char *zone_name);

/*!
 * \brief Cleanup DNSSEC signing context.
 */
void kdnssec_ctx_deinit(kdnssec_ctx_t *ctx);
