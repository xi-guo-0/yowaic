// parser.c
// create abstract syntax tree
// Copyright (C) 2018: see LICENSE
#include "parser.h"
#include "token.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 6
#define EXPR_LEN 50

rt_t* rt_char_t = &(rt_t){RT_CHAR, NULL};
rt_t* rt_int_t = &(rt_t){RT_INT, NULL};
rt_t* rt_void_t = &(rt_t){RT_VOID, NULL};
ywlist* globals = &(ywlist){.length=0, .head=NULL, .tail=NULL};
ywlist* fparams = NULL;
ywlist* locals = NULL;
char* REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

Ast* parseCompoundStatement();
static Ast* parseBlockItem();
static Ast* parseBopRHS(int expr_prec);
static char* rtToS(rt_t* rt_type);
static int rtTypeSize(rt_t* rt_type);
static rt_t* createPtrType(rt_t* rt_type);
static rt_t* createArrayType(rt_t* rt_type, int size);
static void astToSBuffer(Ast *ast, ywstr* ys);
static Ast* parseStatement();
static Ast* parseExpressionStatement();
static Ast* parseIfStatement();
char* compoundStatementToS(ywlist* yl);

static Ast* createAstUop(int kind, rt_t* rt_type, Ast* operand) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = kind;
  ret->rt_type = rt_type;
  ret->operand = operand;
  return ret;
}

static Ast* createAstBop(int kind, rt_t* rt_type, Ast* left, Ast* right) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = kind;
  ret->rt_type = rt_type;
  ret->left = left;
  ret->right = right;
  return ret;
}

static Ast* createAstChar(char c) {
  Ast *ret = malloc(sizeof(Ast));
  ret->kind = AST_LITERAL;
  ret->rt_type = rt_char_t;
  ret->cval = c;
  return ret;
}

static Ast* createAstInt(int val) {
  Ast *ret = malloc(sizeof(Ast));
  ret->kind = AST_LITERAL;
  ret->rt_type = rt_int_t;
  ret->ival = val;
  return ret;
}

char* createNextLabel() {
  static unsigned int label_sequence = 0;
  ywstr* ys = ywstrCreate(".L");
  ywstrAppendFormat(ys, "%u", label_sequence++);
  return ywstrGet(ys);
}

static Ast* createAstLvar(rt_t* rt_type, char* name) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_LID;
  ret->rt_type = rt_type;
  ret->lname = name;
  if (locals)
    ywlistAppend(locals, ret);
  return ret;
}

static Ast* createAstLref(rt_t* rt_type, Ast* lvar, int offset) {
  Ast* lref = malloc(sizeof(Ast));
  lref->kind = AST_LREF;
  lref->rt_type = rt_type;
  lref->lref = lvar;
  lref->lref_offset = offset;
  return lref;
}

static Ast* createAstGvar(rt_t* rt_type, char* name, bool filelocal) __attribute__((unused));
static Ast* createAstGvar(rt_t* rt_type, char* name, bool filelocal) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_GID;
  ret->rt_type = rt_type;
  ret->gname = name;
  ret->glabel = filelocal ? createNextLabel() : name;
  ywlistAppend(globals, ret);
  return ret;
}

static Ast* createAstGref(rt_t* rt_type, Ast* gvar, int offset) {
  Ast* gref = malloc(sizeof(Ast));
  gref->kind = AST_GREF;
  gref->rt_type = rt_type;
  gref->gref = gvar;
  gref->gref_offset = offset;
  return gref;
}

static Ast* createAstString(char* str) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_STRING;
  ret->rt_type = createArrayType(rt_char_t, strlen(str) + 1);
  ret->sval = str;
  ret->slabel = createNextLabel();
  return ret;
}

static Ast* createAstFunCall(rt_t* rt_type,char* fun_name, ywlist* args) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_FUN_CALL;
  ret->rt_type = rt_type;
  ret->fun_name = fun_name;
  ret->args = args;
  return ret;
}

static Ast* createAstFun(rt_t* rt_type, char* fun_name, ywlist* params, Ast* body, ywlist* locals) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_FUN_DEFINE;
  ret->rt_type = rt_type;
  ret->fun_name = fun_name;
  ret->params = params;
  ret->locals = locals;
  ret->body = body;
  return ret;
}

