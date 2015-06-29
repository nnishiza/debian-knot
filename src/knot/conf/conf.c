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

#include <assert.h>
#include <grp.h>
#include <pwd.h>

#include "knot/conf/base.h"
#include "knot/conf/confdb.h"
#include "knot/common/log.h"
#include "knot/server/dthreads.h"
#include "libknot/libknot.h"
#include "libknot/internal/macros.h"
#include "libknot/internal/mem.h"
#include "libknot/internal/sockaddr.h"
#include "libknot/internal/strlcat.h"
#include "libknot/yparser/yptrafo.h"

/*! Configuration specific logging. */
#define CONF_LOG(severity, msg, ...) do { \
	log_msg(severity, "config, " msg, ##__VA_ARGS__); \
	} while (0)

#define CONF_LOG_ZONE(severity, zone, msg, ...) do { \
	log_msg_zone(severity, zone, "config, " msg, ##__VA_ARGS__); \
	} while (0)

conf_val_t conf_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key0_name,
	const yp_name_t *key1_name)
{
	conf_val_t val = { NULL };

	if (key0_name == NULL || key1_name == NULL) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_get (%s)", knot_strerror(val.code));
		return val;
	}

	val.code = conf_db_get(conf, txn, key0_name, key1_name, NULL, 0, &val);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read '%s/%s' (%s)",
		         key0_name + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		return val;
	}
}

conf_val_t conf_rawid_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key0_name,
	const yp_name_t *key1_name,
	const uint8_t *id,
	size_t id_len)
{
	conf_val_t val = { NULL };

	if (key0_name == NULL || key1_name == NULL || id == NULL) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_rawid_get (%s)", knot_strerror(val.code));
		return val;
	}

	val.code = conf_db_get(conf, txn, key0_name, key1_name, id, id_len, &val);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read '%s/%s' with identifier (%s)",
		         key0_name + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		return val;
	}
}

conf_val_t conf_id_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key0_name,
	const yp_name_t *key1_name,
	conf_val_t *id)
{
	conf_val_t val = { NULL };

	if (key0_name == NULL || key1_name == NULL || id == NULL ||
	    id->code != KNOT_EOK) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_id_get (%s)", knot_strerror(val.code));
		return val;
	}

	conf_db_val(id);

	val.code = conf_db_get(conf, txn, key0_name, key1_name, id->data,
	                       id->len, &val);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read '%s/%s' with identifier (%s)",
		         key0_name + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		return val;
	}
}

conf_val_t conf_mod_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key1_name,
	const conf_mod_id_t *mod_id)
{
	conf_val_t val = { NULL };

	if (key1_name == NULL || mod_id == NULL) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_mod_get (%s)", knot_strerror(val.code));
		return val;
	}

	val.code = conf_db_get(conf, txn, mod_id->name, key1_name, mod_id->data,
	                       mod_id->len, &val);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read '%s/%s' (%s)",
		         mod_id->name + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		return val;
	}
}

conf_val_t conf_zone_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key1_name,
	const knot_dname_t *dname)
{
	conf_val_t val = { NULL };

	if (key1_name == NULL || dname == NULL) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_zone_get (%s)", knot_strerror(val.code));
		return val;
	}

	int dname_size = knot_dname_size(dname);

	// Try to get explicit value.
	val.code = conf_db_get(conf, txn, C_ZONE, key1_name, dname, dname_size, &val);
	switch (val.code) {
	case KNOT_EOK:
		return val;
	default:
		CONF_LOG_ZONE(LOG_ERR, dname, "failed to read '%s/%s' (%s)",
		              C_ZONE + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_ENOENT:
		break;
	}

	// Check if a template is available.
	val.code = conf_db_get(conf, txn, C_ZONE, C_TPL, dname, dname_size, &val);
	switch (val.code) {
	case KNOT_EOK:
		// Use the specified template.
		conf_db_val(&val);
		val.code = conf_db_get(conf, txn, C_TPL, key1_name,
		                       val.data, val.len, &val);
		break;
	default:
		CONF_LOG_ZONE(LOG_ERR, dname, "failed to read '%s/%s' (%s)",
		              C_ZONE + 1, C_TPL + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_ENOENT:
		// Use the default template.
		val.code = conf_db_get(conf, txn, C_TPL, key1_name,
		                       CONF_DEFAULT_ID + 1, CONF_DEFAULT_ID[0], &val);
	}

	switch (val.code) {
	default:
		CONF_LOG_ZONE(LOG_ERR, dname, "failed to read '%s/%s' (%s)",
		              C_TPL + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		break;
	}

	return val;
}

conf_val_t conf_default_get_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key1_name)
{
	conf_val_t val = { NULL };

	if (key1_name == NULL) {
		val.code = KNOT_EINVAL;
		CONF_LOG(LOG_DEBUG, "conf_default_get (%s)", knot_strerror(val.code));
		return val;
	}

	val.code = conf_db_get(conf, txn, C_TPL, key1_name,
	                       CONF_DEFAULT_ID + 1, CONF_DEFAULT_ID[0], &val);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read default '%s/%s' (%s)",
		         C_TPL + 1, key1_name + 1, knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		break;
	}

	return val;
}

