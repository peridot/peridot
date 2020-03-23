/**
 * Abstract Syntax Tree for Peridot
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

bool pd_ast_is_expr(pd_ast_node* node) {
  pd_ast_type t = node->type; // make it easier to type.

  // NOTE: update this everytime you add a new expression in ast types.
  return t == PD_AST_UNARY || t == PD_AST_CALL || t == PD_AST_BOOLEAN || t == PD_AST_STRING ||
    t == PD_AST_NUMBER || t == PD_AST_ASSIGN || t == PD_AST_BIN_OP || t == PD_AST_FILE || t == PD_AST_NULL ||
    t == PD_AST_VARIABLE || t == PD_AST_TERNARY;
}

pd_ast_node* pd_ast_empty_create(void) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_EMPTY;
  node->line = 0;
  return node;
}

pd_ast_node* pd_ast_while_create(int line, pd_ast_node* condition, pd_ast_node* body) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_WHILE;
  node->line = line;
  node->while_loop.condition = condition;
  node->while_loop.body = body;
  return node;
}

pd_ast_node* pd_ast_string_create(int line, char* value) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_STRING;
  node->line = line;
  node->string.value = strdup(value);
  return node;
}

pd_ast_node* pd_ast_number_create(int line, double value) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_NUMBER;
  node->line = line;
  node->number.value = value;
  return node;
}

pd_ast_node* pd_ast_assign_create(int line, char* name, pd_ast_node* expr) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_ASSIGN;
  node->line = line;
  node->assign.name = strdup(name);
  node->assign.expr = expr;
  return node;
}

pd_ast_node* pd_ast_unary_op_create(int line, pd_unary_op_type type, pd_ast_node* rhs) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_UNARY;
  node->line = line;
  node->unary.type = type;
  node->unary.rhs = rhs;
  return node;
}

pd_ast_node* pd_ast_block_create(pd_ast_node* statement) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_BLOCK;
  node->line = 0;
  node->block.statements = malloc(sizeof(pd_ast_node*));
  // Empty Block.
  if(statement == NULL) {
    node->block.count = 0;
    return node;
  }
  node->block.statements[0] = statement;
  node->block.count = 1;
  return node;
}

// __FILE__ ident resolves to the current executing file.
pd_ast_node* pd_ast_file_create(int line) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_FILE;
  node->line = line;
  return node;
}

pd_ast_node* pd_ast_null_create(int line) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_NULL;
  node->line = line;
  return node;
}

pd_ast_node* pd_ast_conditional_create(int line, pd_ast_node* condition, pd_ast_node* trueNode, pd_ast_node* falseNode) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_CONDITIONAL;
  node->line = line;
  node->conditional.condition = condition;
  node->conditional.trueNode = trueNode;
  node->conditional.falseNode = falseNode;
  return node;
}

pd_ast_node* pd_ast_ternary_create(int line, pd_ast_node* cond, pd_ast_node* trueNode, pd_ast_node* falseNode) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_TERNARY;
  node->line = line;
  // Ternary has the same concept of if-statements except that the nodes are expressions instead of blocks
  // but we can still reuse the same node for ternaries. (node->conditional instead of a new node->ternary)
  node->conditional.condition = cond;
  node->conditional.trueNode = trueNode;
  node->conditional.falseNode = falseNode;
  return node;
}

pd_ast_node* pd_ast_boolean_create(int line, bool value) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_BOOLEAN;
  node->line = line;
  node->boolean.value = value;
  return node;
}

pd_ast_node* pd_ast_binary_op_create(int line, pd_binary_op_type op, pd_ast_node* lhs, pd_ast_node* rhs) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_BIN_OP;
  node->line = line;
  node->binop.op = op;
  node->binop.lhs = lhs;
  node->binop.rhs = rhs;
  return node;
}

pd_ast_node* pd_ast_call_create(int line, char* name, pd_ast_node** args, int argc) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_CALL;
  node->line = line;
  node->call.name = strdup(name);
  node->call.args = malloc(sizeof(pd_ast_node*) * argc);
  memcpy(node->call.args, args, sizeof(pd_ast_node*) * argc);
  node->call.argc = argc;
  return node;
}

pd_ast_node* pd_ast_prototype_create(int line, char* name, char** args, int argc) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_PROTOTYPE;
  node->line = line;
  node->prototype.name = strdup(name);
  node->prototype.args = malloc(sizeof(char*) * argc);
  for(int x = 0; x < argc; x++)
    node->prototype.args[x] = strdup(args[x]);
  node->prototype.argc = argc;
  return node;
}

pd_ast_node* pd_ast_block_append(pd_ast_node* block, pd_ast_node* node) {
  block->block.count++;
  block->block.statements = realloc(block->block.statements, sizeof(pd_ast_node*) * block->block.count);
  block->block.statements[block->block.count - 1] = node;
  return block;
}

pd_ast_node* pd_ast_function_create(int line, pd_ast_node* prototype, pd_ast_node* body) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_FUNCTION;
  node->line = line;
  node->function.prototype = prototype;
  node->function.body = body;
  return node;
}

pd_ast_node* pd_ast_variable_create(int line, char* name) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_VARIABLE;
  node->line = line;
  node->variable.name = strdup(name);
  return node;
}

pd_ast_node* pd_ast_return_create(int line, pd_ast_node* expr) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_RETURN;
  node->line = line;
  node->ret.expr = expr;
  return node;
}

pd_ast_node* pd_ast_class_create(int line, char* name) {
  pd_ast_node* node = malloc(sizeof(pd_ast_node));
  node->type = PD_AST_CLASS;
  node->line = line;
  node->klass.name = strdup(name);
  return node;
}

void pd_ast_node_free(pd_ast_node* node) {
#define FREE(pointer) if(pointer != NULL) free(pointer)
  if(node == NULL) return;
  switch(node->type) {
    case PD_AST_CLASS:
      FREE(node->klass.name);
      break;
    case PD_AST_PROPERTY:
      pd_ast_node_free(node->property.expr);
      // TODO: node->ident (after turning it to string.)
      break;
    case PD_AST_EMPTY:
      break;
    case PD_AST_FILE:
    case PD_AST_NUMBER:
    case PD_AST_BOOLEAN:
    case PD_AST_NULL:
      break; // doesn't need to be freed.
    case PD_AST_VARIABLE:
      FREE(node->variable.name);
      break;
    case PD_AST_WHILE:
      pd_ast_node_free(node->while_loop.condition);
      pd_ast_node_free(node->while_loop.body);
      break;
    case PD_AST_UNARY:
      pd_ast_node_free(node->unary.rhs);
      break;
    case PD_AST_CONDITIONAL:
    case PD_AST_TERNARY:
      pd_ast_node_free(node->conditional.trueNode);
      pd_ast_node_free(node->conditional.falseNode);
      pd_ast_node_free(node->conditional.condition);
      break;
    case PD_AST_STRING:
      FREE(node->string.value);
      break;
    case PD_AST_BIN_OP:
      pd_ast_node_free(node->binop.lhs);
      pd_ast_node_free(node->binop.rhs);
      break;
    case PD_AST_CALL:
      FREE(node->call.name);
      for(int x = 0; x < node->call.argc; x++)
        pd_ast_node_free(node->call.args[x]);
      FREE(node->call.args);
      break;
    case PD_AST_FUNCTION:
      pd_ast_node_free(node->function.body);
      pd_ast_node_free(node->function.prototype);
      break;
    case PD_AST_PROTOTYPE:
      FREE(node->prototype.name);
      for(int x = 0; x < node->prototype.argc; x++)
        FREE(node->prototype.args[x]);
      FREE(node->prototype.args);
      break;
    case PD_AST_BLOCK:
      for(int x = 0; x < node->block.count; x++)
        pd_ast_node_free(node->block.statements[x]);
      //FREE(node->block.statements);
      break;
    case PD_AST_ASSIGN:
      FREE(node->assign.name);
      pd_ast_node_free(node->assign.expr);
      break;
    case PD_AST_RETURN:
      pd_ast_node_free(node->ret.expr);
      break;
  }
#undef FREE
  free(node);
}


static void printIndents(int count) {
  for(int x = 0; x < count; x++) printf(" ");
}

// Recursively dump all nodes in xml
// TODO: cleanup this indention mess, this function is becoming unmaintainable find a different workaround.
static void _pd_ast_node_dump(pd_ast_node node, int indent) {
  switch(node.type) {
    case PD_AST_PROPERTY:
      break; // TODO
    case PD_AST_CLASS:
      break; // TODO
    case PD_AST_EMPTY:
      break;
    case PD_AST_WHILE:
      break; // TODO: implement this.
    case PD_AST_RETURN:
      printIndents(indent);
      printf("<return>\n");
      if(node.ret.expr)
        _pd_ast_node_dump(*node.ret.expr, indent + 2);
      else {
        printIndents(indent);
        printf("  <void/>\n");
      }
      printIndents(indent);
      printf("</return>\n");
    case PD_AST_BLOCK:
      for(int x = 0; x < node.block.count; x++)
        _pd_ast_node_dump(*node.block.statements[x], indent);
      break;
    case PD_AST_ASSIGN:
      printIndents(indent);
      printf("<assignment name=\"%s\">\n", node.assign.name);
      _pd_ast_node_dump(*node.assign.expr, indent + 2);
      printIndents(indent);
      printf("</assignment>\n");
      break;
    case PD_AST_FILE:
      printIndents(indent);
      printf("<file/>\n");
      break;
    case PD_AST_NULL:
      printIndents(indent);
      printf("<null/>\n");
      break;
    case PD_AST_VARIABLE:
      printIndents(indent);
      printf("<variable name=\"%s\"/>\n", node.variable.name);
      break;
    case PD_AST_UNARY:
      printIndents(indent);
      printf("<unary>\n");
      printIndents(indent);
      switch(node.unary.type) {
        case PD_UNARY_MINUS:
          printf("  <minus/>\n");
          break;
        case PD_UNARY_NOT:
          printf("  <not/>\n");
          break;
        case PD_UNARY_BNOT:
          printf("  <bnot/>\n");
          break;
      }
      printIndents(indent);
      printf("  <rhs>\n");
      _pd_ast_node_dump(*node.unary.rhs, indent + 4);
      printIndents(indent);
      printf("  </rhs>\n");
      printIndents(indent);
      printf("</unary>\n");
      break;
    case PD_AST_BIN_OP:
      printIndents(indent);
      printf("<binary>\n");
      printIndents(indent);
      printf("  <lhs>\n");
      //printIndents(indent == 0 ? 2 : indent);
      _pd_ast_node_dump(*node.binop.lhs, indent + 4);
      printIndents(indent);
      printf("  </lhs>\n");
      printIndents(indent);
      switch(node.binop.op) {
        case PD_BIN_XOR:
          printf("  <xor/>\n");
          break;
        case PD_BIN_EQ:
          printf("  <eq/>\n");
          break;
        case PD_BIN_NEQ:
          printf("  <neq/>\n");
          break;
        case PD_BIN_PLUS:
          printf("  <plus/>\n");
          break;
        case PD_BIN_MINUS:
          printf("  <minus/>\n");
          break;
        case PD_BIN_SHR:
          printf("  <shr/>\n");
          break;
        case PD_BIN_SHL:
          printf("  <shl/>\n");
          break;
        case PD_BIN_BOR:
          printf("  <bor/>\n");
          break;
        case PD_BIN_BAND:
          printf("  <band/>\n");
          break;
        case PD_BIN_MUL:
          printf("  <mul/>\n");
          break;
        case PD_BIN_DIV:
          printf("  <div/>\n");
          break;
        case PD_BIN_GT:
          printf("  <gt/>\n");
          break;
        case PD_BIN_GE:
          printf("  <gte/>\n");
          break;
        case PD_BIN_LT:
          printf("  <lt/>\n");
          break;
        case PD_BIN_LE:
          printf("  <lte/>\n");
          break;
        case PD_BIN_OR:
          printf("  <or/>\n");
          break;
        case PD_BIN_AND:
          printf("  <and/>\n");
          break;
      }
      printIndents(indent);
      printf("  <rhs>\n");
      //printIndents(indent == 0 ? 2 : indent);
      _pd_ast_node_dump(*node.binop.rhs, indent + 4);
      printIndents(indent);
      printf("  </rhs>\n");
      printIndents(indent);
      printf("</binary>\n");
      break;
    case PD_AST_BOOLEAN:
      printIndents(indent);
      printf("<boolean value=\"%s\"/>\n", node.boolean.value ? "true" : "false");
      break;
    case PD_AST_CONDITIONAL:
    case PD_AST_TERNARY:
      printIndents(indent);
      printf("<conditional>\n");
      printIndents(indent);
      printf("  <condition>\n");
      //printIndents(indent == 0 ? 2 : indent);
      _pd_ast_node_dump(*node.conditional.condition, indent + 4);
      printIndents(indent);
      printf("  </condition>\n");
      printIndents(indent);
      printf("  <trueNode>\n");
      _pd_ast_node_dump(*node.conditional.trueNode, indent + 4);
      printIndents(indent);
      printf("  </trueNode>\n");
      if(node.conditional.falseNode != NULL) {
        printIndents(indent);
        printf("  <falseNode>\n");
        _pd_ast_node_dump(*node.conditional.falseNode, indent + 4);
        printIndents(indent);
        printf("  </falseNode>\n");
      }
      printIndents(indent);
      printf("</conditional>\n");
      break;
    case PD_AST_NUMBER:
      printIndents(indent);
      printf("<number value=\"%g\"/>\n", node.number.value);
      break;
    case PD_AST_STRING:
      printIndents(indent);
      printf("<string value=\"%s\"/>\n", node.string.value);
      break;
    case PD_AST_CALL:
      printIndents(indent);
      printf("<call>\n");
      printIndents(indent);
      printf("  <name value=\"%s\">\n", node.call.name);
      printIndents(indent);
      printf("  <args>\n");
      for(int x = 0; x < node.call.argc; x++) {
        //printIndents(indent == 0 ? 2 : indent);
        _pd_ast_node_dump(*node.call.args[x], indent + 4);
      }
      printIndents(indent);
      printf("  </args>\n");
      printIndents(indent);
      printf("</call>\n");
      break;
    case PD_AST_FUNCTION:
      printIndents(indent);
      printf("<function>\n");
      _pd_ast_node_dump(*node.function.prototype, indent + 2);
      printIndents(indent);
      printf("  <body>\n");
      if(node.function.body != NULL)
        _pd_ast_node_dump(*node.function.body, indent + 4);
      printIndents(indent);
      printf("  </body>\n");
      printIndents(indent);
      printf("</function>\n");
      break;
    case PD_AST_PROTOTYPE:
      printIndents(indent);
      printf("<prototype argc=\"%d\" name=\"%s\">\n", node.prototype.argc, node.prototype.name);
      for(int x = 0; x < node.prototype.argc; x++) {
        printIndents(indent);
        printf("  <arg>%s</arg>\n", node.prototype.args[x]);
      }
      printIndents(indent);
      printf("</prototype>\n");
      break;
  }
}

inline void pd_ast_node_dump(pd_ast_node node) {
  return _pd_ast_node_dump(node, 0);
}


