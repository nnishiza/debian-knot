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

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <tap/basic.h>

#include "libknot/internal/mempool.h"
#include "libknot/internal/mem.h"
#include "libknot/internal/namedb/namedb_lmdb.h"
#include "libknot/internal/namedb/namedb_trie.h"
#include "libknot/internal/strlcpy.h"
#include "libknot/libknot.h"

/* Constants. */
#define KEY_MAXLEN 64
#define KEY_SET(key, str) key.data = (str); key.len = strlen(str) + 1

/*! \brief Generate random key. */
static const char *alphabet = "abcdefghijklmn0123456789";
static char *str_key_rand(size_t len, mm_ctx_t *pool)
{
	char *s = mm_alloc(pool, len);
	memset(s, 0, len);
	for (unsigned i = 0; i < len - 1; ++i) {
		s[i] = alphabet[rand() % strlen(alphabet)];
	}
	return s;
}

/* UCW array sorting defines. */
#define ASORT_PREFIX(X) str_key_##X
#define ASORT_KEY_TYPE char*
#define ASORT_LT(x, y) (strcmp((x), (y)) < 0)
#include "libknot/internal/array-sort.h"

static void namedb_test_set(unsigned nkeys, char **keys, void *opts,
                            const namedb_api_t *api, mm_ctx_t *pool)
{
	if (api == NULL) {
		skip("API not compiled in");
		return;
	}

	/* Create database */
	namedb_t *db = NULL;
	int ret = api->init(&db, pool, opts);
	ok(ret == KNOT_EOK && db != NULL, "%s: create", api->name);

	/* Start WR transaction. */
	namedb_txn_t txn;
	ret = api->txn_begin(db, &txn, 0);
	ok(ret == KNOT_EOK, "%s: txn_begin(WR)", api->name);

	/* Insert keys */
	namedb_val_t key, val;
	bool passed = true;
	for (unsigned i = 0; i < nkeys; ++i) {
		KEY_SET(key, keys[i]);
		val.len = sizeof(void*);
		val.data = &key.data;

		ret = api->insert(&txn, &key, &val, 0);
		if (ret != KNOT_EOK && ret != KNOT_EEXIST) {
			passed = false;
			break;
		}
	}
	ok(passed, "%s: insert", api->name);

	/* Commit WR transaction. */
	ret = api->txn_commit(&txn);
	ok(ret == KNOT_EOK, "%s: txn_commit(WR)", api->name);

	/* Start RD transaction. */
	ret = api->txn_begin(db, &txn, NAMEDB_RDONLY);
	ok(ret == KNOT_EOK, "%s: txn_begin(RD)", api->name);

	/* Lookup all keys */
	passed = true;
	for (unsigned i = 0; i < nkeys; ++i) {
		KEY_SET(key, keys[i]);

		ret = api->find(&txn, &key, &val, 0);
		if (ret != KNOT_EOK) {
			passed = false;
			break;
		}

		const char **stored_key = val.data;
		if (strcmp(*stored_key, keys[i]) != 0) {
			diag("%s: mismatch on element '%u'", api->name, i);
			passed = false;
			break;
		}
	}
	ok(passed, "%s: lookup all keys", api->name);

	/* Fetch dataset size. */
	int db_size = api->count(&txn);
	ok(db_size > 0 && db_size <= nkeys, "%s: count %d", api->name, db_size);

	/* Unsorted iteration */
	int iterated = 0;
	namedb_iter_t *it = api->iter_begin(&txn, 0);
	while (it != NULL) {
		++iterated;
		it = api->iter_next(it);
	}
	api->iter_finish(it);
	is_int(db_size, iterated, "%s: unsorted iteration", api->name);

	/* Sorted iteration. */
	char first_key[KEY_MAXLEN] = { '\0' };
	char second_key[KEY_MAXLEN] = { '\0' };
	char last_key[KEY_MAXLEN] = { '\0' };
	char key_buf[KEY_MAXLEN] = {'\0'};
	iterated = 0;
	it = api->iter_begin(&txn, NAMEDB_SORTED);
	while (it != NULL) {
		ret = api->iter_key(it, &key);
		if (iterated > 0) { /* Only if previous exists. */
			if (strcmp(key_buf, key.data) > 0) {
				diag("%s: iter_sort '%s' <= '%s' FAIL\n",
				     api->name, key_buf, (const char *)key.data);
				break;
			}
			if (iterated == 1) {
				memcpy(second_key, key.data, key.len);
			}
		} else {
			memcpy(first_key, key.data, key.len);
		}
		++iterated;
		memcpy(key_buf, key.data, key.len);
		it = api->iter_next(it);
	}
	strlcpy(last_key, key_buf, sizeof(last_key));
	is_int(db_size, iterated, "%s: sorted iteration", api->name);
	api->iter_finish(it);

	/* Interactive iteration. */
	it = api->iter_begin(&txn, NAMEDB_NOOP);
	if (it != NULL) { /* If supported. */
		/* Check if first and last keys are reachable */
		it = api->iter_seek(it, NULL, NAMEDB_FIRST);
		ret = api->iter_key(it, &key);
		is_string(first_key, key.data, "%s: iter_set(FIRST)", api->name);
		/* Check left/right iteration. */
		it = api->iter_seek(it, &key, NAMEDB_NEXT);
		ret = api->iter_key(it, &key);
		is_string(second_key, key.data, "%s: iter_set(NEXT)", api->name);
		it = api->iter_seek(it, &key, NAMEDB_PREV);
		ret = api->iter_key(it, &key);
		is_string(first_key, key.data, "%s: iter_set(PREV)", api->name);
		it = api->iter_seek(it, &key, NAMEDB_LAST);
		ret = api->iter_key(it, &key);
		is_string(last_key, key.data, "%s: iter_set(LAST)", api->name);
		/* Check if prev(last_key + 1) is the last_key */
		strlcpy(key_buf, last_key, sizeof(key_buf));
		key_buf[0] += 1;
		KEY_SET(key, key_buf);
		it = api->iter_seek(it, &key, NAMEDB_LEQ);
		ret = api->iter_key(it, &key);
		is_string(last_key, key.data, "%s: iter_set(LEQ)", api->name);
		/* Check if next(first_key - 1) is the first_key */
		strlcpy(key_buf, first_key, sizeof(key_buf));
		key_buf[0] -= 1;
		KEY_SET(key, key_buf);
		it = api->iter_seek(it, &key, NAMEDB_GEQ);
		ret = api->iter_key(it, &key);
		is_string(first_key, key.data, "%s: iter_set(GEQ)", api->name);
		api->iter_finish(it);
	}
	api->txn_abort(&txn);

	/* Clear database and recheck. */
	ret = api->txn_begin(db, &txn, 0);
	ret = api->clear(&txn);
	is_int(0, ret, "%s: clear()", api->name);
	ret = api->txn_commit(&txn);
	is_int(0, ret, "%s: commit clear", api->name);

	/* Check if the database is empty. */
	ret = api->txn_begin(db, &txn, NAMEDB_RDONLY);
	db_size = api->count(&txn);
	is_int(0, db_size, "%s: count after clear = %d", api->name, db_size);
	api->txn_abort(&txn);

	api->deinit(db);
}

