/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

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
#define YYBISON_VERSION "2.7.12-4996"

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

/* Headers */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "common/sockaddr.h"
#include "libknot/dname.h"
#include "libknot/binary.h"
#include "knot/conf/conf.h"
#include "libknotd_la-cf-parse.h" /* Automake generated header. */

extern int cf_lex (YYSTYPE *lvalp, void *scanner);
extern void cf_error(void *scanner, const char *format, ...);
extern conf_t *new_config;
static conf_iface_t *this_iface = 0;
static conf_iface_t *this_remote = 0;
static conf_zone_t *this_zone = 0;
static conf_group_t *this_group = 0;
static list *this_list = 0;
static conf_log_t *this_log = 0;
static conf_log_map_t *this_logmap = 0;
//#define YYERROR_VERBOSE 1

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
   node* r = 0; conf_iface_t* found = 0;
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
	node *n;
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
    knot_dname_t *sample = knot_dname_new_from_str(item, strlen(item), 0);
    conf_key_t* r = 0;
    WALK_LIST (r, new_config->keys) {
        if (knot_dname_compare(r->k.name, sample) == 0) {
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
    knot_dname_t *sample = knot_dname_new_from_str(item, strlen(item), 0);

    conf_key_t* r = 0;
    WALK_LIST (r, new_config->keys) {
        if (knot_dname_compare(r->k.name, sample) == 0) {
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
   this_zone->ixfr_fslimit = -1; // Default policy applies
   this_zone->dbsync_timeout = -1; // Default policy applies
   this_zone->disable_any = -1; // Default policy applies
   this_zone->build_diffs = -1; // Default policy applies

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

   /* Initialize ACL lists. */
   init_list(&this_zone->acl.xfr_in);
   init_list(&this_zone->acl.xfr_out);
   init_list(&this_zone->acl.notify_in);
   init_list(&this_zone->acl.notify_out);
   init_list(&this_zone->acl.update_in);

   /* Check domain name. */
   knot_dname_t *dn = NULL;
   if (this_zone->name != NULL) {
      dn = knot_dname_new_from_str(this_zone->name, nlen, 0);
   }
   if (dn == NULL) {
     free(this_zone->name);
     free(this_zone);
     this_zone = NULL;
     cf_error(scanner, "invalid zone origin");
   } else {
     /* Check for duplicates. */
     if (hattrie_tryget(new_config->names, (const char*)dn->name, dn->size) != NULL) {
           cf_error(scanner, "zone '%s' is already present, refusing to "
			     "duplicate", this_zone->name);
           knot_dname_free(&dn);
           free(this_zone->name);
           this_zone->name = NULL;
           /* Must not free, some versions of flex might continue after error and segfault.
            * free(this_zone); this_zone = NULL;
            */
           return;
     }

     /* Directly discard dname, won't be needed. */
     add_tail(&new_config->zones, &this_zone->n);
     *hattrie_get(new_config->names, (const char*)dn->name, dn->size) = (void *)1;
     ++new_config->zones_count;
     knot_dname_free(&dn);
   }
}

static int conf_mask(void* scanner, int nval, int prefixlen) {
    if (nval < 0 || nval > prefixlen) {
        cf_error(scanner, "IPv%c subnet prefix '%d' is out of range <0,%d>",
                 prefixlen == IPV4_PREFIXLEN ? '4' : '6', nval, prefixlen);
        return prefixlen; /* single host */
    }
    return nval;
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
#line 468 "knot/conf/libknotd_la-cf-parse.c"

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
     STORAGE = 271,
     KEY = 272,
     KEYS = 273,
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
     INTERFACES = 302,
     ADDRESS = 303,
     PORT = 304,
     IPA = 305,
     IPA6 = 306,
     VIA = 307,
     CONTROL = 308,
     ALLOW = 309,
     LISTEN_ON = 310,
     LOG = 311,
     LOG_DEST = 312,
     LOG_SRC = 313,
     LOG_LEVEL = 314
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
#define STORAGE 271
#define KEY 272
#define KEYS 273
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
#define INTERFACES 302
#define ADDRESS 303
#define PORT 304
#define IPA 305
#define IPA6 306
#define VIA 307
#define CONTROL 308
#define ALLOW 309
#define LISTEN_ON 310
#define LOG 311
#define LOG_DEST 312
#define LOG_SRC 313
#define LOG_LEVEL 314



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 421 "knot/conf/cf-parse.y"

    struct {
       char *t;
       long i;
       size_t l;
       knot_tsig_algorithm_t alg;
    } tok;


/* Line 387 of yacc.c  */
#line 639 "knot/conf/libknotd_la-cf-parse.c"
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
#line 666 "knot/conf/libknotd_la-cf-parse.c"

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

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
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
#define YYLAST   267

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  150
/* YYNRULES -- Number of states.  */
#define YYNSTATES  297

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   314

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    65,     2,     2,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    60,
       2,     2,     2,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    62,     2,    63,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,    10,    11,    13,    15,    17,
      19,    21,    23,    24,    29,    34,    41,    46,    53,    56,
      62,    65,    70,    75,    80,    85,    90,    95,   100,   105,
     110,   115,   120,   126,   131,   136,   141,   146,   151,   156,
     161,   166,   171,   176,   179,   185,   186,   188,   190,   192,
     194,   196,   197,   202,   207,   214,   221,   226,   233,   240,
     245,   250,   255,   260,   263,   269,   271,   272,   274,   278,
     280,   283,   289,   291,   293,   295,   297,   299,   300,   302,
     304,   306,   308,   310,   311,   315,   319,   320,   324,   325,
     327,   329,   331,   333,   335,   337,   341,   343,   346,   352,
     356,   361,   366,   371,   376,   381,   386,   391,   396,   401,
     406,   409,   413,   418,   423,   428,   433,   438,   443,   448,
     453,   458,   459,   461,   465,   469,   470,   474,   476,   479,
     480,   481,   487,   493,   494,   500,   502,   504,   507,   513,
     518,   524,   528,   530,   533,   536,   539,   542,   545,   548,
     551
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      67,     0,    -1,    68,     3,    -1,    -1,    68,   100,    -1,
      -1,     5,    -1,    24,    -1,    58,    -1,    56,    -1,    59,
      -1,    53,    -1,    -1,    70,    49,     7,    60,    -1,    70,
      48,    50,    60,    -1,    70,    48,    50,    61,     7,    60,
      -1,    70,    48,    51,    60,    -1,    70,    48,    51,    61,
       7,    60,    -1,    47,    62,    -1,    71,    69,    62,    70,
      63,    -1,    11,    62,    -1,    72,    14,     5,    60,    -1,
      72,    14,    10,    60,    -1,    72,    12,     5,    60,    -1,
      72,    12,    10,    60,    -1,    72,    13,     5,    60,    -1,
      72,    15,     6,    60,    -1,    72,    15,     5,    60,    -1,
      72,    15,    10,    60,    -1,    72,    16,     5,    60,    -1,
      72,    22,     5,    60,    -1,    72,    23,     5,    60,    -1,
      72,    17,    19,     5,    60,    -1,    72,    20,     7,    60,
      -1,    72,    21,     5,    60,    -1,    72,    40,     8,    60,
      -1,    72,    41,     8,    60,    -1,    72,    42,     8,    60,
      -1,    72,    43,     7,    60,    -1,    72,    44,     9,    60,
      -1,    72,    44,     7,    60,    -1,    72,    45,     7,    60,
      -1,    72,    46,     7,    60,    -1,    18,    62,    -1,    73,
       5,    19,     5,    60,    -1,    -1,     5,    -1,    58,    -1,
      56,    -1,    59,    -1,    53,    -1,    -1,    75,    49,     7,
      60,    -1,    75,    48,    50,    60,    -1,    75,    48,    50,
      64,     7,    60,    -1,    75,    48,    50,    61,     7,    60,
      -1,    75,    48,    51,    60,    -1,    75,    48,    51,    64,
       7,    60,    -1,    75,    48,    51,    61,     7,    60,    -1,
      75,    17,     5,    60,    -1,    75,    52,    50,    60,    -1,
      75,    52,    51,    60,    -1,    75,    52,     5,    60,    -1,
      24,    62,    -1,    76,    74,    62,    75,    63,    -1,     5,
      -1,    -1,    77,    -1,    78,    65,    77,    -1,     5,    -1,
      25,    62,    -1,    80,    79,    62,    78,    63,    -1,    34,
      -1,    35,    -1,    37,    -1,    38,    -1,    36,    -1,    -1,
       5,    -1,    58,    -1,    56,    -1,    59,    -1,    53,    -1,
      -1,    83,    82,    65,    -1,    83,    82,    60,    -1,    -1,
      84,     5,    60,    -1,    -1,    21,    -1,    24,    -1,    58,
      -1,    56,    -1,    59,    -1,    53,    -1,     7,    64,     5,
      -1,     5,    -1,    85,    62,    -1,    86,    81,    62,    84,
      63,    -1,    86,    81,    83,    -1,    86,    27,     5,    60,
      -1,    86,    39,    10,    60,    -1,    86,    29,    10,    60,
      -1,    86,    28,    10,    60,    -1,    86,    32,     7,    60,
      -1,    86,    32,     8,    60,    -1,    86,    33,     9,    60,
      -1,    86,    33,     7,    60,    -1,    86,    30,     7,    60,
      -1,    86,    31,     7,    60,    -1,    26,    62,    -1,    87,
      86,    63,    -1,    87,    28,    10,    60,    -1,    87,    39,
      10,    60,    -1,    87,    29,    10,    60,    -1,    87,    33,
       9,    60,    -1,    87,    33,     7,    60,    -1,    87,    30,
       7,    60,    -1,    87,    31,     7,    60,    -1,    87,    32,
       7,    60,    -1,    87,    32,     8,    60,    -1,    -1,    88,
      -1,    89,    59,    65,    -1,    89,    59,    60,    -1,    -1,
      90,    58,    89,    -1,    57,    -1,    27,     5,    -1,    -1,
      -1,    94,    91,    62,    90,    63,    -1,    94,    92,    62,
      90,    63,    -1,    -1,    56,    96,    62,    94,    93,    -1,
      55,    -1,    54,    -1,    53,    62,    -1,    99,    97,    62,
      70,    63,    -1,    99,    97,     5,    60,    -1,    99,    98,
      62,    84,    63,    -1,    99,    98,    83,    -1,    60,    -1,
      72,    63,    -1,    71,    63,    -1,    73,    63,    -1,    76,
      63,    -1,    80,    63,    -1,    87,    63,    -1,    95,    63,
      -1,    99,    63,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   483,   483,   485,   487,   490,   491,   492,   493,   494,
     495,   496,   499,   500,   507,   515,   528,   536,   552,   553,
     561,   562,   563,   564,   565,   566,   571,   572,   573,   574,
     575,   576,   577,   582,   589,   610,   611,   612,   613,   614,
     615,   616,   617,   621,   622,   674,   675,   676,   677,   678,
     679,   682,   683,   690,   699,   708,   722,   731,   740,   754,
     762,   766,   770,   777,   778,   786,   789,   791,   792,   796,
     800,   801,   805,   808,   811,   814,   817,   822,   823,   824,
     825,   826,   827,   830,   831,   832,   835,   836,   864,   865,
     866,   867,   868,   869,   870,   871,   888,   892,   893,   894,
     895,   896,   897,   898,   899,   900,   901,   902,   903,   910,
     920,   921,   922,   923,   924,   925,   926,   927,   934,   941,
     948,   951,   960,   961,   962,   965,   966,   972,   995,  1022,
    1026,  1027,  1028,  1031,  1031,  1035,  1039,  1045,  1046,  1053,
    1059,  1060,  1063,  1063,  1063,  1063,  1063,  1063,  1063,  1063,
    1063
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "INVALID_TOKEN", "TEXT", "HEXSTR",
  "NUM", "INTERVAL", "SIZE", "BOOL", "SYSTEM", "IDENTITY", "HOSTNAME",
  "SVERSION", "NSID", "STORAGE", "KEY", "KEYS", "TSIG_ALGO_NAME",
  "WORKERS", "USER", "RUNDIR", "PIDFILE", "REMOTES", "GROUPS", "ZONES",
  "FILENAME", "DISABLE_ANY", "SEMANTIC_CHECKS", "NOTIFY_RETRIES",
  "NOTIFY_TIMEOUT", "DBSYNC_TIMEOUT", "IXFR_FSLIMIT", "XFR_IN", "XFR_OUT",
  "UPDATE_IN", "NOTIFY_IN", "NOTIFY_OUT", "BUILD_DIFFS", "MAX_CONN_IDLE",
  "MAX_CONN_HS", "MAX_CONN_REPLY", "RATE_LIMIT", "RATE_LIMIT_SIZE",
  "RATE_LIMIT_SLIP", "TRANSFERS", "INTERFACES", "ADDRESS", "PORT", "IPA",
  "IPA6", "VIA", "CONTROL", "ALLOW", "LISTEN_ON", "LOG", "LOG_DEST",
  "LOG_SRC", "LOG_LEVEL", "';'", "'@'", "'{'", "'}'", "'/'", "','",
  "$accept", "config", "conf_entries", "interface_start", "interface",
  "interfaces", "system", "keys", "remote_start", "remote", "remotes",
  "group_member", "group", "group_start", "groups", "zone_acl_start",
  "zone_acl_item", "zone_acl_list", "zone_acl", "zone_start", "zone",
  "zones", "log_prios_start", "log_prios", "log_src", "log_dest",
  "log_file", "log_end", "log_start", "log", "$@1", "ctl_listen_start",
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
      59,    64,   123,   125,    47,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    68,    68,    69,    69,    69,    69,    69,
      69,    69,    70,    70,    70,    70,    70,    70,    71,    71,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    73,    73,    74,    74,    74,    74,    74,
      74,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    76,    76,    77,    78,    78,    78,    79,
      80,    80,    81,    81,    81,    81,    81,    82,    82,    82,
      82,    82,    82,    83,    83,    83,    84,    84,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    88,    89,    89,    89,    90,    90,    91,    92,    93,
      94,    94,    94,    96,    95,    97,    98,    99,    99,    99,
      99,    99,   100,   100,   100,   100,   100,   100,   100,   100,
     100
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     0,     4,     4,     6,     4,     6,     2,     5,
       2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     5,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     2,     5,     0,     1,     1,     1,     1,
       1,     0,     4,     4,     6,     6,     4,     6,     6,     4,
       4,     4,     4,     2,     5,     1,     0,     1,     3,     1,
       2,     5,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     1,     1,     0,     3,     3,     0,     3,     0,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     5,     3,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       2,     3,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     0,     1,     3,     3,     0,     3,     1,     2,     0,
       0,     5,     5,     0,     5,     1,     1,     2,     5,     4,
       5,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     2,     0,     0,     0,     0,     0,
       0,     0,   133,   142,     5,     0,     0,    45,     0,    88,
       0,     0,     4,    20,    43,    63,    70,   110,    18,   137,
       0,     6,     7,    11,     9,     8,    10,   144,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,     0,   145,    46,
      50,    48,    47,    49,   146,     0,    69,   147,     0,    96,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
      94,    92,    91,    93,   148,     0,     0,   149,   136,   135,
     150,     0,    83,   130,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,    66,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,     0,     0,     0,    72,    73,
      76,    74,    75,     0,   111,    83,     0,    12,    86,   141,
     129,     0,    23,    24,    25,    21,    22,    27,    26,    28,
      29,     0,    33,    34,    30,    31,    35,    36,    37,    38,
      40,    39,    41,    42,     0,     0,    65,    67,     0,    95,
     112,   114,   117,   118,   119,   120,   116,   115,   113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    86,
      99,   139,     0,     0,    78,    82,    80,    79,    81,     0,
       0,   127,     0,     0,   134,     0,     0,    19,    32,    44,
       0,     0,     0,     0,    64,    71,     0,   100,   103,   102,
     108,   109,   104,   105,   107,   106,   101,     0,   138,     0,
     140,    85,    84,   128,   125,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    68,    98,    87,     0,
       0,    14,     0,    16,     0,    13,    59,    53,     0,     0,
      56,     0,     0,    52,    62,    60,    61,   121,   131,   132,
       0,     0,     0,     0,     0,     0,   122,   126,    15,    17,
      55,    54,    58,    57,     0,   124,   123
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    38,   151,    14,    15,    16,    65,   175,
      17,   177,   178,    68,    18,   145,   209,   149,   203,    85,
      86,    19,   286,   287,   259,   212,   213,   214,   150,    20,
      30,    91,    92,    21,    22
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -47
static const yytype_int16 yypact[] =
{
     -47,    16,    72,   -47,   -47,   -25,     9,    12,    25,    33,
      58,    91,   -47,   -47,    14,    93,     0,    23,     1,     3,
       5,   -42,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
      98,   -47,   -47,   -47,   -47,   -47,   -47,   -47,    99,    13,
      73,    35,    20,    79,   105,   152,   157,   158,   159,   160,
     161,   162,   164,    84,   165,   166,   -47,   146,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   112,   -47,   -47,   115,   -47,
     102,   -47,   -47,   168,   169,   173,   174,    36,    95,   172,
     -47,   -47,   -47,   -47,   -47,   121,   113,   -47,   -47,   -47,
     -47,    -4,   122,   -47,   -47,   107,   125,   126,   127,   128,
     129,   130,   131,   132,   170,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   200,   -47,   201,
     202,   148,   149,   150,   151,   153,   154,   155,   156,   163,
     -47,   207,   208,   209,   210,   213,   104,   120,   -47,   -47,
     -47,   -47,   -47,   211,   -47,   167,   171,   -47,   -47,    -5,
     -18,   -46,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   175,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   176,    40,   -47,   -47,   -43,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   -47,
      -5,   -47,   -34,     2,   -47,   -47,   -47,   -47,   -47,   -19,
     217,   -47,   187,   188,   -47,    50,   218,   -47,   -47,   -47,
     219,    80,   220,    -1,   -47,   -47,   201,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,     6,   -47,   191,
     -47,   -47,   -47,   -47,   -47,   -47,    94,    97,   192,   193,
      57,    62,   194,   195,   196,   197,   -47,   -47,   -47,   -11,
      22,   -47,   221,   -47,   223,   -47,   -47,   -47,   225,   226,
     -47,   227,   240,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     198,   199,   203,   204,   205,   206,   -47,   189,   -47,   -47,
     -47,   -47,   -47,   -47,    34,   -47,   -47
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -47,   -47,   -47,   -47,   114,   -47,   -47,   -47,   -47,   -47,
     -47,    41,   -47,   -47,   -47,   -47,   -47,    81,    61,   -47,
     -47,   -47,   -47,   -47,    17,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -78
static const yytype_int16 yytable[] =
{
     204,   146,   215,   216,   253,    57,    66,   239,    69,   210,
      70,   239,    88,    89,   215,   216,     3,   217,    95,    31,
     225,    90,   226,    96,    71,   100,   101,    72,    59,   238,
     102,    73,    74,    75,    76,    77,    78,    23,    32,   211,
      98,   241,    79,   125,   126,    99,   242,   277,   205,   254,
     255,   206,   278,   207,   208,   -77,    80,   220,   147,    81,
     -77,    82,    83,    58,    67,   240,    84,    33,    87,   257,
      34,    24,    35,    36,    25,     4,    60,    37,    97,    61,
     277,    62,    63,     5,   103,   279,    64,    26,   221,   222,
       6,   113,   223,   114,   295,    27,     7,     8,     9,   296,
     246,   247,   127,   224,   128,    39,    40,    41,    42,    43,
      44,   194,   195,    45,    46,    47,    48,   267,   268,    10,
      28,   269,   270,   271,   104,    11,   272,   196,    12,   197,
     250,   251,    13,    49,    50,    51,    52,    53,    54,    55,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    29,   261,   262,    56,   263,   264,   105,
      93,    94,   106,   107,   108,   117,   120,   152,   109,   110,
     111,   112,   115,   116,   118,   161,   144,   119,   121,   122,
     123,   124,   129,   130,   148,   153,   154,   155,   156,   157,
     158,   159,   160,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   176,   179,   180,   181,
     182,   183,   189,   184,   185,   186,   187,   192,   190,   191,
     193,   198,   243,   188,   249,   248,   200,   252,   280,   199,
     281,   201,   282,   283,   284,   218,   219,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   285,   294,   244,
     245,   258,   265,   266,   273,   274,   275,   276,   288,   289,
     237,   202,   260,   290,   291,   292,   293,   256
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-47)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_uint8 yycheck[] =
{
       5,     5,    48,    49,     5,     5,     5,     5,     5,    27,
       7,     5,    54,    55,    48,    49,     0,    63,     5,     5,
      63,    63,    65,    10,    21,     5,     6,    24,     5,    63,
      10,    28,    29,    30,    31,    32,    33,    62,    24,    57,
       5,    60,    39,     7,     8,    10,    65,    58,    53,    50,
      51,    56,    63,    58,    59,    60,    53,    17,    62,    56,
      65,    58,    59,    63,    63,    63,    63,    53,    63,    63,
      56,    62,    58,    59,    62,     3,    53,    63,     5,    56,
      58,    58,    59,    11,     5,    63,    63,    62,    48,    49,
      18,     7,    52,     9,    60,    62,    24,    25,    26,    65,
      50,    51,     7,    63,     9,    12,    13,    14,    15,    16,
      17,     7,     8,    20,    21,    22,    23,    60,    61,    47,
      62,    64,    60,    61,    19,    53,    64,     7,    56,     9,
      50,    51,    60,    40,    41,    42,    43,    44,    45,    46,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    62,    60,    61,    63,    60,    61,     7,
      62,    62,     5,     5,     5,    19,    64,    60,     8,     8,
       8,     7,     7,     7,    62,     5,    63,    62,    10,    10,
       7,     7,    10,    62,    62,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,     5,     5,     5,    60,    60,
      60,    60,     5,    60,    60,    60,    60,     7,    10,    10,
       7,    10,     5,    60,     5,     7,   145,     7,     7,    62,
       7,    60,     7,     7,     7,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,     7,    59,    62,
      62,    60,    60,    60,    60,    60,    60,    60,    60,    60,
     199,   147,   245,    60,    60,    60,    60,   226
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    67,    68,     0,     3,    11,    18,    24,    25,    26,
      47,    53,    56,    60,    71,    72,    73,    76,    80,    87,
      95,    99,   100,    62,    62,    62,    62,    62,    62,    62,
      96,     5,    24,    53,    56,    58,    59,    63,    69,    12,
      13,    14,    15,    16,    17,    20,    21,    22,    23,    40,
      41,    42,    43,    44,    45,    46,    63,     5,    63,     5,
      53,    56,    58,    59,    63,    74,     5,    63,    79,     5,
       7,    21,    24,    28,    29,    30,    31,    32,    33,    39,
      53,    56,    58,    59,    63,    85,    86,    63,    54,    55,
      63,    97,    98,    62,    62,     5,    10,     5,     5,    10,
       5,     6,    10,     5,    19,     7,     5,     5,     5,     8,
       8,     8,     7,     7,     9,     7,     7,    19,    62,    62,
      64,    10,    10,     7,     7,     7,     8,     7,     9,    10,
      62,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    63,    81,     5,    62,    62,    83,
      94,    70,    60,    60,    60,    60,    60,    60,    60,    60,
      60,     5,    60,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,     5,    75,     5,    77,    78,     5,
      60,    60,    60,    60,    60,    60,    60,    60,    60,     5,
      10,    10,     7,     7,     7,     8,     7,     9,    10,    62,
      83,    60,    70,    84,     5,    53,    56,    58,    59,    82,
      27,    57,    91,    92,    93,    48,    49,    63,    60,    60,
      17,    48,    49,    52,    63,    63,    65,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    84,    63,     5,
      63,    60,    65,     5,    62,    62,    50,    51,     7,     5,
      50,    51,     7,     5,    50,    51,    77,    63,    60,    90,
      90,    60,    61,    60,    61,    60,    60,    60,    61,    64,
      60,    61,    64,    60,    60,    60,    60,    58,    63,    63,
       7,     7,     7,     7,     7,     7,    88,    89,    60,    60,
      60,    60,    60,    60,    59,    60,    65
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
  YYUSE (yytype);
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

  YYUSE (yytype);
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
/* Line 1787 of yacc.c  */
#line 483 "knot/conf/cf-parse.y"
    { return 0; }
    break;

  case 6:
/* Line 1787 of yacc.c  */
#line 491 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 492 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 8:
/* Line 1787 of yacc.c  */
#line 493 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 9:
/* Line 1787 of yacc.c  */
#line 494 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 10:
/* Line 1787 of yacc.c  */
#line 495 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 496 "knot/conf/cf-parse.y"
    { conf_start_iface(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 500 "knot/conf/cf-parse.y"
    {
     if (this_iface->port > 0) {
       cf_error(scanner, "only one port definition is allowed in interface section\n");
     } else {
       this_iface->port = (yyvsp[(3) - (4)].tok).i;
     }
   }
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 507 "knot/conf/cf-parse.y"
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
/* Line 1787 of yacc.c  */
#line 515 "knot/conf/cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (6)].tok).t;
       this_iface->family = AF_INET;
       if (this_iface->port > 0) {
	 cf_error(scanner, "only one port definition is allowed in interface section\n");
       } else {
	 this_iface->port = (yyvsp[(5) - (6)].tok).i;
       }
     }
   }
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 528 "knot/conf/cf-parse.y"
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
/* Line 1787 of yacc.c  */
#line 536 "knot/conf/cf-parse.y"
    {
     if (this_iface->address != 0) {
       cf_error(scanner, "only one address is allowed in interface section\n");
     } else {
       this_iface->address = (yyvsp[(3) - (6)].tok).t;
       this_iface->family = AF_INET6;
       if (this_iface->port > 0) {
          cf_error(scanner, "only one port definition is allowed in interface section\n");
       } else {
          this_iface->port = (yyvsp[(5) - (6)].tok).i;
       }
     }
   }
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 553 "knot/conf/cf-parse.y"
    {
   if (this_iface->address == 0) {
     cf_error(scanner, "interface '%s' has no defined address", this_iface->name);
   }
 }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 562 "knot/conf/cf-parse.y"
    { new_config->version = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 563 "knot/conf/cf-parse.y"
    { ident_auto(SVERSION, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 564 "knot/conf/cf-parse.y"
    { new_config->identity = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 565 "knot/conf/cf-parse.y"
    { ident_auto(IDENTITY, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 566 "knot/conf/cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.hostname' is deprecated. "
                     "Use 'system.identity' instead.\n");
     free((yyvsp[(3) - (4)].tok).t);
 }
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 571 "knot/conf/cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 572 "knot/conf/cf-parse.y"
    { new_config->nsid = (yyvsp[(3) - (4)].tok).t; new_config->nsid_len = strlen(new_config->nsid); }
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 573 "knot/conf/cf-parse.y"
    { ident_auto(NSID, new_config, (yyvsp[(3) - (4)].tok).i); }
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 574 "knot/conf/cf-parse.y"
    { new_config->storage = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 575 "knot/conf/cf-parse.y"
    { new_config->rundir = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 576 "knot/conf/cf-parse.y"
    { new_config->pidfile = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 577 "knot/conf/cf-parse.y"
    {
     fprintf(stderr, "warning: Config option 'system.key' is deprecated "
                     "and has no effect.\n");
     free((yyvsp[(4) - (5)].tok).t);
 }
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 582 "knot/conf/cf-parse.y"
    {
     if ((yyvsp[(3) - (4)].tok).i <= 0) {
        cf_error(scanner, "worker count must be greater than 0\n");
     } else {
        new_config->workers = (yyvsp[(3) - (4)].tok).i;
     }
 }
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 589 "knot/conf/cf-parse.y"
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

  case 35:
/* Line 1787 of yacc.c  */
#line 610 "knot/conf/cf-parse.y"
    { new_config->max_conn_idle = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 611 "knot/conf/cf-parse.y"
    { new_config->max_conn_hs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 612 "knot/conf/cf-parse.y"
    { new_config->max_conn_reply = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 613 "knot/conf/cf-parse.y"
    { new_config->rrl = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 614 "knot/conf/cf-parse.y"
    { new_config->rrl_size = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 615 "knot/conf/cf-parse.y"
    { new_config->rrl_size = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 616 "knot/conf/cf-parse.y"
    { new_config->rrl_slip = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 617 "knot/conf/cf-parse.y"
    { new_config->xfers = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 622 "knot/conf/cf-parse.y"
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
         knot_dname_t *dname = knot_dname_new_from_str(fqdn, fqdnl, 0);
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

  case 46:
/* Line 1787 of yacc.c  */
#line 675 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 676 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 48:
/* Line 1787 of yacc.c  */
#line 677 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 49:
/* Line 1787 of yacc.c  */
#line 678 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 679 "knot/conf/cf-parse.y"
    { conf_start_remote(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 52:
/* Line 1787 of yacc.c  */
#line 683 "knot/conf/cf-parse.y"
    {
     if (this_remote->port != 0) {
       cf_error(scanner, "only one port definition is allowed in remote section\n");
     } else {
       this_remote->port = (yyvsp[(3) - (4)].tok).i;
     }
   }
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 690 "knot/conf/cf-parse.y"
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

  case 54:
/* Line 1787 of yacc.c  */
#line 699 "knot/conf/cf-parse.y"
    {
       if (this_remote->address != 0) {
         cf_error(scanner, "only one address is allowed in remote section\n");
       } else {
         this_remote->address = (yyvsp[(3) - (6)].tok).t;
         this_remote->family = AF_INET;
         this_remote->prefix = conf_mask(scanner, (yyvsp[(5) - (6)].tok).i, IPV4_PREFIXLEN);
       }
     }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 708 "knot/conf/cf-parse.y"
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
	 this_remote->port = (yyvsp[(5) - (6)].tok).i;
       }
     }
   }
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 722 "knot/conf/cf-parse.y"
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

  case 57:
/* Line 1787 of yacc.c  */
#line 731 "knot/conf/cf-parse.y"
    {
       if (this_remote->address != 0) {
         cf_error(scanner, "only one address is allowed in remote section\n");
       } else {
         this_remote->address = (yyvsp[(3) - (6)].tok).t;
         this_remote->family = AF_INET6;
         this_remote->prefix = conf_mask(scanner, (yyvsp[(5) - (6)].tok).i, IPV6_PREFIXLEN);
       }
     }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 740 "knot/conf/cf-parse.y"
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
	 this_remote->port = (yyvsp[(5) - (6)].tok).i;
       }
     }
   }
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 754 "knot/conf/cf-parse.y"
    {
     if (this_remote->key != 0) {
       cf_error(scanner, "only one TSIG key definition is allowed in remote section\n");
     } else {
        conf_key_add(scanner, &this_remote->key, (yyvsp[(3) - (4)].tok).t);
     }
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 762 "knot/conf/cf-parse.y"
    {
     sockaddr_set(&this_remote->via, AF_INET, (yyvsp[(3) - (4)].tok).t, 0);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 766 "knot/conf/cf-parse.y"
    {
     sockaddr_set(&this_remote->via, AF_INET6, (yyvsp[(3) - (4)].tok).t, 0);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 770 "knot/conf/cf-parse.y"
    {
     conf_remote_set_via(scanner, (yyvsp[(3) - (4)].tok).t);
     free((yyvsp[(3) - (4)].tok).t);
   }
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 778 "knot/conf/cf-parse.y"
    {
     if (this_remote->address == 0) {
       cf_error(scanner, "remote '%s' has no defined address", this_remote->name);
     }
   }
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 786 "knot/conf/cf-parse.y"
    { conf_add_member_into_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 796 "knot/conf/cf-parse.y"
    { conf_start_group(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 805 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_in;
   }
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 808 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.xfr_out;
   }
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 811 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.notify_in;
   }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 814 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.notify_out;
   }
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 817 "knot/conf/cf-parse.y"
    {
      this_list = &this_zone->acl.update_in;
 }
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 823 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 824 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 825 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 826 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 827 "knot/conf/cf-parse.y"
    { conf_acl_item(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 836 "knot/conf/cf-parse.y"
    {
      /* Find existing node in remotes. */
      node* r = 0; conf_iface_t* found = 0;
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

  case 89:
/* Line 1787 of yacc.c  */
#line 865 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 866 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 867 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 868 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 869 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 870 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, strdup((yyvsp[(1) - (1)].tok).t)); }
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 871 "knot/conf/cf-parse.y"
    {
    if ((yyvsp[(1) - (3)].tok).i < 0 || (yyvsp[(1) - (3)].tok).i > 255) {
        cf_error(scanner, "rfc2317 origin prefix '%ld' out of bounds", (yyvsp[(1) - (3)].tok).i);
    }
    size_t len = 3 + 1 + strlen((yyvsp[(3) - (3)].tok).t) + 1; /* <0,255> '/' rest */
    char *name = malloc(len * sizeof(char));
    if (name == NULL) {
        cf_error(scanner, "out of memory");
    } else {
        name[0] = '\0';
        if (snprintf(name, len, "%ld/%s", (yyvsp[(1) - (3)].tok).i, (yyvsp[(3) - (3)].tok).t) < 0) {
            cf_error(scanner,"failed to convert rfc2317 origin to string");
        }
    }
    free((yyvsp[(3) - (3)].tok).t);
    conf_zone_start(scanner, name);
 }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 888 "knot/conf/cf-parse.y"
    { conf_zone_start(scanner, (yyvsp[(1) - (1)].tok).t); }
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 895 "knot/conf/cf-parse.y"
    { this_zone->file = (yyvsp[(3) - (4)].tok).t; }
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 896 "knot/conf/cf-parse.y"
    { this_zone->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 897 "knot/conf/cf-parse.y"
    { this_zone->enable_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 898 "knot/conf/cf-parse.y"
    { this_zone->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 899 "knot/conf/cf-parse.y"
    { this_zone->dbsync_timeout = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 105:
/* Line 1787 of yacc.c  */
#line 900 "knot/conf/cf-parse.y"
    { this_zone->dbsync_timeout = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 901 "knot/conf/cf-parse.y"
    { new_config->ixfr_fslimit = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 902 "knot/conf/cf-parse.y"
    { this_zone->ixfr_fslimit = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 903 "knot/conf/cf-parse.y"
    {
       if ((yyvsp[(3) - (4)].tok).i < 1) {
	   cf_error(scanner, "notify retries must be positive integer");
       } else {
	   this_zone->notify_retries = (yyvsp[(3) - (4)].tok).i;
       }
   }
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 910 "knot/conf/cf-parse.y"
    {
	if ((yyvsp[(3) - (4)].tok).i < 1) {
	   cf_error(scanner, "notify timeout must be positive integer");
       } else {
	   this_zone->notify_timeout = (yyvsp[(3) - (4)].tok).i;
       }
   }
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 922 "knot/conf/cf-parse.y"
    { new_config->disable_any = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 923 "knot/conf/cf-parse.y"
    { new_config->build_diffs = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 924 "knot/conf/cf-parse.y"
    { new_config->zone_checks = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 115:
/* Line 1787 of yacc.c  */
#line 925 "knot/conf/cf-parse.y"
    { new_config->ixfr_fslimit = (yyvsp[(3) - (4)].tok).l; }
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 926 "knot/conf/cf-parse.y"
    { new_config->ixfr_fslimit = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 117:
/* Line 1787 of yacc.c  */
#line 927 "knot/conf/cf-parse.y"
    {
       if ((yyvsp[(3) - (4)].tok).i < 1) {
	   cf_error(scanner, "notify retries must be positive integer");
       } else {
	   new_config->notify_retries = (yyvsp[(3) - (4)].tok).i;
       }
   }
    break;

  case 118:
/* Line 1787 of yacc.c  */
#line 934 "knot/conf/cf-parse.y"
    {
	if ((yyvsp[(3) - (4)].tok).i < 1) {
	   cf_error(scanner, "notify timeout must be positive integer");
       } else {
	   new_config->notify_timeout = (yyvsp[(3) - (4)].tok).i;
       }
   }
    break;

  case 119:
/* Line 1787 of yacc.c  */
#line 941 "knot/conf/cf-parse.y"
    {
	if ((yyvsp[(3) - (4)].tok).i < 1) {
	   cf_error(scanner, "zonefile sync timeout must be positive integer");
       } else {
	   new_config->dbsync_timeout = (yyvsp[(3) - (4)].tok).i;
       }
 }
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 948 "knot/conf/cf-parse.y"
    { new_config->dbsync_timeout = (yyvsp[(3) - (4)].tok).i; }
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 951 "knot/conf/cf-parse.y"
    {
  this_logmap = malloc(sizeof(conf_log_map_t));
  this_logmap->source = 0;
  this_logmap->prios = 0;
  add_tail(&this_log->map, &this_logmap->n);
}
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 961 "knot/conf/cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i; }
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 962 "knot/conf/cf-parse.y"
    { this_logmap->prios |= (yyvsp[(2) - (3)].tok).i; }
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 966 "knot/conf/cf-parse.y"
    {
     this_logmap->source = (yyvsp[(2) - (3)].tok).i;
     this_logmap = 0;
   }
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 972 "knot/conf/cf-parse.y"
    {
  /* Find already existing rule. */
  this_log = 0;
  node *n = 0;
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

  case 128:
/* Line 1787 of yacc.c  */
#line 995 "knot/conf/cf-parse.y"
    {
  /* Find already existing rule. */
  this_log = 0;
  node *n = 0;
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

  case 129:
/* Line 1787 of yacc.c  */
#line 1022 "knot/conf/cf-parse.y"
    {
}
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 1031 "knot/conf/cf-parse.y"
    { new_config->logs_count = 0; }
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 1035 "knot/conf/cf-parse.y"
    { conf_init_iface(scanner, NULL, -1); }
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 1039 "knot/conf/cf-parse.y"
    {
    this_list = &new_config->ctl.allow;
  }
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 1045 "knot/conf/cf-parse.y"
    { new_config->ctl.have = true; }
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 1046 "knot/conf/cf-parse.y"
    {
     if (this_iface->address == 0) {
       cf_error(scanner, "control interface has no defined address");
     } else {
       new_config->ctl.iface = this_iface;
     }
 }
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 1053 "knot/conf/cf-parse.y"
    {
     this_iface->address = (yyvsp[(3) - (4)].tok).t;
     this_iface->family = AF_UNIX;
     this_iface->port = 0;
     new_config->ctl.iface = this_iface;
 }
    break;


/* Line 1787 of yacc.c  */
#line 3147 "knot/conf/libknotd_la-cf-parse.c"
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


/* Line 2050 of yacc.c  */
#line 1065 "knot/conf/cf-parse.y"

