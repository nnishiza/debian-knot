/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         cf_parse
#define yylex           cf_lex
#define yyerror         cf_error
#define yydebug         cf_debug
#define yynerrs         cf_nerrs


/* Copy the first part of user declarations.  */
#line 23 "knot/conf/cf-parse.y" /* yacc.c:339  */


#include <config.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "common-knot/sockaddr.h"
#include "common-knot/strlcat.h"
#include "common-knot/strlcpy.h"
#include "libknot/dname.h"
#include "libknot/binary.h"
#include "libknot/rrtype/opt.h"
#include "knot/server/rrl.h"
#include "knot/nameserver/query_module.h"
#include "knot/conf/conf.h"
#include "knot/conf/libknotd_la-cf-parse.h" /* Automake generated header. */

extern int cf_lex (YYSTYPE *lvalp, void *scanner);
extern void cf_error(void *scanner, const char *format, ...);
extern void cf_warning(void *scanner, const char *format, ...);
extern conf_t *new_config;
static conf_iface_t *this_iface = 0;
static conf_iface_t *this_remote = 0;
static conf_zone_t *this_zone = 0;
static conf_group_t *this_group = 0;
static list_t *this_list = 0;
static conf_log_t *this_log = 0;
static conf_log_map_t *this_logmap = 0;
//#define YYERROR_VERBOSE 1

static char *cache_hostname = NULL;

#define SET_NUM(out, in, min, max, name)				\
{									\
	if (in < min || in > max) {					\
		cf_error(scanner, "'%s' is out of range (%u-%u)",	\
		         name, min, max);				\
	} else {							\
		out = in;						\
	}								\
}

#define SET_UINT16(out, in, name) SET_NUM(out, in, 0, UINT16_MAX, name);
#define SET_INT(out, in, name) SET_NUM(out, in, 0, INT_MAX, name);
#define SET_SIZE(out, in, name) SET_NUM(out, in, 0, SIZE_MAX, name);

static void conf_start(void *scanner)
{
	cache_hostname = NULL;
}

static void conf_init_iface(void *scanner, char* ifname)
{
	this_iface = malloc(sizeof(conf_iface_t));
	if (this_iface == NULL) {
		cf_error(scanner, "not enough memory when allocating interface");
		return;
	}
	memset(this_iface, 0, sizeof(conf_iface_t));
	this_iface->name = ifname;
}

static void conf_set_iface(void *scanner, struct sockaddr_storage *ss, int family, char* addr, int port)
{
	int ret = sockaddr_set(ss, family, addr, port);
	if (ret != KNOT_EOK) {
		cf_error(scanner, "invalid address for '%s': %s@%d\n",
		                  this_iface->name, addr, port);
	}
	free(addr);
}

static void conf_start_iface(void *scanner, char* ifname)
{
	conf_init_iface(scanner, ifname);
	add_tail(&new_config->ifaces, &this_iface->n);
}

static conf_iface_t *conf_get_remote(const char *name)
{
	conf_iface_t *remote;
	WALK_LIST (remote, new_config->remotes) {
		if (strcmp(remote->name, name) == 0) {
			return remote;
		}
	}

	return NULL;
}

static void conf_start_remote(void *scanner, char *remote)
{
	if (conf_get_remote(remote) != NULL) {
		cf_error(scanner, "remote '%s' already defined", remote);
		return;
	}

	this_remote = malloc(sizeof(conf_iface_t));
	if (this_remote == NULL) {
		cf_error(scanner, "not enough memory when allocating remote");
		return;
	}

	memset(this_remote, 0, sizeof(conf_iface_t));
	this_remote->name = remote;
	add_tail(&new_config->remotes, &this_remote->n);
}

static void conf_remote_set_via(void *scanner, char *item) {
	/* Find existing node in interfaces. */
	node_t* r = 0; conf_iface_t* found = 0;
	WALK_LIST (r, new_config->ifaces) {
		if (strcmp(((conf_iface_t*)r)->name, item) == 0) {
			found = (conf_iface_t*)r;
			break;
		}
	}

	/* Check */
	if (!found) {
		cf_error(scanner, "interface '%s' is not defined", item);
	} else {
		memcpy(&this_remote->via, &found->addr, sizeof(struct sockaddr_storage));
		sockaddr_port_set(&this_remote->via, 0);
	}
}

static conf_group_t *conf_get_group(const char *name)
{
	conf_group_t *group;
	WALK_LIST (group, new_config->groups) {
		if (strcmp(group->name, name) == 0) {
			return group;
		}
	}

	return NULL;
}

static void conf_start_group(void *scanner, char *name)
{
	conf_group_t *group = conf_get_group(name);
	if (group) {
		cf_error(scanner, "group '%s' already defined", name);
		return;
	}

	if (conf_get_remote(name) != NULL) {
		cf_error(scanner, "group name '%s' conflicts with remote name",
		         name);
		free(name);
		return;
	}

	/* Add new group. */

	group = calloc(1, sizeof(conf_group_t));
	if (!group) {
		cf_error(scanner, "out of memory");
		free(name);
		return;
	}

	group->name = name;
	init_list(&group->remotes);

	add_tail(&new_config->groups, &group->n);
	this_group = group;
}

static void conf_add_member_into_group(void *scanner, char *name)
{
	if (!this_group) {
		cf_error(scanner, "parser error, variable 'this_group' null");
		free(name);
		return;
	}

	if (conf_get_remote(name) == NULL) {
		cf_error(scanner, "remote '%s' is not defined", name);
		free(name);
		return;
	}

	// add the remote into the group while silently ignoring duplicates

	conf_group_remote_t *remote;
	node_t *n;
	WALK_LIST (n, this_group->remotes) {
		remote = (conf_group_remote_t *)n;
		if (strcmp(remote->name, name) == 0) {
			free(name);
			return;
		}
	}

	remote = calloc(1, sizeof(conf_group_remote_t));
	remote->name = name;
	add_tail(&this_group->remotes, &remote->n);
}

static bool set_remote_or_group(void *scanner, char *name,
				void (*install)(void *, conf_iface_t *))
{
	// search remotes

	conf_iface_t *remote = conf_get_remote(name);
	if (remote) {
		install(scanner, remote);
		return true;
	}

	// search groups

	conf_group_t *group = conf_get_group(name);
	if (group) {
		conf_group_remote_t *group_remote;
		WALK_LIST (group_remote, group->remotes) {
			remote = conf_get_remote(group_remote->name);
			if (!remote)
				continue;
			install(scanner, remote);
		}

		return true;
	}

	return false;
}

