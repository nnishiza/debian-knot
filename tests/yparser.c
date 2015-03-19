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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <tap/basic.h>

#include "libknot/yparser/yparser.h"
#include "libknot/libknot.h"

const char *syntax_ok =
	"a:\n"
	"a :\n"
	"a : #comment\n"
	"\n"
	"b: \"b\"\n"
	"b: b #comment\n"
	"b : b\n"
	"b: [ b] # comment\n"
	"b: [b ]\n"
	"b: [ b ]\n"
	"\n"
	" f: \"f\"\n"
	" f: f #comment\n"
	" f : f\n"
	" f: [ f] # comment\n"
	" f: [f ]\n"
	" f: [ f ]\n"
	"\n"
	"c: [a,b]\n"
	"c: [a, b]\n"
	"c: [a ,b]\n"
	"c: [a , b]\n"
	"c: [ a , b ]\n"
	"\n"
	"- d: d\n"
	"- d : d # comment\n"
	"\n"
	"e: \"a#b' c[d,]\"\n"
	"\n"
	"zone:\n"
	"  -   domain: example.\n"
	"      master: bind\n"
	"  - domain: example.\n"
	"    master: bind\n";

const char *syntax_error1 =
	"f:\n"
	"  -  a: b\n"
	"   - b: c\n";

const char *syntax_error2 =
	"f:\n"
	"  -  a: b\n"
	"      c: d\n";

const char *syntax_error3 =
	"f:\n"
	"   a: b\n"
	"  c: d\n";

int main(int argc, char *argv[])
{
	plan_lazy();

	int ret;
	size_t line;

	yp_parser_t yparser;
	yp_parser_t *yp = &yparser;
	yp_init(yp);

	// OK input.
	ret = yp_set_input_string(yp, syntax_ok, strlen(syntax_ok));
	ok(ret == KNOT_EOK, "set input string");

	line = 1;
	for (int i = 0; i < 3; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key0", i);
		ok(yp->key_len == 1 && yp->key[0] == 'a' &&
		   yp->data_len == 0 && yp->event == YP_EKEY0 &&
		   yp->line_count == line + i, "compare %i. key0", i);
	}

	line = 5;
	for (int i = 0; i < 6; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key0 with value", i);
		ok(yp->key_len == 1 && yp->key[0] == 'b' &&
		   yp->data_len == 1 && yp->key[0] == 'b' &&
		   yp->event == YP_EKEY0 && yp->line_count == line + i,
		   "compare %i. key0 with value", i);
	}

	line = 12;
	for (int i = 0; i < 6; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key1 with value", i);
		ok(yp->key_len == 1 && yp->key[0] == 'f' &&
		   yp->data_len == 1 && yp->key[0] == 'f' &&
		   yp->event == YP_EKEY1 && yp->line_count == line + i,
		   "compare %i. key1 with value", i);
	}

	line = 19;
	for (int i = 0; i < 5; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key0 with first value", i);
		ok(yp->key_len == 1 && yp->key[0] == 'c' &&
		   yp->data_len == 1 && yp->data[0] == 'a' &&
		   yp->event == YP_EKEY0 && yp->line_count == line + i,
		   "compare %i. key0 with first value", i);

		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key0 with second value", i);
		ok(yp->key_len == 1 && yp->key[0] == 'c' &&
		   yp->data_len == 1 && yp->data[0] == 'b' &&
		   yp->event == YP_EKEY0 && yp->line_count == line + i,
		   "compare %i. key0 with second value", i);
	}

	line = 25;
	for (int i = 0; i < 2; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. id", i);
		ok(yp->key_len == 1 && yp->key[0] == 'd' &&
		   yp->data_len == 1 && yp->key[0] == 'd' &&
		   yp->event == YP_EID && yp->line_count == line + i,
		   "compare %i. id", i);
	}

	line = 28;
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key0 with quoted value");
	ok(yp->key_len == 1 && yp->key[0] == 'e' && yp->data_len == 10 &&
	   memcmp(yp->data, "a#b' c[d,]", yp->data_len) == 0 &&
	   yp->event == YP_EKEY0 && yp->line_count == line,
	   "compare key0 with quoted value");

	line = 30;
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key0");
	ok(yp->key_len == 4 && strcmp(yp->key, "zone") == 0 &&
	   yp->data_len == 0 &&
	   yp->event == YP_EKEY0 && yp->line_count == line,
	   "compare key0 value");

	line = 31;
	for (int i = 0; i < 2; i++) {
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. id", i);
		ok(yp->key_len == 6 && strcmp(yp->key, "domain") == 0 &&
		   yp->data_len == 8 && strcmp(yp->data, "example.") == 0 &&
		   yp->event == YP_EID && yp->line_count == line + 2 * i,
		   "compare id");
		ret = yp_parse(yp);
		ok(ret == KNOT_EOK, "parse %i. key1", i);
		ok(yp->key_len == 6 && strcmp(yp->key, "master") == 0 &&
		   yp->data_len == 4 && strcmp(yp->data, "bind") == 0 &&
		   yp->event == YP_EKEY1 && yp->line_count == line + 2 * i + 1,
		   "compare key1");
	}

	ret = yp_parse(yp);
	ok(ret == KNOT_EOF, "parse EOF");

	// Error input 1.
	ret = yp_set_input_string(yp, syntax_error1, strlen(syntax_error1));
	ok(ret == KNOT_EOK, "set error input string 1");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key0");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key1");
	ret = yp_parse(yp);
	ok(ret == KNOT_EPARSEFAIL, "parse invalid key1");

	// Error input 2.
	ret = yp_set_input_string(yp, syntax_error2, strlen(syntax_error2));
	ok(ret == KNOT_EOK, "set error input string 2");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key0");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key1");
	ret = yp_parse(yp);
	ok(ret == KNOT_EPARSEFAIL, "parse invalid key1");

	// Error input 3.
	ret = yp_set_input_string(yp, syntax_error3, strlen(syntax_error3));
	ok(ret == KNOT_EOK, "set error input string 3");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key0");
	ret = yp_parse(yp);
	ok(ret == KNOT_EOK, "parse key1");
	ret = yp_parse(yp);
	ok(ret == KNOT_EPARSEFAIL, "parse invalid key1");

	yp_deinit(yp);

	return 0;
}
