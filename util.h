// util.h
// commonly used utility functions
// Copyright (C) 2018: see LICENSE
#ifndef _YOWAIC_UTIL_H_
#define _YOWAIC_UTIL_H_
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

/**
 * Enum of token
 * single operator's enum is there ascii code
 * all operator's enum is little than TK_EOF
 * all common type's enum is great than TK_IDENTIFIER
 */
enum {
  TK_ELLIPSIS=257,
  TK_RIGHT_ASSIGN,
  TK_LEFT_ASSIGN,
  TK_ADD_ASSIGN,
  TK_SUB_ASSIGN,
  TK_MUL_ASSIGN,
  TK_DIV_ASSIGN,
  TK_MOD_ASSIGN,
  TK_AND_ASSIGN,
  TK_XOR_ASSIGN,
  TK_OR_ASSIGN,
  TK_RIGHT_OP,
  TK_LEFT_OP,
  TK_INC_OP,
  TK_DEC_OP,
  TK_PTR_OP,
  TK_AND_OP,
  TK_OR_OP,
  TK_LE_OP,
  TK_GE_OP,
  TK_EQ_OP,
  TK_NE_OP,
  
  TK_EOF,
  TK_CHAR_LITERAL,
  TK_INT_LITERAL,
  TK_DOUBLE_LITERAL,  
  TK_STRING_LITERAL,
  
  TK_BREAK,
  TK_CASE,
  TK_CONTINUE,
  TK_DO,
  TK_DEFAULT,
  TK_ELSE,
  TK_ENUM,
  TK_FOR,
  TK_GOTO,
  TK_IF,
  TK_IMAGINARY,
  TK_RETURN,
  TK_SIZEOF,
  TK_STRUCT,
  TK_SWITCH,
  TK_TYPEDEF,
  TK_UNION,
  TK_WHILE,
  TK_AUTO,  
  TK_CONST,  
  TK_EXTERN,
  TK_INLINE,
  TK_RESTRICT,  
  TK_VOLATILE,
  TK_SIGNED,  
  TK_UNSIGNED,
  TK_STATIC,
  TK_REGISTER,  
  
  TK_IDENTIFIER,
  TK_BOOL,
  TK_CHAR,
  TK_COMPLEX,
  TK_DOUBLE,
  TK_FLOAT,
  TK_INT,
  TK_LONG,
  TK_SHORT,
  TK_VOID,
};

/**
 * report error while compiling
 */
#define error(...) do{errorf(__FILE__, __LINE__, __VA_ARGS__);}while(0)
#define warning(...) do{warningf(__FILE__, __LINE__, __VA_ARGS__);}while(0)

void errorf(char *file, int line, char *fmt, ...);
void warningf(char *file, int line, char *fmt, ...);

/**
 * some function for String
 */
typedef struct {
  char* stack;
  size_t size;
  size_t length;
} ywstr;

char* ywstrCopy(char* s);
ywstr* ywstrCreate(char* s);
char* ywstrGet(ywstr* ys);
void ywstrAppend(ywstr* ys, char c);
void ywstrAppendFormat(ywstr* ys, char* fmt, ...);

typedef struct ywlist_node {
  void* element;
  struct ywlist_node* next;
} ywlist_node;

typedef struct ywlist {
  size_t length;
  ywlist_node* head;
  ywlist_node* tail;
} ywlist;

typedef struct ywiter {
  ywlist_node* ptr;
} ywiter;

ywlist* ywlistCreate();
void ywlistAppend(ywlist* yl, void* element);
size_t ywlistLen(ywlist* yl);
ywiter* ywlistIter(ywlist* yl);
void* ywiterNext(ywiter* iter);
bool ywiterEnd(ywiter* iter);

#endif

