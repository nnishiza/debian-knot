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

#include <stdio.h>
#include <tap/basic.h>

#include "knot/conf/conf.h"

/* Resources. */
#include "sample_conf.h"

/*! Run all scheduled tests for given parameters.
 */
int main(int argc, char *argv[])
{
	plan(19);

	// Test 1: Allocate new config
	const char *config_fn = "rc:/sample_conf";
	conf_t *conf = conf_new(strdup(config_fn));
	ok(conf != 0, "config_new()");

	// Test 2: Parse config
	int ret = conf_parse_str(conf, sample_conf_rc);
	is_int(0, ret, "parsing configuration file %s", config_fn);
	if (ret != 0) {
		skip_block(19, "Parse err");
		goto skip_all;
	}

	// Test 3: Test server version (0-level depth)
	is_string("Infinitesimal", conf->version, "server version loaded ok");

	// Test 4: Test interfaces (1-level depth)
	ok(!EMPTY_LIST(conf->ifaces), "configured interfaces exist");

	// Test 5,6: Interfaces content (2-level depth)
	struct node *n = HEAD(conf->ifaces);
	conf_iface_t *iface = (conf_iface_t*)n;
	struct sockaddr_storage addr_ref;
	sockaddr_set(&addr_ref, AF_INET, "10.10.1.1", 53531);
	is_int(0, sockaddr_cmp(&iface->addr, &addr_ref), "interface0 address check");

	n = n->next;
	iface = (conf_iface_t*)n;
	sockaddr_set(&addr_ref, AF_INET6, "::0", 53);
	is_int(0, sockaddr_cmp(&iface->addr, &addr_ref), "interface1 address check");

	// Test 9,10: Check server key
	if (EMPTY_LIST(conf->keys)) {
		ok(0, "TSIG key algorithm check - NO KEY FOUND");
		ok(0, "TSIG key secret check - NO KEY FOUND");
	} else {
		knot_tsig_key_t *k = &((conf_key_t *)HEAD(conf->keys))->k;
		uint8_t decoded_secret[] = { 0x5a };

		ok(k->algorithm == KNOT_TSIG_ALG_HMAC_MD5,
		       "TSIG key algorithm check");
		ok(k->secret.size == sizeof(decoded_secret)
		   && memcmp(k->secret.data, decoded_secret,
			     sizeof(decoded_secret)) == 0,
		   "TSIG key secret check");
	}

	// Test 11,12,13,14,15,16,17,18: Check logging facilities
	ok(list_size(&conf->logs) == 4, "log facilites count check");
	n = HEAD(conf->logs);
	ok(!EMPTY_LIST(conf->logs), "log facilities not empty");

	conf_log_t *log = (conf_log_t*)n;
	node_t *nm = HEAD(log->map);
	conf_log_map_t *m = (conf_log_map_t*)nm;
	ok(log->type == LOGT_SYSLOG, "log0 is syslog");

	if (EMPTY_LIST(log->map)) {
		skip_block(5, "Empty list");
	} else {
		ok(m->source == LOG_ANY, "syslog first rule is ANY");
		int mask = LOG_UPTO(LOG_NOTICE);
		ok(m->prios == mask, "syslog mask is equal");
		nm = nm->next;
		m = (conf_log_map_t*)nm;
		ok(m != 0, "syslog has more than 1 rule");
		if (m == 0) {
			skip_block(2, "No mapping");
		} else {
			ok(m->source == LOG_ZONE, "syslog next rule is for zone");
			ok(m->prios == LOG_UPTO(LOG_INFO), "rule for zone is: info level");
		}
	}

	// Test 19,20: File facility checks
	n = n->next;
	log = (conf_log_t*)n;
	ok(n != 0, "log has next facility");
	if (n == 0) {
		skip("No mapping");
	} else {
		is_string("/var/log/knot/server.err", log->file, "log file matches");
	}

	// Test 21: Load key dname
	const char *sample_str = "key0.example.net";
	knot_dname_t *sample = knot_dname_from_str_alloc(sample_str);
	if (list_size(&conf->keys) > 0) {
		knot_tsig_key_t *k = &((conf_key_t *)HEAD(conf->keys))->k;
		ok(knot_dname_cmp(sample, k->name) == 0,
		   "TSIG key dname check");
	} else {
		ok(0, "TSIG key dname check - NO KEY FOUND");
	}
	knot_dname_free(&sample, NULL);

skip_all:

	// Deallocating config
	conf_free(conf);

	return 0;
}
