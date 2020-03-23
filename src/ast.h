#ifndef _PERIDOT_AST_H
#define _PERIDOT_AST_H
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <stdbool.h>

typedef struct pd_ast_node pd_ast_node;

typedef enum {
  PD_AST_NUMBER, // numbers
  PD_AST_STRING, // "strings"
  PD_AST_CONDITIONAL, // if condition; code; end
  PD_AST_BOOLEAN, // true/false
  PD_AST_BIN_OP, // 5 + 2 etc
  PD_AST_CALL, // fn(args)
  PD_AST_FUNCTION, // function ident(args); code; end
  PD_AST_PROTOTYPE, // function's prototype such as name and arg names.
  PD_AST_VARIABLE, // ident
  PD_AST_FILE, // __FILE__
  PD_AST_ASSIGN, // x = 5, etc.
  PD_AST_BLOCK, // block end
  PD_AST_RETURN, // return expr
  PD_AST_UNARY, // -5
  PD_AST_NULL, // null
  PD_AST_TERNARY, // cond ? expr : expr
  PD_AST_WHILE, // while cond; body; end
  PD_AST_EMPTY, // Used when the input is empty, nothing to parse at all.
  PD_AST_PROPERTY, // object.property getter
  PD_AST_CLASS
} pd_ast_type;

// Represents a number.
typedef struct {
  double value;
} pd_ast_number;

// Represents a while loop.
typedef struct {
  pd_ast_node* condition;
  pd_ast_node* body;
} pd_ast_while;

// Represents a block of statements.
typedef struct {
  pd_ast_node** statements;
  int count;
} pd_ast_block;

// Represents a string.
typedef struct {
  char* value;
} pd_ast_string;

// Represents a conditional, the trueNode is the body of the truthy-case and likewise falseNode is for the falsy-case
// This node is also used for ternaries (x ? y : z) except that the node's type is PD_AST_TERNARY to differentiate them.
typedef struct {
  pd_ast_node* condition;
  pd_ast_node* trueNode;
  pd_ast_node* falseNode;
} pd_ast_conditional;

// Represents a boolean, true or false.
typedef struct {
  bool value;
} pd_ast_boolean;

// Represents a function/method call
// args is an array of argument expressions.
// argc is the length of arguments.
typedef struct {
  char* name;
  pd_ast_node** args;
  int argc;
} pd_ast_call;

// Represents the type of the binary expression.
typedef enum {
  PD_BIN_PLUS, // Add +
  PD_BIN_MINUS, // Subtract -
  PD_BIN_MUL, // Multiply *
  PD_BIN_DIV, // Divide /
  PD_BIN_GT, // Greater Than >
  PD_BIN_LT, // Less Than <
  PD_BIN_GE, // Greater Than or Equal >=
  PD_BIN_LE, // Less Than or Equal <=
  PD_BIN_EQ, // Equal ==
  PD_BIN_NEQ, // Not Equal !=
  PD_BIN_OR, // Logical OR ||
  PD_BIN_AND, // Logical AND &&
  PD_BIN_SHL, // Bitwise <<
  PD_BIN_SHR, // Bitwise >>
  PD_BIN_BAND, // Bitwise &
  PD_BIN_BOR, // Bitwise |
  PD_BIN_XOR, // Bitwise ^
} pd_binary_op_type;

// Represents the type of the unary expression.
typedef enum {
  PD_UNARY_MINUS, // -5
  PD_UNARY_NOT, // !condition
  PD_UNARY_BNOT, // Bitewise ~
} pd_unary_op_type;

// Represents a unary expression like -5
typedef struct {
  pd_unary_op_type type;
  // Unary operations only has a right hand side.
  pd_ast_node* rhs;
} pd_ast_unary_op;

// Represents a binary expression like 5 + 2
typedef struct {
  pd_binary_op_type op;
  pd_ast_node* lhs;
  pd_ast_node* rhs;
} pd_ast_binary_op;

// Represents the function's prototype, which is the name and arguments without the body.
typedef struct {
  char* name;
  char** args;
  int argc;
} pd_ast_prototype;

