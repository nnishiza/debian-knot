/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         cf_parse
#define yylex           cf_lex
#define yyerror         cf_error
#define yylval          cf_lval
#define yychar          cf_char
#define yydebug         cf_debug
#define yynerrs         cf_nerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 23 "cf-parse.y"


#include <config.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "common/sockaddr.h"
#include "libknot/dname.h"
#include "libknot/binary.h"
#include "libknot/edns.h"
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

static void conf_init_iface(void *scanner, char* ifname, int port)
{
	this_iface = malloc(sizeof(conf_iface_t));
	if (this_iface == NULL) {
		cf_error(scanner, "not enough memory when allocating interface");
		return;
	}
	memset(this_iface, 0, sizeof(conf_iface_t));
	this_iface->name = ifname;
	this_iface->port = port;
}

static void conf_start_iface(void *scanner, char* ifname)
{
	conf_init_iface(scanner, ifname, -1);
	add_tail(&new_config->ifaces, &this_iface->n);
	++new_config->ifaces_count;
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
	sockaddr_init(&this_remote->via, -1);
	++new_config->remotes_count;
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
		sockaddr_set(&this_remote->via, found->family, found->address, 0);
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
	// silently skip duplicates

	conf_remote_t *remote;
	WALK_LIST (remote, *this_list) {
		if (remote->remote == found) {
			return;
		}
	}

	// additional check for transfers

	if ((this_list == &this_zone->acl.xfr_in ||
	    this_list == &this_zone->acl.notify_out) && found->port == 0)
	{
		cf_error(scanner, "remote specified for XFR/IN or "
		"NOTIFY/OUT needs to have valid port!");
		return;
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

static int conf_key_exists(void *scanner, char *item)
{
	/* Find existing node in keys. */
	knot_dname_t *sample = knot_dname_from_str(item);
	knot_dname_to_lower(sample);
	conf_key_t* r = 0;
	WALK_LIST (r, new_config->keys) {
		if (knot_dname_cmp(r->k.name, sample) == 0) {
			cf_error(scanner, "key '%s' is already defined", item);
			knot_dname_free(&sample);
			return 1;
		}
	}

	knot_dname_free(&sample);
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
			knot_dname_free(&sample);
			return 0;
		}
	}

	cf_error(scanner, "key '%s' is not defined", item);
	knot_dname_free(&sample);
	return 1;
}

