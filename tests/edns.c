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

#include <tap/basic.h>

#include <assert.h>
#include "libknot/libknot.h"
#include "libknot/rrtype/opt.h"
#include "libknot/descriptor.h"
#include "libknot/internal/sockaddr.h"

static const uint16_t E_MAX_PLD = 10000;
static const uint16_t E_MAX_PLD2 = 20000;
static const uint8_t E_VERSION = 1;
static const uint8_t E_VERSION2 = 2;
static const uint8_t E_RCODE = 0;
static const uint8_t E_RCODE2 = 200;

static const char *E_NSID_STR = "FooBar";
static const uint16_t E_NSID_LEN = 6;

#define E_NSID_SIZE (4 + E_NSID_LEN)

static const uint16_t E_OPT3_CODE = 15;
static const char *E_OPT3_FAKE_DATA = "Not used";
static const char *E_OPT3_DATA = NULL;
static const uint16_t E_OPT3_LEN = 0;
static const uint16_t E_OPT3_FAKE_LEN = 8;

#define E_OPT3_SIZE (4 + E_OPT3_LEN)

static const uint16_t E_OPT4_CODE = 30;
static const char *E_OPT4_DATA = NULL;
static const uint16_t E_OPT4_LEN = 0;

#define E_OPT4_SIZE (4 + E_OPT4_LEN)

enum offsets {
	/*! \brief Offset of Extended RCODE in wire order of TTL. */
	OFFSET_ERCODE = 0,
	/*! \brief Offset of Version in wire order of TTL. */
	OFFSET_VER = 1,
	/*! \brief Offset of Flags in wire order of TTL. */
	OFFSET_FLAGS = 2,
	/*! \brief Offset of OPTION code in one OPTION in RDATA. */
	OFFSET_OPT_CODE = 0,
	/*! \brief Offset of OPTION size in one OPTION in RDATA. */
	OFFSET_OPT_SIZE = 2,
	/*! \brief Offset of OPTION data in one OPTION in RDATA. */
	OFFSET_OPT_DATA = 4
};

static bool check_ttl(knot_rdata_t *rdata, uint8_t ext_rcode, uint8_t ver,
                      uint16_t flags, char *msg, int *done)
{
	/* TTL should be stored in machine byte order.
	   We need network byte order to compare its parts. */
	uint8_t ttl_wire[4] = { 0, 0, 0, 0 };
	wire_write_u32(ttl_wire, knot_rdata_ttl(rdata));

	/* Convert Flags from EDNS parameters to wire format for comparison. */
	uint8_t flags_wire[2] = { 0, 0 };
	wire_write_u16(flags_wire, flags);

	bool success = true;

	/* TTL = Ext RCODE + Version + Flags */
	bool check = (ttl_wire[OFFSET_ERCODE] == ext_rcode);
	ok(check, "%s: extended RCODE", msg);
	success &= check;
	(*done)++;

	check = (ttl_wire[OFFSET_VER] == ver);
	ok(check, "%s: version", msg);
	success &= check;
	(*done)++;

	check = (memcmp(flags_wire, ttl_wire + OFFSET_FLAGS, 2) == 0);
	ok(check, "%s: flags", msg);
	success &= check;
	(*done)++;

	return success;
}

static bool check_option(knot_rdata_t *rdata, uint16_t opt_code,
                         uint16_t opt_len, uint8_t *opt_data, char *msg,
                         int *done)
{
	assert(rdata != NULL);
	bool success = true;

	uint8_t *data = knot_rdata_data(rdata);
	uint16_t data_len = knot_rdata_rdlen(rdata);

	/* Check RDLENGTH according to given data length. */
	bool check = (data_len >= 4 + opt_len);
	ok(check, "%s: RDLENGTH (%u)", msg, data_len);
	success &= check;
	(*done)++;

	/* Find the desired option. */
	bool found = false;
	int pos = 0;
	while (pos <= data_len - 4) {
		uint16_t code = wire_read_u16(data + pos + OFFSET_OPT_CODE);
		if (code == opt_code) {
			found = true;
			break;
		}
		uint16_t len = wire_read_u16(data + pos + OFFSET_OPT_SIZE);
		pos += 4 + len;
	}

	/* Check that the option is present. */
	ok(found, "%s: find OPTION %u in OPT RR", msg, opt_code);
	success &= found;
	(*done)++;

	/* Check that the first OPTION's size si the size of the option data. */
	uint16_t opt_size = wire_read_u16(data + pos + OFFSET_OPT_SIZE);
	check = (opt_size == opt_len);
	ok(check, "%s: OPTION data size", msg);
	success &= check;
	(*done)++;

	/* Check the actual NSID data. */
	check = (memcmp(data + pos + OFFSET_OPT_DATA, opt_data, opt_len)) == 0;
	ok(check, "%s: OPTION data", msg);
	success &= check;
	(*done)++;

	return success;
}