static void conf_acl_item_install(void *scanner, conf_iface_t *found)
{

	// additional check for transfers

	if ((this_list == &this_zone->acl.xfr_in || this_list == &this_zone->acl.notify_out)
	    && sockaddr_port(&found->addr) == 0)
	{
		cf_error(scanner, "remote specified for XFR/IN or "
		"NOTIFY/OUT needs to have valid port!");
		return;
	}

	// silently skip duplicates

	conf_remote_t *remote;
	WALK_LIST (remote, *this_list) {
		if (remote->remote == found) {
			return;
		}
	}

	// add into the list

	remote = malloc(sizeof(conf_remote_t));
	if (!remote) {
		cf_error(scanner, "out of memory");
		return;
	}

	remote->remote = found;
	add_tail(this_list, &remote->n);
}

static void conf_acl_item(void *scanner, char *item)
{
	if (!set_remote_or_group(scanner, item, conf_acl_item_install)) {
		cf_error(scanner, "remote or group '%s' not defined", item);
	}

	free(item);
}

static void query_module_create(void *scanner, const char *name, const char *param, bool on_zone)
{
	struct query_module *module = query_module_open(new_config, name, param, NULL);
	if (module == NULL) {
		cf_error(scanner, "cannot load query module '%s'", name);
		return;
	}

	if (on_zone) {
		add_tail(&this_zone->query_modules, &module->node);
	} else {
		add_tail(&new_config->query_modules, &module->node);
	}
}

static int conf_key_exists(void *scanner, char *item)
{
	/* Find existing node in keys. */
	knot_dname_t *sample = knot_dname_from_str_alloc(item);
	knot_dname_to_lower(sample);
	conf_key_t* r = 0;
	WALK_LIST (r, new_config->keys) {
		if (knot_dname_cmp(r->k.name, sample) == 0) {
			cf_error(scanner, "key '%s' is already defined", item);
			knot_dname_free(&sample, NULL);
			return 1;
		}
	}

	knot_dname_free(&sample, NULL);
	return 0;
}

static int conf_key_add(void *scanner, knot_tsig_key_t **key, char *item)
{
	/* Reset */
	*key = 0;

	/* Find in keys */
	knot_dname_t *sample = knot_dname_from_str_alloc(item);
	knot_dname_to_lower(sample);

	conf_key_t* r = 0;
	WALK_LIST (r, new_config->keys) {
		if (knot_dname_cmp(r->k.name, sample) == 0) {
			*key = &r->k;
			knot_dname_free(&sample, NULL);
			return 0;
		}
	}

	cf_error(scanner, "key '%s' is not defined", item);
	knot_dname_free(&sample, NULL);
	return 1;
}

static void conf_zone_start(void *scanner, char *name) {
	this_zone = malloc(sizeof(conf_zone_t));
	if (this_zone == NULL || name == NULL) {
		cf_error(scanner, "out of memory while allocating zone config");
		return;
	}

	conf_init_zone(this_zone);

	// Append mising dot to ensure FQDN
	size_t nlen = strlen(name);
	if (name[nlen - 1] != '.') {
		this_zone->name = malloc(nlen + 2);
		if (this_zone->name != NULL) {
			memcpy(this_zone->name, name, nlen);
			this_zone->name[nlen] = '.';
			this_zone->name[++nlen] = '\0';
		}
		free(name);
	} else {
		this_zone->name = name; /* Already FQDN */
	}

	// Convert zone name to lower-case.
	for (size_t i = 0; this_zone->name[i]; i++) {
		this_zone->name[i] = tolower((unsigned char)this_zone->name[i]);
	}

	/* Check domain name. */
	knot_dname_t *dn = NULL;
	if (this_zone->name != NULL) {
		dn = knot_dname_from_str_alloc(this_zone->name);
	}
	if (dn == NULL) {
		free(this_zone->name);
		free(this_zone);
		this_zone = NULL;
		cf_error(scanner, "invalid zone origin");
	} else {
	/* Check for duplicates. */
	if (hattrie_tryget(new_config->zones, (const char *)dn,
	                   knot_dname_size(dn)) != NULL) {
		cf_error(scanner, "zone '%s' is already present, refusing to "
		         "duplicate", this_zone->name);
		knot_dname_free(&dn, NULL);
		free(this_zone->name);
		this_zone->name = NULL;
		/* Must not free, some versions of flex might continue after
		 * error and segfault.
		 * free(this_zone); this_zone = NULL;
		 */
		return;
	}

	*hattrie_get(new_config->zones, (const char *)dn,
	             knot_dname_size(dn)) = this_zone;
	knot_dname_free(&dn, NULL);
	}
}

/*! \brief Replace string value. */
static void opt_replace(char **opt, char *new_opt, bool val)
{
	/* Clear old value. */
	free(*opt);
	*opt = NULL;
	/* Replace if val is True. */
	if (val) {
		*opt = new_opt;
	} else {
		free(new_opt);
	}
}

static char *get_hostname(void *scanner)
{
	if (cache_hostname) {
		return strdup(cache_hostname);
	}

	char *fqdn = sockaddr_hostname();
	if (!fqdn) {
		cf_warning(scanner, "cannot retrieve host FQDN");
		return NULL;
	}

	cache_hostname = fqdn;

	return fqdn;
}

/*! \brief Generate automatic defaults for server identity, version and NSID. */
static void ident_auto(void *scanner, int tok, conf_t *conf, bool val)
{
	switch(tok) {
	case SVERSION:
		opt_replace(&conf->version, strdup("Knot DNS " PACKAGE_VERSION), val);
		break;
	case IDENTITY:
		opt_replace(&conf->identity, get_hostname(scanner), val);
		break;
	case NSID:
		opt_replace(&conf->nsid, get_hostname(scanner), val);
		if (conf->nsid) {
			conf->nsid_len = strlen(conf->nsid);
		}
		break;
	default:
		break;
	}
}


