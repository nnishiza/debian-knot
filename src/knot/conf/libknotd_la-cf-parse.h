/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
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
/* Line 2058 of yacc.c  */
#line 449 "knot/conf/cf-parse.y"

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;


/* Line 2058 of yacc.c  */
#line 203 "knot/conf/libknotd_la-cf-parse.h"
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