int main(int argc, char *argv[])
{
	plan_lazy();

	mm_ctx_t pool;
	mm_ctx_mempool(&pool, MM_DEFAULT_BLKSIZE);

	/* Temporary DB identifier. */
	char dbid_buf[] = "/tmp/namedb.XXXXXX";
	char *dbid = mkdtemp(dbid_buf);

	/* Random keys. */
	unsigned nkeys = 10000;
	char **keys = mm_alloc(&pool, sizeof(char*) * nkeys);
	for (unsigned i = 0; i < nkeys; ++i) {
		keys[i] = str_key_rand(KEY_MAXLEN, &pool);
	}

	/* Sort random keys. */
	str_key_sort(keys, nkeys);

	/* Execute test set for all backends. */
	struct namedb_lmdb_opts lmdb_opts = NAMEDB_LMDB_OPTS_INITIALIZER;
	lmdb_opts.path = dbid;
	struct namedb_trie_opts trie_opts = NAMEDB_TRIE_OPTS_INITIALIZER;
	namedb_test_set(nkeys, keys, &lmdb_opts, namedb_lmdb_api(), &pool);
	namedb_test_set(nkeys, keys, &trie_opts, namedb_trie_api(), &pool);

	/* Cleanup. */
	mp_delete(pool.ctx);

	/* Cleanup temporary DB. */
	DIR *dir = opendir(dbid);
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		char *file = sprintf_alloc("%s/%s", dbid, dp->d_name);
		remove(file);
		free(file);
	}
	closedir(dir);
	remove(dbid);

	return 0;
}
