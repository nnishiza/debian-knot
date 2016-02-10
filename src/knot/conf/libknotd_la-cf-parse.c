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
    TIMER_DB = 307,
    DNSSEC_ENABLE = 308,
    DNSSEC_KEYDIR = 309,
    SIGNATURE_LIFETIME = 310,
    SERIAL_POLICY = 311,
    SERIAL_POLICY_VAL = 312,
    QUERY_MODULE = 313,
    INTERFACES = 314,
    ADDRESS = 315,
    PORT = 316,
    IPA = 317,
    IPA6 = 318,
    VIA = 319,
    CONTROL = 320,
    ALLOW = 321,
    LISTEN_ON = 322,
    LOG = 323,
    LOG_DEST = 324,
    LOG_SRC = 325,
    LOG_LEVEL = 326
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
#define TIMER_DB 307
#define DNSSEC_ENABLE 308
#define DNSSEC_KEYDIR 309
#define SIGNATURE_LIFETIME 310
#define SERIAL_POLICY 311
#define SERIAL_POLICY_VAL 312
#define QUERY_MODULE 313
#define INTERFACES 314
#define ADDRESS 315
#define PORT 316
#define IPA 317
#define IPA6 318
#define VIA 319
#define CONTROL 320
#define ALLOW 321
#define LISTEN_ON 322
#define LOG 323
#define LOG_DEST 324
#define LOG_SRC 325
#define LOG_LEVEL 326

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

#line 710 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cf_parse (void *scanner);

#endif /* !YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 724 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:358  */

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
#define YYLAST   372

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  185
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  387

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

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
       2,     2,     2,     2,    77,     2,     2,    76,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    72,
       2,     2,     2,     2,    73,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    74,     2,    75,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   550,   550,   550,   552,   554,   557,   558,   559,   560,
     561,   562,   563,   566,   567,   574,   577,   580,   583,   589,
     590,   598,   599,   600,   601,   602,   603,   608,   609,   610,
     611,   615,   620,   621,   622,   626,   629,   632,   635,   656,
     659,   662,   665,   668,   671,   674,   677,   680,   683,   686,
     689,   695,   696,   745,   746,   747,   748,   749,   750,   753,
     754,   761,   765,   769,   773,   777,   781,   785,   793,   796,
     799,   806,   807,   815,   818,   820,   821,   825,   829,   830,
     834,   837,   840,   843,   846,   851,   852,   853,   854,   855,
     856,   859,   860,   861,   864,   865,   894,   897,   898,   901,
     902,   903,   904,   905,   906,   907,   908,   924,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   940,   943,
     946,   949,   952,   955,   958,   959,   962,   965,   968,   969,
     974,   982,   984,   985,   989,   990,   991,   992,   993,   994,
     997,  1000,  1003,  1006,  1009,  1012,  1015,  1016,  1017,  1018,
    1019,  1022,  1025,  1028,  1033,  1038,  1041,  1050,  1051,  1055,
    1058,  1059,  1065,  1087,  1113,  1117,  1118,  1119,  1122,  1122,
    1126,  1130,  1136,  1137,  1144,  1149,  1150,  1153,  1153,  1153,
    1153,  1153,  1153,  1153,  1153,  1153
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
  "RATE_LIMIT_SLIP", "TRANSFERS", "STORAGE", "TIMER_DB", "DNSSEC_ENABLE",
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
     325,   326,    59,    64,   123,   125,    47,    44
};
# endif

#define YYPACT_NINF -73

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-73)))

