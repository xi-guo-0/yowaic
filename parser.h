// parser.h
// create abstract syntax tree
// Copyright (C) 2018: see LICENSE
#ifndef _YOWAIC_PARSER_H_
#define _YOWAIC_PARSER_H_
#include "util.h"
#include <stddef.h>

// kind of ast
enum {
  AST_ADDRESS = 257,
  AST_ARRAY_INIT,
  AST_COMPOUND,
  AST_DECLARATION,
  AST_DEREFERENCE,
  AST_FOR,
  AST_FUN_CALL,
  AST_FUN_DEFINE,
  AST_GID,
  AST_GREF,
  AST_IF,
  AST_LID,
  AST_LREF,
  AST_LITERAL,
  AST_RETURN,
  AST_STRING,
};

// kind of run time type
enum {
  RT_ARRAY,
  RT_CHAR,
  RT_INT,  
  RT_PTR,
  RT_VOID,  
};

typedef struct rt_t {
  int type;
  struct rt_t* ptr;
  int size;
} rt_t;

typedef struct Ast {
  int kind;
  rt_t* rt_type;

  union {
    // Char
    char cval;
    // Integer
    int ival;
    // String
    struct {
      char* sval;
      char* slabel;
    };
    // Local variable
    struct {
      char* lname;
      int loffset;
    };
    // Global variable
    struct {
      char* gname;
      char* glabel;
    };
    // Local reference
    struct {
      struct Ast* lref;
      int lref_offset;
    };
    // Global reference
    struct {
      struct Ast* gref;
      int gref_offset;
    };
    // Unary Operator
    struct {
      struct Ast *operand;
    };
    // Binary Operator
    struct {
      struct Ast *left;
      struct Ast *right;
    };
    // Function call or declaration
    struct {
      char *fun_name;
      struct {
        ywlist* args;
        struct {
          ywlist* params;
          ywlist* locals;
          struct Ast* body;
        };
      };
    };
    // Declaration
    struct {
      struct Ast* decl_var;
      struct Ast* decl_init;
    };
    // Array initializer
    ywlist* array_init;
    // If statement
    struct {
      struct Ast* s_cond;
      struct Ast* s_then;
      struct Ast* s_else;
    };
    // For statement
    struct {
      struct Ast* forinit;
      struct Ast* forcond;
      struct Ast* forstep;
      struct Ast* forbody;
    };
    // Return statement
    struct {
      struct Ast* ret;
    };
    // Compound statement
    ywlist* compound;
  };
} Ast;

extern rt_t* rt_type_char;
extern rt_t* rt_type_int;
extern rt_t* rt_type_void;
extern ywlist* globals;
extern ywlist* locals;

ywlist* parseFunList();

char* createNextLabel();
// print abstract syntax tree
char* astToS(Ast *ast);

#endif
