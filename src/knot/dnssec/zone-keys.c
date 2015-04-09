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

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

#include "dnssec/error.h"
#include "dnssec/keystore.h"
#include "knot/common/log.h"
#include "knot/dnssec/zone-keys.h"
#include "libknot/libknot.h"
#include "libknot/rrtype/dnskey.h"

/*!
 * \brief Get key feature flags from key parameters.
 */
static int set_key(dnssec_kasp_key_t *kasp_key, time_t now, zone_key_t *zone_key)
{
	assert(kasp_key);
	assert(zone_key);

	dnssec_kasp_key_timing_t *timing = &kasp_key->timing;

	// cryptographic context

	dnssec_sign_ctx_t *ctx = NULL;
	int r = dnssec_sign_new(&ctx, kasp_key->key);
	if (r != DNSSEC_EOK) {
		return r;
	}

	zone_key->key = kasp_key->key;
	zone_key->ctx = ctx;

	// next event computation

	time_t next = LONG_MAX;
	time_t timestamps[4] = {
	        timing->active,
		timing->publish,
	        timing->remove,
	        timing->retire,
	};

	for (int i = 0; i < 4; i++) {
		time_t ts = timestamps[i];
		if (ts != 0 && now < ts && ts < next) {
			next = ts;
		}
	}

	zone_key->next_event = next;

	// key use flags

	uint16_t flags = dnssec_key_get_flags(kasp_key->key);
	zone_key->is_ksk = flags & KNOT_RDATA_DNSKEY_FLAG_KSK;
	zone_key->is_zsk = !zone_key->is_ksk;

	zone_key->is_active = timing->active <= now &&
	                      (timing->retire == 0 || now < timing->retire);
	zone_key->is_public = timing->publish <= now &&
	                      (timing->remove == 0 || now < timing->remove);

	return KNOT_EOK;
}

/*!
 * \brief Check if algorithm is allowed with NSEC3.
 */
static bool is_nsec3_allowed(uint8_t algorithm)
{
	switch (algorithm) {
	case DNSSEC_KEY_ALGORITHM_RSA_SHA1:
	case DNSSEC_KEY_ALGORITHM_DSA_SHA1:
		return false;
	default:
		return true;
	}
}

/*!
 * \brief Algorithm usage information.
 */
typedef struct algorithm_usage {
	unsigned ksk_count;  //!< Available KSK count.
	unsigned zsk_count;  //!< Available ZSK count.

	bool is_public;      //!< DNSKEY is published.
	bool is_stss;        //!< Used to sign all types of records.
	bool is_ksk_active;  //!< Used to sign DNSKEY records.
	bool is_zsk_active;  //!< Used to sign non-DNSKEY records.
} algorithm_usage_t;

/*!
 * \brief Check correct key usage, enable Single-Type Signing Scheme if needed.
 *
 * Each record in the zone has to be signed at least by one key for each
 * algorithm published in the DNSKEY RR set in the zone apex.
 *
 * Therefore, publishing a DNSKEY creates a requirement on active keys with
 * the same algorithm. At least one KSK key and one ZSK has to be enabled.
 * If one key type is unavailable (not just inactive and not-published), the
 * algorithm is switched to Single-Type Signing Scheme.
 */
static int prepare_and_check_keys(const char *zone_name, bool nsec3_enabled,
                                  zone_keyset_t *keyset)
{
	assert(zone_name);
	assert(keyset);

	const size_t max_algorithms = KNOT_DNSSEC_ALG_ECDSAP384SHA384 + 1;
	algorithm_usage_t usage[max_algorithms];
	memset(usage, 0, max_algorithms * sizeof(algorithm_usage_t));

	// count available keys

	for (size_t i = 0; i < keyset->count; i++) {
		zone_key_t *key = &keyset->keys[i];
		uint8_t algorithm = dnssec_key_get_algorithm(key->key);

		assert(algorithm < max_algorithms);
		algorithm_usage_t *u = &usage[algorithm];

		if (nsec3_enabled && !is_nsec3_allowed(algorithm)) {
			log_zone_str_warning(zone_name, "DNSSEC, key '%d' "
			                     "cannot be used with NSEC3",
			                     dnssec_key_get_keytag(key->key));
			key->is_public = false;
			key->is_active = false;
			continue;
		}

		if (key->is_ksk) { u->ksk_count += 1; }
		if (key->is_zsk) { u->zsk_count += 1; }
	}

	// enable Single-Type Signing scheme if applicable

	for (int i = 0; i < max_algorithms; i++) {
		algorithm_usage_t *u = &usage[i];

		// either KSK or ZSK keys are available
		if ((u->ksk_count == 0) != (u->zsk_count == 0)) {
			u->is_stss = true;
			log_zone_str_info(zone_name, "DNSSEC, Single-Type Signing "
			                  "scheme enabled, algorithm '%d'", i);
		}
	}

	// update key flags for STSS, collect information about usage

	for (size_t i = 0; i < keyset->count; i++) {
		zone_key_t *key = &keyset->keys[i];
		algorithm_usage_t *u = &usage[dnssec_key_get_algorithm(key->key)];

		if (u->is_stss) {
			key->is_ksk = true;
			key->is_zsk = true;
		}

		if (key->is_public) { u->is_public = true; }
		if (key->is_active) {
			if (key->is_ksk) { u->is_ksk_active = true; }
			if (key->is_zsk) { u->is_zsk_active = true; }
		}
	}

	// validate conditions for used algorithms

	unsigned public_count = 0;

	for (int i = 0; i < max_algorithms; i++) {
		algorithm_usage_t *u = &usage[i];
		if (u->is_public) {
			public_count += 1;
			if (!u->is_ksk_active || !u->is_zsk_active) {
				return KNOT_DNSSEC_EMISSINGKEYTYPE;
			}
		}
	}

	if (public_count == 0) {
		return KNOT_DNSSEC_ENOKEY;
	}

	return KNOT_EOK;
}

