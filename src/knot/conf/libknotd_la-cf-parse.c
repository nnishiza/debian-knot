/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.7"

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
#define yylval          cf_lval
#define yychar          cf_char
#define yydebug         cf_debug
#define yynerrs         cf_nerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 23 "knot/conf/cf-parse.y"


#include <config.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "common/sockaddr.h"
#include "common/strlcat.h"
#include "common/strlcpy.h"
#include "libknot/dname.h"
#include "libknot/binary.h"
#include "libknot/rrtype/opt.h"
#include "knot/server/rrl.h"
#include "knot/nameserver/query_module.h"
#include "knot/conf/conf.h"
#include "knot/conf/libknotd_la-cf-parse.h" /* Automake generated header. */

extern int cf_lex (YYSTYPE *lvalp, void *scanner);
extern void cf_error(void *scanner, const char *format, ...);
extern conf_t *new_config;
static conf_iface_t *this_iface = 0;
static conf_iface_t *this_remote = 0;
static conf_zone_t *this_zone = 0;
static conf_group_t *this_group = 0;
static list_t *this_list = 0;
static conf_log_t *this_log = 0;
static conf_log_map_t *this_logmap = 0;
//#define YYERROR_VERBOSE 1

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
	knot_dname_t *sample = knot_dname_from_str(item);
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
	knot_dname_t *sample = knot_dname_from_str(item);
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
		dn = knot_dname_from_str(this_zone->name);
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

/*! \brief Generate automatic defaults for server identity, version and NSID. */
static void ident_auto(int tok, conf_t *conf, bool val)
{
	switch(tok) {
	case SVERSION:
		opt_replace(&conf->version, strdup("Knot DNS " PACKAGE_VERSION), val);
		break;
	case IDENTITY:
		opt_replace(&conf->identity, sockaddr_hostname(), val);
		break;
	case NSID:
		opt_replace(&conf->nsid, sockaddr_hostname(), val);
		if (conf->nsid) {
			conf->nsid_len = strlen(conf->nsid);
		}
		break;
	default:
		break;
	}
}


/* Line 371 of yacc.c  */
#line 496 "knot/conf/libknotd_la-cf-parse.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
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
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cf_debug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     TSIG_ALGO_NAME = 274,
     WORKERS = 275,
     BACKGROUND_WORKERS = 276,
     ASYNC_START = 277,
     USER = 278,
     RUNDIR = 279,
     PIDFILE = 280,
     REMOTES = 281,
     GROUPS = 282,
     ZONES = 283,
     FILENAME = 284,
     DISABLE_ANY = 285,
     SEMANTIC_CHECKS = 286,
     NOTIFY_RETRIES = 287,
     NOTIFY_TIMEOUT = 288,
     DBSYNC_TIMEOUT = 289,
     IXFR_FSLIMIT = 290,
     XFR_IN = 291,
     XFR_OUT = 292,
     UPDATE_IN = 293,
     NOTIFY_IN = 294,
     NOTIFY_OUT = 295,
     BUILD_DIFFS = 296,
     MAX_CONN_IDLE = 297,
     MAX_CONN_HS = 298,
     MAX_CONN_REPLY = 299,
     RATE_LIMIT = 300,
     RATE_LIMIT_SIZE = 301,
     RATE_LIMIT_SLIP = 302,
     TRANSFERS = 303,
     STORAGE = 304,
     DNSSEC_ENABLE = 305,
     DNSSEC_KEYDIR = 306,
     SIGNATURE_LIFETIME = 307,
     SERIAL_POLICY = 308,
     SERIAL_POLICY_VAL = 309,
     QUERY_MODULE = 310,
     INTERFACES = 311,
     ADDRESS = 312,
     PORT = 313,
     IPA = 314,
     IPA6 = 315,
     VIA = 316,
     CONTROL = 317,
     ALLOW = 318,
     LISTEN_ON = 319,
     LOG = 320,
     LOG_DEST = 321,
     LOG_SRC = 322,
     LOG_LEVEL = 323
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
#define TSIG_ALGO_NAME 274
#define WORKERS 275
#define BACKGROUND_WORKERS 276
#define ASYNC_START 277
#define USER 278
#define RUNDIR 279
#define PIDFILE 280
#define REMOTES 281
#define GROUPS 282
#define ZONES 283
#define FILENAME 284
#define DISABLE_ANY 285
#define SEMANTIC_CHECKS 286
#define NOTIFY_RETRIES 287
#define NOTIFY_TIMEOUT 288
#define DBSYNC_TIMEOUT 289
#define IXFR_FSLIMIT 290
#define XFR_IN 291
#define XFR_OUT 292
#define UPDATE_IN 293
#define NOTIFY_IN 294
#define NOTIFY_OUT 295
#define BUILD_DIFFS 296
#define MAX_CONN_IDLE 297
#define MAX_CONN_HS 298
#define MAX_CONN_REPLY 299
#define RATE_LIMIT 300
#define RATE_LIMIT_SIZE 301
#define RATE_LIMIT_SLIP 302
#define TRANSFERS 303
#define STORAGE 304
#define DNSSEC_ENABLE 305
#define DNSSEC_KEYDIR 306
#define SIGNATURE_LIFETIME 307
#define SERIAL_POLICY 308
#define SERIAL_POLICY_VAL 309
#define QUERY_MODULE 310
#define INTERFACES 311
#define ADDRESS 312
#define PORT 313
#define IPA 314
#define IPA6 315
#define VIA 316
#define CONTROL 317
#define ALLOW 318
#define LISTEN_ON 319
#define LOG 320
#define LOG_DEST 321
#define LOG_SRC 322
#define LOG_LEVEL 323



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 449 "knot/conf/cf-parse.y"

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;