size_t conf_id_count_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key0_name)
{
	size_t count = 0;
	conf_iter_t iter = { NULL };

	int ret = conf_db_iter_begin(conf, txn, key0_name, &iter);
	switch (ret) {
	case KNOT_EOK:
		break;
	default:
		CONF_LOG(LOG_ERR, "failed to iterate through '%s' (%s)",
		         key0_name + 1, knot_strerror(ret));
		// FALLTHROUGH
	case KNOT_ENOENT:
		return count;
	}

	while (ret == KNOT_EOK) {
		count++;
		ret = conf_db_iter_next(conf, &iter);
	}
	conf_db_iter_finish(conf, &iter);

	return count;
}

conf_iter_t conf_iter_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const yp_name_t *key0_name)
{
	conf_iter_t iter = { NULL };

	iter.code = conf_db_iter_begin(conf, txn, key0_name, &iter);
	switch (iter.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to iterate thgrough '%s' (%s)",
		          key0_name + 1, knot_strerror(iter.code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_ENOENT:
		return iter;
	}
}

void conf_iter_next(
	conf_t *conf,
	conf_iter_t *iter)
{
	iter->code = conf_db_iter_next(conf, iter);
	switch (iter->code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read next item (%s)",
		          knot_strerror(iter->code));
		// FALLTHROUGH
	case KNOT_EOK:
	case KNOT_EOF:
		return;
	}
}

conf_val_t conf_iter_id(
	conf_t *conf,
	conf_iter_t *iter)
{
	conf_val_t val = { NULL };

	val.code = conf_db_iter_id(conf, iter, (uint8_t **)&val.blob,
	                           &val.blob_len);
	switch (val.code) {
	default:
		CONF_LOG(LOG_ERR, "failed to read identifier (%s)",
		          knot_strerror(val.code));
		// FALLTHROUGH
	case KNOT_EOK:
		val.item = iter->item;
		return val;
	}
}

void conf_iter_finish(
	conf_t *conf,
	conf_iter_t *iter)
{
	conf_db_iter_finish(conf, iter);
}

size_t conf_val_count(
	conf_val_t *val)
{
	if (val == NULL || val->code != KNOT_EOK) {
		return 0;
	}

	if (!(val->item->flags & YP_FMULTI)) {
		return 1;
	}

	size_t count = 0;
	conf_db_val(val);
	while (val->code == KNOT_EOK) {
		count++;
		conf_db_val_next(val);
	}
	if (val->code != KNOT_EOF) {
		return 0;
	}

	// Reset to the initial state.
	conf_db_val(val);

	return count;
}

void conf_val_next(
	conf_val_t *val)
{
	conf_db_val_next(val);
}

int64_t conf_int(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TINT ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TINT));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		return yp_int(val->data, val->len);
	} else {
		return val->item->var.i.dflt;
	}
}

bool conf_bool(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TBOOL ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TBOOL));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		return yp_bool(val->len);
	} else {
		return val->item->var.b.dflt;
	}
}