static bool check_header(knot_rrset_t *opt_rr, uint16_t payload, uint8_t ver,
                         uint16_t flags, uint8_t ext_rcode, char *msg,
                         int *done)
{
	assert(opt_rr != NULL);
	assert(done != NULL);
	bool check;
	bool success = true;

	/* Check values in OPT RR by hand. */
	/* CLASS == Max UDP payload */
	check = (opt_rr->rclass == payload);
	ok(check, "%s: max payload", msg);
	success &= check;
	(*done)++;

	/* The OPT RR should have exactly one RDATA. */
	check = (opt_rr->rrs.rr_count == 1);
	ok(check, "%s: RR count == 1", msg);
	success &= check;
	(*done)++;

	knot_rdata_t *rdata = knot_rdataset_at(&opt_rr->rrs, 0);
	check = (rdata != NULL);
	ok(check, "%s: RDATA exists", msg);
	success &= check;
	(*done)++;

	success &= check_ttl(rdata, ext_rcode, ver, flags, msg, done);

	return success;
}

static bool test_getters(knot_rrset_t *opt_rr, int *done)
{
	assert(opt_rr != NULL);
	assert(done != NULL);
	bool success = true;

	/* These values should be set from the setters test:
	 * Max UDP payload: E_MAX_PLD2
	 * Version:         E_VERSION2
	 * RCODE:           E_RCODE2
	 * Flags:           E_FLAGS | KNOT_EDNS_FLAG_DO
	 * OPTIONs:         1) KNOT_EDNS_OPTION_NSID, E_NSID_LEN, E_NSID_STR
	 *                  2) E_OPT3_CODE, 0, 0
	 *                  3) E_OPT4_CODE, 0, 0
	 */

	/* Payload */
	bool check = (knot_edns_get_payload(opt_rr) == E_MAX_PLD2);
	ok(check, "OPT RR getters: payload");
	success &= check;
	(*done)++;

	/* Extended RCODE */
	check = (knot_edns_get_ext_rcode(opt_rr) == E_RCODE2);
	ok(check, "OPT RR getters: extended RCODE");
	success &= check;
	(*done)++;

	/* Extended RCODE */
	check = (knot_edns_get_version(opt_rr) == E_VERSION2);
	ok(check, "OPT RR getters: version");
	success &= check;
	(*done)++;

	/* DO bit */
	check = knot_edns_do(opt_rr);
	ok(check, "OPT RR getters: DO bit check");
	success &= check;
	(*done)++;

	/* Wire size */
	size_t total_size = KNOT_EDNS_MIN_SIZE
	                    + E_NSID_SIZE + E_OPT3_SIZE + E_OPT4_SIZE;
	size_t actual_size = knot_edns_wire_size(opt_rr);
	check = actual_size == total_size;
	ok(check, "OPT RR getters: wire size (expected: %zu, actual: %zu)",
	   total_size, actual_size);
	success &= check;
	(*done)++;

	/* NSID */
	check = knot_edns_has_option(opt_rr, KNOT_EDNS_OPTION_NSID);
	ok(check, "OPT RR getters: NSID check");
	success &= check;
	(*done)++;

	/* Other OPTIONs */
	check = knot_edns_has_option(opt_rr, E_OPT3_CODE);
	ok(check, "OPT RR getters: empty option 1");
	success &= check;
	(*done)++;

	check = knot_edns_has_option(opt_rr, E_OPT4_CODE);
	ok(check, "OPT RR getters: empty option 2");
	success &= check;
	(*done)++;

	return success;
}