/* Line 387 of yacc.c  */
#line 685 "knot/conf/libknotd_la-cf-parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int cf_parse (void *YYPARSE_PARAM);
#else
int cf_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int cf_parse (void *scanner);
#else
int cf_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 712 "knot/conf/libknotd_la-cf-parse.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

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
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   342

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  173
/* YYNRULES -- Number of states.  */
#define YYNSTATES  358

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    74,     2,     2,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    69,
       2,     2,     2,     2,    70,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    71,     2,    72,     2,     2,     2,     2,
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
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,    10,    11,    13,    15,    17,
      19,    21,    23,    24,    29,    34,    41,    46,    53,    56,
      62,    65,    70,    75,    80,    85,    90,    95,   100,   105,
     110,   115,   120,   125,   131,   136,   141,   146,   151,   156,
     161,   166,   171,   176,   181,   186,   191,   194,   200,   201,
     203,   205,   207,   209,   211,   212,   217,   222,   229,   236,
     241,   248,   255,   260,   265,   270,   275,   278,   284,   286,
     287,   289,   293,   295,   298,   304,   306,   308,   310,   312,
     314,   315,   317,   319,   321,   323,   325,   326,   330,   334,
     335,   339,   342,   343,   347,   348,   350,   352,   354,   356,
     358,   360,   364,   366,   369,   375,   379,   384,   389,   394,
     399,   404,   409,   414,   419,   424,   429,   434,   439,   444,
     449,   454,   459,   465,   468,   469,   473,   476,   480,   485,
     490,   495,   500,   505,   510,   515,   520,   525,   530,   535,
     540,   545,   550,   555,   561,   562,   564,   568,   572,   573,
     577,   579,   582,   583,   584,   590,   596,   597,   603,   605,
     607,   610,   616,   621,   627,   631,   633,   636,   639,   642,
     645,   648,   651,   654
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      76,     0,    -1,    77,     3,    -1,    -1,    77,   113,    -1,
      -1,     5,    -1,    26,    -1,    67,    -1,    65,    -1,    68,
      -1,    62,    -1,    -1,    79,    58,     7,    69,    -1,    79,
      57,    59,    69,    -1,    79,    57,    59,    70,     7,    69,
      -1,    79,    57,    60,    69,    -1,    79,    57,    60,    70,
       7,    69,    -1,    56,    71,    -1,    80,    78,    71,    79,
      72,    -1,    11,    71,    -1,    81,    14,     5,    69,    -1,
      81,    14,    10,    69,    -1,    81,    12,     5,    69,    -1,
      81,    12,    10,    69,    -1,    81,    13,     5,    69,    -1,
      81,    15,     6,    69,    -1,    81,    15,     5,    69,    -1,
      81,    15,    10,    69,    -1,    81,    18,     7,    69,    -1,
      81,    49,     5,    69,    -1,    81,    24,     5,    69,    -1,
      81,    25,     5,    69,    -1,    81,    16,    19,     5,    69,
      -1,    81,    20,     7,    69,    -1,    81,    21,     7,    69,
      -1,    81,    22,    10,    69,    -1,    81,    23,     5,    69,
      -1,    81,    42,     8,    69,    -1,    81,    43,     8,    69,
      -1,    81,    44,     8,    69,    -1,    81,    45,     7,    69,
      -1,    81,    46,     9,    69,    -1,    81,    46,     7,    69,
      -1,    81,    47,     7,    69,    -1,    81,    48,     7,    69,
      -1,    17,    71,    -1,    82,     5,    19,     5,    69,    -1,
      -1,     5,    -1,    67,    -1,    65,    -1,    68,    -1,    62,
      -1,    -1,    84,    58,     7,    69,    -1,    84,    57,    59,
      69,    -1,    84,    57,    59,    73,     7,    69,    -1,    84,
      57,    59,    70,     7,    69,    -1,    84,    57,    60,    69,
      -1,    84,    57,    60,    73,     7,    69,    -1,    84,    57,
      60,    70,     7,    69,    -1,    84,    16,     5,    69,    -1,
      84,    61,    59,    69,    -1,    84,    61,    60,    69,    -1,
      84,    61,     5,    69,    -1,    26,    71,    -1,    85,    83,
      71,    84,    72,    -1,     5,    -1,    -1,    86,    -1,    87,
      74,    86,    -1,     5,    -1,    27,    71,    -1,    89,    88,
      71,    87,    72,    -1,    36,    -1,    37,    -1,    39,    -1,
      40,    -1,    38,    -1,    -1,     5,    -1,    67,    -1,    65,
      -1,    68,    -1,    62,    -1,    -1,    92,    91,    74,    -1,
      92,    91,    69,    -1,    -1,    93,     5,    69,    -1,     5,
       5,    -1,    -1,    94,    69,    95,    -1,    -1,    23,    -1,
      26,    -1,    67,    -1,    65,    -1,    68,    -1,    62,    -1,
       7,    73,     5,    -1,     5,    -1,    96,    71,    -1,    97,
      90,    71,    93,    72,    -1,    97,    90,    92,    -1,    97,
      29,     5,    69,    -1,    97,    41,    10,    69,    -1,    97,
      31,    10,    69,    -1,    97,    49,     5,    69,    -1,    97,
      51,     5,    69,    -1,    97,    30,    10,    69,    -1,    97,
      34,     7,    69,    -1,    97,    34,     8,    69,    -1,    97,
      35,     9,    69,    -1,    97,    35,     7,    69,    -1,    97,
      32,     7,    69,    -1,    97,    33,     7,    69,    -1,    97,
      50,    10,    69,    -1,    97,    52,     7,    69,    -1,    97,
      52,     8,    69,    -1,    97,    53,    54,    69,    -1,    97,
      55,    71,    95,    72,    -1,     5,     5,    -1,    -1,    98,
      69,    99,    -1,    28,    71,    -1,   100,    97,    72,    -1,
     100,    30,    10,    69,    -1,   100,    41,    10,    69,    -1,
     100,    31,    10,    69,    -1,   100,    35,     9,    69,    -1,
     100,    35,     7,    69,    -1,   100,    32,     7,    69,    -1,
     100,    33,     7,    69,    -1,   100,    34,     7,    69,    -1,
     100,    34,     8,    69,    -1,   100,    49,     5,    69,    -1,
     100,    50,    10,    69,    -1,   100,    51,     5,    69,    -1,
     100,    52,     7,    69,    -1,   100,    52,     8,    69,    -1,
     100,    53,    54,    69,    -1,   100,    55,    71,    99,    72,
      -1,    -1,   101,    -1,   102,    68,    74,    -1,   102,    68,
      69,    -1,    -1,   103,    67,   102,    -1,    66,    -1,    29,
       5,    -1,    -1,    -1,   107,   104,    71,   103,    72,    -1,
     107,   105,    71,   103,    72,    -1,    -1,    65,   109,    71,
     107,   106,    -1,    64,    -1,    63,    -1,    62,    71,    -1,
     112,   110,    71,    79,    72,    -1,   112,   110,     5,    69,
      -1,   112,   111,    71,    93,    72,    -1,   112,   111,    92,
      -1,    69,    -1,    81,    72,    -1,    80,    72,    -1,    82,
      72,    -1,    85,    72,    -1,    89,    72,    -1,   100,    72,
      -1,   108,    72,    -1,   112,    72,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   521,   521,   523,   525,   528,   529,   530,   531,   532,
     533,   534,   537,   538,   545,   548,   551,   554,   560,   561,
     569,   570,   571,   572,   573,   574,   579,   580,   581,   582,
     586,   591,   592,   593,   598,   601,   604,   607,   628,   631,
     634,   637,   640,   643,   646,   649,   655,   656,   705,   706,
     707,   708,   709,   710,   713,   714,   721,   725,   729,   733,
     737,   741,   745,   753,   756,   759,   766,   767,   775,   778,
     780,   781,   785,   789,   790,   794,   797,   800,   803,   806,
     811,   812,   813,   814,   815,   816,   819,   820,   821,   824,
     825,   854,   857,   858,   861,   862,   863,   864,   865,   866,
     867,   868,   884,   888,   889,   890,   891,   892,   893,   894,
     895,   896,   897,   900,   903,   906,   909,   912,   915,   916,
     919,   922,   925,   929,   931,   932,   936,   937,   938,   939,
     940,   941,   944,   947,   950,   953,   956,   959,   960,   961,
     962,   965,   968,   971,   974,   983,   984,   988,   991,   992,
     998,  1020,  1046,  1050,  1051,  1052,  1055,  1055,  1059,  1063,
    1069,  1070,  1077,  1082,  1083,  1086,  1086,  1086,  1086,  1086,
    1086,  1086,  1086,  1086
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "INVALID_TOKEN", "TEXT", "HEXSTR",
  "NUM", "INTERVAL", "SIZE", "BOOL", "SYSTEM", "IDENTITY", "HOSTNAME",
  "SVERSION", "NSID", "KEY", "KEYS", "MAX_UDP_PAYLOAD", "TSIG_ALGO_NAME",
  "WORKERS", "BACKGROUND_WORKERS", "ASYNC_START", "USER", "RUNDIR",
  "PIDFILE", "REMOTES", "GROUPS", "ZONES", "FILENAME", "DISABLE_ANY",
  "SEMANTIC_CHECKS", "NOTIFY_RETRIES", "NOTIFY_TIMEOUT", "DBSYNC_TIMEOUT",
  "IXFR_FSLIMIT", "XFR_IN", "XFR_OUT", "UPDATE_IN", "NOTIFY_IN",
  "NOTIFY_OUT", "BUILD_DIFFS", "MAX_CONN_IDLE", "MAX_CONN_HS",
  "MAX_CONN_REPLY", "RATE_LIMIT", "RATE_LIMIT_SIZE", "RATE_LIMIT_SLIP",
  "TRANSFERS", "STORAGE", "DNSSEC_ENABLE", "DNSSEC_KEYDIR",
  "SIGNATURE_LIFETIME", "SERIAL_POLICY", "SERIAL_POLICY_VAL",
  "QUERY_MODULE", "INTERFACES", "ADDRESS", "PORT", "IPA", "IPA6", "VIA",
  "CONTROL", "ALLOW", "LISTEN_ON", "LOG", "LOG_DEST", "LOG_SRC",
  "LOG_LEVEL", "';'", "'@'", "'{'", "'}'", "'/'", "','", "$accept",
  "config", "conf_entries", "interface_start", "interface", "interfaces",
  "system", "keys", "remote_start", "remote", "remotes", "group_member",
  "group", "group_start", "groups", "zone_acl_start", "zone_acl_item",
  "zone_acl_list", "zone_acl", "query_module", "query_module_list",
  "zone_start", "zone", "query_genmodule", "query_genmodule_list", "zones",
  "log_prios_start", "log_prios", "log_src", "log_dest", "log_file",
  "log_end", "log_start", "log", "$@1", "ctl_listen_start",
  "ctl_allow_start", "control", "conf", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,    59,
      64,   123,   125,    47,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    77,    77,    78,    78,    78,    78,    78,
      78,    78,    79,    79,    79,    79,    79,    79,    80,    80,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    83,
      83,    83,    83,    83,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    85,    85,    86,    87,
      87,    87,    88,    89,    89,    90,    90,    90,    90,    90,
      91,    91,    91,    91,    91,    91,    92,    92,    92,    93,
      93,    94,    95,    95,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    98,    99,    99,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   101,   102,   102,   102,   103,   103,
     104,   105,   106,   107,   107,   107,   109,   108,   110,   111,
     112,   112,   112,   112,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     0,     4,     4,     6,     4,     6,     2,     5,
       2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     5,     0,     1,
       1,     1,     1,     1,     0,     4,     4,     6,     6,     4,
       6,     6,     4,     4,     4,     4,     2,     5,     1,     0,
       1,     3,     1,     2,     5,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     0,     3,     3,     0,
       3,     2,     0,     3,     0,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     5,     3,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     5,     2,     0,     3,     2,     3,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     0,     1,     3,     3,     0,     3,
       1,     2,     0,     0,     5,     5,     0,     5,     1,     1,
       2,     5,     4,     5,     3,     1,     2,     2,     2,     2,
       2,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     2,     0,     0,     0,     0,     0,
       0,     0,   156,   165,     5,     0,     0,    48,     0,    94,
       0,     0,     4,    20,    46,    66,    73,   126,    18,   160,
       0,     6,     7,    11,     9,     8,    10,   167,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
       0,   168,    49,    53,    51,    50,    52,   169,     0,    72,
     170,     0,   102,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
      98,    97,    99,   171,     0,     0,   172,   159,   158,   173,
       0,    86,   153,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      54,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   124,   103,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    79,
      77,    78,     0,     0,     0,     0,     0,     0,     0,   127,
      86,     0,    12,    89,   164,   152,     0,    23,    24,    25,
      21,    22,    27,    26,    28,     0,    29,    34,    35,    36,
      37,    31,    32,    38,    39,    40,    41,    43,    42,    44,
      45,    30,     0,     0,    68,    70,     0,   101,   128,   130,
     133,   134,   135,   136,   132,   131,   129,   137,   138,   139,
     140,   141,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    89,   105,   162,     0,     0,    81,    85,
      83,    82,    84,     0,     0,   150,     0,     0,   157,     0,
       0,    19,    33,    47,     0,     0,     0,     0,    67,    74,
       0,   123,   124,   143,   106,   111,   108,   116,   117,   112,
     113,   115,   114,   107,   109,   118,   110,   119,   120,   121,
       0,     0,     0,     0,   161,     0,   163,    88,    87,   151,
     148,   148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    71,   125,    91,    92,   122,   104,    90,     0,
       0,    14,     0,    16,     0,    13,    62,    56,     0,     0,
      59,     0,     0,    55,    65,    63,    64,    93,   144,   154,
     155,     0,     0,     0,     0,     0,     0,   145,   149,    15,
      17,    58,    57,    61,    60,     0,   147,   146
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    38,   176,    14,    15,    16,    68,   203,
      17,   205,   206,    71,    18,   170,   253,   174,   247,   291,
     292,    94,    95,   224,   225,    19,   347,   348,   319,   256,
     257,   258,   175,    20,    30,   100,   101,    21,    22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -55
