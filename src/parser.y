
%{

#if !YYPURE
#error Needs Pure Parser
#endif // !YYPURE
#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#include <assert.h>
#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"
%}

%code requires {
#include "ast.h"
}
%code provides {
int yyerror(YYLTYPE* yylloc, void* scanner, pd_ast_node* ast, const char* msg);
}

/* Output target */
%output "parser.c"
%defines "parser.h"
/* We expect no Shift/Reduce conflicts */
%expect 0
/* Tell bison we want a Reentrant/Thread-Safe parser */
%define api.pure
/* Debugging and verbose output file. */
%debug
%verbose
/* Params for the lexer/parser */
%lex-param {void* scanner}
%parse-param {void* scanner}
/* This argument is a pointer to write out the final ast to. */
%parse-param {pd_ast_node* ast}
%locations

%union {
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
}

/* Literals */
%token <str> tIDENT tSTRING
%token <num> tNUMBER
%token tTRUE tFALSE tNULL
/* Operators */
%token tGT tGE tLT tLE tPLUS tMINUS tSLASH tSTAR tEQ tEQEQ tNOT tNEQ tAND tOR tQU tSHR tSHL tBOR tBAND tBNOT tXOR
%token tCOLON tDOT
/* Keywords */
%token tFUNCTION tWHILE tEND tFILE tMACRO tRETURN tIF tELSE tDO tCLASS tSTATIC tIMPORT
/* Symbols */
%token tLPAREN tRPAREN tLBRACE tRBRACE tCOMMA tSEMI

/* Operator precedence */
%precedence tIDENT
/*%precedence tLPAREN*/
%right tEQ
%left tQU tCOLON
%left tGT tGE tLT tLE tBOR tBAND tSHR tSHL tEQEQ tNEQ tXOR tAND tOR
%left tPLUS tMINUS
%left tSTAR tSLASH
%precedence UNARY

/* Types of each non-terminal */
%type <node> expr number ternary stmt stmts func proto ident bool string return_expr import_stmt class_stmt
%type <node> cond call file assign do_block while_loop
%type <fnargs> fnargs args

/* Start symbol */
%start program

%%

program: /* empty */ { *ast = *pd_ast_empty_create(); }
       |
       stmts { *ast = *$1; }
       ;

stmts:
     stmt tSEMI { $$ = pd_ast_block_create($1); }
     |
     stmts stmt tSEMI { $$ = pd_ast_block_append($1, $2); }
     ;

stmt:
    expr
    |
    func
    |
    return_expr
    |
    cond
    |
    do_block
    |
    while_loop
    |
    import_stmt
    |
    class_stmt
    ;

import_stmt: tIMPORT tIDENT { $$ = pd_ast_empty_create(); } /* TODO */
           ;

class_method: proto tSEMI stmts tEND /* TODO */
            |
            proto tSEMI tEND
            ;

class_body: class_method tSEMI /* TODO */
          |
          class_body class_method tSEMI /* TODO */
          ;

class_stmt: tCLASS tIDENT tSEMI /* empty */ tEND { $$ = pd_ast_class_create(@1.first_line, $2); free($2); }
          |
          tCLASS tIDENT tSEMI class_body tEND { $$ = pd_ast_empty_create(); } /* TODO */
          ;

do_block: tDO stmts tEND { $$ = $2; }
        ;

while_loop:
          tWHILE expr tSEMI stmts tEND { $$ = pd_ast_while_create(@1.first_line, $2, $4); }
          |
          tWHILE expr tSEMI tEND { $$ = pd_ast_while_create(@1.first_line, $2, NULL); }
          ;

func:
    tFUNCTION proto tSEMI stmts tEND { $$ = pd_ast_function_create(@1.first_line, $2, $4); }
    |
    tFUNCTION proto tSEMI tEND { $$ = pd_ast_function_create(@1.first_line, $2, NULL); }
    ;

proto:
     tIDENT tLPAREN fnargs tRPAREN {
       $$ = pd_ast_prototype_create(@1.first_line, $1, $3.args, $3.count);
       free($1);
       for(int x = 0; x < $3.count; x++) free($3.args[x]);
     }
     ;

fnargs: /* empty */ { $$.count = 0; $$.args = NULL; }
      |
      tIDENT { $$.count = 1; $$.args = malloc(sizeof(char*)); $$.args[0] = strdup($1); }
      | fnargs tCOMMA tIDENT { $1.count++; $1.args = realloc($1.args, sizeof(char*) * $1.count); $1.args[$1.count-1] = strdup($3); $$ = $1; }
      ;