#line 519 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED
# define YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cf_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 258,
    INVALID_TOKEN = 259,
    TEXT = 260,
    HEXSTR = 261,
    NUM = 262,
    INTERVAL = 263,
    SIZE = 264,
    BOOL = 265,
    SYSTEM = 266,
    IDENTITY = 267,
    HOSTNAME = 268,
    SVERSION = 269,
    NSID = 270,
    KEY = 271,
    KEYS = 272,
    MAX_UDP_PAYLOAD = 273,
    REQ_EDNS_OPT = 274,
    TSIG_ALGO_NAME = 275,
    WORKERS = 276,
    BACKGROUND_WORKERS = 277,
    ASYNC_START = 278,
    USER = 279,
    RUNDIR = 280,
    PIDFILE = 281,
    REMOTES = 282,
    GROUPS = 283,
    ZONES = 284,
    FILENAME = 285,
    DISABLE_ANY = 286,
    SEMANTIC_CHECKS = 287,
    NOTIFY_RETRIES = 288,
    NOTIFY_TIMEOUT = 289,
    DBSYNC_TIMEOUT = 290,
    IXFR_FSLIMIT = 291,
    XFR_IN = 292,
    XFR_OUT = 293,
    UPDATE_IN = 294,
    NOTIFY_IN = 295,
    NOTIFY_OUT = 296,
    BUILD_DIFFS = 297,
    MAX_CONN_IDLE = 298,
    MAX_CONN_HS = 299,
    MAX_CONN_REPLY = 300,
    MAX_TCP_CLIENTS = 301,
    RATE_LIMIT = 302,
    RATE_LIMIT_SIZE = 303,
    RATE_LIMIT_SLIP = 304,
    TRANSFERS = 305,
    STORAGE = 306,
    DNSSEC_ENABLE = 307,
    DNSSEC_KEYDIR = 308,
    SIGNATURE_LIFETIME = 309,
    SERIAL_POLICY = 310,
    SERIAL_POLICY_VAL = 311,
    QUERY_MODULE = 312,
    INTERFACES = 313,
    ADDRESS = 314,
    PORT = 315,
    IPA = 316,
    IPA6 = 317,
    VIA = 318,
    CONTROL = 319,
    ALLOW = 320,
    LISTEN_ON = 321,
    LOG = 322,
    LOG_DEST = 323,
    LOG_SRC = 324,
    LOG_LEVEL = 325
  };
#endif
/* Tokens.  */
#define END 258
#define INVALID_TOKEN 259
#define TEXT 260
#define HEXSTR 261
#define NUM 262
#define INTERVAL 263
#define SIZE 264
#define BOOL 265
#define SYSTEM 266
#define IDENTITY 267
#define HOSTNAME 268
#define SVERSION 269
#define NSID 270
#define KEY 271
#define KEYS 272
#define MAX_UDP_PAYLOAD 273
#define REQ_EDNS_OPT 274
#define TSIG_ALGO_NAME 275
#define WORKERS 276
#define BACKGROUND_WORKERS 277
#define ASYNC_START 278
#define USER 279
#define RUNDIR 280
#define PIDFILE 281
#define REMOTES 282
#define GROUPS 283
#define ZONES 284
#define FILENAME 285
#define DISABLE_ANY 286
#define SEMANTIC_CHECKS 287
#define NOTIFY_RETRIES 288
#define NOTIFY_TIMEOUT 289
#define DBSYNC_TIMEOUT 290
#define IXFR_FSLIMIT 291
#define XFR_IN 292
#define XFR_OUT 293
#define UPDATE_IN 294
#define NOTIFY_IN 295
#define NOTIFY_OUT 296
#define BUILD_DIFFS 297
#define MAX_CONN_IDLE 298
#define MAX_CONN_HS 299
#define MAX_CONN_REPLY 300
#define MAX_TCP_CLIENTS 301
#define RATE_LIMIT 302
#define RATE_LIMIT_SIZE 303
#define RATE_LIMIT_SLIP 304
#define TRANSFERS 305
#define STORAGE 306
#define DNSSEC_ENABLE 307
#define DNSSEC_KEYDIR 308
#define SIGNATURE_LIFETIME 309
#define SERIAL_POLICY 310
#define SERIAL_POLICY_VAL 311
#define QUERY_MODULE 312
#define INTERFACES 313
#define ADDRESS 314
#define PORT 315
#define IPA 316
#define IPA6 317
#define VIA 318
#define CONTROL 319
#define ALLOW 320
#define LISTEN_ON 321
#define LOG 322
#define LOG_DEST 323
#define LOG_SRC 324
#define LOG_LEVEL 325

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 475 "knot/conf/cf-parse.y" /* yacc.c:355  */

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;

#line 708 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cf_parse (void *scanner);

#endif /* !YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 722 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   368

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  184
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  384

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   325

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    76,     2,     2,    75,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    71,
       2,     2,     2,     2,    72,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,     2,    74,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   549,   549,   549,   551,   553,   556,   557,   558,   559,
     560,   561,   562,   565,   566,   573,   576,   579,   582,   588,
     589,   597,   598,   599,   600,   601,   602,   607,   608,   609,
     610,   614,   619,   620,   621,   625,   628,   631,   634,   655,
     658,   661,   664,   667,   670,   673,   676,   679,   682,   685,
     688,   694,   695,   744,   745,   746,   747,   748,   749,   752,
     753,   760,   764,   768,   772,   776,   780,   784,   792,   795,
     798,   805,   806,   814,   817,   819,   820,   824,   828,   829,
     833,   836,   839,   842,   845,   850,   851,   852,   853,   854,
     855,   858,   859,   860,   863,   864,   893,   896,   897,   900,
     901,   902,   903,   904,   905,   906,   907,   923,   927,   928,
     929,   930,   931,   932,   933,   934,   935,   936,   939,   942,
     945,   948,   951,   954,   957,   958,   961,   964,   967,   968,
     973,   981,   983,   984,   988,   989,   990,   991,   992,   993,
     996,   999,  1002,  1005,  1008,  1011,  1014,  1015,  1016,  1017,
    1020,  1023,  1026,  1031,  1036,  1039,  1048,  1049,  1053,  1056,
    1057,  1063,  1085,  1111,  1115,  1116,  1117,  1120,  1120,  1124,
    1128,  1134,  1135,  1142,  1147,  1148,  1151,  1151,  1151,  1151,
    1151,  1151,  1151,  1151,  1151
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "INVALID_TOKEN", "TEXT", "HEXSTR",
  "NUM", "INTERVAL", "SIZE", "BOOL", "SYSTEM", "IDENTITY", "HOSTNAME",
  "SVERSION", "NSID", "KEY", "KEYS", "MAX_UDP_PAYLOAD", "REQ_EDNS_OPT",
  "TSIG_ALGO_NAME", "WORKERS", "BACKGROUND_WORKERS", "ASYNC_START", "USER",
  "RUNDIR", "PIDFILE", "REMOTES", "GROUPS", "ZONES", "FILENAME",
  "DISABLE_ANY", "SEMANTIC_CHECKS", "NOTIFY_RETRIES", "NOTIFY_TIMEOUT",
  "DBSYNC_TIMEOUT", "IXFR_FSLIMIT", "XFR_IN", "XFR_OUT", "UPDATE_IN",
  "NOTIFY_IN", "NOTIFY_OUT", "BUILD_DIFFS", "MAX_CONN_IDLE", "MAX_CONN_HS",
  "MAX_CONN_REPLY", "MAX_TCP_CLIENTS", "RATE_LIMIT", "RATE_LIMIT_SIZE",
  "RATE_LIMIT_SLIP", "TRANSFERS", "STORAGE", "DNSSEC_ENABLE",
  "DNSSEC_KEYDIR", "SIGNATURE_LIFETIME", "SERIAL_POLICY",
  "SERIAL_POLICY_VAL", "QUERY_MODULE", "INTERFACES", "ADDRESS", "PORT",
  "IPA", "IPA6", "VIA", "CONTROL", "ALLOW", "LISTEN_ON", "LOG", "LOG_DEST",
  "LOG_SRC", "LOG_LEVEL", "';'", "'@'", "'{'", "'}'", "'/'", "','",
  "$accept", "config", "$@1", "conf_entries", "interface_start",
  "interface", "interfaces", "system", "keys", "remote_start", "remote",
  "remotes", "group_member", "group", "group_start", "groups",
  "zone_acl_start", "zone_acl_item", "zone_acl_list", "zone_acl",
  "query_module", "query_module_list", "zone_start", "zone",
  "query_genmodule", "query_genmodule_list", "zones", "log_prios_start",
  "log_prios", "log_src", "log_dest", "log_file", "log_end", "log_start",
  "log", "$@2", "ctl_listen_start", "ctl_allow_start", "control", "conf", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,    59,    64,   123,   125,    47,    44
};
# endif

