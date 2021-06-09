// generator.h
// emit assembly language
// Copyright (C) 2018: see LICENSE
#ifndef _YOWAIC_GENERATOR_H_
#define _YOWAIC_GENERATOR_H_
#include "parser.h"
#include "util.h"
void emitExpr(Ast *ast);
void emitCompoundStatement(ywlist* yl);
void emitAsmHeader();
void emitDataSection();
void emitFun(Ast* fun);
#endif

