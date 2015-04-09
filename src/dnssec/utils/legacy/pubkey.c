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

// Knot zone scanner
#include "zscanner/scanner.h"

#include "dnssec/error.h"
#include "dnssec/binary.h"
#include "dnssec/key.h"

#include "legacy/pubkey.h"

#define CLASS_IN 1
#define RTYPE_DNSKEY 48

/*!
 * Parse DNSKEY record.
 *
 * \todo Currently, the function waits for the first DNSKEY record, and skips
 * the others. We should be more strict and report other records as errors.
 * However, there is currently no API to stop the scanner.
 */
static void parse_record(zs_scanner_t *scanner)
{
	assert(scanner);
	assert(scanner->data);

	dnssec_key_t *key = scanner->data;

	if (dnssec_key_get_dname(key) != NULL) {
		// skip till the the parser finishes
		return;
	}

	if (scanner->r_type != RTYPE_DNSKEY) {
		// should report error
		return;
	}

	dnssec_binary_t rdata = {
		.data = scanner->r_data,
		.size = scanner->r_data_length
	};
	dnssec_key_set_dname(key, scanner->dname);
	dnssec_key_set_rdata(key, &rdata);
}

int legacy_pubkey_parse(const char *filename, dnssec_key_t **key_ptr)
{
	assert(filename);
	assert(key_ptr);

	dnssec_key_t *key = NULL;
	int result = dnssec_key_new(&key);
	if (result != DNSSEC_EOK) {
		return result;
	}

	uint16_t cls = CLASS_IN;
	uint32_t ttl = 0;
	zs_scanner_t *scanner = zs_scanner_create(".", cls, ttl,
						  parse_record, NULL, key);
	if (!scanner) {
		dnssec_key_free(key);
		return DNSSEC_NOT_FOUND;
	}

	result = zs_scanner_parse_file(scanner, filename);
	zs_scanner_free(scanner);
	if (result != 0 || dnssec_key_get_dname(key) == NULL) {
		dnssec_key_free(key);
		return DNSSEC_INVALID_PUBLIC_KEY;
	}

	*key_ptr = key;
	return DNSSEC_EOK;
}
