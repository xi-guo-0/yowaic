// token.c
// define token struct
// and method for get token
// Copyright (C) 2018: see LICENSE
#include "token.h"
#include "util.h"
#include <stdlib.h>

YYSTYPE yylval;

static Token *ungotten = NULL;

Token *readToken() {
  Token *token = malloc(sizeof(Token));
  token->kind = yylex();
  switch (token->kind) {
  case TK_CHAR_LITERAL:
    token->cval = yylval.cval;
    break;
  case TK_DOUBLE_LITERAL:
    token->dval = yylval.dval;
    break;
  case TK_INT_LITERAL:
    token->ival = yylval.ival;
    break;
  case TK_IDENTIFIER:
  case TK_STRING_LITERAL:
    token->sval = yylval.sval;
  };
  return token;
}

void ungetToken(Token *tk) {
  if (NULL != ungotten)
    error("Push back buffer is already full");
  ungotten = tk;
}

Token *nextToken() {
  if (ungotten) {
    Token *tk = ungotten;
    ungotten = NULL;
    return tk;
  }
  return readToken();
}

Token *peekToken() {
  Token *tk = nextToken();
  ungetToken(tk);
  return tk;
}

char *tokenToS(int kind) {
  switch (kind) {
  case ';':
    return ";";
  case '{':
    return "{";
  case '}':
    return "}";
  case ',':
    return ",";
  case ':':
    return ":";
  case '=':
    return "=";
  case '(':
    return "(";
  case ')':
    return ")";
  case '[':
    return "[";
  case ']':
    return "]";
  case '.':
    return ".";
  case '&':
    return "&";
  case '!':
    return "!";
  case '~':
    return "~";
  case '-':
    return "-";
  case '+':
    return "+";
  case '*':
    return "*";
  case '/':
    return "/";
  case '%':
    return "%";
  case '<':
    return "<";
  case '>':
    return ">";
  case '^':
    return "^";
  case '|':
    return "|";
  case '?':
    return "?";
  case TK_ELLIPSIS:
    return "...";
  case TK_RIGHT_ASSIGN:
    return ">>=";
  case TK_LEFT_ASSIGN:
    return "<<=";
  case TK_ADD_ASSIGN:
    return "+=";
  case TK_SUB_ASSIGN:
    return "-=";
  case TK_MUL_ASSIGN:
    return "*=";
  case TK_DIV_ASSIGN:
    return "/=";
  case TK_MOD_ASSIGN:
    return "%=";
  case TK_AND_ASSIGN:
    return "&=";
  case TK_XOR_ASSIGN:
    return "^=";
  case TK_OR_ASSIGN:
    return "|=";
  case TK_RIGHT_OP:
    return ">>";
  case TK_LEFT_OP:
    return "<<";
  case TK_INC_OP:
    return "++";
  case TK_DEC_OP:
    return "--";
  case TK_PTR_OP:
    return "->";
  case TK_AND_OP:
    return "&&";
  case TK_OR_OP:
    return "||";
  case TK_LE_OP:
    return "<=";
  case TK_GE_OP:
    return ">=";
  case TK_EQ_OP:
    return "==";
  case TK_NE_OP:
    return "!=";
  case TK_EOF:
    return "EOF";
  case TK_CHAR_LITERAL:
    return "char_literal";
  case TK_INT_LITERAL:
    return "int_literal";
  case TK_DOUBLE_LITERAL:
    return "double_literal";
  case TK_STRING_LITERAL:
    return "string_literal";
  case TK_BREAK:
    return "break";
  case TK_CASE:
    return "case";
  case TK_CONTINUE:
    return "continue";
  case TK_DO:
    return "do";
  case TK_DEFAULT:
    return "default";
  case TK_ELSE:
    return "else";
  case TK_ENUM:
    return "enum";
  case TK_FOR:
    return "for";
  case TK_GOTO:
    return "goto";
  case TK_IF:
    return "if";
  case TK_IMAGINARY:
    return "imaginary";
  case TK_RETURN:
    return "return";
  case TK_SIZEOF:
    return "sizeof";
  case TK_STRUCT:
    return "struct";
  case TK_SWITCH:
    return "switch";
  case TK_TYPEDEF:
    return "typeof";
  case TK_UNION:
    return "union";
  case TK_WHILE:
    return "while";
  case TK_AUTO:
    return "auto";
  case TK_CONST:
    return "const";
  case TK_EXTERN:
    return "extern";
  case TK_INLINE:
    return "inline";
  case TK_RESTRICT:
    return "restrict";
  case TK_VOLATILE:
    return "volatile";
  case TK_SIGNED:
    return "signed";
  case TK_UNSIGNED:
    return "unsigned";
  case TK_STATIC:
    return "static";
  case TK_REGISTER:
    return "register";

  case TK_IDENTIFIER:
    return "identifier";
  case TK_BOOL:
    return "bool";
  case TK_CHAR:
    return "char";
  case TK_COMPLEX:
    return "complex";
  case TK_DOUBLE:
    return "double";
  case TK_FLOAT:
    return "float";
  case TK_INT:
    return "int";
  case TK_LONG:
    return "long";
  case TK_SHORT:
    return "short";
  case TK_VOID:
    return "void";
  }
  return "unknown";
}