static Ast* createAstDeclaration(Ast* var, Ast* init) {
  Ast* decl = malloc(sizeof(Ast));
  decl->kind = AST_DECLARATION;
  decl->rt_type = rt_void_t;
  decl->decl_var = var;
  decl->decl_init = init;
  return decl;
}

static Ast* createAstArrayInit(ywlist* yl) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_ARRAY_INIT;
  ret->rt_type = rt_void_t;
  ret->array_init = yl;
  return ret;
}

static Ast* createAstIf(Ast* s_cond, Ast* s_then, Ast* s_else) {
  Ast *ret = malloc(sizeof(Ast));
  ret->kind = AST_IF;
  ret->rt_type = rt_void_t;
  ret->s_cond = s_cond;
  ret->s_then = s_then;
  ret->s_else = s_else;
}

static Ast* createAstFor(Ast* init, Ast* cond, Ast* step, Ast* body) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_FOR;
  ret->rt_type = rt_void_t;
  ret->forinit = init;
  ret->forcond = cond;
  ret->forstep = step;
  ret->forbody = body;
  return ret;
}

static Ast* createAstReturn(Ast* r) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_RETURN;
  ret->rt_type = rt_void_t;
  ret->ret = r;
  return ret;
}

static Ast* createAstCompoundStatement(ywlist* yl) {
  Ast* ret = malloc(sizeof(Ast));
  ret->kind = AST_COMPOUND;
  ret->rt_type = rt_void_t;
  ret->compound = yl;
  return ret;
}

static rt_t* createPtrType(rt_t* rt_type) {
  rt_t* ret = malloc(sizeof(rt_t));
  ret->type = RT_PTR;
  ret->ptr = rt_type;
  return ret;
}

static rt_t* createArrayType(rt_t* rt_type, int size) {
  rt_t* ret = malloc(sizeof(rt_t));
  ret->type = RT_ARRAY;
  ret->ptr = rt_type;
  ret->size = size;
  return ret;
}

static Ast* findVarSub(ywlist* yl, char* name) {
  for (ywiter* i = ywlistIter(yl); !ywiterEnd(i);) {
    Ast* p = ywiterNext(i);
    if (!strcmp(name, p->lname))
      return p;
  }
  return NULL;
}

static Ast *findVar(char *name) {
  Ast* ret = findVarSub(locals, name);
  if (ret)
    return ret;
  ret = findVarSub(fparams, name);
  if (ret)
    return ret;
  return findVarSub(globals, name);
}

static bool isRightAssociate(Token* tk) {
  return '=' == tk->kind;
}

static int priority(Token* tk) {
  switch (tk->kind) {
  case '=':
    return 10;
  case TK_EQ_OP:
    return 20;
  case '<':
  case '>':
    return 30;
  case '+':
  case '-':
    return 40;
  case '*':
  case '/':
    return 50;
  default:
    return -10;
  }
}

static Ast *parseFunCallArgs(char *fun_name) {
  ywlist* args = ywlistCreate();
  for (;;) {
    Token* tk = nextToken();
    if (')' == tk->kind)
      break;
    ungetToken(tk);
    ywlistAppend(args, parseBopRHS(0));
    Token* tk2 = nextToken();
    if (')' == tk2->kind)
      break;
    else if (',' == tk2->kind)
      ;
    else
      error("Unexpected token: %s", tokenToS(tk2->kind));
  }
  if (MAX_ARGS < ywlistLen(args))
    error("Too many arguments: %s", fun_name);
  return createAstFunCall(rt_int_t, fun_name, args);
}

static Ast* parseIdentifierOrFunCall(char* name) {
  Token* tk = nextToken();
  if ('(' == tk->kind)
    return parseFunCallArgs(name);
  ungetToken(tk);
  Ast* v = findVar(name);
  if (!v)
    error("Undefined variable: %s", name);
  return v;
}

static Ast *parsePrime() {
  Token* tk = nextToken();
  switch (tk->kind) {
  case TK_CHAR_LITERAL:
    return createAstChar(tk->cval);
  case TK_INT_LITERAL:
    return createAstInt(tk->ival);
  case TK_IDENTIFIER:
    return parseIdentifierOrFunCall(tk->sval);
  case TK_STRING_LITERAL: {
    Ast* ret = createAstString(tk->sval);
    ywlistAppend(globals, ret);
    return ret;
  } break;
  case TK_EOF:
    return NULL;
  }
  error("Don't know how to handle %s", tokenToS(tk->kind));
}

