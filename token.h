// token.h
// define token struct
// and method for get token
// Copyright (C) 2018: see LICENSE
#ifndef _YOWAIC_TOKEN_H_
#define _YOWAIC_TOKEN_H_

typedef union {
  char cval;
  int ival;
  double dval;
  char *sval;
} YYSTYPE;

typedef struct Token {
  int kind;
  union {
    char cval;
    int ival;
    double dval;
    char *sval;
  };
} Token;

extern YYSTYPE yylval;
extern int yylex();

Token *nextToken();
Token *peekToken();
void ungetToken(Token *tk);
char *tokenToS(int kind);
#endif