unsigned conf_opt(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TOPT ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TOPT));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		return yp_opt(val->data);
	} else {
		return val->item->var.o.dflt;
	}
}

const char* conf_str(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TSTR ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TSTR));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		return yp_str(val->data);
	} else {
		return val->item->var.s.dflt;
	}
}

const knot_dname_t* conf_dname(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TDNAME ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TDNAME));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		return yp_dname(val->data);
	} else {
		return (const knot_dname_t *)val->item->var.d.dflt;
	}
}

void conf_data(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TB64 || val->item->type == YP_TDATA ||
	       (val->item->type == YP_TREF &&
	        (val->item->var.r.ref->var.g.id->type == YP_TB64 ||
	         val->item->var.r.ref->var.g.id->type == YP_TDATA)));

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
	} else {
		val->data = (const uint8_t *)val->item->var.d.dflt;
		val->len = val->item->var.d.dflt_len;
	}
}

struct sockaddr_storage conf_addr(
	conf_val_t *val,
	const char *sock_base_dir)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TADDR ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TADDR));

	struct sockaddr_storage out = { AF_UNSPEC };

	if (val->code == KNOT_EOK) {
		int port;
		conf_db_val(val);
		out = yp_addr(val->data, val->len, &port);

		if (out.ss_family == AF_UNIX) {
			// val->data[0] is socket type identifier!
			if (val->data[1] == '/' || sock_base_dir == NULL) {
				val->code = sockaddr_set(&out, AF_UNIX,
				                         (char *)val->data + 1, 0);
			} else {
				char *tmp = sprintf_alloc("%s/%s", sock_base_dir,
				                          val->data + 1);
				val->code = sockaddr_set(&out, AF_UNIX, tmp, 0);
				free(tmp);
			}
		} else if (port != -1) {
			sockaddr_port_set(&out, port);
		} else {
			sockaddr_port_set(&out, val->item->var.a.dflt_port);
		}
	} else {
		const char *dflt_socket = val->item->var.a.dflt_socket;
		if (dflt_socket != NULL) {
			if (dflt_socket[0] == '/' || sock_base_dir == NULL) {
				val->code = sockaddr_set(&out, AF_UNIX,
				                         dflt_socket, 0);
			} else {
				char *tmp = sprintf_alloc("%s/%s", sock_base_dir,
				                          dflt_socket);
				val->code = sockaddr_set(&out, AF_UNIX, tmp, 0);
				free(tmp);
			}
		}
	}

	return out;
}

struct sockaddr_storage conf_net(
	conf_val_t *val,
	int *prefix_length)
{
	assert(val != NULL && val->item != NULL && prefix_length != NULL);
	assert(val->item->type == YP_TNET ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TNET));

	struct sockaddr_storage out = { AF_UNSPEC };

	if (val->code == KNOT_EOK) {
		conf_db_val(val);
		out = yp_addr(val->data, val->len, prefix_length);
	} else {
		*prefix_length = 0;
	}

	return out;
}

char* conf_abs_path(
	conf_val_t *val,
	const char *base_dir)
{
	const char *path = conf_str(val);
	if (path == NULL) {
		return NULL;
	} else if (path[0] == '/') {
		return strdup(path);
	} else {
		char *abs_path;
		if (base_dir == NULL) {
			char *cwd = realpath("./", NULL);
			abs_path = sprintf_alloc("%s/%s", cwd, path);
			free(cwd);
		} else {
			abs_path = sprintf_alloc("%s/%s", base_dir, path);
		}
		return abs_path;
	}
}

