/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

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

#ifndef YY_CF_Y_TAB_H_INCLUDED
# define YY_CF_Y_TAB_H_INCLUDED
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
/* Line 2053 of yacc.c  */
#line 386 "knot/conf/cf-parse.y"

    struct {
       char *t;
       long i;
       size_t l;
       knot_tsig_algorithm_t alg;
    } tok;


/* Line 2053 of yacc.c  */
#line 185 "knot/conf/libknotd_la-cf-parse.h"
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

#endif /* !YY_CF_Y_TAB_H_INCLUDED  */
