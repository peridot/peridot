
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

// Return a copy of str with first and last characters removed.
// Used to remove the quotes for strings.
char* getString(char *str, size_t len) {
  char* copy = strdup(str);
  memmove(copy, copy + 1, len - 2);
  copy[len - 2] = 0;
  return copy;
}

int shouldInsertSemicolon(yyscan_t scanner);

#define RETURN(token) do_yylloc; yyextra->last = token; return (token)

#define do_yylloc \
  do { \
    yylloc->first_column = yylloc->last_column + 1; \
    yylloc->last_column += strlen(yytext); \
  } while(0)

#define do_yylloc_nl \
  do { \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = 1; \
    yylloc->last_column = 0; \
  } while (0)

%}

%option outfile="lexer.c" header-file="lexer.h"
%option reentrant yylineno
%option warn nodefault
%option noyywrap
%option bison-bridge
%option bison-locations
%option nounput
%option noinput

IDENT [A-Za-z_][A-Za-z_0-9]*
WS [ \t\r]
NUM 0+|[1-9][0-9]*("."[0-9]*)?
OCTAL 0+[1-9]+
HEX 0x[0-9A-Fa-f]+
BIN 0b[01]+
STRING \"[^\"]*\"
STRING_SINGLE \'[^\']*\'

%x COMMENTS

%top {
typedef struct {
  int last; // last stored token, used for implicit semicolon insertion.
  int parens; // depth of parens used to verify that we can actually insert a semicolon.
} pd_lexer_state;
}

%option extra-type="pd_lexer_state*"

%%

{WS} ;
";" { RETURN(tSEMI); }
\n { do_yylloc_nl; if(shouldInsertSemicolon(yyg)) { /* printf("Implicitly inserted a semicolon at line %d:%d previous token was %d\n", yylineno, yycolumn, yyextra->last); */ yyextra->last = tSEMI; return tSEMI; } }
{STRING} |
{STRING_SINGLE} { yylval->str = getString(yytext, yyleng); RETURN(tSTRING); }
"#"[^\n\r]* ;
"/*" { BEGIN(COMMENTS); }
<COMMENTS>{WS} ;
<COMMENTS>\n ;
<COMMENTS>"*/" { BEGIN(INITIAL); }
<COMMENTS>. ;
<COMMENTS><<EOF>> { yyerror(yylloc, yyg, NULL, "Unterminated multiline comment."); yyterminate(); }
{NUM} { yylval->num = atof(yytext); RETURN(tNUMBER); }
{HEX} { yylval->num = (double)strtol(yytext + 2 /* Skip 0x */, NULL, 16); RETURN(tNUMBER); }
{BIN} { yylval->num = (double)strtol(yytext + 2 /* Skip 0b */, NULL, 2); RETURN(tNUMBER); }
{OCTAL} { yylval->num = (double)strtol(yytext /* No need for skip, 0s are harmless */, NULL, 8); RETURN(tNUMBER); }
"+" { RETURN(tPLUS); }
"/" { RETURN(tSLASH); }
"*" { RETURN(tSTAR); }
"-" { RETURN(tMINUS); }
"function" { RETURN(tFUNCTION); }
"while" { RETURN(tWHILE); }
"import" { RETURN(tIMPORT); }
"(" { yyextra->parens++; RETURN(tLPAREN); }
")" { yyextra->parens--; RETURN(tRPAREN); }
"{" { RETURN(tLBRACE); }
"}" { RETURN(tRBRACE); }
"==" { RETURN(tEQEQ); }
"=" { RETURN(tEQ); }
"!=" { RETURN(tNEQ); }
"!" { RETURN(tNOT); }
"||" { RETURN(tOR); }
"&&" { RETURN(tAND); }
"|" { RETURN(tBOR); }
"&" { RETURN(tBAND); }
">=" { RETURN(tGE); }
"<=" { RETURN(tLE); }
">>" { RETURN(tSHR); }
"<<" { RETURN(tSHL); }
">" { RETURN(tGT); }
"<" { RETURN(tLT); }
"~" { RETURN(tBNOT); }
"^" { RETURN(tXOR); }
"do" { RETURN(tDO); }
"true" { RETURN(tTRUE); }
"false" { RETURN(tFALSE); }
"null" { RETURN(tNULL); }
"macro" { RETURN(tMACRO); }
"return" { RETURN(tRETURN); }
"class" { RETURN(tCLASS); }
"static" { RETURN(tSTATIC); }
"end" { RETURN(tEND); }
"." { RETURN(tDOT); }
":" { RETURN(tCOLON); }
"?" { RETURN(tQU); }
"," { RETURN(tCOMMA); }
"if" { RETURN(tIF); }
"else" { RETURN(tELSE); }
"__FILE__" { RETURN(tFILE); }
{IDENT} { yylval->str = strdup(yytext); RETURN(tIDENT); }
. { printf("SyntaxError: Unexpected Token '%s' at line %d\n", yytext, yylineno); yyterminate(); }

%%

// Peridot uses semicolons in the grammar but doesn't require the user to use it.
// To do that we implicitly insert a semicolon when we think it's safe to do so.
int shouldInsertSemicolon(yyscan_t scanner) {
  pd_lexer_state* state = yyget_extra(scanner);
  int last = state->last;
  // if the last token is already a semicolon then don't insert another.
  if(last == tSEMI) return 0;
  // Make sure all parenthesises are closed before inserting a semicolon
  // This allows users to use newlines safely in between parenthesis.
  // e.g the common practice of using a newline when a function call becomes too long
  // some_very_long_function_name(some_very_long_argument,
  //   more_arguments_follow_in_a_newline())
  // We don't want to terminate that statement on it's first line!
  if(state->parens != 0) return 0;
  // List of tokens that we can safely insert a semicolon.
  return last == tIDENT || last == tNUMBER ||
last == tSTRING || last == tEND || last == tFILE || last == tTRUE || last == tFALSE || last == tRETURN || last == tRPAREN || last == tNULL;
}