#define YYTABLE_NINF -86

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -73,     9,   -73,   -73,    84,   -73,   -72,   -34,   -21,    12,
      16,    31,    36,   -73,   -73,    14,   114,     0,    23,     1,
       3,   -29,   -54,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,    45,   -73,   -73,   -73,   -73,   -73,   -73,   -73,    48,
      13,    75,    15,    97,    63,    89,   124,   141,   168,   150,
     177,   178,    25,   117,   126,   179,   180,    17,   181,   196,
     199,   -73,   164,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     111,   -73,   -73,   131,   -73,   130,   200,   -73,   -73,   198,
     202,   203,   134,   137,    43,   207,   204,   214,   210,   216,
     139,   166,   148,   -73,   -73,   -73,   -73,   -73,   151,   160,
     -73,   -73,   -73,   -73,    -4,   152,   -73,   -73,   155,   156,
     157,   158,   162,   167,   169,   170,   219,   171,   172,   173,
     174,   175,   176,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   226,   -73,   227,
     228,   145,   197,   201,   205,   206,   208,   209,   211,   212,
     213,   215,   217,   218,   220,   221,   222,   223,   224,   231,
     -73,   230,   233,   239,   240,   244,   146,   159,    99,   -73,
     -73,   -73,   -73,   -73,   242,   235,   243,   263,   161,   225,
     229,   -73,   232,   236,   -73,   -73,    -5,   -26,   -46,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   237,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   238,   -13,
     -73,   -73,    46,   -73,   241,   245,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   265,   246,   247,   165,   248,   249,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   264,   266,   267,   269,   -73,    -5,   -73,   -44,     2,
     -73,   -73,   -73,   -73,   -73,   -28,   270,   -73,   268,   271,
     -73,   110,   272,   -73,   -73,   -73,   281,   112,   284,    37,
     -73,   -73,   227,   -73,   -73,   -73,   231,   -73,   274,   275,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   283,   276,   277,
       6,   -73,   278,   -73,   -73,   -73,   -73,   -73,   -73,   104,
     108,   279,   282,    42,    44,   285,   286,   287,   288,   -73,
     -73,   -73,   -73,   -73,   269,   -73,   -73,   -73,    -6,    22,
     -73,   290,   -73,   291,   -73,   -73,   -73,   292,   293,   -73,
     294,   295,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     289,   296,   297,   298,   299,   300,   -73,   234,   -73,   -73,
     -73,   -73,   -73,   -73,    32,   -73,   -73
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     4,     1,     0,     3,     0,     0,     0,     0,
       0,     0,     0,   168,   177,     6,     0,     0,    53,     0,
      99,     0,     0,     5,    21,    51,    71,    78,   134,    19,
     172,     0,     7,     8,    12,    10,     9,    11,   179,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   178,     0,   180,    54,    58,    56,    55,    57,   181,
       0,    77,   182,     0,   107,     0,     0,   100,   101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   103,   102,   104,   183,     0,     0,
     184,   171,   170,   185,     0,    91,   165,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   132,
     108,     0,     0,     0,     0,     0,     0,     0,     0,    80,
      81,    84,    82,    83,     0,     0,     0,     0,     0,     0,
       0,   135,    91,     0,    13,    94,   176,   164,     0,    24,
      25,    26,    22,    23,    28,    27,    29,     0,    30,    35,
      36,    37,    38,    32,    33,    40,    39,    42,    41,    44,
      43,    45,    46,    48,    47,    49,    50,    31,     0,     0,
      73,    75,     0,   106,     0,     0,   136,   138,   141,   142,
     143,   144,   145,   140,   139,   137,   146,   147,   148,   149,
     150,   151,   152,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    94,   110,   174,     0,     0,
      86,    90,    88,    87,    89,     0,     0,   162,     0,     0,
     169,     0,     0,    20,    34,    52,     0,     0,     0,     0,
      72,    79,     0,   154,   153,   131,   132,   155,     0,     0,
     111,   116,   113,   121,   123,   122,   117,   118,   120,   119,
     112,   114,   124,   115,   125,   126,   127,     0,     0,     0,
       0,   173,     0,   175,    93,    92,   163,   160,   160,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    76,
     133,   130,   129,    96,    97,   128,   109,    95,     0,     0,
      15,     0,    17,     0,    14,    67,    61,     0,     0,    64,
       0,     0,    60,    70,    68,    69,    98,   156,   166,   167,
       0,     0,     0,     0,     0,     0,   157,   161,    16,    18,
      63,    62,    66,    65,     0,   159,   158
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -73,   -73,   -73,   -73,   -73,    87,   -73,   -73,   -73,   -73,
     -73,   -73,   -20,   -73,   -73,   -73,   -73,   -73,    94,    39,
     -73,   -37,   -73,   -73,   -73,    18,   -73,   -73,   -73,   -17,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    39,   188,    15,    16,    17,    70,
     219,    18,   221,   222,    73,    19,   182,   275,   186,   269,
     318,   319,    98,    99,   244,   245,    20,   376,   377,   348,
     278,   279,   280,   187,    21,    31,   104,   105,    22,    23
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     270,   183,    24,   286,   276,    62,    71,   322,    74,     3,
      75,   322,   101,   102,   281,   282,   281,   282,   108,    32,
     111,   103,    76,   109,   132,   112,   133,    77,    64,   283,
      78,   321,   124,   125,    79,    80,    81,    82,    83,    84,
      25,    33,   336,   277,   324,    85,   100,   287,   288,   325,
     149,   289,   150,    26,    86,    87,    88,    89,    90,    91,
     271,    92,   290,   272,   367,   273,   274,   -85,    93,   368,
     184,    94,   -85,    95,    96,    63,    72,   323,    97,    34,
     110,   346,    35,   116,    36,    37,    27,     5,    65,    38,
      28,    66,   367,    67,    68,     6,   117,   369,    69,   337,
     338,     7,   113,   114,   385,    29,   255,   115,   256,   386,
      30,     8,     9,    10,   356,   357,   359,   360,   358,   106,
     361,   291,   107,   292,   126,   127,    40,    41,    42,    43,
      44,   118,    45,   128,   129,    46,    47,    48,    49,    50,
      51,   145,   146,    11,   147,   148,   156,   157,   119,    12,
     224,   225,    13,   251,   252,   121,    14,    52,    53,    54,
      55,    56,    57,    58,    59,    60,   253,   254,   261,   262,
     298,   299,   329,   330,   333,   334,   350,   351,   120,   161,
     352,   353,   122,   123,   137,   138,   130,   131,   134,    61,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   135,   136,   139,   140,   141,   142,   152,
     144,   175,   143,   176,   177,   178,   179,   151,   180,   153,
     154,   155,   159,   158,   197,   160,   185,   189,   190,   191,
     192,   218,   220,   223,   193,   181,   243,   246,   247,   194,
     258,   195,   196,   198,   199,   200,   201,   202,   203,   248,
     249,   250,   257,   259,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   260,   226,
     295,   268,   339,   227,   317,   326,   266,   228,   229,   331,
     230,   231,   263,   232,   233,   234,   332,   235,   343,   236,
     237,   335,   238,   239,   240,   241,   242,   370,   371,   372,
     373,   374,   375,   264,   320,   384,   265,   366,   267,   284,
     285,   349,     0,   293,   340,     0,     0,   294,   296,     0,
     300,   301,   297,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,     0,   314,     0,   315,   316,
       0,     0,   327,     0,     0,   328,   341,   342,   344,     0,
     347,   354,   345,     0,   355,     0,     0,   362,   363,   364,
     365,   378,     0,     0,     0,     0,     0,     0,   379,   380,
     381,   382,   383
};