args: /* empty */ { $$.count = 0; $$.call = NULL; }
    |
    expr { $$.count = 1; $$.call = malloc(sizeof(pd_ast_node*)); $$.call[0] = $1; }
    |
    args tCOMMA expr { $1.count++; $1.call = realloc($1.call, sizeof(pd_ast_node*) * $1.count); $1.call[$1.count - 1] = $3; $$ = $1; }
    ;

number: tNUMBER { $$ = pd_ast_number_create(@1.first_line, $1); }
      ;

bool: tTRUE { $$ = pd_ast_boolean_create(@1.first_line, true); }
    | tFALSE { $$ = pd_ast_boolean_create(@1.first_line, false); }
    ;

return_expr:
           tRETURN { $$ = pd_ast_return_create(@1.first_line, NULL); }
           |
           tRETURN expr { $$ = pd_ast_return_create(@1.first_line, $2); }
           ;

string: tSTRING { $$ = pd_ast_string_create(@1.first_line, $1); }
      ;

ternary:
       expr tQU expr tCOLON expr { $$ = pd_ast_ternary_create(@1.first_line, $1, $3, $5); }
       ;

ident: tIDENT { $$ = pd_ast_variable_create(@1.first_line, $1); }
     |
     file
     ;

file: tFILE { $$ = pd_ast_file_create(@1.first_line); }

call:
    tIDENT tLPAREN args tRPAREN { $$ = pd_ast_call_create(@1.first_line, $1, $3.call, $3.count); free($1); free($3.call); }
    ;

cond:
    tIF expr tSEMI stmts tEND { $$ = pd_ast_conditional_create(@1.first_line, $2, $4, NULL); }
    |
    tIF expr tSEMI stmts tELSE stmts tEND { $$ = pd_ast_conditional_create(@1.first_line, $2, $4, $6); }
    ;

assign: tIDENT tEQ expr { $$ = pd_ast_assign_create(@1.first_line, $1, $3); }
      ;

expr:
    number
    |
    ternary
    |
    bool
    |
    tNULL { $$ = pd_ast_null_create(@1.first_line); }
    |
    assign
    |
    string
    |
    ident %prec tIDENT
    |
    call
    |
    tLPAREN expr tRPAREN { $$ = $2; }
    |
    expr tPLUS expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_PLUS, $1, $3); }
    |
    expr tMINUS expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_MINUS, $1, $3); }
    |
    expr tSLASH expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_DIV, $1, $3); }
    |
    expr tSTAR expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_MUL, $1, $3); }
    |
    expr tGT expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_GT, $1, $3); }
    |
    expr tGE expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_GE, $1, $3); }
    |
    expr tLT expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_LT, $1, $3); }
    |
    expr tLE expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_LE, $1, $3); }
    |
    expr tSHR expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_SHR, $1, $3); }
    |
    expr tSHL expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_SHL, $1, $3); }
    |
    expr tBOR expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_BOR, $1, $3); }
    |
    expr tBAND expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_BAND, $1, $3); }
    |
    expr tEQEQ expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_EQ, $1, $3); }
    |
    expr tXOR expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_XOR, $1, $3); }
    |
    expr tNEQ expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_NEQ, $1, $3); }
    |
    expr tAND expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_AND, $1, $3); }
    |
    expr tOR expr { $$ = pd_ast_binary_op_create(@1.first_line, PD_BIN_OR, $1, $3); }
    |
    tBNOT expr %prec UNARY { $$ = pd_ast_unary_op_create(@1.first_line, PD_UNARY_BNOT, $2); }
    |
    tMINUS expr %prec UNARY { $$ = pd_ast_unary_op_create(@1.first_line, PD_UNARY_MINUS, $2); /* pd_ast_binary_op_create(@1.first_line, PD_BIN_MINUS, pd_ast_number_create(@1.first_line, 0), $2); */ }
    |
    tNOT expr %prec UNARY { $$ = pd_ast_unary_op_create(@1.first_line, PD_UNARY_NOT, $2); }
    ;

%%

// TODO improve error handling, find out how to point the locations etc.
int yyerror(YYLTYPE* yylloc, void* scanner, pd_ast_node* ast, const char* msg) {
  // Supress warnings
  (void)ast;
  (void)scanner;

  // fprintf(stderr, "ParseError: (line: %d) %s\n", yyget_lineno(scanner), msg);
  // fprintf(stderr, "ParseError: (line %d) %s\n", yylloc->first_line, msg);
  fprintf(stderr, "[line %d] Error: %s\n", yylloc->first_line, msg);
  return 0;
}