static const yytype_int16 yypact[] =
{
     -55,     9,    74,   -55,   -55,   -40,   -12,    12,    17,    24,
      60,    62,   -55,   -55,    14,   102,     0,    22,     1,     3,
       8,   -48,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
      71,   -55,   -55,   -55,   -55,   -55,   -55,   -55,    83,     7,
     105,    36,    15,    87,   165,   166,   173,   172,   178,   179,
     180,   181,   182,   183,   185,    23,   186,   187,   190,   -55,
     167,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   116,   -55,
     -55,   117,   -55,   123,   -55,   -55,   189,   191,   193,   195,
      41,    96,   194,   192,   196,   198,   127,   151,   136,   -55,
     -55,   -55,   -55,   -55,   137,   126,   -55,   -55,   -55,   -55,
      -4,   138,   -55,   -55,   141,   142,   143,   144,   145,   146,
     147,   148,   213,   150,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   168,   169,   215,
     -55,   229,   230,   170,   171,   174,   175,   176,   177,   184,
     188,   197,   199,   200,   201,   202,   203,   204,   231,   -55,
     236,   232,   237,   241,   242,   130,   104,   -55,   -55,   -55,
     -55,   -55,   240,   246,   244,   247,   133,   205,   206,   -55,
     207,   210,   -55,   -55,    -5,   -27,   -54,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,   211,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   212,    35,   -55,   -55,    47,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,   250,   214,   216,   217,   218,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   233,   234,   235,
     238,   239,   251,   -55,    -5,   -55,   -44,     2,   -55,   -55,
     -55,   -55,   -55,   -24,   253,   -55,   243,   245,   -55,   -37,
     254,   -55,   -55,   -55,   255,    93,   256,    38,   -55,   -55,
     229,   -55,   231,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     257,   248,   249,     6,   -55,   258,   -55,   -55,   -55,   -55,
     -55,   -55,    99,   101,   259,   260,    39,    59,   261,   262,
     263,   264,   -55,   -55,   -55,   251,   -55,   -55,   -55,   -25,
      -6,   -55,   267,   -55,   268,   -55,   -55,   -55,   269,   275,
     -55,   277,   278,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   265,   266,   270,   271,   272,   273,   -55,   252,   -55,
     -55,   -55,   -55,   -55,   -55,    30,   -55,   -55
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -55,   -55,   -55,   -55,    92,   -55,   -55,   -55,   -55,   -55,
     -55,    -3,   -55,   -55,   -55,   -55,   -55,    95,    55,   -55,
     -16,   -55,   -55,   -55,    28,   -55,   -55,   -55,     4,   -55,
     -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -81
static const yytype_int16 yytable[] =
{
     248,   171,   254,   259,   260,    60,    69,   295,    72,     3,
      73,   295,   104,   259,   260,    97,    98,   105,   261,    31,
     109,   110,   302,   303,    99,   111,    74,    62,   294,    75,
     124,    23,   125,    76,    77,    78,    79,    80,    81,   255,
      32,   107,   338,   309,    82,   297,   108,   339,   137,   138,
     298,   264,    83,    84,    85,    86,    87,   249,    88,    24,
     250,   338,   251,   252,   -80,    89,   340,   172,    90,   -80,
      91,    92,    61,    70,   296,    93,    33,     4,   317,    34,
      96,    35,    36,    25,    63,     5,    37,    64,    26,    65,
      66,     6,   265,   266,    67,    27,   267,   310,   311,   356,
       7,     8,     9,   139,   357,   140,   112,   268,   327,   328,
     106,   233,   329,   234,    39,    40,    41,    42,    43,   269,
      44,   270,    45,    46,    47,    48,    49,    50,   330,   331,
      10,    28,   332,    29,   145,   146,    11,   231,   232,    12,
     239,   240,   102,    13,    51,    52,    53,    54,    55,    56,
      57,    58,   306,   307,   103,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   321,   322,
     323,   324,   113,   114,    59,   163,   164,   165,   166,   167,
     115,   168,   116,   117,   118,   119,   129,   130,   131,   120,
     121,   122,   123,   126,   127,   128,   132,   142,   169,   133,
     135,   134,   136,   144,   141,   147,   143,   148,   149,   173,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     202,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   204,   207,   223,   200,   201,   208,
     209,   226,   227,   210,   211,   212,   213,   228,   229,   230,
     235,   236,   238,   214,   237,   271,   290,   215,   299,   241,
     305,   304,   314,   308,   246,   244,   216,   312,   217,   218,
     219,   220,   221,   222,   341,   342,   343,   242,   243,   245,
     262,   263,   344,   272,   345,   346,   274,   275,   273,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   293,   337,
     313,     0,   285,   286,   287,   320,     0,   288,   289,     0,
       0,     0,     0,     0,   300,     0,   301,   315,     0,     0,
     355,   316,     0,     0,     0,     0,     0,   318,   325,   326,
     333,   334,   335,   336,   349,   350,     0,     0,     0,   351,
     352,   353,   354
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-55)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       5,     5,    29,    57,    58,     5,     5,     5,     5,     0,
       7,     5,     5,    57,    58,    63,    64,    10,    72,     5,
       5,     6,    59,    60,    72,    10,    23,     5,    72,    26,
       7,    71,     9,    30,    31,    32,    33,    34,    35,    66,
      26,     5,    67,     5,    41,    69,    10,    72,     7,     8,
      74,    16,    49,    50,    51,    52,    53,    62,    55,    71,
      65,    67,    67,    68,    69,    62,    72,    71,    65,    74,
      67,    68,    72,    72,    72,    72,    62,     3,    72,    65,
      72,    67,    68,    71,    62,    11,    72,    65,    71,    67,
      68,    17,    57,    58,    72,    71,    61,    59,    60,    69,
      26,    27,    28,     7,    74,     9,    19,    72,    69,    70,
       5,     7,    73,     9,    12,    13,    14,    15,    16,    72,
      18,    74,    20,    21,    22,    23,    24,    25,    69,    70,
      56,    71,    73,    71,     7,     8,    62,     7,     8,    65,
       7,     8,    71,    69,    42,    43,    44,    45,    46,    47,
      48,    49,    59,    60,    71,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    69,    70,
      69,    70,     7,     7,    72,    49,    50,    51,    52,    53,
       7,    55,    10,     5,     5,     5,    19,    71,    71,     8,
       8,     8,     7,     7,     7,     5,    73,     5,    72,    10,
       7,    10,     7,     5,    10,    54,    10,    71,    71,    71,
      69,    69,    69,    69,    69,    69,    69,    69,     5,    69,
       5,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,     5,     5,     5,    69,    69,    69,
      69,     5,    10,    69,    69,    69,    69,    10,     7,     7,
      10,     5,     5,    69,    10,     5,     5,    69,     5,    54,
       5,     7,     5,     7,   172,   170,    69,   270,    69,    69,
      69,    69,    69,    69,     7,     7,     7,    71,    71,    69,
      69,    69,     7,    69,     7,     7,    69,    69,    72,    69,
      69,    69,    69,    69,    69,    69,    69,    69,   243,   315,
     272,    -1,    69,    69,    69,   301,    -1,    69,    69,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    71,    69,    -1,    -1,
      68,    72,    -1,    -1,    -1,    -1,    -1,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    -1,    -1,    -1,    69,
      69,    69,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    76,    77,     0,     3,    11,    17,    26,    27,    28,
      56,    62,    65,    69,    80,    81,    82,    85,    89,   100,
     108,   112,   113,    71,    71,    71,    71,    71,    71,    71,
     109,     5,    26,    62,    65,    67,    68,    72,    78,    12,
      13,    14,    15,    16,    18,    20,    21,    22,    23,    24,
      25,    42,    43,    44,    45,    46,    47,    48,    49,    72,
       5,    72,     5,    62,    65,    67,    68,    72,    83,     5,
      72,    88,     5,     7,    23,    26,    30,    31,    32,    33,
      34,    35,    41,    49,    50,    51,    52,    53,    55,    62,
      65,    67,    68,    72,    96,    97,    72,    63,    64,    72,
     110,   111,    71,    71,     5,    10,     5,     5,    10,     5,
       6,    10,    19,     7,     7,     7,    10,     5,     5,     5,
       8,     8,     8,     7,     7,     9,     7,     7,     5,    19,
      71,    71,    73,    10,    10,     7,     7,     7,     8,     7,
       9,    10,     5,    10,     5,     7,     8,    54,    71,    71,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    49,    50,    51,    52,    53,    55,    72,
      90,     5,    71,    71,    92,   107,    79,    69,    69,    69,
      69,    69,    69,    69,    69,     5,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,     5,    84,     5,    86,    87,     5,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,     5,    98,    99,     5,    10,    10,     7,
       7,     7,     8,     7,     9,    10,     5,    10,     5,     7,
       8,    54,    71,    71,    92,    69,    79,    93,     5,    62,
      65,    67,    68,    91,    29,    66,   104,   105,   106,    57,
      58,    72,    69,    69,    16,    57,    58,    61,    72,    72,
      74,     5,    69,    72,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
       5,    94,    95,    93,    72,     5,    72,    69,    74,     5,
      71,    71,    59,    60,     7,     5,    59,    60,     7,     5,
      59,    60,    86,    99,     5,    69,    72,    72,    69,   103,
     103,    69,    70,    69,    70,    69,    69,    69,    70,    73,
      69,    70,    73,    69,    69,    69,    69,    95,    67,    72,
      72,     7,     7,     7,     7,     7,     7,   101,   102,    69,
      69,    69,    69,    69,    69,    68,    69,    74
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, scanner)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *scanner;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (scanner);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *scanner;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, void *scanner)
#else
static void
yy_reduce_print (yyvsp, yyrule, scanner)
    YYSTYPE *yyvsp;
    int yyrule;
    void *scanner;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, scanner); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
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

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, scanner)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    void *scanner;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (scanner);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *scanner)
#else
int
yyparse (scanner)
    void *scanner;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

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
      yychar = YYLEX;
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
     `$$ = $1'.

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
/* Line 1792 of yacc.c  */
#line 521 "knot/conf/cf-parse.y"
    { return 0; }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 529 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 530 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 531 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 532 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 533 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 534 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 538 "knot/conf/cf-parse.y"
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_iface->addr, (yyvsp[(3) - (4)].tok).i);
     }
   }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 545 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[(3) - (4)].tok).t, CONFIG_DEFAULT_PORT);
   }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 548 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET, (yyvsp[(3) - (6)].tok).t, (yyvsp[(5) - (6)].tok).i);
   }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 551 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[(3) - (4)].tok).t, CONFIG_DEFAULT_PORT);
   }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 554 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_iface->addr, AF_INET6, (yyvsp[(3) - (6)].tok).t, (yyvsp[(5) - (6)].tok).i);
   }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 561 "knot/conf/cf-parse.y"
    {
   if (this_iface->addr.ss_family == AF_UNSPEC) {
     cf_error(scanner, "interface '%s' has no defined address", this_iface->name);
   }
 }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 570 "knot/conf/cf-parse.y"
    { new_config->version = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 571 "knot/conf/cf-parse.y"
    { ident_auto(SVERSION, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 572 "knot/conf/cf-parse.y"
    { new_config->identity = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 573 "knot/conf/cf-parse.y"
    { ident_auto(IDENTITY, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 574 "knot/conf/cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.hostname' is deprecated. "
                     "Use 'system.identity' instead.\n");
     free((yyvsp[(3) - (4)].tok).t);
 }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 579 "knot/conf/cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 580 "knot/conf/cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = strlen(new_config->nsid); }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 581 "knot/conf/cf-parse.y"
    { ident_auto(NSID, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 582 "knot/conf/cf-parse.y"
    {
     SET_NUM(new_config->max_udp_payload, (yyvsp[(3) - (4)].tok).i, KNOT_EDNS_MIN_UDP_PAYLOAD,
             KNOT_EDNS_MAX_UDP_PAYLOAD, "max-udp-payload");
 }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 586 "knot/conf/cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.storage' was relocated. "
                     "Use 'zones.storage' instead.\n");
     new_config->storage = (yyvsp[(3) - (4)].tok).t;
 }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 591 "knot/conf/cf-parse.y"
    { new_config->rundir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 592 "knot/conf/cf-parse.y"
    { new_config->pidfile = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 593 "knot/conf/cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.key' is deprecated "
                     "and has no effect.\n");
     free((yyvsp[(4) - (5)].tok).t);
 }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 598 "knot/conf/cf-parse.y"
    {
     SET_NUM(new_config->workers, (yyvsp[(3) - (4)].tok).i, 1, 255, "workers");
 }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 601 "knot/conf/cf-parse.y"
    {
     SET_NUM(new_config->bg_workers, (yyvsp[(3) - (4)].tok).i, 1, 255, "background-workers");
 }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 604 "knot/conf/cf-parse.y"
    {
     new_config->async_start = (yyvsp[(3) - (4)].tok).i;
 }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 607 "knot/conf/cf-parse.y"
    {
     new_config->uid = new_config->gid = -1; // Invalidate
     char* dpos = strchr((yyvsp[(3) - (4)].tok).t, '.'); // Find uid.gid format
     if (dpos != NULL) {
        struct group *grp = getgrnam(dpos + 1); // Skip dot
        if (grp != NULL) {
          new_config->gid = grp->gr_gid;
        } else {
          cf_error(scanner, "invalid group name '%s'", dpos + 1);
        }
        *dpos = '\0'; // Cut off
     }
     struct passwd* pwd = getpwnam((yyvsp[(3) - (4)].tok).t);
     if (pwd != NULL) {
       new_config->uid = pwd->pw_uid;
     } else {
       cf_error(scanner, "invalid user name '%s'", (yyvsp[(3) - (4)].tok).t);
     }

     free((yyvsp[(3) - (4)].tok).t);
 }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 628 "knot/conf/cf-parse.y"
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[(3) - (4)].tok).i, "max-conn-idle");
 }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 631 "knot/conf/cf-parse.y"
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[(3) - (4)].tok).i, "max-conn-handshake");
 }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 634 "knot/conf/cf-parse.y"
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[(3) - (4)].tok).i, "max-conn-reply");
 }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 637 "knot/conf/cf-parse.y"
    {
	SET_INT(new_config->rrl, (yyvsp[(3) - (4)].tok).i, "rate-limit");
 }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 640 "knot/conf/cf-parse.y"
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[(3) - (4)].tok).l, "rate-limit-size");
 }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 643 "knot/conf/cf-parse.y"
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[(3) - (4)].tok).i, "rate-limit-size");
 }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 646 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->rrl_slip, (yyvsp[(3) - (4)].tok).i, 1, RRL_SLIP_MAX, "rate-limit-slip");
 }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 649 "knot/conf/cf-parse.y"
    {
	SET_INT(new_config->xfers, (yyvsp[(3) - (4)].tok).i, "transfers");
 }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 656 "knot/conf/cf-parse.y"
    {
     /* Check algorithm length. */
     if (knot_tsig_digest_length((yyvsp[(3) - (5)].tok).alg) == 0) {
        cf_error(scanner, "unsupported digest algorithm");
     }

     /* Normalize to FQDN */
     char *fqdn = (yyvsp[(2) - (5)].tok).t;
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
         knot_dname_t *dname = knot_dname_from_str(fqdn);
	 if (!dname) {
             cf_error(scanner, "key name '%s' not in valid domain name format",
                      fqdn);
         } else {
             knot_dname_to_lower(dname);
             conf_key_t *k = malloc(sizeof(conf_key_t));
             memset(k, 0, sizeof(conf_key_t));
             k->k.name = dname;
             k->k.algorithm = (yyvsp[(3) - (5)].tok).alg;
             if (knot_binary_from_base64((yyvsp[(4) - (5)].tok).t, &(k->k.secret)) != 0) {
                 cf_error(scanner, "invalid key secret '%s'", (yyvsp[(4) - (5)].tok).t);
                 knot_dname_free(&dname, NULL);
                 free(k);
             } else {
                 add_tail(&new_config->keys, &k->n);
             }
         }
     }

     free((yyvsp[(4) - (5)].tok).t);
     free(fqdn);
}
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 706 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 707 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 708 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 709 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 710 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 714 "knot/conf/cf-parse.y"
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "can't set port number before interface address\n");
     } else {
       sockaddr_port_set(&this_remote->addr, (yyvsp[(3) - (4)].tok).i);
     }
   }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 721 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[(3) - (4)].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 725 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[(3) - (6)].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[(5) - (6)].tok).i, 0, IPV4_PREFIXLEN, "prefix length");
   }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 729 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET, (yyvsp[(3) - (6)].tok).t, (yyvsp[(5) - (6)].tok).i);
     this_remote->prefix = IPV4_PREFIXLEN;
   }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 733 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[(3) - (4)].tok).t, CONFIG_DEFAULT_PORT);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 737 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[(3) - (6)].tok).t, 0);
     SET_NUM(this_remote->prefix, (yyvsp[(5) - (6)].tok).i, 0, IPV6_PREFIXLEN, "prefix length");
   }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 741 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->addr, AF_INET6, (yyvsp[(3) - (6)].tok).t, (yyvsp[(5) - (6)].tok).i);
     this_remote->prefix = IPV6_PREFIXLEN;
   }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 745 "knot/conf/cf-parse.y"
    {
     if (this_remote->key != 0) {
       cf_error(scanner, "only one TSIG key definition is allowed in remote section\n");
     } else {
        conf_key_add(scanner, &this_remote->key, (yyvsp[(3) - (4)].tok).t);
     }
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 753 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET, (yyvsp[(3) - (4)].tok).t, 0);
   }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 756 "knot/conf/cf-parse.y"
    {
     conf_set_iface(scanner, &this_remote->via, AF_INET6, (yyvsp[(3) - (4)].tok).t, 0);
   }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 759 "knot/conf/cf-parse.y"
    {
     conf_remote_set_via(scanner, (yyvsp[(3) - (4)].tok).t);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 767 "knot/conf/cf-parse.y"
    {
     if (this_remote->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "remote '%s' has no defined address", this_remote->name);
     }
   }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 775 "knot/conf/cf-parse.y"
    { conf_add_member_into_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 785 "knot/conf/cf-parse.y"
    { conf_start_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 794 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_in;
   }
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 797 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_out;
   }
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 800 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.notify_in;
   }
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 803 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.notify_out;
   }
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 806 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.update_in;
 }
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 812 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 813 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 814 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 815 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 816 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 825 "knot/conf/cf-parse.y"
    {
      /* Find existing node in remotes. */
      node_t* r = 0; conf_iface_t* found = 0;
      WALK_LIST (r, new_config->remotes) {
	 if (strcmp(((conf_iface_t*)r)->name, (yyvsp[(2) - (3)].tok).t) == 0) {
	    found = (conf_iface_t*)r;
	    break;
	 }
      }

      /* Append to list if found. */
      if (!found) {
	 cf_error(scanner, "remote '%s' is not defined", (yyvsp[(2) - (3)].tok).t);
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
      free((yyvsp[(2) - (3)].tok).t);
   }
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 854 "knot/conf/cf-parse.y"
    { query_module_create(scanner, (yyvsp[(1) - (2)].tok).t, (yyvsp[(2) - (2)].tok).t, true); free((yyvsp[(1) - (2)].tok).t); free((yyvsp[(2) - (2)].tok).t); }
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 862 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 96:
/* Line 1792 of yacc.c  */
#line 863 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 864 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 865 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 866 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 867 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 868 "knot/conf/cf-parse.y"
    {
    unsigned prefix_len = 0;
    SET_NUM(prefix_len, (yyvsp[(1) - (3)].tok).i, 0, 255, "origin prefix length");
    size_t len = 3 + 1 + strlen((yyvsp[(3) - (3)].tok).t) + 1; /* <0,255> '/' rest */
    char *name = malloc(len * sizeof(char));
    if (name == NULL) {
        cf_error(scanner, "out of memory");
    } else {
        name[0] = '\0';
        if (snprintf(name, len, "%u/%s", prefix_len, (yyvsp[(3) - (3)].tok).t) < 0) {
            cf_error(scanner,"failed to convert rfc2317 origin to string");
        }
    }
    free((yyvsp[(3) - (3)].tok).t);
    conf_zone_start(scanner, name);
 }
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 884 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 891 "knot/conf/cf-parse.y"
    { this_zone->file = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 892 "knot/conf/cf-parse.y"
    { this_zone->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 108:
/* Line 1792 of yacc.c  */
#line 893 "knot/conf/cf-parse.y"
    { this_zone->enable_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 109:
/* Line 1792 of yacc.c  */
#line 894 "knot/conf/cf-parse.y"
    { this_zone->storage = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 895 "knot/conf/cf-parse.y"
    { this_zone->dnssec_keydir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 896 "knot/conf/cf-parse.y"
    { this_zone->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 897 "knot/conf/cf-parse.y"
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, "zonefile-sync");
 }
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 900 "knot/conf/cf-parse.y"
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, "zonefile-sync");
 }
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 903 "knot/conf/cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).l, "ixfr-fslimit");
 }
    break;

  case 115:
/* Line 1792 of yacc.c  */
#line 906 "knot/conf/cf-parse.y"
    {
	SET_SIZE(this_zone->ixfr_fslimit, (yyvsp[(3) - (4)].tok).i, "ixfr-fslimit");
 }
    break;

  case 116:
/* Line 1792 of yacc.c  */
#line 909 "knot/conf/cf-parse.y"
    {
	SET_NUM(this_zone->notify_retries, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-retries");
   }
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 912 "knot/conf/cf-parse.y"
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-timeout");
   }
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 915 "knot/conf/cf-parse.y"
    { this_zone->dnssec_enable = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 916 "knot/conf/cf-parse.y"
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 919 "knot/conf/cf-parse.y"
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 922 "knot/conf/cf-parse.y"
    {
	this_zone->serial_policy = (yyvsp[(3) - (4)].tok).i;
 }
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 929 "knot/conf/cf-parse.y"
    { query_module_create(scanner, (yyvsp[(1) - (2)].tok).t, (yyvsp[(2) - (2)].tok).t, false); free((yyvsp[(1) - (2)].tok).t); free((yyvsp[(2) - (2)].tok).t); }
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 938 "knot/conf/cf-parse.y"
    { new_config->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 939 "knot/conf/cf-parse.y"
    { new_config->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 940 "knot/conf/cf-parse.y"
    { new_config->zone_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 941 "knot/conf/cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).l, "ixfr-fslimit");
 }
    break;

  case 132:
/* Line 1792 of yacc.c  */
#line 944 "knot/conf/cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).i, "ixfr-fslimit");
 }
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 947 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->notify_retries, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-retries");
   }
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 950 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-timeout");
   }
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 953 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, 0, INT_MAX, "zonefile-sync");
 }
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 956 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, 0, INT_MAX, "zonefile-sync");
 }
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 959 "knot/conf/cf-parse.y"
    { new_config->storage = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 960 "knot/conf/cf-parse.y"
    { new_config->dnssec_enable = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 961 "knot/conf/cf-parse.y"
    { new_config->dnssec_keydir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 962 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 965 "knot/conf/cf-parse.y"
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 968 "knot/conf/cf-parse.y"
    {
	new_config->serial_policy = (yyvsp[(3) - (4)].tok).i;
 }
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 974 "knot/conf/cf-parse.y"
    {
  this_logmap = malloc(sizeof(conf_log_map_t));
  this_logmap->source = 0;
  this_logmap->prios = 0;
  add_tail(&this_log->map, &this_logmap->n);
}
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 984 "knot/conf/cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i;
	fprintf(stderr, "Warning: more log severities per statement is deprecated. "
	                "Using the least serious one.\n");
 }
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 988 "knot/conf/cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i; }
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 992 "knot/conf/cf-parse.y"
    {
     this_logmap->source = (yyvsp[(2) - (3)].tok).i;
     this_logmap = 0;
   }
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 998 "knot/conf/cf-parse.y"
    {
  /* Find already existing rule. */
  this_log = 0;
  node_t *n = 0;
  WALK_LIST(n, new_config->logs) {
    conf_log_t* log = (conf_log_t*)n;
    if (log->type == (yyvsp[(1) - (1)].tok).i) {
      this_log = log;
      break;
    }
  }

  if (!this_log) {
    this_log = malloc(sizeof(conf_log_t));
    this_log->type = (yyvsp[(1) - (1)].tok).i;
    this_log->file = 0;
    init_list(&this_log->map);
    add_tail(&new_config->logs, &this_log->n);
  }
}
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 1020 "knot/conf/cf-parse.y"
    {
  /* Find already existing rule. */
  this_log = 0;
  node_t *n = 0;
  WALK_LIST(n, new_config->logs) {
    conf_log_t* log = (conf_log_t*)n;
    if (log->type == LOGT_FILE) {
      if (strcmp((yyvsp[(2) - (2)].tok).t, log->file) == 0) {
        this_log = log;
	free((yyvsp[(2) - (2)].tok).t);
        break;
      }
    }
  }

  /* Create new rule. */
  if (!this_log) {
    this_log = malloc(sizeof(conf_log_t));
    this_log->type = LOGT_FILE;
    this_log->file = strcpath((yyvsp[(2) - (2)].tok).t);
    init_list(&this_log->map);
    add_tail(&new_config->logs, &this_log->n);
  }
}
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 1046 "knot/conf/cf-parse.y"
    {
}
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 1055 "knot/conf/cf-parse.y"
    { }
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 1059 "knot/conf/cf-parse.y"
    { conf_init_iface(scanner, NULL); }
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 1063 "knot/conf/cf-parse.y"
    {
    this_list = &new_config->ctl.allow;
  }
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 1069 "knot/conf/cf-parse.y"
    { new_config->ctl.have = true; }
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 1070 "knot/conf/cf-parse.y"
    {
     if (this_iface->addr.ss_family == AF_UNSPEC) {
       cf_error(scanner, "control interface has no defined address");
     } else {
       new_config->ctl.iface = this_iface;
     }
 }
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 1077 "knot/conf/cf-parse.y"
    {
     sockaddr_set(&this_iface->addr, AF_UNIX, (yyvsp[(3) - (4)].tok).t, 0);
     new_config->ctl.iface = this_iface;
     free((yyvsp[(3) - (4)].tok).t);
 }
    break;


/* Line 1792 of yacc.c  */
#line 3306 "knot/conf/libknotd_la-cf-parse.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

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
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 1088 "knot/conf/cf-parse.y"