static void ensureLHS(Ast *ast) {
  switch (ast->kind) {
  case AST_LID:
  case AST_LREF:
  case AST_GID:
  case AST_GREF:
  case AST_DEREFERENCE:
    break;
  default:
    error("lvalue expected, but got %s", astToS(ast));
  }
}

static bool isTypeEqual(rt_t *rt_a, rt_t *rt_b) {
  if (rt_a->type != rt_b->type)
    return false;
  rt_t *tmpa = rt_a->ptr;
  rt_t *tmpb = rt_b->ptr;
  while (RT_PTR == tmpa->type && RT_PTR == tmpb->type) {
    tmpa = tmpa->ptr;
    tmpb = tmpb->ptr;
  }
  if (tmpa->type != tmpb->type)
    return false;
  return true;
}

static rt_t *resultType(int op, Ast *a, Ast *b) {
  switch (a->rt_type->type) {
  case RT_CHAR:
    switch (b->rt_type->type) {
    case RT_CHAR:
      return rt_char_t;
    case RT_INT:
      return rt_int_t;
    case RT_PTR:
    case RT_ARRAY:
      return b->rt_type;
    }
  case RT_INT:
    switch (b->rt_type->type) {
    case RT_CHAR:
    case RT_INT:
      return rt_int_t;
    case RT_PTR:
    case RT_ARRAY:
      return b->rt_type;
    }
    break;
  case RT_PTR:
  case RT_ARRAY:
    if ('=' == op)
      return a->rt_type;
    else if ('+' != op && '-' != op)
      break;
    else if (RT_INT == b->rt_type->type)
      return a->rt_type;
    else if (isTypeEqual(a->rt_type, b->rt_type))
      return a->rt_type;
  }
  error("incompatible operator: %s and %s for %d", astToS(a),
        astToS(b), op);
}

static Ast* convertArray(Ast* ast) {
  if (AST_STRING == ast->kind)
    return createAstGref(createPtrType(rt_char_t), ast, 0);
  if (RT_ARRAY != ast->rt_type->type)
    return ast;
  if (AST_LID == ast->kind)
    return createAstLref(createPtrType(ast->rt_type->ptr), ast, 0);
  if (AST_GID != ast->kind)
    error("Gvar expected, but got %s", astToS(ast));
  return createAstGref(createPtrType(ast->rt_type->ptr), ast, 0);
}

static Ast* parseUnaryExpr() {
  Token* tk = nextToken();
  if ('&' == tk->kind) {
    Ast* operand = parseUnaryExpr();
    ensureLHS(operand);
    return createAstUop(AST_ADDRESS, createPtrType(operand->rt_type), operand);
  }
  if ('*' == tk->kind) {
    Ast* operand = convertArray(parseUnaryExpr());
    if (RT_PTR != operand->rt_type->type)
      error("pointer type expected, but got %s", astToS(operand));
    return createAstUop(AST_DEREFERENCE, operand->rt_type->ptr, operand);
  }
  ungetToken(tk);
  return parsePrime();
}

static Ast* parseBopRHS(int expr_prec) {
  Ast* LHS = parseUnaryExpr();
  if (!LHS)
    return NULL;
  for (;;) {
    Token* tk = nextToken();
    if (TK_EOF < tk->kind) {
      ungetToken(tk);
      return LHS;
    }
    int tk_prec = priority(tk);
    if (tk_prec < expr_prec) {
      ungetToken(tk);
      return LHS;
    }
    if ('=' == tk->kind)
      ensureLHS(LHS);
    Ast* RHS = parseBopRHS(tk_prec + (isRightAssociate(tk) ? 0 : 1));
    RHS = convertArray(RHS);
    rt_t* rt_type = resultType(tk->kind, LHS, RHS);
    LHS = createAstBop(tk->kind, rt_type, LHS, RHS);
  }
}

static void eat(int punct) {
  Token* tk = nextToken();
  if (punct != tk->kind)
    error("%s expected, but got %s", tokenToS(punct), tokenToS(tk->kind));
}