#define YYPACT_NINF -54

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-54)))

#define YYTABLE_NINF -86

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -54,    49,   -54,   -54,    83,   -54,   -21,     9,    12,    16,
      29,    47,    56,   -54,   -54,    14,   112,     0,    23,     1,
       3,     5,   -53,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,    73,   -54,   -54,   -54,   -54,   -54,   -54,   -54,    80,
      -1,    90,    13,    38,    85,   168,   172,   173,   105,   159,
     176,   177,    25,   101,   115,   178,   193,    17,   194,   195,
     179,   -54,   163,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     130,   -54,   -54,   131,   -54,   138,   198,   -54,   -54,   196,
     197,   208,   124,   132,    44,   206,   212,   209,   213,   135,
     164,   148,   -54,   -54,   -54,   -54,   -54,   149,   157,   -54,
     -54,   -54,   -54,    -4,   150,   -54,   -54,   153,   154,   155,
     156,   161,   165,   166,   167,   223,   169,   170,   171,   174,
     175,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   199,   200,   224,   -54,   225,   228,
     139,   201,   202,   203,   204,   205,   207,   210,   211,   214,
     215,   216,   217,   218,   219,   220,   221,   229,   -54,   232,
     230,   233,   234,   240,   141,   144,   109,   -54,   -54,   -54,
     -54,   -54,   238,   244,   254,   245,   158,   227,   222,   -54,
     226,   231,   -54,   -54,    -5,   -28,   -45,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   235,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   236,   -13,   -54,   -54,
      45,   -54,   237,   239,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     260,   241,   242,   162,   243,   246,   247,   248,   249,   250,
     251,   252,   253,   255,   256,   257,   258,   259,   261,   262,
     263,   264,   -54,    -5,   -54,   -43,     2,   -54,   -54,   -54,
     -54,   -54,   -51,   272,   -54,   265,   266,   -54,   108,   273,
     -54,   -54,   -54,   274,   110,   277,    37,   -54,   -54,   225,
     -54,   -54,   -54,   229,   -54,   269,   270,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   288,   271,   275,     6,   -54,   276,
     -54,   -54,   -54,   -54,   -54,   -54,   102,   106,   279,   280,
      32,    42,   281,   282,   283,   284,   -54,   -54,   -54,   -54,
     -54,   264,   -54,   -54,   -54,    -6,    22,   -54,   287,   -54,
     289,   -54,   -54,   -54,   290,   291,   -54,   293,   294,   -54,
     -54,   -54,   -54,   -54,   -54,   -54,   -54,   285,   286,   292,
     295,   296,   297,   -54,   267,   -54,   -54,   -54,   -54,   -54,
     -54,    30,   -54,   -54
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     4,     1,     0,     3,     0,     0,     0,     0,
       0,     0,     0,   167,   176,     6,     0,     0,    53,     0,
      99,     0,     0,     5,    21,    51,    71,    78,   134,    19,
     171,     0,     7,     8,    12,    10,     9,    11,   178,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   177,     0,   179,    54,    58,    56,    55,    57,   180,
       0,    77,   181,     0,   107,     0,     0,   100,   101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,   103,   102,   104,   182,     0,     0,   183,
     170,   169,   184,     0,    91,   164,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   132,   108,     0,
       0,     0,     0,     0,     0,     0,     0,    80,    81,    84,
      82,    83,     0,     0,     0,     0,     0,     0,     0,   135,
      91,     0,    13,    94,   175,   163,     0,    24,    25,    26,
      22,    23,    28,    27,    29,     0,    30,    35,    36,    37,
      38,    32,    33,    40,    39,    42,    41,    44,    43,    45,
      46,    48,    47,    49,    50,    31,     0,     0,    73,    75,
       0,   106,     0,     0,   136,   138,   141,   142,   143,   144,
     145,   140,   139,   137,   146,   147,   148,   149,   150,   151,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,    94,   110,   173,     0,     0,    86,    90,    88,
      87,    89,     0,     0,   161,     0,     0,   168,     0,     0,
      20,    34,    52,     0,     0,     0,     0,    72,    79,     0,
     153,   152,   131,   132,   154,     0,     0,   111,   116,   113,
     121,   123,   122,   117,   118,   120,   119,   112,   114,   124,
     115,   125,   126,   127,     0,     0,     0,     0,   172,     0,
     174,    93,    92,   162,   159,   159,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,   133,   130,   129,
      96,    97,   128,   109,    95,     0,     0,    15,     0,    17,
       0,    14,    67,    61,     0,     0,    64,     0,     0,    60,
      70,    68,    69,    98,   155,   165,   166,     0,     0,     0,
       0,     0,     0,   156,   160,    16,    18,    63,    62,    66,
      65,     0,   158,   157
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -54,   -54,   -54,   -54,   -54,    84,   -54,   -54,   -54,   -54,
     -54,   -54,   -22,   -54,   -54,   -54,   -54,   -54,    88,    41,
     -54,   -37,   -54,   -54,   -54,    18,   -54,   -54,   -54,   -20,
     -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    39,   186,    15,    16,    17,    70,
     217,    18,   219,   220,    73,    19,   180,   272,   184,   266,
     315,   316,    97,    98,   241,   242,    20,   373,   374,   345,
     275,   276,   277,   185,    21,    31,   103,   104,    22,    23
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     267,   181,   273,   283,   107,    62,    71,   319,    74,   108,
      75,   319,   100,   101,   278,   279,   278,   279,   110,    32,
     321,   102,    76,   111,   131,   322,   132,    77,    64,   280,
      78,   318,   123,   124,    79,    80,    81,    82,    83,    84,
     274,    33,   333,   112,   113,    85,   284,   285,   114,     3,
     286,   148,    24,   149,    86,    87,    88,    89,    90,   268,
      91,   287,   269,   364,   270,   271,   -85,    92,   365,   182,
      93,   -85,    94,    95,    63,    72,   320,    96,    34,    99,
     343,    35,    25,    36,    37,    26,     5,    65,    38,    27,
      66,   364,    67,    68,     6,   109,   366,    69,   334,   335,
       7,   382,    28,   353,   354,   115,   383,   355,   125,   126,
       8,     9,    10,   356,   357,   119,   252,   358,   253,   288,
      29,   289,   127,   128,    40,    41,    42,    43,    44,    30,
      45,   144,   145,    46,    47,    48,    49,    50,    51,   146,
     147,    11,   154,   155,   222,   223,   105,    12,   248,   249,
      13,   250,   251,   106,    14,    52,    53,    54,    55,    56,
      57,    58,    59,    60,   120,   258,   259,   295,   296,   326,
     327,   330,   331,   347,   348,   116,   159,   349,   350,   117,
     118,   121,   122,   136,   135,   129,    61,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     130,   133,   134,   137,   138,   140,   141,   142,   173,   174,
     175,   176,   177,   139,   178,   143,   150,   151,   153,   152,
     156,   157,   158,   183,   187,   188,   189,   190,   195,   216,
     218,   179,   191,   221,   240,   244,   192,   193,   194,   243,
     196,   197,   198,   245,   246,   199,   200,   247,   254,   255,
     257,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   256,   292,   265,   336,   263,   314,
     214,   215,   224,   225,   226,   227,   228,   323,   229,   329,
     328,   230,   231,   260,   332,   232,   233,   234,   235,   236,
     237,   238,   239,   340,   367,   261,   368,   369,   370,   262,
     371,   372,   264,   317,   363,   346,   281,   282,   290,     0,
     291,   337,   293,     0,   297,     0,   294,   298,   299,   300,
     301,   302,   303,   304,   305,     0,   306,   307,   308,   309,
     310,     0,   311,   312,   313,     0,     0,   381,   324,   325,
     338,   339,   341,     0,     0,     0,     0,   344,     0,   342,
     351,   352,   359,   360,   361,   362,   375,   376,     0,     0,
       0,     0,     0,   377,     0,     0,   378,   379,   380
};

