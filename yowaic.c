// yowaic.c
// weak c compiler
// Copyright (C) 2018: see LICENSE
#include "parser.h"
#include "generator.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


int main(int argc, char **argv) {
  extern FILE *yyin;
  yyin = stdin;

  bool want_ast = (argc > 1 && !strcmp(argv[1], "-a"));

  ywlist* yl = parseFunList();
  if (!want_ast)
    emitDataSection();
  for (ywiter* i = ywlistIter(yl); !ywiterEnd(i);) {
    Ast* fun = ywiterNext(i);
    if (want_ast)
      printf("%s", astToS(fun));
    else
      emitFun(fun);
  }
  return 0;
}

