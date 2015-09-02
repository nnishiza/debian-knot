/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
#line 475 "knot/conf/cf-parse.y" /* yacc.c:1909  */

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;

#line 203 "knot/conf/libknotd_la-cf-parse.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cf_parse (void *scanner);

#endif /* !YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED  */