static void conf_zone_start(void *scanner, char *name) {
	this_zone = malloc(sizeof(conf_zone_t));
	if (this_zone == NULL || name == NULL) {
		cf_error(scanner, "out of memory while allocating zone config");
		return;
	}
	memset(this_zone, 0, sizeof(conf_zone_t));
	this_zone->enable_checks = -1; // Default policy applies
	this_zone->notify_timeout = -1; // Default policy applies
	this_zone->notify_retries = 0; // Default policy applies
	this_zone->dbsync_timeout = -1; // Default policy applies
	this_zone->disable_any = -1; // Default policy applies
	this_zone->build_diffs = -1; // Default policy applies
	this_zone->sig_lifetime = -1; // Default policy applies

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

	// DNSSEC configuration
	this_zone->dnssec_enable = -1;

	/* Initialize ACL lists. */
	init_list(&this_zone->acl.xfr_in);
	init_list(&this_zone->acl.xfr_out);
	init_list(&this_zone->acl.notify_in);
	init_list(&this_zone->acl.notify_out);
	init_list(&this_zone->acl.update_in);

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
	if (hattrie_tryget(new_config->names, (const char *)dn,
	                   knot_dname_size(dn)) != NULL) {
		cf_error(scanner, "zone '%s' is already present, refusing to "
		         "duplicate", this_zone->name);
		knot_dname_free(&dn);
		free(this_zone->name);
		this_zone->name = NULL;
		/* Must not free, some versions of flex might continue after
		 * error and segfault.
		 * free(this_zone); this_zone = NULL;
		 */
		return;
	}

	/* Directly discard dname, won't be needed. */
	add_tail(&new_config->zones, &this_zone->n);
	*hattrie_get(new_config->names, (const char *)dn,
	             knot_dname_size(dn)) = (void *)1;
	++new_config->zones_count;
	knot_dname_free(&dn);
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



/* Line 268 of yacc.c  */
#line 488 "knot/conf/libknotd_la-cf-parse.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
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
     USER = 276,
     RUNDIR = 277,
     PIDFILE = 278,
     REMOTES = 279,
     GROUPS = 280,
     ZONES = 281,
     FILENAME = 282,
     DISABLE_ANY = 283,
     SEMANTIC_CHECKS = 284,
     NOTIFY_RETRIES = 285,
     NOTIFY_TIMEOUT = 286,
     DBSYNC_TIMEOUT = 287,
     IXFR_FSLIMIT = 288,
     XFR_IN = 289,
     XFR_OUT = 290,
     UPDATE_IN = 291,
     NOTIFY_IN = 292,
     NOTIFY_OUT = 293,
     BUILD_DIFFS = 294,
     MAX_CONN_IDLE = 295,
     MAX_CONN_HS = 296,
     MAX_CONN_REPLY = 297,
     RATE_LIMIT = 298,
     RATE_LIMIT_SIZE = 299,
     RATE_LIMIT_SLIP = 300,
     TRANSFERS = 301,
     STORAGE = 302,
     DNSSEC_ENABLE = 303,
     DNSSEC_KEYDIR = 304,
     SIGNATURE_LIFETIME = 305,
     INTERFACES = 306,
     ADDRESS = 307,
     PORT = 308,
     IPA = 309,
     IPA6 = 310,
     VIA = 311,
     CONTROL = 312,
     ALLOW = 313,
     LISTEN_ON = 314,
     LOG = 315,
     LOG_DEST = 316,
     LOG_SRC = 317,
     LOG_LEVEL = 318
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
#define USER 276
#define RUNDIR 277
#define PIDFILE 278
#define REMOTES 279
#define GROUPS 280
#define ZONES 281
#define FILENAME 282
#define DISABLE_ANY 283
#define SEMANTIC_CHECKS 284
#define NOTIFY_RETRIES 285
#define NOTIFY_TIMEOUT 286
#define DBSYNC_TIMEOUT 287
#define IXFR_FSLIMIT 288
#define XFR_IN 289
#define XFR_OUT 290
#define UPDATE_IN 291
#define NOTIFY_IN 292
#define NOTIFY_OUT 293
#define BUILD_DIFFS 294
#define MAX_CONN_IDLE 295
#define MAX_CONN_HS 296
#define MAX_CONN_REPLY 297
#define RATE_LIMIT 298
#define RATE_LIMIT_SIZE 299
#define RATE_LIMIT_SLIP 300
#define TRANSFERS 301
#define STORAGE 302
#define DNSSEC_ENABLE 303
#define DNSSEC_KEYDIR 304
#define SIGNATURE_LIFETIME 305
#define INTERFACES 306
#define ADDRESS 307
#define PORT 308
#define IPA 309
#define IPA6 310
#define VIA 311
#define CONTROL 312
#define ALLOW 313
#define LISTEN_ON 314
#define LOG 315
#define LOG_DEST 316
#define LOG_SRC 317
#define LOG_LEVEL 318




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 436 "cf-parse.y"

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;



/* Line 293 of yacc.c  */
#line 661 "knot/conf/libknotd_la-cf-parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 673 "knot/conf/libknotd_la-cf-parse.c"

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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
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
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   304

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  161
/* YYNRULES -- Number of states.  */
#define YYNSTATES  328

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    69,     2,     2,    68,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    64,
       2,     2,     2,     2,    65,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    66,     2,    67,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63
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
     161,   166,   171,   176,   181,   184,   190,   191,   193,   195,
     197,   199,   201,   202,   207,   212,   219,   226,   231,   238,
     245,   250,   255,   260,   265,   268,   274,   276,   277,   279,
     283,   285,   288,   294,   296,   298,   300,   302,   304,   305,
     307,   309,   311,   313,   315,   316,   320,   324,   325,   329,
     330,   332,   334,   336,   338,   340,   342,   346,   348,   351,
     357,   361,   366,   371,   376,   381,   386,   391,   396,   401,
     406,   411,   416,   421,   426,   431,   436,   439,   443,   448,
     453,   458,   463,   468,   473,   478,   483,   488,   493,   498,
     503,   508,   513,   514,   516,   520,   524,   525,   529,   531,
     534,   535,   536,   542,   548,   549,   555,   557,   559,   562,
     568,   573,   579,   583,   585,   588,   591,   594,   597,   600,
     603,   606
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      71,     0,    -1,    72,     3,    -1,    -1,    72,   104,    -1,
      -1,     5,    -1,    24,    -1,    62,    -1,    60,    -1,    63,
      -1,    57,    -1,    -1,    74,    53,     7,    64,    -1,    74,
      52,    54,    64,    -1,    74,    52,    54,    65,     7,    64,
      -1,    74,    52,    55,    64,    -1,    74,    52,    55,    65,
       7,    64,    -1,    51,    66,    -1,    75,    73,    66,    74,
      67,    -1,    11,    66,    -1,    76,    14,     5,    64,    -1,
      76,    14,    10,    64,    -1,    76,    12,     5,    64,    -1,
      76,    12,    10,    64,    -1,    76,    13,     5,    64,    -1,
      76,    15,     6,    64,    -1,    76,    15,     5,    64,    -1,
      76,    15,    10,    64,    -1,    76,    18,     7,    64,    -1,
      76,    47,     5,    64,    -1,    76,    22,     5,    64,    -1,
      76,    23,     5,    64,    -1,    76,    16,    19,     5,    64,
      -1,    76,    20,     7,    64,    -1,    76,    21,     5,    64,
      -1,    76,    40,     8,    64,    -1,    76,    41,     8,    64,
      -1,    76,    42,     8,    64,    -1,    76,    43,     7,    64,
      -1,    76,    44,     9,    64,    -1,    76,    44,     7,    64,
      -1,    76,    45,     7,    64,    -1,    76,    46,     7,    64,
      -1,    17,    66,    -1,    77,     5,    19,     5,    64,    -1,
      -1,     5,    -1,    62,    -1,    60,    -1,    63,    -1,    57,
      -1,    -1,    79,    53,     7,    64,    -1,    79,    52,    54,
      64,    -1,    79,    52,    54,    68,     7,    64,    -1,    79,
      52,    54,    65,     7,    64,    -1,    79,    52,    55,    64,
      -1,    79,    52,    55,    68,     7,    64,    -1,    79,    52,
      55,    65,     7,    64,    -1,    79,    16,     5,    64,    -1,
      79,    56,    54,    64,    -1,    79,    56,    55,    64,    -1,
      79,    56,     5,    64,    -1,    24,    66,    -1,    80,    78,
      66,    79,    67,    -1,     5,    -1,    -1,    81,    -1,    82,
      69,    81,    -1,     5,    -1,    25,    66,    -1,    84,    83,
      66,    82,    67,    -1,    34,    -1,    35,    -1,    37,    -1,
      38,    -1,    36,    -1,    -1,     5,    -1,    62,    -1,    60,
      -1,    63,    -1,    57,    -1,    -1,    87,    86,    69,    -1,
      87,    86,    64,    -1,    -1,    88,     5,    64,    -1,    -1,
      21,    -1,    24,    -1,    62,    -1,    60,    -1,    63,    -1,
      57,    -1,     7,    68,     5,    -1,     5,    -1,    89,    66,
      -1,    90,    85,    66,    88,    67,    -1,    90,    85,    87,
      -1,    90,    27,     5,    64,    -1,    90,    39,    10,    64,
      -1,    90,    29,    10,    64,    -1,    90,    47,     5,    64,
      -1,    90,    49,     5,    64,    -1,    90,    28,    10,    64,
      -1,    90,    32,     7,    64,    -1,    90,    32,     8,    64,
      -1,    90,    33,     9,    64,    -1,    90,    33,     7,    64,
      -1,    90,    30,     7,    64,    -1,    90,    31,     7,    64,
      -1,    90,    48,    10,    64,    -1,    90,    50,     7,    64,
      -1,    90,    50,     8,    64,    -1,    26,    66,    -1,    91,
      90,    67,    -1,    91,    28,    10,    64,    -1,    91,    39,
      10,    64,    -1,    91,    29,    10,    64,    -1,    91,    33,
       9,    64,    -1,    91,    33,     7,    64,    -1,    91,    30,
       7,    64,    -1,    91,    31,     7,    64,    -1,    91,    32,
       7,    64,    -1,    91,    32,     8,    64,    -1,    91,    47,
       5,    64,    -1,    91,    48,    10,    64,    -1,    91,    49,
       5,    64,    -1,    91,    50,     7,    64,    -1,    91,    50,
       8,    64,    -1,    -1,    92,    -1,    93,    63,    69,    -1,
      93,    63,    64,    -1,    -1,    94,    62,    93,    -1,    61,
      -1,    27,     5,    -1,    -1,    -1,    98,    95,    66,    94,
      67,    -1,    98,    96,    66,    94,    67,    -1,    -1,    60,
     100,    66,    98,    97,    -1,    59,    -1,    58,    -1,    57,
      66,    -1,   103,   101,    66,    74,    67,    -1,   103,   101,
       5,    64,    -1,   103,   102,    66,    88,    67,    -1,   103,
     102,    87,    -1,    64,    -1,    76,    67,    -1,    75,    67,
      -1,    77,    67,    -1,    80,    67,    -1,    84,    67,    -1,
      91,    67,    -1,    99,    67,    -1,   103,    67,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   503,   503,   505,   507,   510,   511,   512,   513,   514,
     515,   516,   519,   520,   527,   535,   548,   556,   572,   573,
     581,   582,   583,   584,   585,   586,   591,   592,   593,   594,
     598,   603,   604,   605,   610,   613,   634,   637,   640,   643,
     646,   649,   652,   655,   661,   662,   714,   715,   716,   717,
     718,   719,   722,   723,   730,   739,   748,   762,   771,   780,
     794,   802,   806,   810,   817,   818,   826,   829,   831,   832,
     836,   840,   841,   845,   848,   851,   854,   857,   862,   863,
     864,   865,   866,   867,   870,   871,   872,   875,   876,   904,
     905,   906,   907,   908,   909,   910,   911,   927,   931,   932,
     933,   934,   935,   936,   937,   938,   939,   940,   943,   946,
     949,   952,   955,   958,   959,   962,   968,   969,   970,   971,
     972,   973,   976,   979,   982,   985,   988,   991,   992,   993,
     994,   997,  1002,  1011,  1012,  1013,  1016,  1017,  1023,  1046,
    1073,  1077,  1078,  1079,  1082,  1082,  1086,  1090,  1096,  1097,
    1104,  1110,  1111,  1114,  1114,  1114,  1114,  1114,  1114,  1114,
    1114,  1114
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "INVALID_TOKEN", "TEXT", "HEXSTR",
  "NUM", "INTERVAL", "SIZE", "BOOL", "SYSTEM", "IDENTITY", "HOSTNAME",
  "SVERSION", "NSID", "KEY", "KEYS", "MAX_UDP_PAYLOAD", "TSIG_ALGO_NAME",
  "WORKERS", "USER", "RUNDIR", "PIDFILE", "REMOTES", "GROUPS", "ZONES",
  "FILENAME", "DISABLE_ANY", "SEMANTIC_CHECKS", "NOTIFY_RETRIES",
  "NOTIFY_TIMEOUT", "DBSYNC_TIMEOUT", "IXFR_FSLIMIT", "XFR_IN", "XFR_OUT",
  "UPDATE_IN", "NOTIFY_IN", "NOTIFY_OUT", "BUILD_DIFFS", "MAX_CONN_IDLE",
  "MAX_CONN_HS", "MAX_CONN_REPLY", "RATE_LIMIT", "RATE_LIMIT_SIZE",
  "RATE_LIMIT_SLIP", "TRANSFERS", "STORAGE", "DNSSEC_ENABLE",
  "DNSSEC_KEYDIR", "SIGNATURE_LIFETIME", "INTERFACES", "ADDRESS", "PORT",
  "IPA", "IPA6", "VIA", "CONTROL", "ALLOW", "LISTEN_ON", "LOG", "LOG_DEST",
  "LOG_SRC", "LOG_LEVEL", "';'", "'@'", "'{'", "'}'", "'/'", "','",
  "$accept", "config", "conf_entries", "interface_start", "interface",
  "interfaces", "system", "keys", "remote_start", "remote", "remotes",
  "group_member", "group", "group_start", "groups", "zone_acl_start",
  "zone_acl_item", "zone_acl_list", "zone_acl", "zone_start", "zone",
  "zones", "log_prios_start", "log_prios", "log_src", "log_dest",
  "log_file", "log_end", "log_start", "log", "$@1", "ctl_listen_start",
  "ctl_allow_start", "control", "conf", 0
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
     315,   316,   317,   318,    59,    64,   123,   125,    47,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    70,    71,    72,    72,    73,    73,    73,    73,    73,
      73,    73,    74,    74,    74,    74,    74,    74,    75,    75,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    77,    77,    78,    78,    78,    78,
      78,    78,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    80,    80,    81,    82,    82,    82,
      83,    84,    84,    85,    85,    85,    85,    85,    86,    86,
      86,    86,    86,    86,    87,    87,    87,    88,    88,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    92,    93,    93,    93,    94,    94,    95,    96,
      97,    98,    98,    98,   100,    99,   101,   102,   103,   103,
     103,   103,   103,   104,   104,   104,   104,   104,   104,   104,
     104,   104
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     0,     4,     4,     6,     4,     6,     2,     5,
       2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     2,     5,     0,     1,     1,     1,
       1,     1,     0,     4,     4,     6,     6,     4,     6,     6,
       4,     4,     4,     4,     2,     5,     1,     0,     1,     3,
       1,     2,     5,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     1,     1,     0,     3,     3,     0,     3,     0,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     5,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     3,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     0,     1,     3,     3,     0,     3,     1,     2,
       0,     0,     5,     5,     0,     5,     1,     1,     2,     5,
       4,     5,     3,     1,     2,     2,     2,     2,     2,     2,
       2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     2,     0,     0,     0,     0,     0,
       0,     0,   144,   153,     5,     0,     0,    46,     0,    89,
       0,     0,     4,    20,    44,    64,    71,   116,    18,   148,
       0,     6,     7,    11,     9,     8,    10,   155,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   154,     0,   156,
      47,    51,    49,    48,    50,   157,     0,    70,   158,     0,
      97,     0,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    95,    93,    92,    94,   159,
       0,     0,   160,   147,   146,   161,     0,    84,   141,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,    67,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    98,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    77,    75,    76,     0,     0,     0,     0,     0,   117,
      84,     0,    12,    87,   152,   140,     0,    23,    24,    25,
      21,    22,    27,    26,    28,     0,    29,    34,    35,    31,
      32,    36,    37,    38,    39,    41,    40,    42,    43,    30,
       0,     0,    66,    68,     0,    96,   118,   120,   123,   124,
     125,   126,   122,   121,   119,   127,   128,   129,   130,   131,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    87,   100,   150,     0,     0,
      79,    83,    81,    80,    82,     0,     0,   138,     0,     0,
     145,     0,     0,    19,    33,    45,     0,     0,     0,     0,
      65,    72,     0,   101,   106,   103,   111,   112,   107,   108,
     110,   109,   102,   104,   113,   105,   114,   115,     0,   149,
       0,   151,    86,    85,   139,   136,   136,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,    99,    88,
       0,     0,    14,     0,    16,     0,    13,    60,    54,     0,
       0,    57,     0,     0,    53,    63,    61,    62,   132,   142,
     143,     0,     0,     0,     0,     0,     0,   133,   137,    15,
      17,    56,    55,    59,    58,     0,   135,   134
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    38,   166,    14,    15,    16,    66,   191,
      17,   193,   194,    69,    18,   160,   235,   164,   229,    90,
      91,    19,   317,   318,   290,   238,   239,   240,   165,    20,
      30,    96,    97,    21,    22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -57
static const yytype_int16 yypact[] =
{
     -57,    21,    92,   -57,   -57,   -56,   -39,   -35,   -24,   -21,
      19,    31,   -57,   -57,    17,   117,    -2,    48,    -1,    39,
      -6,   -43,   -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,
      38,   -57,   -57,   -57,   -57,   -57,   -57,   -57,    41,     2,
      71,     9,    20,    94,   121,   129,   141,   142,   143,   145,
     146,   147,   144,    30,   171,   172,   175,   -57,   131,   -57,
     -57,   -57,   -57,   -57,   -57,   -57,   115,   -57,   -57,   116,
     -57,   122,   -57,   -57,   173,   179,   184,   185,    10,    47,
     183,   189,   186,   190,    42,   -57,   -57,   -57,   -57,   -57,
     132,   138,   -57,   -57,   -57,   -57,    -4,   133,   -57,   -57,
     136,   137,   139,   140,   148,   149,   150,   151,   192,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   197,   -57,   201,   202,   166,   167,   168,
     169,   170,   174,   176,   177,   178,   180,   181,   182,   187,
     188,   -57,   203,   199,   200,   204,   228,    85,    91,   -57,
     -57,   -57,   -57,   -57,   226,   232,   229,   238,   114,   -57,
     191,   194,   -57,   -57,    -5,   -25,   -47,   -57,   -57,   -57,
     -57,   -57,   -57,   -57,   -57,   195,   -57,   -57,   -57,   -57,
     -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,
     196,    67,   -57,   -57,    45,   -57,   -57,   -57,   -57,   -57,
     -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,
     198,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   -57,    -5,   -57,   -44,     6,
     -57,   -57,   -57,   -57,   -57,   -36,   242,   -57,   219,   220,
     -57,    70,   241,   -57,   -57,   -57,   244,    72,   243,    27,
     -57,   -57,   201,   -57,   -57,   -57,   -57,   -57,   -57,   -57,
     -57,   -57,   -57,   -57,   -57,   -57,   -57,   -57,     8,   -57,
     223,   -57,   -57,   -57,   -57,   -57,   -57,    77,    80,   224,
     225,   -17,    26,   227,   230,   231,   233,   -57,   -57,   -57,
     -33,   -27,   -57,   246,   -57,   247,   -57,   -57,   -57,   248,
     249,   -57,   254,   256,   -57,   -57,   -57,   -57,   -57,   -57,
     -57,   234,   235,   236,   237,   239,   240,   -57,   221,   -57,
     -57,   -57,   -57,   -57,   -57,   -26,   -57,   -57
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -57,   -57,   -57,   -57,   102,   -57,   -57,   -57,   -57,   -57,
     -57,    13,   -57,   -57,   -57,   -57,   -57,   106,    43,   -57,
     -57,   -57,   -57,   -57,    -9,   -57,   -57,   -57,   -57,   -57,
     -57,   -57,   -57,   -57,   -57
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -79
static const yytype_int16 yytable[] =
{
     230,   161,   236,    58,    67,   241,   242,   100,   241,   242,
      23,   270,   101,   270,   103,    93,    94,   131,   132,   104,
     243,     3,    31,   269,    95,   105,   106,    24,   272,   308,
     107,    25,   284,   273,   309,   308,   237,   118,   326,   119,
     310,    32,    26,   327,    70,    27,    71,   298,   299,   139,
     140,   300,   231,    60,   133,   232,   134,   233,   234,   -78,
      72,    92,   162,    73,   -78,    59,    68,    74,    75,    76,
      77,    78,    79,   271,    33,   288,   102,    34,    80,    35,
      36,   285,   286,   246,    37,    28,    81,    82,    83,    84,
     301,   302,   215,   216,   303,     4,    85,    29,   217,    86,
     218,    87,    88,     5,    98,    61,    89,    99,    62,     6,
      63,    64,   251,   108,   252,    65,     7,     8,     9,   247,
     248,   223,   224,   249,   277,   278,   281,   282,   109,    39,
      40,    41,    42,    43,   250,    44,   110,    45,    46,    47,
      48,   292,   293,    10,   294,   295,   111,   112,   113,    11,
     123,   117,    12,   114,   115,   116,    13,    49,    50,    51,
      52,    53,    54,    55,    56,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   120,   121,
     122,   124,   125,   127,    57,   155,   156,   157,   158,   128,
     126,   129,   130,   135,   136,   138,   137,   175,   141,   163,
     167,   168,   190,   169,   170,   159,   192,   195,   210,   211,
     212,   213,   171,   172,   173,   174,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     196,   197,   198,   199,   200,   214,   219,   220,   201,   221,
     202,   203,   204,   222,   205,   206,   207,   274,   279,   280,
     283,   208,   209,   311,   312,   313,   314,   225,   227,   244,
     245,   315,   253,   316,   228,   287,   226,   291,   268,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,     0,   325,   275,   276,   289,   296,   297,
       0,   304,     0,     0,   305,   306,     0,   307,   319,   320,
     321,   322,     0,   323,   324
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-57))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       5,     5,    27,     5,     5,    52,    53,     5,    52,    53,
      66,     5,    10,     5,     5,    58,    59,     7,     8,    10,
      67,     0,     5,    67,    67,     5,     6,    66,    64,    62,
      10,    66,     5,    69,    67,    62,    61,     7,    64,     9,
      67,    24,    66,    69,     5,    66,     7,    64,    65,     7,
       8,    68,    57,     5,     7,    60,     9,    62,    63,    64,
      21,    67,    66,    24,    69,    67,    67,    28,    29,    30,
      31,    32,    33,    67,    57,    67,     5,    60,    39,    62,
      63,    54,    55,    16,    67,    66,    47,    48,    49,    50,
      64,    65,     7,     8,    68,     3,    57,    66,     7,    60,
       9,    62,    63,    11,    66,    57,    67,    66,    60,    17,
      62,    63,    67,    19,    69,    67,    24,    25,    26,    52,
      53,     7,     8,    56,    54,    55,    54,    55,     7,    12,
      13,    14,    15,    16,    67,    18,     7,    20,    21,    22,
      23,    64,    65,    51,    64,    65,     5,     5,     5,    57,
      19,     7,    60,     8,     8,     8,    64,    40,    41,    42,
      43,    44,    45,    46,    47,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,     7,     7,
       5,    66,    66,    10,    67,    47,    48,    49,    50,    10,
      68,     7,     7,    10,     5,     5,    10,     5,    66,    66,
      64,    64,     5,    64,    64,    67,     5,     5,     5,    10,
      10,     7,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,     7,    10,     5,    64,    10,
      64,    64,    64,     5,    64,    64,    64,     5,     7,     5,
       7,    64,    64,     7,     7,     7,     7,    66,    64,    64,
      64,     7,    64,     7,   162,   252,   160,   276,   225,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    -1,    63,    66,    66,    64,    64,    64,
      -1,    64,    -1,    -1,    64,    64,    -1,    64,    64,    64,
      64,    64,    -1,    64,    64
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    71,    72,     0,     3,    11,    17,    24,    25,    26,
      51,    57,    60,    64,    75,    76,    77,    80,    84,    91,
      99,   103,   104,    66,    66,    66,    66,    66,    66,    66,
     100,     5,    24,    57,    60,    62,    63,    67,    73,    12,
      13,    14,    15,    16,    18,    20,    21,    22,    23,    40,
      41,    42,    43,    44,    45,    46,    47,    67,     5,    67,
       5,    57,    60,    62,    63,    67,    78,     5,    67,    83,
       5,     7,    21,    24,    28,    29,    30,    31,    32,    33,
      39,    47,    48,    49,    50,    57,    60,    62,    63,    67,
      89,    90,    67,    58,    59,    67,   101,   102,    66,    66,
       5,    10,     5,     5,    10,     5,     6,    10,    19,     7,
       7,     5,     5,     5,     8,     8,     8,     7,     7,     9,
       7,     7,     5,    19,    66,    66,    68,    10,    10,     7,
       7,     7,     8,     7,     9,    10,     5,    10,     5,     7,
       8,    66,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    47,    48,    49,    50,    67,
      85,     5,    66,    66,    87,    98,    74,    64,    64,    64,
      64,    64,    64,    64,    64,     5,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
       5,    79,     5,    81,    82,     5,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
       5,    10,    10,     7,     7,     7,     8,     7,     9,    10,
       5,    10,     5,     7,     8,    66,    87,    64,    74,    88,
       5,    57,    60,    62,    63,    86,    27,    61,    95,    96,
      97,    52,    53,    67,    64,    64,    16,    52,    53,    56,
      67,    67,    69,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    88,    67,
       5,    67,    64,    69,     5,    66,    66,    54,    55,     7,
       5,    54,    55,     7,     5,    54,    55,    81,    67,    64,
      94,    94,    64,    65,    64,    65,    64,    64,    64,    65,
      68,    64,    65,    68,    64,    64,    64,    64,    62,    67,
      67,     7,     7,     7,     7,     7,     7,    92,    93,    64,
      64,    64,    64,    64,    64,    63,    64,    69
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

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


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
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
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

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void *scanner);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


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

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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
  *++yyvsp = yylval;

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