static const yytype_int16 yycheck[] =
{
       5,     5,    74,    16,    30,     5,     5,     5,     5,     0,
       7,     5,    66,    67,    60,    61,    60,    61,     5,     5,
       5,    75,    19,    10,     7,    10,     9,    24,     5,    75,
      27,    75,     7,     8,    31,    32,    33,    34,    35,    36,
      74,    27,     5,    69,    72,    42,    75,    60,    61,    77,
       7,    64,     9,    74,    51,    52,    53,    54,    55,    56,
      65,    58,    75,    68,    70,    70,    71,    72,    65,    75,
      74,    68,    77,    70,    71,    75,    75,    75,    75,    65,
       5,    75,    68,    20,    70,    71,    74,     3,    65,    75,
      74,    68,    70,    70,    71,    11,     7,    75,    75,    62,
      63,    17,     5,     6,    72,    74,     7,    10,     9,    77,
      74,    27,    28,    29,    72,    73,    72,    73,    76,    74,
      76,    75,    74,    77,     7,     8,    12,    13,    14,    15,
      16,     7,    18,     7,     8,    21,    22,    23,    24,    25,
      26,     7,     8,    59,     7,     8,     7,     8,     7,    65,
       5,     6,    68,     7,     8,     5,    72,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     7,     8,     7,     8,
       5,     6,    62,    63,    62,    63,    72,    73,    10,    19,
      72,    73,     5,     5,    20,    74,     7,     7,     7,    75,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     7,     5,    74,    76,     7,    10,     5,
       7,    51,    10,    53,    54,    55,    56,    10,    58,     5,
      10,     5,    74,    57,     5,    74,    74,    72,    72,    72,
      72,     5,     5,     5,    72,    75,     5,     7,     5,    72,
       5,    72,    72,    72,    72,    72,    72,    72,    72,    10,
      10,     7,    10,    10,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,     5,    72,
       5,   184,   292,    72,     5,     5,   182,    72,    72,     7,
      72,    72,    57,    72,    72,    72,     5,    72,     5,    72,
      72,     7,    72,    72,    72,    72,    72,     7,     7,     7,
       7,     7,     7,    74,   265,    71,    74,   344,    72,    72,
      72,   328,    -1,    72,   296,    -1,    -1,    72,    72,    -1,
      72,    72,    75,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    -1,    72,    -1,    72,    72,
      -1,    -1,    74,    -1,    -1,    74,    72,    72,    72,    -1,
      72,    72,    75,    -1,    72,    -1,    -1,    72,    72,    72,
      72,    72,    -1,    -1,    -1,    -1,    -1,    -1,    72,    72,
      72,    72,    72
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    79,    80,     0,    81,     3,    11,    17,    27,    28,
      29,    59,    65,    68,    72,    84,    85,    86,    89,    93,
     104,   112,   116,   117,    74,    74,    74,    74,    74,    74,
      74,   113,     5,    27,    65,    68,    70,    71,    75,    82,
      12,    13,    14,    15,    16,    18,    21,    22,    23,    24,
      25,    26,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    75,     5,    75,     5,    65,    68,    70,    71,    75,
      87,     5,    75,    92,     5,     7,    19,    24,    27,    31,
      32,    33,    34,    35,    36,    42,    51,    52,    53,    54,
      55,    56,    58,    65,    68,    70,    71,    75,   100,   101,
      75,    66,    67,    75,   114,   115,    74,    74,     5,    10,
       5,     5,    10,     5,     6,    10,    20,     7,     7,     7,
      10,     5,     5,     5,     7,     8,     7,     8,     7,     8,
       7,     7,     7,     9,     7,     7,     5,    20,    74,    74,
      76,     7,    10,    10,     7,     7,     8,     7,     8,     7,
       9,    10,     5,     5,    10,     5,     7,     8,    57,    74,
      74,    19,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    51,    53,    54,    55,    56,
      58,    75,    94,     5,    74,    74,    96,   111,    83,    72,
      72,    72,    72,    72,    72,    72,    72,     5,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,     5,    88,
       5,    90,    91,     5,     5,     6,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,     5,   102,   103,     7,     5,    10,    10,
       7,     7,     8,     7,     8,     7,     9,    10,     5,    10,
       5,     7,     8,    57,    74,    74,    96,    72,    83,    97,
       5,    65,    68,    70,    71,    95,    30,    69,   108,   109,
     110,    60,    61,    75,    72,    72,    16,    60,    61,    64,
      75,    75,    77,    72,    72,     5,    72,    75,     5,     6,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,     5,    98,    99,
      97,    75,     5,    75,    72,    77,     5,    74,    74,    62,
      63,     7,     5,    62,    63,     7,     5,    62,    63,    90,
     103,    72,    72,     5,    72,    75,    75,    72,   107,   107,
      72,    73,    72,    73,    72,    72,    72,    73,    76,    72,
      73,    76,    72,    72,    72,    72,    99,    70,    75,    75,
       7,     7,     7,     7,     7,     7,   105,   106,    72,    72,
      72,    72,    72,    72,    71,    72,    77
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    80,    79,    81,    81,    82,    82,    82,    82,
      82,    82,    82,    83,    83,    83,    83,    83,    83,    84,
      84,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    86,    86,    87,    87,    87,    87,    87,    87,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    89,    89,    90,    91,    91,    91,    92,    93,    93,
      94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
      95,    96,    96,    96,    97,    97,    98,    99,    99,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   102,   103,   103,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   105,   106,   106,   106,
     107,   107,   108,   109,   110,   111,   111,   111,   113,   112,
     114,   115,   116,   116,   116,   116,   116,   117,   117,   117,
     117,   117,   117,   117,   117,   117
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
       4,     4,     4,     5,     5,     5,     0,     1,     3,     3,
       0,     3,     1,     2,     0,     0,     5,     5,     0,     5,
       1,     1,     2,     5,     4,     5,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2
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
#line 550 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start(scanner); }
#line 2077 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 3:
#line 550 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { return 0; }
#line 2083 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 7:
#line 558 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, (yyvsp[0].tok).t); }
#line 2089 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 8:
#line 559 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2095 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 9:
#line 560 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2101 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 10:
#line 561 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2107 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 11:
#line 562 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2113 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 12:
#line 563 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_iface(scanner, strdup((yyvsp[0].tok).t)); }
#line 2119 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 14:
#line 567 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_iface->addr, (yyvsp[-1].tok).i);
     }
   }