static bool test_setters(knot_rrset_t *opt_rr, int *done)
{
	assert(opt_rr != NULL);
	assert(done != NULL);

	/* Header-related setters. */
	knot_edns_set_payload(opt_rr, E_MAX_PLD2);
	knot_edns_set_ext_rcode(opt_rr, E_RCODE2);
	knot_edns_set_version(opt_rr, E_VERSION2);
	knot_edns_set_do(opt_rr);

	bool success = true;
	bool check = check_header(opt_rr, E_MAX_PLD2, E_VERSION2,
	                          KNOT_EDNS_FLAG_DO, E_RCODE2,
	                          "OPT RR setters", done);
	success &= check;

	/* OPTION(RDATA)-related setters. */

	/* Proper option. */
	int ret = knot_edns_add_option(opt_rr, KNOT_EDNS_OPTION_NSID,
	                           E_NSID_LEN, (uint8_t *)E_NSID_STR, NULL);
	ok(ret == KNOT_EOK, "OPT RR setters: add option with data (ret = %s)",
	   knot_strerror(ret));
	(*done)++;

	/* Wrong argument: no OPT RR. */
	ret = knot_edns_add_option(NULL, E_OPT3_CODE, E_OPT3_FAKE_LEN,
	                           (uint8_t *)E_OPT3_FAKE_DATA, NULL);
	ok(ret == KNOT_EINVAL, "OPT RR setters: add option (rr == NULL) "
	   "(ret = %s)", knot_strerror(ret));
	(*done)++;

	/* Wrong argument: option length != 0 && data == NULL. */
	ret = knot_edns_add_option(opt_rr, E_OPT3_CODE, E_OPT3_FAKE_LEN, NULL,
	                           NULL);
	ok(ret == KNOT_EINVAL, "OPT RR setters: add option (data == NULL, "
	   "len != 0) (ret = %s)", knot_strerror(ret));
	(*done)++;

	/* Empty OPTION (length 0, data != NULL). */
	ret = knot_edns_add_option(opt_rr, E_OPT3_CODE, E_OPT3_LEN,
	                           (uint8_t *)E_OPT3_FAKE_DATA, NULL);
	ok(ret == KNOT_EOK, "OPT RR setters: add empty option 1 (ret = %s)",
	   knot_strerror(ret));
	(*done)++;

	/* Empty OPTION (length 0, data == NULL). */
	ret = knot_edns_add_option(opt_rr, E_OPT4_CODE, E_OPT4_LEN,
	                           (uint8_t *)E_OPT4_DATA, NULL);
	ok(ret == KNOT_EOK, "OPT RR setters: add empty option 2 (ret = %s)",
	   knot_strerror(ret));
	(*done)++;

	knot_rdata_t *rdata = knot_rdataset_at(&opt_rr->rrs, 0);
	if (rdata == NULL) {
		skip_block(2, "No RDATA in OPT RR.");
		return false;
	}

	/* Check proper option */
	check = check_option(rdata, KNOT_EDNS_OPTION_NSID, E_NSID_LEN,
	                     (uint8_t *)E_NSID_STR,
	                     "OPT RR setters (proper option)", done);
	success &= check;

	/* Check empty option 1 */
	check = check_option(rdata, E_OPT3_CODE, E_OPT3_LEN,
	                     (uint8_t *)E_OPT3_DATA,
	                     "OPT RR setters (empty option 1)", done);
	success &= check;

	/* Check empty option 2 */
	check = check_option(rdata, E_OPT4_CODE, E_OPT4_LEN,
	                     (uint8_t *)E_OPT4_DATA,
	                     "OPT RR setters (empty option 2)", done);
	success &= check;

	return success;
}