/* Line 1806 of yacc.c  */
#line 503 "cf-parse.y"
    { return 0; }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 511 "cf-parse.y"
    { conf_start_iface(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 512 "cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 513 "cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 514 "cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 515 "cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 516 "cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 520 "cf-parse.y"
    {
     if (this_iface->port > 0) {
       cf_error(scanner, "only one port definition is allowed in interface section\n");
     } else {
       SET_UINT16(this_iface->port, (yyvsp[(3) - (4)].tok).i, "port");
     }
   }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 527 "cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (4)].tok).t;
       this_iface->family = AF_INET;
     }
   }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 535 "cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (6)].tok).t;
       this_iface->family = AF_INET;
       if (this_iface->port > 0) {
	 cf_error(scanner, "only one port definition is allowed in interface section\n");
       } else {
         SET_UINT16(this_iface->port, (yyvsp[(5) - (6)].tok).i, "port");
       }
     }
   }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 548 "cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (4)].tok).t;
       this_iface->family = AF_INET6;
     }
   }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 556 "cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (6)].tok).t;
       this_iface->family = AF_INET6;
       if (this_iface->port > 0) {
          cf_error(scanner, "only one port definition is allowed in interface section\n");
       } else {
          SET_UINT16(this_iface->port, (yyvsp[(5) - (6)].tok).i, "port");
       }
     }
   }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 573 "cf-parse.y"
    {
   if (this_iface->address == 0) {
     cf_error(scanner, "interface '%s' has no defined address", this_iface->name);
   }
 }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 582 "cf-parse.y"
    { new_config->version = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 583 "cf-parse.y"
    { ident_auto(SVERSION, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 584 "cf-parse.y"
    { new_config->identity = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 585 "cf-parse.y"
    { ident_auto(IDENTITY, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 586 "cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.hostname' is deprecated. "
                     "Use 'system.identity' instead.\n");
     free((yyvsp[(3) - (4)].tok).t);
 }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 591 "cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 592 "cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = strlen(new_config->nsid); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 593 "cf-parse.y"
    { ident_auto(NSID, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 594 "cf-parse.y"
    {
     SET_NUM(new_config->max_udp_payload, (yyvsp[(3) - (4)].tok).i, EDNS_MIN_UDP_PAYLOAD,
             EDNS_MAX_UDP_PAYLOAD, "max-udp-payload");
 }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 598 "cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.storage' was relocated. "
                     "Use 'zones.storage' instead.\n");
     new_config->storage = (yyvsp[(3) - (4)].tok).t;
 }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 603 "cf-parse.y"
    { new_config->rundir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 604 "cf-parse.y"
    { new_config->pidfile = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 605 "cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.key' is deprecated "
                     "and has no effect.\n");
     free((yyvsp[(4) - (5)].tok).t);
 }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 610 "cf-parse.y"
    {
     SET_NUM(new_config->workers, (yyvsp[(3) - (4)].tok).i, 1, 255, "workers");
 }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 613 "cf-parse.y"
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

  case 36:

/* Line 1806 of yacc.c  */
#line 634 "cf-parse.y"
    {
	SET_INT(new_config->max_conn_idle, (yyvsp[(3) - (4)].tok).i, "max-conn-idle");
 }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 637 "cf-parse.y"
    {
	SET_INT(new_config->max_conn_hs, (yyvsp[(3) - (4)].tok).i, "max-conn-handshake");
 }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 640 "cf-parse.y"
    {
	SET_INT(new_config->max_conn_reply, (yyvsp[(3) - (4)].tok).i, "max-conn-reply");
 }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 643 "cf-parse.y"
    {
	SET_INT(new_config->rrl, (yyvsp[(3) - (4)].tok).i, "rate-limit");
 }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 646 "cf-parse.y"
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[(3) - (4)].tok).l, "rate-limit-size");
 }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 649 "cf-parse.y"
    {
	SET_SIZE(new_config->rrl_size, (yyvsp[(3) - (4)].tok).i, "rate-limit-size");
 }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 652 "cf-parse.y"
    {
	SET_INT(new_config->rrl_slip, (yyvsp[(3) - (4)].tok).i, "rate-limit-slip");
 }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 655 "cf-parse.y"
    {
	SET_INT(new_config->xfers, (yyvsp[(3) - (4)].tok).i, "transfers");
 }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 662 "cf-parse.y"
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
	   fqdnl = 0;
	} else {
	   strncpy(tmpdn, fqdn, fqdnl);
	   strncat(tmpdn, ".", 1);
	   free(fqdn);
	   fqdn = tmpdn;
	   fqdnl = strlen(fqdn);
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
                 knot_dname_free(&dname);
                 free(k);
             } else {
                 add_tail(&new_config->keys, &k->n);
                 ++new_config->key_count;
             }
         }
     }

     free((yyvsp[(4) - (5)].tok).t);
     free(fqdn);
}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 715 "cf-parse.y"
    { conf_start_remote(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 716 "cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 717 "cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 718 "cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 719 "cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 723 "cf-parse.y"
    {
     if (this_remote->port != 0) {
       cf_error(scanner, "only one port definition is allowed in remote section\n");
     } else {
       SET_UINT16(this_remote->port, (yyvsp[(3) - (4)].tok).i, "port");
     }
   }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 730 "cf-parse.y"
    {
     if (this_remote->address != 0) {
       cf_error(scanner, "only one address is allowed in remote section\n");
     } else {
       this_remote->address = (yyvsp[(3) - (4)].tok).t;
       this_remote->prefix = IPV4_PREFIXLEN;
       this_remote->family = AF_INET;
     }
   }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 739 "cf-parse.y"
    {
       if (this_remote->address != 0) {
         cf_error(scanner, "only one address is allowed in remote section\n");
       } else {
         this_remote->address = (yyvsp[(3) - (6)].tok).t;
         this_remote->family = AF_INET;
         SET_NUM(this_remote->prefix, (yyvsp[(5) - (6)].tok).i, 0, IPV4_PREFIXLEN, "prefix length");
       }
     }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 748 "cf-parse.y"
    {
     if (this_remote->address != 0) {
       cf_error(scanner, "only one address is allowed in remote section\n");
     } else {
       this_remote->address = (yyvsp[(3) - (6)].tok).t;
       this_remote->family = AF_INET;
       this_remote->prefix = IPV4_PREFIXLEN;
       if (this_remote->port != 0) {
	 cf_error(scanner, "only one port definition is allowed in remote section\n");
       } else {
         SET_UINT16(this_remote->port, (yyvsp[(5) - (6)].tok).i, "port");
       }
     }
   }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 762 "cf-parse.y"
    {
     if (this_remote->address != 0) {
       cf_error(scanner, "only one address is allowed in remote section\n");
     } else {
       this_remote->address = (yyvsp[(3) - (4)].tok).t;
       this_remote->family = AF_INET6;
       this_remote->prefix = IPV6_PREFIXLEN;
     }
   }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 771 "cf-parse.y"
    {
       if (this_remote->address != 0) {
         cf_error(scanner, "only one address is allowed in remote section\n");
       } else {
         this_remote->address = (yyvsp[(3) - (6)].tok).t;
         this_remote->family = AF_INET6;
         SET_NUM(this_remote->prefix, (yyvsp[(5) - (6)].tok).i, 0, IPV6_PREFIXLEN, "prefix length");
       }
     }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 780 "cf-parse.y"
    {
     if (this_remote->address != 0) {
       cf_error(scanner, "only one address is allowed in remote section\n");
     } else {
       this_remote->address = (yyvsp[(3) - (6)].tok).t;
       this_remote->family = AF_INET6;
       this_remote->prefix = IPV6_PREFIXLEN;
       if (this_remote->port != 0) {
	 cf_error(scanner, "only one port definition is allowed in remote section\n");
       } else {
         SET_UINT16(this_remote->port, (yyvsp[(5) - (6)].tok).i, "port");
       }
     }
   }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 794 "cf-parse.y"
    {
     if (this_remote->key != 0) {
       cf_error(scanner, "only one TSIG key definition is allowed in remote section\n");
     } else {
        conf_key_add(scanner, &this_remote->key, (yyvsp[(3) - (4)].tok).t);
     }
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 802 "cf-parse.y"
    {
     sockaddr_set(&this_remote->via, AF_INET, (yyvsp[(3) - (4)].tok).t, 0);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 806 "cf-parse.y"
    {
     sockaddr_set(&this_remote->via, AF_INET6, (yyvsp[(3) - (4)].tok).t, 0);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 810 "cf-parse.y"
    {
     conf_remote_set_via(scanner, (yyvsp[(3) - (4)].tok).t);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 818 "cf-parse.y"
    {
     if (this_remote->address == 0) {
       cf_error(scanner, "remote '%s' has no defined address", this_remote->name);
     }
   }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 826 "cf-parse.y"
    { conf_add_member_into_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 836 "cf-parse.y"
    { conf_start_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 845 "cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_in;
   }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 848 "cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_out;
   }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 851 "cf-parse.y"
    {
      this_list = &this_zone->acl.notify_in;
   }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 854 "cf-parse.y"
    {
      this_list = &this_zone->acl.notify_out;
   }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 857 "cf-parse.y"
    {
      this_list = &this_zone->acl.update_in;
 }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 863 "cf-parse.y"
    { conf_acl_item(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 864 "cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 865 "cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 866 "cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 867 "cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 876 "cf-parse.y"
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

  case 90:

/* Line 1806 of yacc.c  */
#line 905 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 906 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 907 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 908 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 909 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 910 "cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 911 "cf-parse.y"
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

  case 97:

/* Line 1806 of yacc.c  */
#line 927 "cf-parse.y"
    { conf_zone_start(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 934 "cf-parse.y"
    { this_zone->file = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 935 "cf-parse.y"
    { this_zone->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 936 "cf-parse.y"
    { this_zone->enable_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 937 "cf-parse.y"
    { this_zone->storage = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 938 "cf-parse.y"
    { this_zone->dnssec_keydir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 939 "cf-parse.y"
    { this_zone->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 940 "cf-parse.y"
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, "zonefile-sync");
 }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 943 "cf-parse.y"
    {
	SET_INT(this_zone->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, "zonefile-sync");
 }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 946 "cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).l, "ixfr-fslimit");
 }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 949 "cf-parse.y"
    {
	SET_SIZE(this_zone->ixfr_fslimit, (yyvsp[(3) - (4)].tok).i, "ixfr-fslimit");
 }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 952 "cf-parse.y"
    {
	SET_NUM(this_zone->notify_retries, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-retries");
   }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 955 "cf-parse.y"
    {
	SET_NUM(this_zone->notify_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-timeout");
   }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 958 "cf-parse.y"
    { this_zone->dnssec_enable = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 959 "cf-parse.y"
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 962 "cf-parse.y"
    {
	SET_NUM(this_zone->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 970 "cf-parse.y"
    { new_config->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 971 "cf-parse.y"
    { new_config->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 972 "cf-parse.y"
    { new_config->zone_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 973 "cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).l, "ixfr-fslimit");
 }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 976 "cf-parse.y"
    {
	SET_SIZE(new_config->ixfr_fslimit, (yyvsp[(3) - (4)].tok).i, "ixfr-fslimit");
 }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 979 "cf-parse.y"
    {
	SET_NUM(new_config->notify_retries, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-retries");
   }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 982 "cf-parse.y"
    {
	SET_NUM(new_config->notify_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "notify-timeout");
   }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 985 "cf-parse.y"
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "zonefile-sync");
 }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 988 "cf-parse.y"
    {
	SET_NUM(new_config->dbsync_timeout, (yyvsp[(3) - (4)].tok).i, 1, INT_MAX, "zonefile-sync");
 }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 991 "cf-parse.y"
    { new_config->storage = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 992 "cf-parse.y"
    { new_config->dnssec_enable = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 993 "cf-parse.y"
    { new_config->dnssec_keydir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 994 "cf-parse.y"
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 997 "cf-parse.y"
    {
	SET_NUM(new_config->sig_lifetime, (yyvsp[(3) - (4)].tok).i, 10800, INT_MAX, "signature-lifetime");
 }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1002 "cf-parse.y"
    {
  this_logmap = malloc(sizeof(conf_log_map_t));
  this_logmap->source = 0;
  this_logmap->prios = 0;
  add_tail(&this_log->map, &this_logmap->n);
}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1012 "cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i; }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1013 "cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i; }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1017 "cf-parse.y"
    {
     this_logmap->source = (yyvsp[(2) - (3)].tok).i;
     this_logmap = 0;
   }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1023 "cf-parse.y"
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
    ++new_config->logs_count;
  }
}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1046 "cf-parse.y"
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
    ++new_config->logs_count;
  }
}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1073 "cf-parse.y"
    {
}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1082 "cf-parse.y"
    { new_config->logs_count = 0; }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1086 "cf-parse.y"
    { conf_init_iface(scanner, NULL, -1); }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1090 "cf-parse.y"
    {
    this_list = &new_config->ctl.allow;
  }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1096 "cf-parse.y"
    { new_config->ctl.have = true; }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1097 "cf-parse.y"
    {
     if (this_iface->address == 0) {
       cf_error(scanner, "control interface has no defined address");
     } else {
       new_config->ctl.iface = this_iface;
     }
 }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1104 "cf-parse.y"
    {
     this_iface->address = (yyvsp[(3) - (4)].tok).t;
     this_iface->family = AF_UNIX;
     this_iface->port = 0;
     new_config->ctl.iface = this_iface;
 }
    break;



/* Line 1806 of yacc.c  */
#line 3404 "knot/conf/libknotd_la-cf-parse.c"
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

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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



/* Line 2067 of yacc.c  */
#line 1116 "cf-parse.y"