static const yytype_int16 yycheck[] =
{
       5,     5,    30,    16,     5,     5,     5,     5,     5,    10,
       7,     5,    65,    66,    59,    60,    59,    60,     5,     5,
      71,    74,    19,    10,     7,    76,     9,    24,     5,    74,
      27,    74,     7,     8,    31,    32,    33,    34,    35,    36,
      68,    27,     5,     5,     6,    42,    59,    60,    10,     0,
      63,     7,    73,     9,    51,    52,    53,    54,    55,    64,
      57,    74,    67,    69,    69,    70,    71,    64,    74,    73,
      67,    76,    69,    70,    74,    74,    74,    74,    64,    74,
      74,    67,    73,    69,    70,    73,     3,    64,    74,    73,
      67,    69,    69,    70,    11,     5,    74,    74,    61,    62,
      17,    71,    73,    71,    72,    20,    76,    75,     7,     8,
      27,    28,    29,    71,    72,    10,     7,    75,     9,    74,
      73,    76,     7,     8,    12,    13,    14,    15,    16,    73,
      18,     7,     8,    21,    22,    23,    24,    25,    26,     7,
       8,    58,     7,     8,     5,     6,    73,    64,     7,     8,
      67,     7,     8,    73,    71,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     5,     7,     8,     5,     6,    61,
      62,    61,    62,    71,    72,     7,    19,    71,    72,     7,
       7,     5,     5,    20,     5,     7,    74,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
       7,     7,     7,    73,    73,     7,    10,    10,    51,    52,
      53,    54,    55,    75,    57,     7,    10,     5,     5,    10,
      56,    73,    73,    73,    71,    71,    71,    71,     5,     5,
       5,    74,    71,     5,     5,     5,    71,    71,    71,     7,
      71,    71,    71,    10,    10,    71,    71,     7,    10,     5,
       5,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    10,     5,   182,   289,   180,     5,
      71,    71,    71,    71,    71,    71,    71,     5,    71,     5,
       7,    71,    71,    56,     7,    71,    71,    71,    71,    71,
      71,    71,    71,     5,     7,    73,     7,     7,     7,    73,
       7,     7,    71,   262,   341,   325,    71,    71,    71,    -1,
      71,   293,    71,    -1,    71,    -1,    74,    71,    71,    71,
      71,    71,    71,    71,    71,    -1,    71,    71,    71,    71,
      71,    -1,    71,    71,    71,    -1,    -1,    70,    73,    73,
      71,    71,    71,    -1,    -1,    -1,    -1,    71,    -1,    74,
      71,    71,    71,    71,    71,    71,    71,    71,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    71,    71,    71
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    78,    79,     0,    80,     3,    11,    17,    27,    28,
      29,    58,    64,    67,    71,    83,    84,    85,    88,    92,
     103,   111,   115,   116,    73,    73,    73,    73,    73,    73,
      73,   112,     5,    27,    64,    67,    69,    70,    74,    81,
      12,    13,    14,    15,    16,    18,    21,    22,    23,    24,
      25,    26,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    74,     5,    74,     5,    64,    67,    69,    70,    74,
      86,     5,    74,    91,     5,     7,    19,    24,    27,    31,
      32,    33,    34,    35,    36,    42,    51,    52,    53,    54,
      55,    57,    64,    67,    69,    70,    74,    99,   100,    74,
      65,    66,    74,   113,   114,    73,    73,     5,    10,     5,
       5,    10,     5,     6,    10,    20,     7,     7,     7,    10,
       5,     5,     5,     7,     8,     7,     8,     7,     8,     7,
       7,     7,     9,     7,     7,     5,    20,    73,    73,    75,
       7,    10,    10,     7,     7,     8,     7,     8,     7,     9,
      10,     5,    10,     5,     7,     8,    56,    73,    73,    19,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    51,    52,    53,    54,    55,    57,    74,
      93,     5,    73,    73,    95,   110,    82,    71,    71,    71,
      71,    71,    71,    71,    71,     5,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,     5,    87,     5,    89,
      90,     5,     5,     6,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
       5,   101,   102,     7,     5,    10,    10,     7,     7,     8,
       7,     8,     7,     9,    10,     5,    10,     5,     7,     8,
      56,    73,    73,    95,    71,    82,    96,     5,    64,    67,
      69,    70,    94,    30,    68,   107,   108,   109,    59,    60,
      74,    71,    71,    16,    59,    60,    63,    74,    74,    76,
      71,    71,     5,    71,    74,     5,     6,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,     5,    97,    98,    96,    74,     5,
      74,    71,    76,     5,    73,    73,    61,    62,     7,     5,
      61,    62,     7,     5,    61,    62,    89,   102,    71,    71,
       5,    71,    74,    74,    71,   106,   106,    71,    72,    71,
      72,    71,    71,    71,    72,    75,    71,    72,    75,    71,
      71,    71,    71,    98,    69,    74,    74,     7,     7,     7,
       7,     7,     7,   104,   105,    71,    71,    71,    71,    71,
      71,    70,    71,    76
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    79,    78,    80,    80,    81,    81,    81,    81,
      81,    81,    81,    82,    82,    82,    82,    82,    82,    83,
      83,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    85,    85,    86,    86,    86,    86,    86,    86,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    88,    88,    89,    90,    90,    90,    91,    92,    92,
      93,    93,    93,    93,    93,    94,    94,    94,    94,    94,
      94,    95,    95,    95,    96,    96,    97,    98,    98,    99,
      99,    99,    99,    99,    99,    99,    99,    99,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   101,   102,   102,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   104,   105,   105,   105,   106,
     106,   107,   108,   109,   110,   110,   110,   112,   111,   113,
     114,   115,   115,   115,   115,   115,   116,   116,   116,   116,
     116,   116,   116,   116,   116
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     3,     0,     2,     0,     1,     1,     1,
       1,     1,     1,     0,     4,     4,     6,     4,     6,     2,
       5,     2,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     5,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     2,     5,     0,     1,     1,     1,     1,     1,     0,
       4,     4,     6,     6,     4,     6,     6,     4,     4,     4,
       4,     2,     5,     1,     0,     1,     3,     1,     2,     5,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     1,
       1,     0,     3,     3,     0,     3,     2,     0,     3,     0,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     5,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     5,     5,
       5,     2,     0,     3,     2,     3,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     5,     5,     5,     0,     1,     3,     3,     0,
       3,     1,     2,     0,     0,     5,     5,     0,     5,     1,
       1,     2,     5,     4,     5,     3,     1,     2,     2,     2,
       2,     2,     2,     2,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void *scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 549 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start(scanner); }