conf_mod_id_t* conf_mod_id(
	conf_val_t *val)
{
	assert(val != NULL && val->item != NULL);
	assert(val->item->type == YP_TDATA ||
	       (val->item->type == YP_TREF &&
	        val->item->var.r.ref->var.g.id->type == YP_TDATA));

	conf_mod_id_t *mod_id = NULL;

	if (val->code == KNOT_EOK) {
		conf_db_val(val);

		mod_id = malloc(sizeof(conf_mod_id_t));
		if (mod_id == NULL) {
			return NULL;
		}

		// Set module name in yp_name_t format + add zero termination.
		size_t name_len = 1 + val->data[0];
		mod_id->name = malloc(name_len + 1);
		if (mod_id->name == NULL) {
			free(mod_id);
			return NULL;
		}
		memcpy(mod_id->name, val->data, name_len);
		mod_id->name[name_len] = '\0';

		// Set module identifier.
		mod_id->len = val->len - name_len;
		mod_id->data = malloc(mod_id->len);
		if (mod_id->data == NULL) {
			free(mod_id->name);
			free(mod_id);
			return NULL;
		}
		memcpy(mod_id->data, val->data + name_len, mod_id->len);
	}

	return mod_id;
}

void conf_free_mod_id(
	conf_mod_id_t *mod_id)
{
	free(mod_id->name);
	free(mod_id->data);
	free(mod_id);
}

static char* get_filename(
	conf_t *conf,
	namedb_txn_t *txn,
	const knot_dname_t *zone,
	const char *name)
{
	assert(name);

	const char *end = name + strlen(name);
	char out[1024] = "";

	do {
		// Search for a formatter.
		const char *pos = strchr(name, '%');

		// If no formatter, copy the rest of the name.
		if (pos == NULL) {
			if (strlcat(out, name, sizeof(out)) >= sizeof(out)) {
				return NULL;
			}
			break;
		}

		// Copy constant block.
		char *block = strndup(name, pos - name);
		if (block == NULL ||
		    strlcat(out, block, sizeof(out)) >= sizeof(out)) {
			return NULL;
		}
		free(block);

		// Move name pointer behind the formatter.
		name = pos + 2;

		char buff[512] = "";

		const char type = *(pos + 1);
		switch (type) {
		case '%':
			strlcat(buff, "%", sizeof(buff));
			break;
		case 's':
			if (knot_dname_to_str(buff, zone, sizeof(buff)) == NULL) {
				return NULL;
			}

			// Replace possible slashes with underscores.
			char *ch;
			for (ch = buff; *ch != '\0'; ch++) {
				if (*ch == '/') {
					*ch = '_';
				}
			}

			// Remove trailing dot if not root zone.
			if (ch - buff > 1) {
				*(--ch) = '\0';
			}

			break;
		case '\0':
			CONF_LOG_ZONE(LOG_WARNING, zone, "ignoring missing "
			              "trailing zonefile formatter");
			continue;
		default:
			CONF_LOG_ZONE(LOG_WARNING, zone, "ignoring zonefile "
			              "formatter '%%%c'", type);
			continue;
		}

		if (strlcat(out, buff, sizeof(out)) >= sizeof(out)) {
			return NULL;
		}
	} while (name < end);

	// Use storage prefix if not absolute path.
	if (out[0] == '/') {
		return strdup(out);
	} else {
		conf_val_t val = conf_zone_get_txn(conf, txn, C_STORAGE, zone);
		char *storage = conf_abs_path(&val, NULL);
		if (storage == NULL) {
			return NULL;
		}
		char *abs = sprintf_alloc("%s/%s", storage, out);
		free(storage);
		return abs;
	}
}

char* conf_zonefile_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const knot_dname_t *zone)
{
	if (zone == NULL) {
		return NULL;
	}

	conf_val_t val = conf_zone_get_txn(conf, txn, C_FILE, zone);
	const char *file = conf_str(&val);

	// Use default zonefile name pattern if not specified.
	if (file == NULL) {
		file = "%s.zone";
	}

	return get_filename(conf, txn, zone, file);
}

char* conf_journalfile_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	const knot_dname_t *zone)
{
	if (zone == NULL) {
		return NULL;
	}

	return get_filename(conf, txn, zone, "%s.db");
}

size_t conf_udp_threads_txn(
	conf_t *conf,
	namedb_txn_t *txn)
{
	conf_val_t val = conf_get_txn(conf, txn, C_SRV, C_UDP_WORKERS);
	int64_t workers = conf_int(&val);
	if (workers == YP_NIL) {
		return dt_optimal_size();
	}

	return workers;
}