// Represents a function declaration.
typedef struct {
  pd_ast_node* prototype;
  pd_ast_node* body;
} pd_ast_function;

// Represents a variable access.
typedef struct {
  char* name;
} pd_ast_variable;

// Represents an assignment like x = 5
// Variables in Peridot are implicitly declared.
typedef struct {
  char* name;
  pd_ast_node* expr;
} pd_ast_assign;

// Represents a return statement like return 5
typedef struct {
  pd_ast_node* expr; // NULL if no return value specified.
} pd_ast_return;

// Represents a property getter.
// such as one.two
// expr is the initial expression like "one" in this example.
// ident is the final identifier like "two" in this example.
// expr is however recursive it could be representing a yet another ast_property
// e.g one.two.three = { expr: { expr: one, ident: two }, ident: three }
typedef struct {
  pd_ast_node* expr;
  pd_ast_node* ident;
} pd_ast_property;

typedef struct {
  char* name;
} pd_ast_class;

// Represents a node in the abstract syntax tree. (AST)
typedef struct pd_ast_node {
  pd_ast_type type; // Type of this node
  int line; // The line number of this node.
  union {
    pd_ast_number number;
    pd_ast_string string;
    pd_ast_conditional conditional;
    pd_ast_boolean boolean;
    pd_ast_binary_op binop;
    pd_ast_call call;
    pd_ast_prototype prototype;
    pd_ast_function function;
    pd_ast_variable variable;
    pd_ast_assign assign;
    pd_ast_block block;
    pd_ast_return ret;
    pd_ast_unary_op unary;
    pd_ast_while while_loop;
    pd_ast_property property;
    pd_ast_class klass;
  };
} pd_ast_node;

// Functions to create ast nodes.
pd_ast_node* pd_ast_number_create(int line, double value);
pd_ast_node* pd_ast_assign_create(int line, char* name, pd_ast_node* expr);
pd_ast_node* pd_ast_block_create(pd_ast_node* statement);
pd_ast_node* pd_ast_string_create(int line, char* value);
pd_ast_node* pd_ast_boolean_create(int line, bool value);
pd_ast_node* pd_ast_prototype_create(int line, char* name, char** args, int argc);
pd_ast_node* pd_ast_function_create(int line, pd_ast_node* prototype, pd_ast_node* body);
pd_ast_node* pd_ast_conditional_create(int line, pd_ast_node* condition, pd_ast_node* trueNode, pd_ast_node* falseNode);
pd_ast_node* pd_ast_variable_create(int line, char* name);
pd_ast_node* pd_ast_binary_op_create(int line, pd_binary_op_type op, pd_ast_node* lhs, pd_ast_node* rhs);
pd_ast_node* pd_ast_call_create(int line, char* name, pd_ast_node** args, int argc);
pd_ast_node* pd_ast_block_append(pd_ast_node* block, pd_ast_node* node);
pd_ast_node* pd_ast_return_create(int line, pd_ast_node* expr);
pd_ast_node* pd_ast_file_create(int line);
pd_ast_node* pd_ast_null_create(int line);
pd_ast_node* pd_ast_unary_op_create(int line, pd_unary_op_type type, pd_ast_node* rhs);
pd_ast_node* pd_ast_ternary_create(int line, pd_ast_node* cond, pd_ast_node* trueNode, pd_ast_node* falseNode);
pd_ast_node* pd_ast_while_create(int line, pd_ast_node* condition, pd_ast_node* body);
pd_ast_node* pd_ast_property_create(int line, pd_ast_node* expr, pd_ast_node* ident);
pd_ast_node* pd_ast_empty_create(void);
pd_ast_node* pd_ast_class_create(int line, char* name);

// returns true if the node is an expression statement.
bool pd_ast_is_expr(pd_ast_node* node);

// Dumps the parsed node for inspecting.
void pd_ast_node_dump(pd_ast_node node);
// Frees the memory allocated by a node.
void pd_ast_node_free(pd_ast_node* node);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PERIDOT_AST_H