#line 2073 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 3:
#line 549 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { return 0; }
#line 2079 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 7:
#line 557 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, (yyvsp[0].tok).t); }
#line 2085 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 8:
#line 558 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2091 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 9:
#line 559 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2097 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 10:
#line 560 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2103 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 11:
#line 561 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2109 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 12:
#line 562 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2115 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 14:
#line 566 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_iface->addr, (yyvsp[-1].tok).i);
     }
   }
#line 2127 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 15:
#line 573 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
   }
#line 2135 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 16:
#line 576 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
   }
#line 2143 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 17:
#line 579 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
   }
#line 2151 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 18:
#line 582 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
   }
#line 2159 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 20:
#line 589 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
   if (this_iface->addr.ss_family == AF_UNSPEC) {
     cf_error(scanner, "interface '%s' has no defined address", this_iface->name);
   }
 }
#line 2169 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 22:
#line 598 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->version = (yyvsp[-1].tok).t; }
#line 2175 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 23:
#line 599 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, SVERSION, new_config, (yyvsp[-1].tok).i); }
#line 2181 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 24:
#line 600 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->identity = (yyvsp[-1].tok).t; }
#line 2187 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 25:
#line 601 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, IDENTITY, new_config, (yyvsp[-1].tok).i); }
#line 2193 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 26:
#line 602 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.hostname' is deprecated, "
                         "use 'system.identity' instead");
     free((yyvsp[-1].tok).t);
 }
#line 2203 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 27:
#line 607 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->nsid = (yyvsp[-1].tok).t; new_config->nsid_len = (yyvsp[-1].tok).l; }
#line 2209 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 28:
#line 608 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->nsid = (yyvsp[-1].tok).t; new_config->nsid_len = strlen(new_config->nsid); }
#line 2215 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 29:
#line 609 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, NSID, new_config, (yyvsp[-1].tok).i); }
#line 2221 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 30:
#line 610 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->max_udp_payload, (yyvsp[-1].tok).i, KNOT_EDNS_MIN_UDP_PAYLOAD,
             KNOT_EDNS_MAX_UDP_PAYLOAD, "max-udp-payload");
 }
#line 2230 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 31:
#line 614 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.storage' was relocated, "
                         "use 'zones.storage' instead");
     new_config->storage = (yyvsp[-1].tok).t;
 }
#line 2240 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 32:
#line 619 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->rundir = (yyvsp[-1].tok).t; }
#line 2246 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 33:
#line 620 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->pidfile = (yyvsp[-1].tok).t; }
#line 2252 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 34:
#line 621 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.key' is deprecated and it has no effect");
     free((yyvsp[-1].tok).t);
 }
#line 2261 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 35:
#line 625 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->workers, (yyvsp[-1].tok).i, 1, 255, "workers");
 }
#line 2269 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 36:
#line 628 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->bg_workers, (yyvsp[-1].tok).i, 1, 255, "background-workers");
 }
#line 2277 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 37:
#line 631 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     new_config->async_start = (yyvsp[-1].tok).i;
 }
#line 2285 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 38:
#line 634 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     new_config->uid = new_config->gid = -1; // Invalidate
     char* dpos = strchr((yyvsp[-1].tok).t, '.'); // Find uid.gid format
     if (dpos != NULL) {
        struct group *grp = getgrnam(dpos + 1); // Skip dot
        if (grp != NULL) {
          new_config->gid = grp->gr_gid;
        } else {
          cf_error(scanner, "invalid group name '%s'", dpos + 1);
        }
        *dpos = '\0'; // Cut off
     }
     struct passwd* pwd = getpwnam((yyvsp[-1].tok).t);
     if (pwd != NULL) {
       new_config->uid = pwd->pw_uid;
     } else {
       cf_error(scanner, "invalid user name '%s'", (yyvsp[-1].tok).t);
     }

     free((yyvsp[-1].tok).t);
 }
#line 2311 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 39:
#line 655 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[-1].tok).i, "max-conn-idle");
 }
#line 2319 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 40:
#line 658 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[-1].tok).i, "max-conn-idle");
 }
#line 2327 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 41:
#line 661 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[-1].tok).i, "max-conn-handshake");
 }
#line 2335 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 42:
#line 664 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[-1].tok).i, "max-conn-handshake");
 }
#line 2343 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 43:
#line 667 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[-1].tok).i, "max-conn-reply");
 }
#line 2351 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 44:
#line 670 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[-1].tok).i, "max-conn-reply");
 }
#line 2359 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 45:
#line 673 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_tcp_clients, (yyvsp[-1].tok).i, "max-tcp-clients");
 }
#line 2367 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 46:
#line 676 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->rrl, (yyvsp[-1].tok).i, "rate-limit");
 }