size_t conf_tcp_threads_txn(
	conf_t *conf,
	namedb_txn_t *txn)
{
	conf_val_t val = conf_get_txn(conf, txn, C_SRV, C_TCP_WORKERS);
	int64_t workers = conf_int(&val);
	if (workers == YP_NIL) {
		return MAX(conf_udp_threads_txn(conf, txn) * 2, CONF_XFERS);
	}

	return workers;
}

size_t conf_bg_threads_txn(
	conf_t *conf,
	namedb_txn_t *txn)
{
	conf_val_t val = conf_get_txn(conf, txn, C_SRV, C_BG_WORKERS);
	int64_t workers = conf_int(&val);
	if (workers == YP_NIL) {
		return MIN(dt_optimal_size(), CONF_XFERS);
	}

	return workers;
}

int conf_user_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	int *uid,
	int *gid)
{
	if (uid == NULL || gid == NULL) {
		return KNOT_EINVAL;
	}

	conf_val_t val = conf_get_txn(conf, txn, C_SRV, C_USER);
	if (val.code == KNOT_EOK) {
		char *user = strdup(conf_str(&val));

		// Search for user:group separator.
		char *sep_pos = strchr(user, ':');
		if (sep_pos != NULL) {
			// Process group name.
			struct group *grp = getgrnam(sep_pos + 1);
			if (grp != NULL) {
				*gid = grp->gr_gid;
			} else {
				CONF_LOG(LOG_ERR, "invalid group name '%s'",
				         sep_pos + 1);
				free(user);
				return KNOT_EINVAL;
			}

			// Cut off group part.
			*sep_pos = '\0';
		} else {
			*gid = getgid();
		}

		// Process user name.
		struct passwd *pwd = getpwnam(user);
		if (pwd != NULL) {
			*uid = pwd->pw_uid;
		} else {
			CONF_LOG(LOG_ERR, "invalid user name '%s'", user);
			free(user);
			return KNOT_EINVAL;
		}

		free(user);
		return KNOT_EOK;
	} else if (val.code == KNOT_ENOENT) {
		*uid = getuid();
		*gid = getgid();
		return KNOT_EOK;
	} else {
		return val.code;
	}
}

conf_remote_t conf_remote_txn(
	conf_t *conf,
	namedb_txn_t *txn,
	conf_val_t *id,
	size_t index)
{
	assert(id != NULL && id->item != NULL);
	assert(id->item->type == YP_TSTR ||
	       (id->item->type == YP_TREF &&
	        id->item->var.r.ref->var.g.id->type == YP_TSTR));

	conf_remote_t out = { { AF_UNSPEC } };

	conf_val_t rundir_val = conf_get_txn(conf, txn, C_SRV, C_RUNDIR);
	char *rundir = conf_abs_path(&rundir_val, NULL);

	// Get indexed remote address.
	conf_val_t val = conf_id_get_txn(conf, txn, C_RMT, C_ADDR, id);
	for (size_t i = 0; val.code == KNOT_EOK && i < index; i++) {
		conf_val_next(&val);
	}
	// Index overflow causes empty socket.
	out.addr = conf_addr(&val, rundir);

	// Get outgoing address if family matches (optional).
	val = conf_id_get_txn(conf, txn, C_RMT, C_VIA, id);
	while (val.code == KNOT_EOK) {
		struct sockaddr_storage via = conf_addr(&val, rundir);
		if (via.ss_family == out.addr.ss_family) {
			out.via = conf_addr(&val, rundir);
			break;
		}
		conf_val_next(&val);
	}

	// Get TSIG key (optional).
	conf_val_t key_id = conf_id_get_txn(conf, txn, C_RMT, C_KEY, id);
	if (key_id.code == KNOT_EOK) {
		out.key.name = (knot_dname_t *)conf_dname(&key_id);

		val = conf_id_get_txn(conf, txn, C_KEY, C_ALG, &key_id);
		out.key.algorithm = conf_opt(&val);

		val = conf_id_get_txn(conf, txn, C_KEY, C_SECRET, &key_id);
		conf_data(&val);
		out.key.secret.data = (uint8_t *)val.data;
		out.key.secret.size = val.len;
	}

	free(rundir);

	return out;
}