#line 2131 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 15:
#line 574 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
   }
#line 2139 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 16:
#line 577 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
   }
#line 2147 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 17:
#line 580 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
   }
#line 2155 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 18:
#line 583 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
   }
#line 2163 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 20:
#line 590 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
   if (this_iface->addr.ss_family == AF_UNSPEC) {
     cf_error(scanner, "interface '%s' has no defined address", this_iface->name);
   }
 }
#line 2173 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 22:
#line 599 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->version = (yyvsp[-1].tok).t; }
#line 2179 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 23:
#line 600 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, SVERSION, new_config, (yyvsp[-1].tok).i); }
#line 2185 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 24:
#line 601 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->identity = (yyvsp[-1].tok).t; }
#line 2191 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 25:
#line 602 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, IDENTITY, new_config, (yyvsp[-1].tok).i); }
#line 2197 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 26:
#line 603 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.hostname' is deprecated, "
                         "use 'system.identity' instead");
     free((yyvsp[-1].tok).t);
 }
#line 2207 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 27:
#line 608 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->nsid = (yyvsp[-1].tok).t; new_config->nsid_len = (yyvsp[-1].tok).l; }
#line 2213 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 28:
#line 609 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->nsid = (yyvsp[-1].tok).t; new_config->nsid_len = strlen(new_config->nsid); }
#line 2219 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 29:
#line 610 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { ident_auto(scanner, NSID, new_config, (yyvsp[-1].tok).i); }
#line 2225 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 30:
#line 611 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->max_udp_payload, (yyvsp[-1].tok).i, KNOT_EDNS_MIN_UDP_PAYLOAD,
             KNOT_EDNS_MAX_UDP_PAYLOAD, "max-udp-payload");
 }