#line 2375 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 47:
#line 679 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[-1].tok).l, "rate-limit-size");
 }
#line 2383 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 48:
#line 682 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[-1].tok).i, "rate-limit-size");
 }
#line 2391 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 49:
#line 685 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->rrl_slip, (yyvsp[-1].tok).i, 1, RRL_SLIP_MAX, "rate-limit-slip");
 }
#line 2399 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 50:
#line 688 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->xfers, (yyvsp[-1].tok).i, "transfers");
 }
#line 2407 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 52:
#line 695 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     /* Check algorithm length. */
     if (knot_tsig_digest_length((yyvsp[-2].tok).alg) == 0) {
        cf_error(scanner, "unsupported digest algorithm");
     }

     /* Normalize to FQDN */
     char *fqdn = (yyvsp[-3].tok).t;
     size_t fqdnl = strlen(fqdn);
     if (fqdn[fqdnl - 1] != '.') {
        fqdnl = ((fqdnl + 2)/4+1)*4; /* '.', '\0' */
        char* tmpdn = malloc(fqdnl);
	if (!tmpdn) {
	   cf_error(scanner, "out of memory when allocating string");
	   free(fqdn);
	   fqdn = NULL;
	} else {
	   strlcpy(tmpdn, fqdn, fqdnl);
	   strlcat(tmpdn, ".", fqdnl);
	   free(fqdn);
	   fqdn = tmpdn;
	}
     }

     if (fqdn != NULL && !conf_key_exists(scanner, fqdn)) {
         knot_dname_t *dname = knot_dname_from_str_alloc(fqdn);
	 if (!dname) {
             cf_error(scanner, "key name '%s' not in valid domain name format",
                      fqdn);
         } else {
             knot_dname_to_lower(dname);
             conf_key_t *k = malloc(sizeof(conf_key_t));
             memset(k, 0, sizeof(conf_key_t));
             k->k.name = dname;
             k->k.algorithm = (yyvsp[-2].tok).alg;
             if (knot_binary_from_base64((yyvsp[-1].tok).t, &(k->k.secret)) != 0) {
                 cf_error(scanner, "invalid key secret '%s'", (yyvsp[-1].tok).t);
                 knot_dname_free(&dname, NULL);
                 free(k);
             } else {
                 add_tail(&new_config->keys, &k->n);
             }
         }
     }

     free((yyvsp[-1].tok).t);
     free(fqdn);
}
#line 2460 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 54:
#line 745 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, (yyvsp[0].tok).t); }
#line 2466 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 55:
#line 746 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2472 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 56:
#line 747 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2478 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 57:
#line 748 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2484 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 58:
#line 749 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2490 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 60:
#line 753 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_remote->addr, (yyvsp[-1].tok).i);
     }
   }
#line 2502 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 61:
#line 760 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
#line 2511 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 62:
#line 764 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-3].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[-1].tok).i, 0, IPV4_PREFIXLEN, "prefix length");
   }
#line 2520 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 63:
#line 768 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
#line 2529 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 64:
#line 772 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
#line 2538 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 65:
#line 776 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-3].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[-1].tok).i, 0, IPV6_PREFIXLEN, "prefix length");
   }
#line 2547 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 66:
#line 780 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
#line 2556 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 67:
#line 784 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->key != 0) {
       cf_error(scanner, "only one TSIG key definition is allowed in remote section\n");
     } else {
        conf_key_add(scanner, &this_remote->key, (yyvsp[-1].tok).t);
     }
     free((yyvsp[-1].tok).t);
   }
#line 2569 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 68:
#line 792 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET, (yyvsp[-1].tok).t, 0);
   }
#line 2577 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 69:
#line 795 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET6, (yyvsp[-1].tok).t, 0);
   }
#line 2585 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 70:
#line 798 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_remote_set_via(scanner, (yyvsp[-1].tok).t);
     free((yyvsp[-1].tok).t);
   }
#line 2594 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 72:
#line 806 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "remote '%s' has no defined address", this_remote->name);
     }
   }
#line 2604 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 73:
#line 814 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_add_member_into_group(scanner, (yyvsp[0].tok).t); }
#line 2610 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 77:
#line 824 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_group(scanner, (yyvsp[0].tok).t); }
#line 2616 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 80:
#line 833 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.xfr_in;
   }
#line 2624 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 81:
#line 836 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.xfr_out;
   }
#line 2632 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 82:
#line 839 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.notify_in;
   }
#line 2640 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 83:
#line 842 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.notify_out;
   }
#line 2648 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 84:
#line 845 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.update_in;
 }
#line 2656 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 86:
#line 851 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, (yyvsp[0].tok).t); }
#line 2662 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 87:
#line 852 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2668 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 88:
#line 853 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2674 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 89:
#line 854 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2680 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 90:
#line 855 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2686 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 95:
#line 864 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      /* Find existing node in remotes. */
      node_t* r = 0; conf_iface_t* found = 0;
      WALK_LIST (r, new_config->remotes) {
	 if (strcmp(((conf_iface_t*)r)->name, (yyvsp[-1].tok).t) == 0) {
	    found = (conf_iface_t*)r;
	    break;
	 }
      }

      /* Append to list if found. */
      if (!found) {
	 cf_error(scanner, "remote '%s' is not defined", (yyvsp[-1].tok).t);
      } else {
	 conf_remote_t *remote = malloc(sizeof(conf_remote_t));
	 if (!remote) {
	    cf_error(scanner, "out of memory");
	 } else {
	    remote->remote = found;
	    add_tail(this_list, &remote->n);
	 }
      }

      /* Free text token. */
      free((yyvsp[-1].tok).t);
   }
#line 2717 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 96:
#line 893 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { query_module_create(scanner, (yyvsp[-1].tok).t, (yyvsp[0].tok).t, true); free((yyvsp[-1].tok).t); free((yyvsp[0].tok).t); }
#line 2723 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 100:
#line 901 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2729 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 101:
#line 902 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2735 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 102:
#line 903 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2741 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 103:
#line 904 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2747 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 104:
#line 905 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2753 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 105:
#line 906 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2759 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 106:
#line 907 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
    unsigned prefix_len = 0;
    SET_NUM(prefix_len, (yyvsp[-2].tok).i, 0, 255, "origin prefix length");
    size_t len = 3 + 1 + strlen((yyvsp[0].tok).t) + 1; /* <0,255> '/' rest */
    char *name = malloc(len * sizeof(char));
    if (name == NULL) {
        cf_error(scanner, "out of memory");
    } else {
        name[0] = '\0';
        if (snprintf(name, len, "%u/%s", prefix_len, (yyvsp[0].tok).t) < 0) {
            cf_error(scanner,"failed to convert rfc2317 origin to string");
        }
    }
    free((yyvsp[0].tok).t);
    conf_zone_start(scanner, name);
 }
