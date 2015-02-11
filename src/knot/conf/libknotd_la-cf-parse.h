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
    MAX_TCP_CLIENTS = 300,
    RATE_LIMIT = 301,
    RATE_LIMIT_SIZE = 302,
    RATE_LIMIT_SLIP = 303,
    TRANSFERS = 304,
    STORAGE = 305,
    DNSSEC_ENABLE = 306,
    DNSSEC_KEYDIR = 307,
    SIGNATURE_LIFETIME = 308,
    SERIAL_POLICY = 309,
    SERIAL_POLICY_VAL = 310,
    QUERY_MODULE = 311,
    INTERFACES = 312,
    ADDRESS = 313,
    PORT = 314,
    IPA = 315,
    IPA6 = 316,
    VIA = 317,
    CONTROL = 318,
    ALLOW = 319,
    LISTEN_ON = 320,
    LOG = 321,
    LOG_DEST = 322,
    LOG_SRC = 323,
    LOG_LEVEL = 324
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
#define MAX_TCP_CLIENTS 300
#define RATE_LIMIT 301
#define RATE_LIMIT_SIZE 302
#define RATE_LIMIT_SLIP 303
#define TRANSFERS 304
#define STORAGE 305
#define DNSSEC_ENABLE 306
#define DNSSEC_KEYDIR 307
#define SIGNATURE_LIFETIME 308
#define SERIAL_POLICY 309
#define SERIAL_POLICY_VAL 310
#define QUERY_MODULE 311
#define INTERFACES 312
#define ADDRESS 313
#define PORT 314
#define IPA 315
#define IPA6 316
#define VIA 317
#define CONTROL 318
#define ALLOW 319
#define LISTEN_ON 320
#define LOG 321
#define LOG_DEST 322
#define LOG_SRC 323
#define LOG_LEVEL 324

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 478 "knot/conf/cf-parse.y" /* yacc.c:1909  */

	struct {
		char *t;
		long i;
		size_t l;
		dnssec_tsig_algorithm_t alg;
	} tok;

#line 201 "knot/conf/libknotd_la-cf-parse.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cf_parse (void *scanner);

#endif /* !YY_CF_KNOT_CONF_LIBKNOTD_LA_CF_PARSE_H_INCLUDED  */
