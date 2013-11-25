/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
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
     MAX_UDP_PAYLOAD = 274,
     TSIG_ALGO_NAME = 275,
     WORKERS = 276,
     USER = 277,
     RUNDIR = 278,
     PIDFILE = 279,
     REMOTES = 280,
     GROUPS = 281,
     ZONES = 282,
     FILENAME = 283,
     DISABLE_ANY = 284,
     SEMANTIC_CHECKS = 285,
     NOTIFY_RETRIES = 286,
     NOTIFY_TIMEOUT = 287,
     DBSYNC_TIMEOUT = 288,
     IXFR_FSLIMIT = 289,
     XFR_IN = 290,
     XFR_OUT = 291,
     UPDATE_IN = 292,
     NOTIFY_IN = 293,
     NOTIFY_OUT = 294,
     BUILD_DIFFS = 295,
     MAX_CONN_IDLE = 296,
     MAX_CONN_HS = 297,
     MAX_CONN_REPLY = 298,
     RATE_LIMIT = 299,
     RATE_LIMIT_SIZE = 300,
     RATE_LIMIT_SLIP = 301,
     TRANSFERS = 302,
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
#define STORAGE 271
#define KEY 272
#define KEYS 273
#define MAX_UDP_PAYLOAD 274
#define TSIG_ALGO_NAME 275
#define WORKERS 276
#define USER 277
#define RUNDIR 278
#define PIDFILE 279
#define REMOTES 280
#define GROUPS 281
#define ZONES 282
#define FILENAME 283
#define DISABLE_ANY 284
#define SEMANTIC_CHECKS 285
#define NOTIFY_RETRIES 286
#define NOTIFY_TIMEOUT 287
#define DBSYNC_TIMEOUT 288
#define IXFR_FSLIMIT 289
#define XFR_IN 290
#define XFR_OUT 291
#define UPDATE_IN 292
#define NOTIFY_IN 293
#define NOTIFY_OUT 294
#define BUILD_DIFFS 295
#define MAX_CONN_IDLE 296
#define MAX_CONN_HS 297
#define MAX_CONN_REPLY 298
#define RATE_LIMIT 299
#define RATE_LIMIT_SIZE 300
#define RATE_LIMIT_SLIP 301
#define TRANSFERS 302
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

/* Line 2068 of yacc.c  */
#line 438 "cf-parse.y"

	struct {
		char *t;
		long i;
		size_t l;
		knot_tsig_algorithm_t alg;
	} tok;



/* Line 2068 of yacc.c  */
#line 187 "knot/conf/libknotd_la-cf-parse.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




