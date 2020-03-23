// Main entry for peridot, this file implements the command line interface and repl

#include <stdio.h>
#include <uv.h>
#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "pvm.h"
#include "compiler.h"
#include "debug.h"
#include "opcodes.h"

// Returns int so we can do return repl(); in main()
int repl() {
  pvm_t* vm = pvm_new();
  printf("                 _     _       _   \n _ __   ___ _ __(_) __| | ___ | |_ \n| '_ \\ / _ \\ '__| |/ _` |/ _ \\| __|\n| |_) |  __/ |  | | (_| | (_) | |_ \n| .__/ \\___|_|  |_|\\__,_|\\___/ \\__|\n|_|                                \n");
  printf("Enter an expression to evaluate. Press CTRL + C to exit.\n");
  for(;;) {
    printf("> ");
    char *input = NULL;
    size_t len = 0;
    if(getline(&input, &len, stdin) == -1) {
      fprintf(stderr, "Error reading from stdin.\n");
      break;
    }
    pd_lexer_state state;
    state.last = -1;
    state.parens = 0;
    yyscan_t scanner;
    if(yylex_init(&scanner)) {
      printf("Failed to initialize scanner.\n");
      return 1;
    }
    yyset_extra(&state, scanner);
    /* YY_BUFFER_STATE buffer = */ yy_scan_string(input, scanner);
    pd_ast_node* node = malloc(sizeof(pd_ast_node));
    int status = yyparse(scanner, node);
    // Parse errors gets printed already.
    if(status != 0 || node == NULL) continue;
    pd_code_ctx ctx;
    pd_compile_ctx_init(&ctx, vm, PD_TYPE_SCRIPT);
    pd_compile(&ctx, node);
    // Compile errors gets printed already.
    if(ctx.errors > 0) continue;
    free(node);
    pvm_exec(vm, pd_compile_ctx_end(&ctx));
  }
  return 0;
}

int main(int argc, char* argv[]) {
  // TODO make this flags to be cli flags.
  // Right now we use getenv as a temporary hack, will need to write a portable argument parser.
  yydebug = getenv("YYDEBUG") ? 1 : 0; // Debug parser state
  int dump_ast = getenv("DUMP_AST") ? 1 : 0; // Dump abstract syntax tree
  int dump_bc = getenv("DUMP_BC") ? 1 : 0; // Dump final bytecode.
  (void)dump_bc;
  // Debugging GC and VM will need to be compiled with the proper macro defined, because they add more runtime overhead
  // since it has to keep checking a condition to whether output debug info, macros are checked at compile time only.
  
  if(argc < 2) return repl();

  // TODO such initializations should be wrapped up in a function to ease usage.
  yyscan_t scanner;
  if(yylex_init(&scanner)) {
    printf("Failed to initialize scanner.\n");
    return 1;
  }

  FILE* f = fopen(argv[1], "r");
  if(f == NULL) {
    printf("Failed to open '%s'\n", argv[1]);
    return 1;
  }

  // Lex & Parse
  PD_TIMER_START;
  pd_lexer_state* state = malloc(sizeof(pd_lexer_state));
  state->last = -1;
  state->parens = 0;
  yyset_extra(state, scanner);
  yyset_in(f, scanner);
  pd_ast_node* node = malloc(sizeof(pd_ast_node));

  yyparse(scanner, node);
  PD_TIMER_STOP;
  PD_TIMER_REPORT("Lexing & Parsing");
  if(node == NULL) return printf("Failed to parse.\n");

  // Dump Tree for debugging
  if(dump_ast) pd_ast_node_dump(*node);

  // Compile
  pd_code_ctx ctx;
  pvm_t* vm = pvm_new();
  pd_compile_ctx_init(&ctx, vm, PD_TYPE_SCRIPT);
  pd_compile(&ctx, node);
  if(ctx.errors > 0) {
    printf("Found %d compilation errors, aborting.\n", ctx.errors);
    pd_ast_node_free(node);
    free(state);
    yylex_destroy(scanner);
    fclose(f);
    pvm_free(ctx.vm);
    return 1;
  }
  pvm_exec(vm, pd_compile_ctx_end(&ctx));
  uv_run(vm->loop, UV_RUN_DEFAULT);
  // Cleanup
  //pd_compile_ctx_end(&ctx);
  pvm_free(vm);
  pd_ast_node_free(node);
  free(state);
  fclose(f);
  yylex_destroy(scanner);
  //pvm_free(ctx.vm);
  uv_tty_reset_mode();
  return 0;
}
