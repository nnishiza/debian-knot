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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "binary.h"
#include "error.h"
#include "kasp.h"
#include "kasp/dir/json.h"
#include "kasp/zone.h"
#include "key/internal.h"
#include "list.h"
#include "shared.h"

/* -- key parameters ------------------------------------------------------- */

/*!
 * Key parameters as writting in zone config file.
 */
struct key_params {
	char *id;
	uint16_t keytag;
	uint8_t algorithm;
	dnssec_binary_t public_key;
	bool is_ksk;
	dnssec_kasp_key_timing_t timing;
};

typedef struct key_params key_params_t;

/*!
 * Free allocated key parameters and clear the structure.
 */
static void key_params_free(key_params_t *params)
{
	assert(params);

	free(params->id);
	dnssec_binary_free(&params->public_key);

	clear_struct(params);
}

#define _cleanup_key_params_ _cleanup_(key_params_free)

static const encode_attr_t KEY_ATTRIBUTES[] = {
	#define off(member) offsetof(key_params_t, member)
	{ "id",         off(id),             encode_keyid,  decode_keyid  },
	{ "keytag",     off(keytag),         encode_uint16, decode_ignore },
	{ "algorithm",  off(algorithm),      encode_uint8,  decode_uint8  },
	{ "public_key", off(public_key),     encode_binary, decode_binary },
	{ "ksk",        off(is_ksk),         encode_bool,   decode_bool   },
	{ "created",    off(timing.created), encode_time,   decode_time   },
	{ "publish",    off(timing.publish), encode_time,   decode_time   },
	{ "active",     off(timing.active),  encode_time,   decode_time   },
	{ "retire",     off(timing.retire),  encode_time,   decode_time   },
	{ "remove",     off(timing.remove),  encode_time,   decode_time   },
	{ 0 }
	#undef off
};

/* -- configuration loading ------------------------------------------------ */

/*!
 * Check if key parameters allow to create a key.
 */
static int key_params_check(key_params_t *params)
{
	assert(params);

	if (params->algorithm == 0) {
		return DNSSEC_INVALID_KEY_ALGORITHM;
	}

	if (params->public_key.size == 0) {
		return DNSSEC_NO_PUBLIC_KEY;
	}

	return DNSSEC_EOK;
}

/*!
 * Create DNSKEY from parameters.
 */
static int create_dnskey(const uint8_t *dname, key_params_t *params,
			 dnssec_key_t **key_ptr)
{
	assert(dname);
	assert(params);
	assert(key_ptr);

	int result = key_params_check(params);
	if (result != DNSSEC_EOK) {
		return result;
	}

	// create key

	dnssec_key_t *key = NULL;
	result = dnssec_key_new(&key);
	if (result != DNSSEC_EOK) {
		return result;
	}

	// set key parameters

	result = dnssec_key_set_dname(key, dname);
	if (result != DNSSEC_EOK) {
		dnssec_key_free(key);
		return result;
	}

	dnssec_key_set_algorithm(key, params->algorithm);

	uint16_t flags = dnskey_flags(params->is_ksk);
	dnssec_key_set_flags(key, flags);

	result = dnssec_key_set_pubkey(key, &params->public_key);
	if (result != DNSSEC_EOK) {
		dnssec_key_free(key);
		return result;
	}

	// validate key ID

	const char *key_id = dnssec_key_get_id(key);
	if (!key_id) {
		dnssec_key_free(key);
		return DNSSEC_INVALID_PUBLIC_KEY;
	}

	if (!dnssec_keyid_equal(params->id, key_id)) {
		dnssec_key_free(key);
		return DNSSEC_INVALID_KEY_ID;
	}

	*key_ptr = key;

	return DNSSEC_EOK;
}

/*!
 * Add DNSKEY into a keyset.
 */
static int keyset_add_dnskey(dnssec_list_t *keyset, dnssec_key_t *dnskey,
			     const dnssec_kasp_key_timing_t *timing)
{
	dnssec_kasp_key_t *kasp_key = malloc(sizeof(*kasp_key));
	if (!kasp_key) {
		return DNSSEC_ENOMEM;
	}

	clear_struct(kasp_key);
	kasp_key->key = dnskey;
	kasp_key->timing = *timing;

	int result = dnssec_list_append(keyset, kasp_key);
	if (result != DNSSEC_EOK) {
		free(kasp_key);
	}

	return result;
}

/*!
 * Load zone keys.
 */
