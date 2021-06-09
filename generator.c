// generator.c
// emit assembly language
// Copyright (C) 2018: see LICENSE
#include "generator.h"
#include "parser.h"
#include "token.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern ywlist* globals;
extern ywlist* locals;
static char* REGS[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int rtTypeSize(rt_t *rt_type) {
  switch (rt_type->type) {
  case RT_CHAR:
    return 1;
  case RT_INT:
    return 4;
  case RT_PTR:
    return 8;
  case RT_ARRAY:
    return rtTypeSize(rt_type->ptr) * rt_type->size;
  default:
    error("internal error");
  }
}

static void emitGload(rt_t* rt_type, char* label, int offset) {
  if (RT_ARRAY == rt_type->type) {
    printf("leaq %s(%%rip), %%rax\n\t", label);
  if (offset)
    printf("addq $%d, %%rax\n\t", rtTypeSize(rt_type->ptr) * offset);
  return;
  }
  int size = rtTypeSize(rt_type);
  switch (size) {
  case 1:
    printf("movq $0, %%rax\n\t"
           "movb %s(%%rip), %%al", label);
    if (offset)
      printf("addq $%d, %%rax\n\t", offset * size);
    printf("movb (%%rax), %%al\n\t");
    break;
  case 4:
    printf("movl %s(%%rip), %%eax", label);
    if (offset)
      printf("addq $%d, %%rax\n\t", offset * size);
    printf("movl (%%rax), %%eax\n\t");    
    break;
  case 8:
    printf("movq %s(%%rip), %%rax", label);
    if (offset)
      printf("addq $%d, %%rax\n\t", offset * size);
    printf("movq (%%rax), %%rax\n\t");    
    break;
  default:
    error("Unknown data size: %d", size);
  }
}

static void emitLload(Ast* var, int offset) {
  if (RT_ARRAY == var->rt_type->type) {
    printf("leaq -%d(%%rbp), %%rax\n\t", var->loffset);
    return;
  }
  int size = rtTypeSize(var->rt_type);
  switch (size) {
  case 1:
    printf("movl %0, %%eax\n\t");
    printf("movb -%d(%%rbp), %%al\n\t", var->loffset);
    break;
  case 4:
    printf("movl -%d(%%rbp), %%eax\n\t", var->loffset);
    break;
  case 8:
    printf("movq -%d(%%rbp), %%rax\n\t", var->loffset);
    break;
  default:
    error("Unknown data size: %s: %d", astToS(var));
  }
  if (offset)
    printf("addq $%d, %%rax\n\t", var->loffset * size);
}

static void emitGsave(Ast* var, int offset) {
  assert(RT_ARRAY != var->rt_type->type);
  char* reg;
  printf("pushq %%rbx\n\t");
  printf("movq %s(%%rip), %%rbx\n\t", var->glabel);
  int size = rtTypeSize(var->rt_type);
  switch (size) {
  case 1:
    printf("movb %%al, %d(%%rbp)\n\t", offset * size);
    break;
  case 4:
    printf("movl %%eax, %d(%%rbp)\n\t", offset * size);
    break;
  case 8:
    printf("movq %%rax, %d(%%rbp)\n\t", offset * size);
    break;
  default:
    error("Unknown data size: %d", size);
  }
  printf("popq %%rbx\n\t");
}

static void emitLsave(rt_t* rt_type, int loffset, int offset) {
  int size = rtTypeSize(rt_type);
  switch (size) {
  case 1:
    printf("movb %%al, -%d(%%rbp)\n\t", loffset + offset * size);
    break;
  case 4:
    printf("movl %%eax, -%d(%%rbp)\n\t", loffset + offset * size);
    break;
  case 8:
    printf("movq %%rax, -%d(%%rbp)\n\t", loffset + offset * size);
  }
}
  
  
static void emitPtrArith(int op, Ast* LHS, Ast* RHS) {
  assert(RT_PTR == LHS->rt_type->type || RT_ARRAY == LHS->rt_type->type);
  emitExpr(LHS);
  printf("pushq %%rax\n\t");
  emitExpr(RHS);
  int shift = rtTypeSize(LHS->rt_type->ptr);
  if (1 < shift)
    printf("imulq $%d, %%rax\n\t", shift);
  printf("movq %%rax, %%rbx\n\t"
         "popq %%rax\n\t"
         "addq %%rbx, %%rax\n\t");
}

static void emitDereference(Ast* var, Ast* value) {
  emitExpr(var->operand);
  printf("push %%rax\n\t");
  emitExpr(value);
  printf("pop %%rcx\n\t");
  switch (rtTypeSize(var->operand->rt_type)) {
  case 1:
    printf("movb %%al, (%%rcx)\n\t");
    break;
  case 4:
    printf("movl %%eax, (%%rcx)\n\t");
    break;
  case 8:
    printf("movq %%rax, (%%rcx)\n\t");
    break;
  }
}

static void emitAssign(Ast* var, Ast* value) {
  emitExpr(value);
  switch (var->kind) {
  case AST_LID:
    emitLsave(var->rt_type, var->loffset, 0);
    break;
  case AST_LREF:
    emitLsave(var->lref->rt_type, var->lref->loffset, var->loffset);
    break;
  case AST_GID:
    emitGsave(var, 0);
    break;
  case AST_GREF:
    emitGsave(var->gref, var->gref_offset);
    break;
  case AST_DEREFERENCE:
    emitDereference(var, value);
    break;
  default:
    error("Unexpected kind %d", astToS(var));
  }
}

static void emitCompare(Ast* a, Ast* b) {
  emitExpr(a);
  printf("pushq %%rax\n\t");
  emitExpr(b);
  printf("popq %%rcx\n\t"
         "cmpq %%rax, %%rcx\n\t"
         "setl %%al\n\t"
         "movzb %%al, %%eax\n\t"
         );
}

static void emitBinop(Ast *ast) {
  if ('=' == ast->kind) {
    emitAssign(ast->left, ast->right);
    return;
  }
  if (RT_PTR == ast->rt_type->type || RT_ARRAY == ast->rt_type->type) {
    emitPtrArith(ast->kind, ast->left, ast->right);
    return;
  }
  char *op;
  switch (ast->kind) {
  case '<':
    emitCompare(ast->left, ast->right);
    return;
  case '>':
    emitCompare(ast->right, ast->left);
    return;
  case '+':
    op = "add";
    break;
  case '-':
    op = "sub";
    break;
  case '*':
    op = "imul";
    break;
  case '/':
    op = "idiv";
    break;
  default:
    error("emitBinop: invalid operator %s", astToS(ast));
  }
  emitExpr(ast->left);
  printf("push %%rax\n\t");
  emitExpr(ast->right);
  if (ast->kind == '/') {
    printf("movq %%rax, %%rcx\n\t");
    printf("popq %%rax\n\t");
    printf("movl $0, %%edx\n\t");
    printf("idivq %%rcx\n\t");
  } else {
    printf("popq %%rcx\n\t");
    printf("%s %%rcx, %%rax\n\t", op);
  }
}

void emitExpr(Ast *ast) {
  switch (ast->kind) {
  case AST_LITERAL:
    switch (ast->rt_type->type) {
    case RT_CHAR:
      printf("movq $%d, %%rax\n\t", ast->cval);
      break;
    case RT_INT:
      printf("movl $%d, %%eax\n\t", ast->ival);
      break;
    case RT_ARRAY:
      printf("leaq .s%d(%%rip), %%rax\n\t", ast->slabel);
      break;
    default:
      error("AST_LITERAL error");
    } break;
  case AST_STRING:
    printf("leaq %s(%%rip), %%rax\n\t", ast->slabel);
    break;
  case AST_LID:
    emitLload(ast, 0);
    break;
  case AST_LREF:
    assert(AST_LID == ast->lref->kind);
    emitLload(ast->lref, ast->lref_offset);
    break;
  case AST_GID:
    emitGload(ast->rt_type, ast->glabel, 0);
    break;
  case AST_GREF:
    if (AST_STRING == ast->gref->kind) {
      printf("leaq %s(%%rip), %%rax\n\t", ast->gref->slabel);
    } else {
      assert(AST_GID == ast->gref->kind);
      emitGload(ast->gref->rt_type, ast->gref->glabel, ast->gref_offset);
    }
    break;
  case AST_FUN_CALL:
    for (int i = 1; i < ywlistLen(ast->args); i++)
      printf("push %%%s\n\t", REGS[i]);
    for (ywiter* i = ywlistIter(ast->args); !ywiterEnd(i);) {
      emitExpr(ywiterNext(i));
      printf("pushq %%rax\n\t");
    }
    for (int i = ywlistLen(ast->args) - 1; i >= 0; i--)
      printf("pop %%%s\n\t", REGS[i]);
    printf("movq $0, %%rax\n\t");
    printf("call %s", ast->fun_name);
    if (!strcmp("printf", ast->fun_name))
      printf("@plt");
    printf("\n\t");
    for (int i = ywlistLen(ast->args) - 1; i > 0; i--)
      printf("pop %%%s\n\t", REGS[i]);
    break;
    case AST_DECLARATION:
      if (AST_ARRAY_INIT == ast->decl_init->kind) {
        int i = 0;
        for (ywiter* iter = ywlistIter(ast->decl_init->array_init); !ywiterEnd(iter);) {
          emitExpr(ywiterNext(iter));
          emitLsave(ast->decl_var->rt_type->ptr,ast->decl_var->loffset, -i);
          i++;
        }
      } else if (RT_ARRAY == ast->decl_var->rt_type->type) {
        assert(AST_STRING == ast->decl_init->kind);
        int i = 0;
        for (char* p = ast->decl_init->sval; *p; p++, i++)
          printf("movb $%d, -%d(%%rbp)\n\t", *p, ast->decl_var->loffset - i);
        printf("movb $0, -%d(%%rbp)\n\t", ast->decl_var->loffset - i);
      } else if (ast->decl_init->kind == AST_STRING) {
        emitGload(ast->decl_init->rt_type, ast->decl_init->slabel, 0);
        emitLsave(ast->decl_var->rt_type, ast->decl_var->loffset, 0);
      } else {
        emitExpr(ast->decl_init);
        emitLsave(ast->decl_var->rt_type, ast->decl_var->loffset, 0);
      }
      break;
  case AST_ADDRESS:
    if (AST_LID != ast->operand->kind)
      error("AST_ADDRESS");
    printf("lea -%d(%%rbp), %%rax\n\t", ast->operand->loffset);
    break;
  case AST_DEREFERENCE:
    if (RT_PTR != ast->operand->rt_type->type)
      error("AST_DEREFERENCE");
    emitExpr(ast->operand);
    char *reg;
    switch (rtTypeSize(ast->operand->rt_type->ptr)) {
    case 1: reg = "%bl";  break;
    case 4: reg = "%ebx"; break;
    case 8: reg = "%rbx"; break;
    default: error("AST_DEREFERENCE");
    }
    printf("movl $0, %%ebx\n\t");
    printf("mov (%%rax), %s\n\t", reg);
    printf("movq %%rbx, %%rax\n\t");
    break;
  case AST_IF:
    emitExpr(ast->s_cond);
    char* ne = createNextLabel();
    printf("test %%rax, %%rax\n\t");
    printf("je %s\n\t", ne);
    /*
    emitCompoundStatement(ast->s_then->compound);
    if (ast->s_else) {
      char* end = createNextLabel();
      printf("jmp %s\n\t", end);
      printf("%s:\n\t", ne);
      emitCompoundStatement(ast->s_else->compound);
      printf("%s:\n\t", end);
    } else {
      printf("%s:\n\t", ne);
    }
    */
    emitCompoundStatement(ast->s_then->compound);
    if (ast->s_else) {
      char* end = createNextLabel();
      printf("jmp %s\n\t", end);
      printf("%s:\n\t", ne);
      emitCompoundStatement(ast->s_else->compound);
      printf("%s:\n\t", end);
    } else {
      printf("%s:\n\t", ne);
    }
    break;
  case AST_FOR:
    if (ast->forinit)
      emitExpr(ast->forinit);
    char* begin = createNextLabel();
    char* end = createNextLabel();
    printf("%s:\n\t", begin);
    if (ast->forcond) {
      emitExpr(ast->forcond);
      printf("test %%rax, %%rax\n\t");
      printf("je %s\n\t", end);
    }
    emitCompoundStatement(ast->forbody->compound);
    if (ast->forstep)
      emitExpr(ast->forstep);
    printf("jmp %s\n\t", begin);
    printf("%s:\n\t", end);
    break;
  case AST_COMPOUND:
    emitCompoundStatement(ast->compound);
    break;
  case AST_RETURN:
    emitExpr(ast->ret);
    printf("leave\n\t"
           "ret\n");
    break;
  default:
    emitBinop(ast);
  }
}

void emitDataSection() {
  if (!globals) return;
  printf("\t.data\n");
  for (ywiter* i = ywlistIter(globals); !ywiterEnd(i);) {
    Ast* p = ywiterNext(i);
    assert(AST_STRING == p->kind);
    printf("%s:\n\t", p->slabel);
    printf(".string \"%s\"\n", p->sval);
  }
  printf("\t");
}

static int ceil8(int n) {
  int rem = n % 8;
  return (0 == rem) ? n : n - rem + 8;
}

void emitAsmHeader() {
  int offset = 0;
  for (ywiter* i = ywlistIter(locals); !ywiterEnd(i);) {
    Ast* p = ywiterNext(i);
    offset += ceil8(rtTypeSize(p->rt_type));
    p->loffset = offset;
  }
  emitDataSection();
  printf(".text\n\t"
         ".global mymain\n"
         "mymain:\n\t"
         "pushq %%rbp\n\t"
         "movq %%rsp, %%rbp\n\t");
  if (locals)
    printf("subq $%d, %%rsp\n\t", offset);
}

void emitCompoundStatement(ywlist* yl) {
  for (ywiter* i = ywlistIter(yl); !ywiterEnd(i);) {
    emitExpr(ywiterNext(i));
  }
}

static void emitFunProlog(Ast* fun) {
  if (ywlistLen(fun->params) > sizeof(REGS) / sizeof(*REGS))
    error("Parameter list too long: %s", fun->fun_name);
  printf(".text\n\t"
         ".global %s\n"
         "%s:\n\t", fun->fun_name, fun->fun_name);
  printf("pushq %%rbp\n\t"
         "movq %%rsp, %%rbp\n\t");
  int off = 0;
  int ri = 0;
  for (ywiter* i = ywlistIter(fun->params); !ywiterEnd(i); ri++) {
    printf("push %%%s\n\t", REGS[ri]);
    Ast* p = ywiterNext(i);
    off += ceil8(rtTypeSize(p->rt_type));
    p->loffset = off;
  }
  for (ywiter* i = ywlistIter(fun->locals); !ywiterEnd(i);) {
    Ast* p = ywiterNext(i);
    off += ceil8(rtTypeSize(p->rt_type));
    p->loffset = off;
  }
  if (off)
    printf("subq $%d, %%rsp\n\t", off);
}

static void emitFunEpilog(void) {
  printf("leave\n\t"
         "ret\n");
}

void emitFun(Ast* fun) {
  assert(AST_FUN_DEFINE == fun->kind);
  emitFunProlog(fun);
  emitCompoundStatement(fun->body->compound);
  emitFunEpilog();
}