static rt_t* tkToRt(int kind) {
  switch (kind) {
  case TK_CHAR:
    return rt_char_t;
  case TK_INT:
    return rt_int_t;
  }
  return rt_void_t;
}

static Ast* parse_decl_array_init(rt_t* rt_type) {
  Token* tk = nextToken();
  if (RT_CHAR ==  rt_type->ptr->type && TK_STRING_LITERAL == tk->kind)
    return createAstString(tk->sval);
  if ('{' != tk->kind)
    error("Expected an initializer list, but got %s", tokenToS(tk->kind));
  ywlist* yl = ywlistCreate();
  for (;;) {
    Token* tk = nextToken();
    if ('}' == tk->kind)
      break;
    ungetToken(tk);
    Ast* init = parseBopRHS(0);
    ywlistAppend(yl, init);
    tk = nextToken();
    if (',' != tk->kind)
      ungetToken(tk);
  }
  return createAstArrayInit(yl);
}

static Ast* parseDeclInitializer(rt_t* rt_type) {
  if (RT_ARRAY ==  rt_type->type)
    return parse_decl_array_init(rt_type);
  return parseBopRHS(0);
}

static rt_t* parseDeclarationSpecifiers() {
  Token* tk = nextToken();
  rt_t* rt_type = tkToRt(tk->kind);
  if (rt_void_t == rt_type)
    error("Type expected");
  for (;;) {
    tk = nextToken();
    if ('*' != tk->kind) {
      ungetToken(tk);
      return rt_type;
    }
    rt_type = createPtrType(rt_type);
  }
}

static Ast* parseDeclaration() {
  rt_t* rt_type = parseDeclarationSpecifiers();
  Token* varname = nextToken();
  if (TK_IDENTIFIER != varname->kind)
    error("Identifier expected, but got %d", varname->kind);
  for (;;) {
    Token* tk = nextToken();
    if ('[' == tk->kind) {
      tk = peekToken();
      if (']' == tk->kind) {
        if (rt_type->size == -1)
          error("Array size is not specified");
        rt_type = createArrayType(rt_type, -1);
      } else {
        Ast* size = parseBopRHS(0);
        if (AST_LITERAL != size->kind || RT_INT != size->rt_type->type)
          error("Integer expected, but got %s", astToS(size));
        rt_type = createArrayType(rt_type, size->ival);
      }
      eat(']');
    } else {
      ungetToken(tk);
      break;
    }
  }
  Ast* LHS = createAstLvar(rt_type, varname->sval);
  Ast* RHS;
  eat('=');
  if (RT_ARRAY == rt_type->type) {
    RHS = parse_decl_array_init(rt_type);
    int len = (AST_STRING ==  RHS->kind) ? strlen(RHS->sval) + 1 : ywlistLen(RHS->array_init);
    if (rt_type->size == -1) {
      rt_type->size = len;
    } else if (rt_type->size != len)
      error("Invalid array initializer: expected %d, but got %d",
            rt_type->size, len);
  } else {
    RHS = parseBopRHS(0);
  }
  eat(';');
  return createAstDeclaration(LHS, RHS);
}

static Ast* parseIfStatement() {
  eat('(');
  Ast* s_cond = parseBopRHS(0);
  eat(')');
  eat('{');
  Ast* s_then = parseCompoundStatement();
  Token* tk = nextToken();
  if (TK_ELSE != tk->kind) {
    ungetToken(tk);
    return createAstIf(s_cond, s_then, NULL);
  }
  eat('{');
  Ast* s_else = parseCompoundStatement();
  return createAstIf(s_cond, s_then, s_else);
}

static Ast* parseExpressionStatement() {
  Token* tk = nextToken();
  if (';' == tk->kind)
    return NULL;
  ungetToken(tk);
  Ast* ret = parseBopRHS(0);
  eat(';');
  return ret;
}

static Ast* parseExpressionStatementOrDeclaration() {
  Token* tk = nextToken();
  if (';' == tk->kind)
    return NULL;
  ungetToken(tk);
  return (TK_IDENTIFIER < tk->kind) ? parseDeclaration() : parseExpressionStatement();
}