#line 2234 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 31:
#line 615 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.storage' was relocated, "
                         "use 'zones.storage' instead");
     new_config->storage = (yyvsp[-1].tok).t;
 }
#line 2244 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 32:
#line 620 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->rundir = (yyvsp[-1].tok).t; }
#line 2250 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 33:
#line 621 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->pidfile = (yyvsp[-1].tok).t; }
#line 2256 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 34:
#line 622 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     cf_warning(scanner, "option 'system.key' is deprecated and it has no effect");
     free((yyvsp[-1].tok).t);
 }
#line 2265 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 35:
#line 626 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->workers, (yyvsp[-1].tok).i, 1, 255, "workers");
 }
#line 2273 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 36:
#line 629 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_NUM(new_config->bg_workers, (yyvsp[-1].tok).i, 1, 255, "background-workers");
 }
#line 2281 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 37:
#line 632 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     new_config->async_start = (yyvsp[-1].tok).i;
 }
#line 2289 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 38:
#line 635 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 2315 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 39:
#line 656 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[-1].tok).i, "max-conn-idle");
 }
#line 2323 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 40:
#line 659 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[-1].tok).i, "max-conn-idle");
 }
#line 2331 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 41:
#line 662 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[-1].tok).i, "max-conn-handshake");
 }
#line 2339 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 42:
#line 665 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[-1].tok).i, "max-conn-handshake");
 }
