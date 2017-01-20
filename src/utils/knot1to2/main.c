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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/common/params.h"
#include "utils/knot1to2/extra.h"
#include "utils/knot1to2/scheme.h"
#include "contrib/qp-trie/trie.h"
#include "contrib/string.h"

#define PROGRAM_NAME "knot1to2"

static int run_parser(const char *file_in, int run, share_t *share)
{
	extern int cf_parse(void *scanner);
	extern int cf_lex_init_extra(void *, void *scanner);
	extern void cf_set_in(FILE *f, void *scanner);
	extern void cf_lex_destroy(void *scanner);
	extern volatile int parser_ret;

	FILE *in = fopen(file_in, "r");
	if (in == NULL) {
		printf("Failed to open input file '%s'\n", file_in);
		return -1;
	}

	void *sc = NULL;
	conf_extra_t *extra = conf_extra_init(file_in, run, share);
	cf_lex_init_extra(extra, &sc);
	cf_set_in(in, sc);
	cf_parse(sc);
	cf_lex_destroy(sc);
	conf_extra_free(extra);

	fclose(in);

	return parser_ret;
}

static int convert(const char *file_out, const char *file_in)
{
	FILE *out = fopen(file_out, "w");
	if (out == NULL) {
		printf("Failed to open output file '%s'\n", file_out);
		return -1;
	}

	fprintf(out,
	        "# This file was generated using knot1to2 conversion utility.\n"
		"#\n"
		"# Take in mind that some constructions have changed and therefore\n"
		"# the conversion cannot be perfect. It is important to do a review\n"
		"# of this file (see the documentation).\n"
		"# It is also possible to reformat the file via knotc, like:\n"
		"#   knotc -c ./this_file.conf conf-export ./reformatted_file.conf\n"
	       );

	share_t share = {
		.out = out,
		.ifaces = trie_create(NULL),
		.groups = trie_create(NULL),
		.remotes = trie_create(NULL),
		.acl_xfer = trie_create(NULL),
		.acl_notify = trie_create(NULL),
		.acl_update = trie_create(NULL)
	};

	// Parse the input file multiple times to get some context.
	int ret = 0;
	for (int i = R_SYS; i <= R_ZONE; i++) {
		ret = run_parser(file_in, i, &share);
		if (ret != 0) {
			break;
		}
	}

	// Remove ifaces data.
	trie_it_t *it = trie_it_begin(share.ifaces);
	for (; !trie_it_finished(it); trie_it_next(it)) {
		char *data = *trie_it_val(it);
		free(data);
	}
	trie_it_free(it);

	// Remove groups data.
	it = trie_it_begin(share.groups);
	for (; !trie_it_finished(it); trie_it_next(it)) {
		trie_t *trie = *trie_it_val(it);
		trie_free(trie);
	}
	trie_it_free(it);

	// Remove empty tries without data.
	trie_free(share.ifaces);
	trie_free(share.groups);
	trie_free(share.remotes);
	trie_free(share.acl_xfer);
	trie_free(share.acl_notify);
	trie_free(share.acl_update);

	fclose(out);

	return ret;
}

static int reformat(const char *file_out, const char *file_in, const char *path)
{
	char *cmd = sprintf_alloc("%s%sknotc -c %s conf-export %s",
	                          path == NULL ? "" : path,
	                          path == NULL ? "" : "/",
	                          file_in, file_out);
	if (cmd == NULL) {
		printf("Failed to reformat file '%s'\n", file_in);
		return -1;
	}

	int ret = system(cmd);
	free(cmd);
	if (ret != 0) {
		return -1;
	}

	return 0;
}

static void print_help(void)
{
	printf("Usage: %s [options] -i <file> -o <file>\n"
	       "\n"
	       " -i, --in <file>      Input config file (Knot version 1.x).\n"
	       " -o, --out <file>     Output config file (Knot version 2.x).\n"
	       "\n"
	       "Options:\n"
	       " -p, --path <dir>     Path to knotc utility.\n"
	       " -r, --raw            Raw output, do not reformat via knotc.\n"
	       "\n"
	       " -h, --help           Print the program help.\n"
	       " -V, --version        Print the program version.\n",
	       PROGRAM_NAME);
}

int main(int argc, char **argv)
{
	const char *file_in = NULL;
	const char *file_out = NULL;
	const char *path = NULL;
	bool raw = false;

	struct option opts[] = {
		{ "in",      required_argument, NULL, 'i' },
		{ "out",     required_argument, NULL, 'o' },
		{ "path",    required_argument, NULL, 'p' },
		{ "raw",     no_argument,       NULL, 'r' },
		{ "help",    no_argument,       NULL, 'h' },
		{ "version", no_argument,       NULL, 'V' },
		{ NULL }
	};

	// Parse parameters.
	int opt = 0, li = 0;
	while ((opt = getopt_long(argc, argv, "i:o:p:rhV", opts, &li)) != -1) {
		switch (opt)
		{
		case 'i':
			file_in = optarg;
			break;
		case 'o':
			file_out = optarg;
			break;
		case 'p':
			path = optarg;
			break;
		case 'r':
			raw = true;
			break;
		case 'h':
			print_help();
			return EXIT_SUCCESS;
		case 'V':
			print_version(PROGRAM_NAME);
			return EXIT_SUCCESS;
		default:
			print_help();
			return EXIT_FAILURE;
		}
	}

	// Check for missing or invalid parameters.
	if (argc - optind > 0 || file_in == NULL || file_out == NULL) {
		print_help();
		return EXIT_FAILURE;
	}

	// Convert the file.
	int ret = convert(file_out, file_in);
	if (ret != 0) {
		return EXIT_FAILURE;
	}

	if (!raw) {
		char *file_tmp = sprintf_alloc("%s.tmp", file_out);
		if (file_tmp == NULL) {
			printf("Failed to reformat file '%s'\n", file_out);
			return EXIT_FAILURE;
		}

		// Store intermediate file.
		ret = rename(file_out, file_tmp);
		if (ret != 0) {
			printf("Failed to reformat file '%s'\n", file_out);
		}

		// Reformat converted file.
		ret = reformat(file_out, file_tmp, path);
		if (ret != 0) {
			free(file_tmp);
			return EXIT_FAILURE;
		}

		// Remove temporary file if successfully reformatted.
		remove(file_tmp);
		free(file_tmp);
	}

	return EXIT_SUCCESS;
}