static Ast* parseForStatement() {
  eat('(');
  Ast* init = parseExpressionStatementOrDeclaration();
  Ast* cond = parseExpressionStatement();
  Ast* step = (')' == peekToken()->kind) ? NULL : parseBopRHS(0);
  eat(')');
  Ast* body = parseStatement();
  return createAstFor(init, cond, step, body);
}

static Ast* parseReturnStatement() {
  Ast* ret = parseBopRHS(0);
  eat(';');
  return createAstReturn(ret);
}

static Ast* parseStatement() {
  Token* tk = nextToken();
  switch (tk->kind) {
  case ';':
    return NULL;
  case '{':
    return parseCompoundStatement();
  case TK_IF:
    return parseIfStatement();
  case TK_FOR:
    return parseForStatement();
  case TK_RETURN:
    return parseReturnStatement();
  default:
    ungetToken(tk);
    return parseExpressionStatement();
  }
}

Ast* parseCompoundStatement() {
  ywlist* yl = ywlistCreate();
  for (;;) {
    Ast* block_item = parseBlockItem();
    if (block_item)
      ywlistAppend(yl, block_item);
    else
      break;
    Token* tk = nextToken();
    if ('}' == tk->kind)
      break;
    ungetToken(tk);
  }
  return createAstCompoundStatement(yl);
}

Ast* parseBlockItem() {
  Token *tk = peekToken();
  if (TK_EOF == tk->kind)
    return NULL;
  Ast *ast = (TK_IDENTIFIER < tk->kind) ? parseDeclaration() : parseStatement();
  /*  
  tk = nextToken();
  if (';' != tk->kind)
    error("parseBlockItem: Unterminated expression: %d", tk->kind);
  */
  return ast;
}

static ywlist* parseParams() {
  ywlist* yl = ywlistCreate();
  Token* tk = nextToken();
  if (')' == tk->kind)
    return yl;
  ungetToken(tk);
  for (;;) {
    rt_t* rt_type = parseDeclarationSpecifiers();
    Token* pname = nextToken();
    if (TK_IDENTIFIER != pname->kind)
      error("Identifier expected, but got %s", tokenToS(tk->kind));
    ywlistAppend(yl, createAstLvar(rt_type, pname->sval));
    Token* tk = nextToken();
    if (')' == tk->kind)
      return yl;
    else if (',' != tk->kind)
      error("',' expected, but got %s", tokenToS(tk->kind));
  }
}

static Ast* parseFunDeclaration() {
  Token* tk = peekToken();
  if (TK_EOF == tk->kind)
    return NULL;
  void* ret_type = parseDeclarationSpecifiers();
  Token* fun_name = nextToken();
  if (TK_IDENTIFIER != fun_name->kind)
    error("Function name expected, but got %s", tokenToS(fun_name->kind));
  eat('(');
  fparams = parseParams();
  eat('{');
  locals = ywlistCreate();
  Ast* body = parseCompoundStatement();
  Ast* ret = createAstFun(ret_type, fun_name->sval, fparams, body, locals);
  fparams = locals = NULL;
  return ret;
}

ywlist* parseFunList() {
  ywlist* yl = ywlistCreate();
  for (;;) {
    Ast* fun = parseFunDeclaration();
    if (!fun)
      return yl;
    ywlistAppend(yl, fun);
  }
}

char *rtToS(rt_t *rt_type) {
  switch (rt_type->type) {
  case RT_CHAR:
    return "char";
  case RT_INT:
    return "int";
  case RT_VOID:
    return "void";
  case RT_PTR:{
    ywstr *ys = ywstrCreate(rtToS(rt_type->ptr));
    ywstrAppend(ys, '*');
    return ywstrGet(ys);
  } break;
  case RT_ARRAY: {
    ywstr *ys = ywstrCreate(rtToS(rt_type->ptr));
    ywstrAppendFormat(ys, "[%d]", rt_type->size);
    return ywstrGet(ys);
  } break;
  default:
    error("Unknown rt_type %d", rt_type->type);
  }
}