#line 2347 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 43:
#line 668 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[-1].tok).i, "max-conn-reply");
 }
#line 2355 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 44:
#line 671 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[-1].tok).i, "max-conn-reply");
 }
#line 2363 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 45:
#line 674 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->max_tcp_clients, (yyvsp[-1].tok).i, "max-tcp-clients");
 }
#line 2371 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 46:
#line 677 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->rrl, (yyvsp[-1].tok).i, "rate-limit");
 }
#line 2379 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 47:
#line 680 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[-1].tok).l, "rate-limit-size");
 }
#line 2387 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 48:
#line 683 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[-1].tok).i, "rate-limit-size");
 }
#line 2395 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 49:
#line 686 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->rrl_slip, (yyvsp[-1].tok).i, 0, RRL_SLIP_MAX, "rate-limit-slip");
 }
#line 2403 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 50:
#line 689 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(new_config->xfers, (yyvsp[-1].tok).i, "transfers");
 }
#line 2411 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 52:
#line 696 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 2464 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 54:
#line 746 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, (yyvsp[0].tok).t); }
#line 2470 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 55:
#line 747 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2476 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 56:
#line 748 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2482 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 57:
#line 749 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2488 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 58:
#line 750 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_remote(scanner, strdup((yyvsp[0].tok).t)); }
#line 2494 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 60:
#line 754 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_remote->addr, (yyvsp[-1].tok).i);
     }
   }
#line 2506 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 61:
#line 761 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
#line 2515 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 62:
#line 765 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-3].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[-1].tok).i, 0, IPV4_PREFIXLEN, "prefix length");
   }
#line 2524 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 63:
#line 769 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
#line 2533 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 64:
#line 773 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-1].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
#line 2542 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 65:
#line 777 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-3].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[-1].tok).i, 0, IPV6_PREFIXLEN, "prefix length");
   }
#line 2551 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 66:
#line 781 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[-3].tok).t, (yyvsp[-1].tok).i);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
#line 2560 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 67:
#line 785 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->key != 0) {
       cf_error(scanner, "only one TSIG key definition is allowed in remote section\n");
     } else {
        conf_key_add(scanner, &this_remote->key, (yyvsp[-1].tok).t);
     }
     free((yyvsp[-1].tok).t);
   }
#line 2573 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 68:
#line 793 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET, (yyvsp[-1].tok).t, 0);
   }
#line 2581 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 69:
#line 796 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET6, (yyvsp[-1].tok).t, 0);
   }