#line 2780 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 107:
#line 923 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, (yyvsp[0].tok).t); }
#line 2786 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 111:
#line 930 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->file = (yyvsp[-1].tok).t; }
#line 2792 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 112:
#line 931 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->build_diffs = (yyvsp[-1].tok).i; }
#line 2798 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 113:
#line 932 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->enable_checks = (yyvsp[-1].tok).i; }
#line 2804 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 114:
#line 933 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->storage = (yyvsp[-1].tok).t; }
#line 2810 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 115:
#line 934 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->dnssec_keydir = (yyvsp[-1].tok).t; }
#line 2816 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 116:
#line 935 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->disable_any = (yyvsp[-1].tok).i; }
#line 2822 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 117:
#line 936 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[-1].tok).i, "zonefile-sync");
 }
#line 2830 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 118:
#line 939 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[-1].tok).i, "zonefile-sync");
 }
#line 2838 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 119:
#line 942 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).l, "ixfr-fslimit");
 }
#line 2846 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 120:
#line 945 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(this_zone->ixfr_fslimit, (yyvsp[-1].tok).i, "ixfr-fslimit");
 }
#line 2854 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 121:
#line 948 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_retries, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-retries");
   }
#line 2862 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 122:
#line 951 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2870 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 123:
#line 954 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2878 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 124:
#line 957 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->dnssec_enable = (yyvsp[-1].tok).i; }
#line 2884 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 125:
#line 958 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 2892 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 126:
#line 961 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 2900 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 127:
#line 964 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	this_zone->serial_policy = (yyvsp[-1].tok).i;
 }
#line 2908 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 129:
#line 968 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_UINT16(this_zone->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
     this_zone->req_edns_data = (yyvsp[-1].tok).t;
     this_zone->req_edns_data_len = (yyvsp[-1].tok).l;
 }
#line 2918 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 130:
#line 973 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_UINT16(this_zone->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
     this_zone->req_edns_data = (yyvsp[-1].tok).t;
     this_zone->req_edns_data_len = strlen(this_zone->req_edns_data);
 }
#line 2928 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 131:
#line 981 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { query_module_create(scanner, (yyvsp[-1].tok).t, (yyvsp[0].tok).t, false); free((yyvsp[-1].tok).t); free((yyvsp[0].tok).t); }
#line 2934 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 136:
#line 990 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->disable_any = (yyvsp[-1].tok).i; }
#line 2940 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 137:
#line 991 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->build_diffs = (yyvsp[-1].tok).i; }
#line 2946 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 138:
#line 992 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->zone_checks = (yyvsp[-1].tok).i; }
#line 2952 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 139:
#line 993 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).l, "ixfr-fslimit");
 }
#line 2960 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 140:
#line 996 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).i, "ixfr-fslimit");
 }
#line 2968 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 141:
#line 999 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_retries, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-retries");
   }
#line 2976 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1002 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2984 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1005 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2992 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1008 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[-1].tok).i, 0, INT_MAX, "zonefile-sync");
 }
#line 3000 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1011 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[-1].tok).i, 0, INT_MAX, "zonefile-sync");
 }
#line 3008 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1014 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->storage = (yyvsp[-1].tok).t; }
#line 3014 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1015 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->dnssec_enable = (yyvsp[-1].tok).i; }
#line 3020 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1016 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->dnssec_keydir = (yyvsp[-1].tok).t; }
#line 3026 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1017 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 3034 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1020 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 3042 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1023 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	new_config->serial_policy = (yyvsp[-1].tok).i;
 }
#line 3050 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1026 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_UINT16(new_config->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
	new_config->req_edns_data = (yyvsp[-1].tok).t;
	new_config->req_edns_data_len = (yyvsp[-1].tok).l;
 }
#line 3060 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1031 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_UINT16(new_config->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
	new_config->req_edns_data = (yyvsp[-1].tok).t;
 	new_config->req_edns_data_len = strlen(new_config->req_edns_data);
 }
#line 3070 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1039 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
  this_logmap = malloc(sizeof(conf_log_map_t));
  this_logmap->source = 0;
  this_logmap->prios = 0;
  add_tail(&this_log->map, &this_logmap->n);
}
#line 3081 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1049 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_logmap->prios |= (yyvsp[-1].tok).i;
	cf_warning(scanner, "multiple log severities are deprecated, "
	                    "using the least serious one");
 }
#line 3090 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1053 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_logmap->prios |= (yyvsp[-1].tok).i; }
#line 3096 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1057 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     this_logmap->source = (yyvsp[-1].tok).i;
     this_logmap = 0;
   }
#line 3105 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1063 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
  /* Find already existing rule. */
  this_log = 0;
  node_t *n = 0;
  WALK_LIST(n, new_config->logs) {
    conf_log_t* log = (conf_log_t*)n;
    if (log->type == (yyvsp[0].tok).i) {
      this_log = log;
      break;
    }
  }

  if (!this_log) {
    this_log = malloc(sizeof(conf_log_t));
    this_log->type = (yyvsp[0].tok).i;
    this_log->file = 0;
    init_list(&this_log->map);
    add_tail(&new_config->logs, &this_log->n);
  }
}
#line 3130 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1085 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
  /* Find already existing rule. */
  this_log = 0;
  node_t *n = 0;
  WALK_LIST(n, new_config->logs) {
    conf_log_t* log = (conf_log_t*)n;
    if (log->type == LOGT_FILE) {
      if (strcmp((yyvsp[0].tok).t, log->file) == 0) {
        this_log = log;
	free((yyvsp[0].tok).t);
        break;
      }
    }
  }

  /* Create new rule. */
  if (!this_log) {
    this_log = malloc(sizeof(conf_log_t));
    this_log->type = LOGT_FILE;
    this_log->file = strcpath((yyvsp[0].tok).t);
    init_list(&this_log->map);
    add_tail(&new_config->logs, &this_log->n);
  }
}
#line 3159 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1111 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
}
#line 3166 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1120 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { }
#line 3172 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1124 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_init_iface(scanner, NULL); }
#line 3178 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1128 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
    this_list = &new_config->ctl.allow;
  }
#line 3186 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1134 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->ctl.have = true; }
#line 3192 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1135 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "control interface has no defined address");
     } else {
       new_config->ctl.iface = this_iface;
     }
 }
#line 3204 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1142 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     sockaddr_set(&this_iface->addr, AF_UNIX, (yyvsp[-1].tok).t, 0);
     new_config->ctl.iface = this_iface;
     free((yyvsp[-1].tok).t);
 }
#line 3214 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;


#line 3218 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1153 "knot/conf/cf-parse.y" /* yacc.c:1906  */