static void astToSBuffer(Ast* ast, ywstr* ys) {
  if (!ast) {
    ywstrAppendFormat(ys, "(null)");
    return;
  }
  switch (ast->kind) {
  case AST_LITERAL:
    switch(ast->rt_type->type) {
    case RT_CHAR:
      ywstrAppendFormat(ys, "'%c'", ast->cval);
      break;
    case RT_INT:
      ywstrAppendFormat(ys, "%d", ast->ival);
      break;
    default:
      error("literal error");
    } break;
  case AST_STRING:
    ywstrAppendFormat(ys, "\"%s\"", ast->sval);
    break;
  case AST_LID:
    ywstrAppendFormat(ys, "%s", ast->lname);
    break;
  case AST_GID:
    ywstrAppendFormat(ys, "%s", ast->gname);
    break;
  case AST_LREF:
    ywstrAppendFormat(ys, "%s[%d]", astToS(ast->lref), ast->lref_offset);
    break;
  case AST_GREF:
    ywstrAppendFormat(ys, "%s[%d]", astToS(ast->gref), ast->gref_offset);
    break;
  case AST_FUN_CALL:
    ywstrAppendFormat(ys, "(%s)%s(", rtToS(ast->rt_type), ast->fun_name);
    for (ywiter* i = ywlistIter(ast->args); !ywiterEnd(i);) {
      ywstrAppendFormat(ys, "%s", astToS(ywiterNext(i)));
      if (!ywiterEnd(i))
        ywstrAppend(ys, ',');
    }
    ywstrAppend(ys, ')');
    break;
  case AST_FUN_DEFINE:
    ywstrAppendFormat(ys, "(%s)%s(", rtToS(ast->rt_type), ast->fun_name);
    for (ywiter* i = ywlistIter(ast->params); !ywiterEnd(i);) {
      Ast* param = ywiterNext(i);
      ywstrAppendFormat(ys, "%s %s", rtToS(param->rt_type), astToS(param));
      if (!ywiterEnd(i))
        ywstrAppend(ys, ',');
    }
    ywstrAppendFormat(ys, ")%s", compoundStatementToS(ast->body->compound));
    break;    
  case AST_DECLARATION:
    ywstrAppendFormat(ys, "(decl %s %s %s)", rtToS(ast->decl_var->rt_type),
                         ast->decl_var->lname, astToS(ast->decl_init));
    break;
  case AST_ARRAY_INIT:
    ywstrAppend(ys, '{');
    for (ywiter* i = ywlistIter(ast->array_init); !ywiterEnd(i);) {
      astToSBuffer(ywiterNext(i), ys);
      if (!ywiterEnd(i))
        ywstrAppend(ys, ',');
    }
    ywstrAppend(ys, '}');
    break;
  case AST_ADDRESS:
    ywstrAppendFormat(ys, "(& %s)", astToS(ast->operand));
    break;
  case AST_DEREFERENCE:
    ywstrAppendFormat(ys, "(* %s)", astToS(ast->operand));
    break;
  case AST_IF:
    ywstrAppendFormat(ys, "(if %s %s",
                      astToS(ast->s_cond),
                      astToS(ast->s_then)
                      );
    if (ast->s_else)
      ywstrAppendFormat(ys, " %s", astToS(ast->s_else));
    ywstrAppend(ys, ')');
    break;
  case AST_FOR:
    ywstrAppendFormat(ys, "(for %s %s %s %s)",
                      astToS(ast->forinit),
                      astToS(ast->forcond),
                      astToS(ast->forstep),
                      astToS(ast->forbody));
    break;
  case AST_COMPOUND:
    ywstrAppendFormat(ys, "%s", compoundStatementToS(ast->compound));
    break;
  case AST_RETURN:
    ywstrAppendFormat(ys, "(return %s)", astToS(ast->ret));
    break;
  default: {
    char *LHS = astToS(ast->left);
    char *RHS = astToS(ast->right);
    ywstrAppendFormat(ys, "(%c %s %s)", ast->kind, LHS, RHS);
  }
  }
}

char* astToS(Ast *ast) {
  ywstr *ys = ywstrCreate("");
  astToSBuffer(ast, ys);
  return ywstrGet(ys);
}

char* compoundStatementToS(ywlist* yl) {
  ywstr* ys = ywstrCreate("{");
  for (ywiter* i = ywlistIter(yl); !ywiterEnd(i);) {
    astToSBuffer(ywiterNext(i), ys);
    ywstrAppend(ys, ';');
  }
  ywstrAppend(ys, '}');
  return ywstrGet(ys);
}