#line 2589 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 70:
#line 799 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     conf_remote_set_via(scanner, (yyvsp[-1].tok).t);
     free((yyvsp[-1].tok).t);
   }
#line 2598 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 72:
#line 807 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "remote '%s' has no defined address", this_remote->name);
     }
   }
#line 2608 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 73:
#line 815 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_add_member_into_group(scanner, (yyvsp[0].tok).t); }
#line 2614 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 77:
#line 825 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_start_group(scanner, (yyvsp[0].tok).t); }
#line 2620 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 80:
#line 834 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.xfr_in;
   }
#line 2628 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 81:
#line 837 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.xfr_out;
   }
#line 2636 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 82:
#line 840 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.notify_in;
   }
#line 2644 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 83:
#line 843 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.notify_out;
   }
#line 2652 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 84:
#line 846 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
      this_list = &this_zone->acl.update_in;
 }
#line 2660 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 86:
#line 852 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, (yyvsp[0].tok).t); }
#line 2666 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 87:
#line 853 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2672 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 88:
#line 854 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2678 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 89:
#line 855 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2684 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 90:
#line 856 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_acl_item(scanner, strdup((yyvsp[0].tok).t)); }
#line 2690 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 95:
#line 865 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 2721 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 96:
#line 894 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { query_module_create(scanner, (yyvsp[-1].tok).t, (yyvsp[0].tok).t, true); free((yyvsp[-1].tok).t); free((yyvsp[0].tok).t); }
#line 2727 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 100:
#line 902 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2733 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 101:
#line 903 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2739 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 102:
#line 904 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2745 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 103:
#line 905 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2751 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 104:
#line 906 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2757 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 105:
#line 907 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, strdup((yyvsp[0].tok).t)); }
#line 2763 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 106:
#line 908 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 2784 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 107:
#line 924 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_zone_start(scanner, (yyvsp[0].tok).t); }
#line 2790 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 111:
#line 931 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->file = (yyvsp[-1].tok).t; }
#line 2796 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 112:
#line 932 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->build_diffs = (yyvsp[-1].tok).i; }
#line 2802 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->enable_checks = (yyvsp[-1].tok).i; }
#line 2808 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->storage = (yyvsp[-1].tok).t; }
#line 2814 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 115:
#line 935 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->dnssec_keydir = (yyvsp[-1].tok).t; }
#line 2820 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 116:
#line 936 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->disable_any = (yyvsp[-1].tok).i; }
#line 2826 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 117:
#line 937 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[-1].tok).i, "zonefile-sync");
 }
#line 2834 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 118:
#line 940 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[-1].tok).i, "zonefile-sync");
 }
#line 2842 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 119:
#line 943 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).l, "ixfr-fslimit");
 }
#line 2850 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 120:
#line 946 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(this_zone->ixfr_fslimit, (yyvsp[-1].tok).i, "ixfr-fslimit");
 }
#line 2858 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 121:
#line 949 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_retries, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-retries");
   }
#line 2866 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 122:
#line 952 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2874 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 123:
#line 955 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2882 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 124:
#line 958 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_zone->dnssec_enable = (yyvsp[-1].tok).i; }
#line 2888 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 125:
#line 959 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 2896 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 126:
#line 962 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 2904 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 127:
#line 965 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	this_zone->serial_policy = (yyvsp[-1].tok).i;
 }
#line 2912 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 129:
#line 969 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_UINT16(this_zone->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
     this_zone->req_edns_data = (yyvsp[-1].tok).t;
     this_zone->req_edns_data_len = (yyvsp[-1].tok).l;
 }
#line 2922 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 130:
#line 974 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     SET_UINT16(this_zone->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
     this_zone->req_edns_data = (yyvsp[-1].tok).t;
     this_zone->req_edns_data_len = strlen(this_zone->req_edns_data);
 }
