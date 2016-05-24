/*  Copyright (C) 2016 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>

#include "libknot/libknot.h"
#include "contrib/macros.h"
#include "contrib/string.h"
#include "knot/common/log.h"
#include "knot/dnssec/zone-nsec.h"
#include "knot/zone/semantic-check.h"
#include "knot/zone/contents.h"
#include "knot/zone/zonefile.h"
#include "knot/zone/zone-dump.h"

#define ERROR(zone, fmt, ...) log_zone_error(zone, "zone loader, " fmt, ##__VA_ARGS__)
#define WARNING(zone, fmt, ...) log_zone_warning(zone, "zone loader, " fmt, ##__VA_ARGS__)
#define INFO(zone, fmt, ...) log_zone_info(zone, "zone loader, " fmt, ##__VA_ARGS__)

static void process_error(zs_scanner_t *s)
{
	zcreator_t *zc = s->process.data;
	const knot_dname_t *zname = zc->z->apex->owner;

	ERROR(zname, "%s in zone, file '%s', line %"PRIu64" (%s)",
	      s->error.fatal ? "fatal error" : "error",
	      s->file.name, s->line_counter,
	      zs_strerror(s->error.code));
}

static int add_rdata_to_rr(knot_rrset_t *rrset, const zs_scanner_t *scanner)
{
	return knot_rrset_add_rdata(rrset, scanner->r_data, scanner->r_data_length,
	                            scanner->r_ttl, NULL);
}

static void log_ttl_error(const zone_contents_t *zone, const zone_node_t *node,
                          const knot_rrset_t *rr)
{
	err_handler_t err_handler;
	err_handler_init(&err_handler);
	// Prepare additional info string.
	char info_str[64] = { '\0' };
	char type_str[16] = { '\0' };
	knot_rrtype_to_string(rr->type, type_str, sizeof(type_str));
	int ret = snprintf(info_str, sizeof(info_str), "record type %s",
	                   type_str);
	if (ret <= 0 || ret >= sizeof(info_str)) {
		*info_str = '\0';
	}

	/*!< \todo REPLACE WITH FATAL ERROR for master. */
	err_handler_handle_error(&err_handler, zone, node,
	                         ZC_ERR_TTL_MISMATCH, info_str);
}

static bool handle_err(zcreator_t *zc, const zone_node_t *node,
                       const knot_rrset_t *rr, int ret, bool master)
{
	const knot_dname_t *zname = zc->z->apex->owner;
	char *rrname = rr ? knot_dname_to_str_alloc(rr->owner) : NULL;
	if (ret == KNOT_EOUTOFZONE) {
		WARNING(zname, "ignoring out-of-zone data, owner '%s'",
		        rrname ? rrname : "unknown");
		free(rrname);
		return true;
	} else if (ret == KNOT_ETTL) {
		free(rrname);
		assert(node);
		log_ttl_error(zc->z, node, rr);
		// Fail if we're the master for this zone.
		return !master;
	} else {
		ERROR(zname, "failed to process record, owner '%s'",
		      rrname ? rrname : "unknown");
		free(rrname);
		return false;
	}
}

int zcreator_step(zcreator_t *zc, const knot_rrset_t *rr)
{
	if (zc == NULL || rr == NULL || rr->rrs.rr_count != 1) {
		return KNOT_EINVAL;
	}

	if (rr->type == KNOT_RRTYPE_SOA &&
	    node_rrtype_exists(zc->z->apex, KNOT_RRTYPE_SOA)) {
		// Ignore extra SOA
		return KNOT_EOK;
	}

	zone_node_t *node = NULL;
	int ret = zone_contents_add_rr(zc->z, rr, &node);
	if (ret != KNOT_EOK) {
		if (!handle_err(zc, node, rr, ret, zc->master)) {
			// Fatal error
			return ret;
		}
		if (ret == KNOT_EOUTOFZONE) {
			// Skip out-of-zone record
			return KNOT_EOK;
		}
	}
	assert(node);

	// Do node semantic checks
	err_handler_t err_handler;
	err_handler_init(&err_handler);
	bool sem_fatal_error = false;

	ret = sem_check_node_plain(zc->z, node,
	                           &err_handler, true,
	                           &sem_fatal_error);
	if (ret != KNOT_EOK) {
		return ret;
	}

	return sem_fatal_error ? KNOT_ESEMCHECK : KNOT_EOK;
}