/*!
 * \brief Load private keys for active keys.
 */
static int load_private_keys(dnssec_keystore_t *keystore, zone_keyset_t *keyset)
{
	assert(keystore);
	assert(keyset);

	for (size_t i = 0; i < keyset->count; i++) {
		if (!keyset->keys[i].is_active) {
			continue;
		}

		dnssec_key_t *key = keyset->keys[i].key;
		int r = dnssec_key_import_private_keystore(key, keystore);
		if (r != DNSSEC_EOK && r != DNSSEC_KEY_ALREADY_PRESENT) {
			return r;
		}
	}

	return DNSSEC_EOK;
}

/*!
 * \brief Log information about zone keys.
 */
static void log_key_info(const zone_key_t *key, const char *zone_name)
{
	assert(key);
	assert(zone_name);

	log_zone_str_info(zone_name, "DNSSEC, loaded key, tag %5d, "
			  "algorithm %d, KSK %s, ZSK %s, public %s, active %s",
			  dnssec_key_get_keytag(key->key),
			  dnssec_key_get_algorithm(key->key),
			  key->is_ksk ? "yes" : "no",
			  key->is_zsk ? "yes" : "no",
			  key->is_public ? "yes" : "no",
			  key->is_active ? "yes" : "no");
}

/*!
 * \brief Load zone keys and init cryptographic context.
 */
int load_zone_keys(dnssec_kasp_zone_t *zone, dnssec_keystore_t *store,
                   bool nsec3_enabled, time_t now, zone_keyset_t *keyset_ptr)
{
	if (!zone || !store || !keyset_ptr) {
		return KNOT_EINVAL;
	}

	zone_keyset_t keyset = { 0 };
	const char *zone_name = dnssec_kasp_zone_get_name(zone);

	dnssec_list_t *kasp_keys = dnssec_kasp_zone_get_keys(zone);
	if (dnssec_list_is_empty(kasp_keys)) {
		log_zone_str_error(zone_name, "DNSSEC, no keys are available");
		return KNOT_DNSSEC_ENOKEY;
	}

	keyset.count = dnssec_list_size(kasp_keys);
	keyset.keys = calloc(keyset.count, sizeof(zone_key_t));
	if (!keyset.keys) {
		free_zone_keys(&keyset);
		return KNOT_ENOMEM;
	}

	size_t i = 0;
	dnssec_list_foreach(item, kasp_keys) {
		dnssec_kasp_key_t *kasp_key = dnssec_item_get(item);
		set_key(kasp_key, now, &keyset.keys[i]);
		log_key_info(&keyset.keys[i], zone_name);
		i += 1;
	}
	assert(i == keyset.count);

	int r = prepare_and_check_keys(zone_name, nsec3_enabled, &keyset);
	if (r != KNOT_EOK) {
		log_zone_str_error(zone_name, "DNSSEC, keys validation failed (%s)",
		                   knot_strerror(r));
		free_zone_keys(&keyset);
		return r;
	}

	r = load_private_keys(store, &keyset);
	if (r != KNOT_EOK) {
		log_zone_str_error(zone_name, "DNSSEC, failed to load private "
		                   "keys (%s)", knot_strerror(r));
		free_zone_keys(&keyset);
		return r;
	}

	*keyset_ptr = keyset;

	return DNSSEC_EOK;
}

/*!
 * \brief Free structure with zone keys and associated DNSSEC contexts.
 */
void free_zone_keys(zone_keyset_t *keyset)
{
	if (!keyset) {
		return;
	}

	for (size_t i = 0; i < keyset->count; i++) {
		dnssec_sign_free(keyset->keys[i].ctx);
	}

	free(keyset->keys);

	memset(keyset, '\0', sizeof(*keyset));
}

/*!
 * \brief Get zone key by a keytag.
 */
const zone_key_t *get_zone_key(const zone_keyset_t *keyset, uint16_t search)
{
	if (!keyset) {
		return NULL;
	}

	for (size_t i = 0; i < keyset->count; i++) {
		zone_key_t *key = &keyset->keys[i];
		if (dnssec_key_get_keytag(key->key) == search) {
			return key;
		}
	}

	return NULL;
}

/*!
 * \brief Get timestamp of next key event.
 */
time_t knot_get_next_zone_key_event(const zone_keyset_t *keyset)
{
	time_t result = LONG_MAX;

	for (size_t i = 0; i < keyset->count; i++) {
		zone_key_t *key = &keyset->keys[i];
		if (key->next_event < result) {
			result = key->next_event;
		}
	}

	return result;
}