static int load_zone_keys(const uint8_t *zone, dnssec_list_t **list_ptr, json_t *keys)
{
	if (!keys || !json_is_array(keys)) {
		return DNSSEC_CONFIG_MALFORMED;
	}

	dnssec_list_t *new_keys = dnssec_list_new();
	if (!new_keys) {
		return DNSSEC_ENOMEM;
	}

	int result = DNSSEC_EOK;

	int index;
	json_t *key;
	json_array_foreach(keys, index, key) {
		_cleanup_key_params_ key_params_t params = { 0 };

		result = decode_object(KEY_ATTRIBUTES, key, &params);
		if (result != DNSSEC_EOK) {
			break;
		}

		dnssec_key_t *dnskey = NULL;
		result = create_dnskey(zone, &params, &dnskey);
		if (result != DNSSEC_EOK) {
			break;
		}

		result = keyset_add_dnskey(new_keys, dnskey, &params.timing);
		if (result != DNSSEC_EOK) {
			dnssec_key_free(dnskey);
			break;
		}
	}

	if (result != DNSSEC_EOK) {
		kasp_zone_keys_free(new_keys);
		return result;
	}

	*list_ptr = new_keys;
	return DNSSEC_EOK;
}

/*!
 * Convert KASP key to serializable parameters.
 */
static void key_to_params(dnssec_kasp_key_t *key, key_params_t *params)
{
	assert(key);
	assert(params);

	params->id = (char *)dnssec_key_get_id(key->key);
	params->keytag = dnssec_key_get_keytag(key->key);
	dnssec_key_get_pubkey(key->key, &params->public_key);
	params->algorithm = dnssec_key_get_algorithm(key->key);
	params->is_ksk = dnssec_key_get_flags(key->key) == DNSKEY_FLAGS_KSK;
	params->timing = key->timing;
}

/*!
 * Convert KASP keys to JSON array.
 */
static int export_zone_keys(dnssec_kasp_zone_t *zone, json_t **keys_ptr)
{
	json_t *keys = json_array();
	if (!keys) {
		return DNSSEC_ENOMEM;
	}

	dnssec_list_foreach(item, zone->keys) {
		dnssec_kasp_key_t *kasp_key = dnssec_item_get(item);
		key_params_t params = { 0 };
		key_to_params(kasp_key, &params);

		json_t *key = NULL;
		int r = encode_object(KEY_ATTRIBUTES, &params, &key);
		if (r != DNSSEC_EOK) {
			json_decref(keys);
			return r;
		}

		if (json_array_append_new(keys, key)) {
			json_decref(key);
			json_decref(keys);
			return DNSSEC_ENOMEM;
		}
	}

	*keys_ptr = keys;

	return DNSSEC_EOK;
}

static int export_zone_config(dnssec_kasp_zone_t *zone, json_t **config_ptr)
{
	assert(zone);
	assert(config_ptr);

	_json_cleanup_ json_t *keys = NULL;
	int r = export_zone_keys(zone, &keys);
	if (r != DNSSEC_EOK) {
		return r;
	}

	_json_cleanup_ json_t *policy = zone->policy ? json_string(zone->policy) : json_null();
	if (!policy) {
		return DNSSEC_ENOMEM;
	}

	json_t *config = json_pack("{sOsO}", "policy", policy, "keys", keys);
	if (!config) {
		return DNSSEC_ENOMEM;
	}

	*config_ptr = config;
	return DNSSEC_EOK;
}

static int parse_zone_config(dnssec_kasp_zone_t *zone, json_t *config)
{
	assert(zone);
	assert(config);

	// get policy

	char *policy = NULL;
	json_t *json_policy = json_object_get(config, "policy");
	if (json_policy && !json_is_null(json_policy)) {
		policy = strdup(json_string_value(json_policy));
		if (!policy) {
			return DNSSEC_ENOMEM;
		}
	}

	// get keys

	dnssec_list_t *keys = NULL;
	json_t *json_keys = json_object_get(config, "keys");
	int r = load_zone_keys(zone->dname, &keys, json_keys);
	if (r != DNSSEC_EOK) {
		free(policy);
		return r;
	}

	// store the result

	zone->policy = policy;
	kasp_zone_keys_free(zone->keys);
	zone->keys = keys;

	return DNSSEC_EOK;
}

/* -- internal API --------------------------------------------------------- */

/*!
 * Load zone configuration.
 */
int load_zone_config(dnssec_kasp_zone_t *zone, const char *filename)
{
	assert(zone);
	assert(filename);

	_cleanup_fclose_ FILE *file = fopen(filename, "r");
	if (!file) {
		return DNSSEC_NOT_FOUND;
	}

	json_error_t error = { 0 };
	_json_cleanup_ json_t *config = json_loadf(file, JSON_LOAD_OPTIONS, &error);
	if (!config) {
		return DNSSEC_CONFIG_MALFORMED;
	}

	return parse_zone_config(zone, config);
}

/*!
 * Save zone configuration.
 */
int save_zone_config(dnssec_kasp_zone_t *zone, const char *filename)
{
	assert(zone);
	assert(filename);

	_json_cleanup_ json_t *config = NULL;
	int r = export_zone_config(zone, &config);
	if (r != DNSSEC_EOK) {
		return r;
	}

	_cleanup_fclose_ FILE *file = fopen(filename, "w");
	if (!file) {
		return DNSSEC_NOT_FOUND;
	}

	r = json_dumpf(config, file, JSON_DUMP_OPTIONS);
	if (r != DNSSEC_EOK) {
		return r;
	}

	fputc('\n', file);
	return DNSSEC_EOK;
}