static void test_client_subnet()
{
	int ret;
	knot_addr_family_t family;
	uint8_t  addr[IPV6_PREFIXLEN / 8] = { 0 };
	uint16_t addr_len = sizeof(addr);
	uint8_t  src_mask, dst_mask;
	uint8_t  data[KNOT_EDNS_MAX_OPTION_CLIENT_SUBNET] = { 0 };
	uint16_t data_len = sizeof(data);

	/* Create IPv4 subnet - src mask 32  */
	family = KNOT_ADDR_FAMILY_IPV4;
	data_len = sizeof(data);
	addr_len = 4;
	memcpy(&addr, "\xFF\xFF\xFF\xFF", 4);
	src_mask = 32;
	dst_mask = 32;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 32)");
	ok(data_len == 8, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x01\x20\x20\xFF\xFF\xFF\xFF", 8) == 0,
           "EDNS-client-subnet: create (cmp out)");

	/* Create IPv4 subnet - src mask 31  */
	data_len = sizeof(data);
	src_mask = 31;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 31)");
	ok(data_len == 8, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x01\x1F\x20\xFF\xFF\xFF\xFE", 8) == 0,
           "EDNS-client-subnet: create (cmp out)");

	/* Create IPv4 subnet - src mask 7  */
	data_len = sizeof(data);
	src_mask = 7;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 7)");
	ok(data_len == 5, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x01\x07\x20\xFE", 5) == 0,
           "EDNS-client-subnet: create (cmp out)");

	/* Create IPv4 subnet - src mask 0  */
	data_len = sizeof(data);
	src_mask = 0;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 0)");
	ok(data_len == 4, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x01\x00\x20", 0) == 0,
           "EDNS-client-subnet: create (cmp out)");

	/* Create IPv6 subnet - src mask 128  */
	data_len = sizeof(data);
	family = KNOT_ADDR_FAMILY_IPV6;
	addr_len = 16;
	memcpy(&addr, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 16);
	src_mask = 128;
	dst_mask = 128;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 128)");
	ok(data_len == 20, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x02\x80\x80\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",
           20) == 0, "EDNS-client-subnet: create (cmp out)");

	/* Create IPv6 subnet - src mask 1  */
	data_len = sizeof(data);
	family = KNOT_ADDR_FAMILY_IPV6;
	addr_len = 1;
	memcpy(&addr, "\xFF", 1);
	src_mask = 1;
	ret = knot_edns_client_subnet_create(family, addr, addr_len, src_mask,
                                             dst_mask, data, &data_len);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: create (src mask 1)");
	ok(data_len == 5, "EDNS-client-subnet: create (cmp out length)");
	ok(memcmp(data, "\x00\x02\x01\x80\x80",
           5) == 0, "EDNS-client-subnet: create (cmp out)");

	/* Parse IPv4 subnet - src mask 31  */
	memcpy(&data, "\x00\x01\x1F\x20\xFF\xFF\xFF\xFE", 8);
	data_len = 8;
	addr_len = sizeof(addr);
	ret = knot_edns_client_subnet_parse(data, data_len, &family, addr,
                                            &addr_len, &src_mask, &dst_mask);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: parse (src mask 31)");
	ok(family == KNOT_ADDR_FAMILY_IPV4,
           "EDNS-client-subnet: parse (cmp family)");
	ok(src_mask == 31, "EDNS-client-subnet: parse (cmp src mask)");
	ok(dst_mask == 32, "EDNS-client-subnet: parse (cmp dst mask)");
	ok(addr_len == 4, "EDNS-client-subnet: parse (cmp addr length)");
	ok(memcmp(addr, "\xFF\xFF\xFF\xFE", 4) == 0,
           "EDNS-client-subnet: parse (cmp addr)");

	/* Parse IPv6 subnet - src mask 1  */
	memcpy(&data, "\x00\x02\x01\x80\x80", 5);
	data_len = 5;
	addr_len = sizeof(addr);
	ret = knot_edns_client_subnet_parse(data, data_len, &family, addr,
                                            &addr_len, &src_mask, &dst_mask);
	ok(ret == KNOT_EOK, "EDNS-client-subnet: parse (src mask 1)");
	ok(family == KNOT_ADDR_FAMILY_IPV6,
           "EDNS-client-subnet: parse (cmp family)");
	ok(src_mask == 1, "EDNS-client-subnet: parse (cmp src mask)");
	ok(dst_mask == 128, "EDNS-client-subnet: parse (cmp dst mask)");
	ok(addr_len == 1, "EDNS-client-subnet: parse (cmp addr length)");
	ok(memcmp(addr, "\x80", 1) == 0,
           "EDNS-client-subnet: parse (cmp addr)");
}

#define TEST_COUNT 68

static inline int remaining(int done) {
	return TEST_COUNT - done;
}

int main(int argc, char *argv[])
{
	plan(TEST_COUNT);
	int done = 0;

	knot_rrset_t opt_rr;
	int ret = knot_edns_init(&opt_rr, E_MAX_PLD, E_RCODE, E_VERSION, NULL);

	ok(ret == KNOT_EOK, "OPT RR: init");
	done++;

	/* Check initialized values (no NSID yet). */
	bool success = check_header(&opt_rr, E_MAX_PLD, E_VERSION, 0, E_RCODE,
	                            "OPT RR: check header", &done);
	if (!success) {
		skip_block(remaining(done), "OPT RR not initialized properly");
		goto exit;
	}

	/* Setters */
	success = test_setters(&opt_rr, &done);

	if (!success) {
		skip_block(remaining(done), "OPT RR: setters error");
		goto exit;
	}

	/* Getters
	   Note: NULL parameters are not supported, so no test for that. */
	success = test_getters(&opt_rr, &done);

	if (!success) {
		skip_block(remaining(done), "OPT RR: getters error");
		goto exit;
	}

	/* EDNS client subnet */
	test_client_subnet();
exit:
	knot_rrset_clear(&opt_rr, NULL);

	return 0;
}
