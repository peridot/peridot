/* A Bison parser, made by GNU Bison 3.4.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 16 "parser.y"

#include "ast.h"

#line 52 "parser.h"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    tIDENT = 258,
    tSTRING = 259,
    tNUMBER = 260,
    tTRUE = 261,
    tFALSE = 262,
    tNULL = 263,
    tGT = 264,
    tGE = 265,
    tLT = 266,
    tLE = 267,
    tPLUS = 268,
    tMINUS = 269,
    tSLASH = 270,
    tSTAR = 271,
    tEQ = 272,
    tEQEQ = 273,
    tNOT = 274,
    tNEQ = 275,
    tAND = 276,
    tOR = 277,
    tQU = 278,
    tSHR = 279,
    tSHL = 280,
    tBOR = 281,
    tBAND = 282,
    tBNOT = 283,
    tXOR = 284,
    tCOLON = 285,
    tDOT = 286,
    tFUNCTION = 287,
    tWHILE = 288,
    tEND = 289,
    tFILE = 290,
    tMACRO = 291,
    tRETURN = 292,
    tIF = 293,
    tELSE = 294,
    tDO = 295,
    tCLASS = 296,
    tSTATIC = 297,
    tIMPORT = 298,
    tLPAREN = 299,
    tRPAREN = 300,
    tLBRACE = 301,
    tRBRACE = 302,
    tCOMMA = 303,
    tSEMI = 304,
    UNARY = 305
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 40 "parser.y"

  char* str;
  double num;
  pd_ast_node* node;
  struct {
    int count;
    union {
      char** args;
      pd_ast_node** call;
    };
  } fnargs;

#line 127 "parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void* scanner, pd_ast_node* ast);
/* "%code provides" blocks.  */
#line 19 "parser.y"

int yyerror(YYLTYPE* yylloc, void* scanner, pd_ast_node* ast, const char* msg);

#line 157 "parser.h"

#endif /* !YY_YY_PARSER_H_INCLUDED  */