/*! \brief Creates RR from parser input, passes it to handling function. */
static void process_data(zs_scanner_t *scanner)
{
	zcreator_t *zc = scanner->process.data;
	if (zc->ret != KNOT_EOK) {
		scanner->state = ZS_STATE_STOP;
		return;
	}

	knot_dname_t *owner = knot_dname_copy(scanner->r_owner, NULL);
	if (owner == NULL) {
		zc->ret = KNOT_ENOMEM;
		return;
	}

	knot_rrset_t rr;
	knot_rrset_init(&rr, owner, scanner->r_type, scanner->r_class);
	int ret = add_rdata_to_rr(&rr, scanner);
	if (ret != KNOT_EOK) {
		char *rr_name = knot_dname_to_str_alloc(rr.owner);
		const knot_dname_t *zname = zc->z->apex->owner;
		ERROR(zname, "failed to add RDATA, file '%s', line %"PRIu64", owner '%s'",
		      scanner->file.name, scanner->line_counter, rr_name);
		free(rr_name);
		knot_dname_free(&owner, NULL);
		zc->ret = ret;
		return;
	}

	/* Convert RDATA dnames to lowercase before adding to zone. */
	ret = knot_rrset_rr_to_canonical(&rr);
	if (ret != KNOT_EOK) {
		knot_dname_free(&owner, NULL);
		zc->ret = ret;
		return;
	}

	zc->ret = zcreator_step(zc, &rr);
	knot_dname_free(&owner, NULL);
	knot_rdataset_clear(&rr.rrs, NULL);
}

int zonefile_open(zloader_t *loader, const char *source,
                  const knot_dname_t *origin, bool semantic_checks)
{
	if (!loader) {
		return KNOT_EINVAL;
	}

	memset(loader, 0, sizeof(zloader_t));

	/* Check zone file. */
	if (access(source, F_OK | R_OK) != 0) {
		return KNOT_EACCES;
	}

	/* Create context. */
	zcreator_t *zc = malloc(sizeof(zcreator_t));
	if (zc == NULL) {
		return KNOT_ENOMEM;
	}
	memset(zc, 0, sizeof(zcreator_t));

	zc->z = zone_contents_new(origin);
	if (zc->z == NULL) {
		free(zc);
		return KNOT_ENOMEM;
	}

	/* Prepare textual owner for zone scanner. */
	char *origin_str = knot_dname_to_str_alloc(origin);
	if (origin_str == NULL) {
		free(zc);
		return KNOT_ENOMEM;
	}

	if (zs_init(&loader->scanner, origin_str, KNOT_CLASS_IN, 3600) != 0 ||
	    zs_set_input_file(&loader->scanner, source) != 0 ||
	    zs_set_processing(&loader->scanner, process_data, process_error, zc) != 0) {
		zs_deinit(&loader->scanner);
		free(origin_str);
		free(zc);

		switch (loader->scanner.error.code) {
		case ZS_FILE_OPEN:
		case ZS_FILE_INVALID:
			return KNOT_EFILE;
		default:
			return KNOT_EOK;
		}
	}
	free(origin_str);

	loader->source = strdup(source);
	loader->creator = zc;
	loader->semantic_checks = semantic_checks;

	return KNOT_EOK;
}

zone_contents_t *zonefile_load(zloader_t *loader)
{
	if (!loader) {
		return NULL;
	}

	zcreator_t *zc = loader->creator;
	const knot_dname_t *zname = zc->z->apex->owner;

	assert(zc);
	int ret = zs_parse_all(&loader->scanner);
	if (ret != 0 && loader->scanner.error.counter == 0) {
		ERROR(zname, "failed to load zone, file '%s' (%s)",
		      loader->source, zs_strerror(loader->scanner.error.code));
		goto fail;
	}

	if (zc->ret != KNOT_EOK) {
		ERROR(zname, "failed to load zone, file '%s' (%s)",
		      loader->source, knot_strerror(zc->ret));
		goto fail;
	}

	if (loader->scanner.error.counter > 0) {
		ERROR(zname, "failed to load zone, file '%s', %"PRIu64" errors",
		      loader->source, loader->scanner.error.counter);
		goto fail;
	}

	if (!node_rrtype_exists(loader->creator->z->apex, KNOT_RRTYPE_SOA)) {
		ERROR(zname, "no SOA record, file '%s'", loader->source);
		goto fail;
	}

	zone_node_t *first_nsec3_node = NULL;
	zone_node_t *last_nsec3_node = NULL;

	ret = zone_contents_adjust_full(zc->z, &first_nsec3_node, &last_nsec3_node);
	if (ret != KNOT_EOK) {
		ERROR(zname, "failed to finalize zone contents (%s)",
		      knot_strerror(ret));
		goto fail;
	}

	if (loader->semantic_checks) {
		int check_level = SEM_CHECK_UNSIGNED;
		knot_rrset_t soa_rr = node_rrset(zc->z->apex, KNOT_RRTYPE_SOA);
		assert(!knot_rrset_empty(&soa_rr)); // In this point, SOA has to exist
		const bool have_nsec3param =
			node_rrtype_exists(zc->z->apex, KNOT_RRTYPE_NSEC3PARAM);
		if (zone_contents_is_signed(zc->z) && !have_nsec3param) {

			/* Set check level to DNSSEC. */
			check_level = SEM_CHECK_NSEC;
		} else if (zone_contents_is_signed(zc->z) && have_nsec3param) {
			check_level = SEM_CHECK_NSEC3;
		}
		err_handler_t err_handler;
		err_handler_init(&err_handler);
		zone_do_sem_checks(zc->z, check_level,
		                   &err_handler, first_nsec3_node,
		                   last_nsec3_node);
		INFO(zname, "semantic check, completed");
	}

	return zc->z;

fail:
	zone_contents_deep_free(&zc->z);
	return NULL;
}