#line 2932 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 131:
#line 982 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { query_module_create(scanner, (yyvsp[-1].tok).t, (yyvsp[0].tok).t, false); free((yyvsp[-1].tok).t); free((yyvsp[0].tok).t); }
#line 2938 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 136:
#line 991 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->disable_any = (yyvsp[-1].tok).i; }
#line 2944 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 137:
#line 992 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->build_diffs = (yyvsp[-1].tok).i; }
#line 2950 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 138:
#line 993 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->zone_checks = (yyvsp[-1].tok).i; }
#line 2956 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 139:
#line 994 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).l, "ixfr-fslimit");
 }
#line 2964 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 140:
#line 997 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[-1].tok).i, "ixfr-fslimit");
 }
#line 2972 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1000 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_retries, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-retries");
   }
#line 2980 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1003 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2988 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1006 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[-1].tok).i, 1, INT_MAX, "notify-timeout");
   }
#line 2996 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1009 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[-1].tok).i, 0, INT_MAX, "zonefile-sync");
 }
#line 3004 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1012 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[-1].tok).i, 0, INT_MAX, "zonefile-sync");
 }
#line 3012 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1015 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->storage = (yyvsp[-1].tok).t; }
#line 3018 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1016 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->timer_db = (yyvsp[-1].tok).t; }
#line 3024 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1017 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->dnssec_enable = (yyvsp[-1].tok).i; }
#line 3030 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1018 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->dnssec_keydir = (yyvsp[-1].tok).t; }
#line 3036 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1019 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 3044 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1022 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[-1].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
#line 3052 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1025 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	new_config->serial_policy = (yyvsp[-1].tok).i;
 }
#line 3060 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1028 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_UINT16(new_config->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
	new_config->req_edns_data = (yyvsp[-1].tok).t;
	new_config->req_edns_data_len = (yyvsp[-1].tok).l;
 }
#line 3070 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1033 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
	SET_UINT16(new_config->req_edns_code, (yyvsp[-2].tok).i, "request-edns-option");
	new_config->req_edns_data = (yyvsp[-1].tok).t;
 	new_config->req_edns_data_len = strlen(new_config->req_edns_data);
 }
#line 3080 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1041 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
  this_logmap = malloc(sizeof(conf_log_map_t));
  this_logmap->source = 0;
  this_logmap->prios = 0;
  add_tail(&this_log->map, &this_logmap->n);
}
#line 3091 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1051 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_logmap->prios |= (yyvsp[-1].tok).i;
	cf_warning(scanner, "multiple log severities are deprecated, "
	                    "using the least serious one");
 }
#line 3100 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1055 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { this_logmap->prios |= (yyvsp[-1].tok).i; }
#line 3106 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1059 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     this_logmap->source = (yyvsp[-1].tok).i;
     this_logmap = 0;
   }
#line 3115 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1065 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 3140 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1087 "knot/conf/cf-parse.y" /* yacc.c:1646  */
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
#line 3169 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1113 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
}
#line 3176 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1122 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { }
#line 3182 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1126 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { conf_init_iface(scanner, NULL); }
#line 3188 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1130 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
    this_list = &new_config->ctl.allow;
  }
#line 3196 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1136 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    { new_config->ctl.have = true; }
#line 3202 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1137 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "control interface has no defined address");
     } else {
       new_config->ctl.iface = this_iface;
     }
 }
#line 3214 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1144 "knot/conf/cf-parse.y" /* yacc.c:1646  */
    {
     sockaddr_set(&this_iface->addr, AF_UNIX, (yyvsp[-1].tok).t, 0);
     new_config->ctl.iface = this_iface;
     free((yyvsp[-1].tok).t);
 }
#line 3224 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
    break;


#line 3228 "knot/conf/libknotd_la-cf-parse.c" /* yacc.c:1646  */
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
#line 1155 "knot/conf/cf-parse.y" /* yacc.c:1906  */