int zonefile_exists(const char *path, time_t *mtime)
{
	if (path == NULL) {
		return KNOT_EINVAL;
	}

	struct stat zonefile_st = { 0 };
	if (stat(path, &zonefile_st) < 0) {
		return knot_map_errno();
	}

	if (mtime != NULL) {
		*mtime = zonefile_st.st_mtime;
	}

	return KNOT_EOK;
}

/*! \brief Open a temporary zonefile. */
static int open_tmp_filename(const char *name, char **tmp_name, FILE **file)
{
	int ret;

	*tmp_name = sprintf_alloc("%s.XXXXXX", name);
	if (*tmp_name == NULL) {
		ret = KNOT_ENOMEM;
		goto open_tmp_failed;
	}

	int fd = mkstemp(*tmp_name);
	if (fd < 0) {
		ret = knot_map_errno();
		goto open_tmp_failed;
	}

	if (fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0) {
		ret = knot_map_errno();
		close(fd);
		unlink(*tmp_name);
		goto open_tmp_failed;
	}

	*file = fdopen(fd, "w");
	if (*file == NULL) {
		ret = knot_map_errno();
		close(fd);
		unlink(*tmp_name);
		goto open_tmp_failed;
	}

	return KNOT_EOK;
open_tmp_failed:
	free(*tmp_name);
	*tmp_name = NULL;

	assert(ret != KNOT_EOK);
	return ret;
}

/*! \brief Prepare a directory for the file. */
static int make_path(const char *path, mode_t mode)
{
	if (path == NULL) {
		return KNOT_EINVAL;
	}

	char *dir = strdup(path);
	if (dir == NULL) {
		return KNOT_ENOMEM;
	}

	for (char *p = strchr(dir + 1, '/'); p != NULL; p = strchr(p + 1, '/')) {
		*p = '\0';
		if (mkdir(dir, mode) == -1 && errno != EEXIST) {
			free(dir);
			return knot_map_errno();
		}
		*p = '/';
	}

	free(dir);

	return KNOT_EOK;
}

int zonefile_write(const char *path, zone_contents_t *zone)
{
	if (!zone || !path) {
		return KNOT_EINVAL;
	}

	int ret = make_path(path, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
	if (ret != KNOT_EOK) {
		return ret;
	}

	FILE *file = NULL;
	char *tmp_name = NULL;
	ret = open_tmp_filename(path, &tmp_name, &file);
	if (ret != KNOT_EOK) {
		return ret;
	}

	ret = zone_dump_text(zone, file);
	fclose(file);
	if (ret != KNOT_EOK) {
		unlink(tmp_name);
		free(tmp_name);
		return ret;
	}

	/* Swap temporary zonefile and new zonefile. */
	ret = rename(tmp_name, path);
	if (ret != 0) {
		ret = knot_map_errno();
		unlink(tmp_name);
		free(tmp_name);
		return ret;
	}

	free(tmp_name);

	return KNOT_EOK;
}

void zonefile_close(zloader_t *loader)
{
	if (!loader) {
		return;
	}

	zs_deinit(&loader->scanner);
	free(loader->source);
	free(loader->creator);
}

#undef ERROR
#undef WARNING
#undef INFO
