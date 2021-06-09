%option noyywrap
%{
#include "util.h"
#include "token.h"
#include <stdio.h>

void comment() {
  char c, prev = 0;
  while (EOF != (c = getc(yyin))) {
    if (c == '/' && prev == '*')
      return;
    prev = c;
  }
  error("unterminated comment");
}
char *p;
%}
D             [0-9]
L             [a-zA-Z_]
H             [a-fA-F0-9]
E             ([Ee][+-]?{D}+)
P             ([Pp][+-]?{D}+)
FS            (f|F|l|L)
IS            ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))
%%
"/*"          {comment();}
"//"[^\n]*    {/* consume //-comment */}

"auto"        {return(TK_AUTO);}
"_Bool"       {return(TK_BOOL);}
"break"       {return(TK_BREAK);}
"case"        {return(TK_CASE);}
"char"        {return(TK_CHAR);}
"_Complex"    {return(TK_COMPLEX);}
"const"       {return(TK_CONST);}
"continue"    {return(TK_CONTINUE);}
"default"     {return(TK_DEFAULT);}
"do"          {return(TK_DO);}
"double"      {return(TK_DOUBLE);}
"else"        {return(TK_ELSE);}
"enum"        {return(TK_ENUM);}
"extern"      {return(TK_EXTERN);}
"float"       {return(TK_FLOAT);}
"for"         {return(TK_FOR);}
"goto"        {return(TK_GOTO);}
"if"          {return(TK_IF);}
"_Imaginary"  {return(TK_IMAGINARY);}
"inline"      {return(TK_INLINE);}
"int"         {return(TK_INT);}
"long"        {return(TK_LONG);}
"register"    {return(TK_REGISTER);}
"restrict"    {return(TK_RESTRICT);}
"return"      {return(TK_RETURN);}
"short"       {return(TK_SHORT);}
"signed"      {return(TK_SIGNED);}
"sizeof"      {return(TK_SIZEOF);}
"static"      {return(TK_STATIC);}
"struct"      {return(TK_STRUCT);}
"switch"      {return(TK_SWITCH);}
"typedef"     {return(TK_TYPEDEF);}
"union"       {return(TK_UNION);}
"unsigned"    {return(TK_UNSIGNED);}
"void"        {return(TK_VOID);}
"volatile"    {return(TK_VOLATILE);}
"while"       {return(TK_WHILE);}

{L}({L}|{D})*           {yylval.sval = ywstrCopy(yytext); return TK_IDENTIFIER; }

0[xX]{H}+{IS}?          {sscanf(yytext, "%x", &yylval.ival); return TK_INT_LITERAL;}
0[0-7]*{IS}?            {sscanf(yytext, "%o", &yylval.ival); return TK_INT_LITERAL;}
[1-9]{D}*{IS}?          {sscanf(yytext, "%d", &yylval.ival); return TK_INT_LITERAL; }
L?'(\\.|[^\\'\n])+'     {p = yytext; p++; if ('\'' == *p) p++; sscanf(p, "%c", &yylval.cval); return TK_CHAR_LITERAL;}

{D}+{E}{FS}?            {sscanf(yytext, "%lf", &yylval.dval); return TK_DOUBLE_LITERAL; }
{D}*"."{D}+{E}?{FS}?    {sscanf(yytext, "%lf", &yylval.dval); return TK_DOUBLE_LITERAL; }
{D}+"."{D}*{E}?{FS}?    {sscanf(yytext, "%lf", &yylval.dval); return TK_DOUBLE_LITERAL; }

L?\"(\\.|[^\\"\n])*\"   {p = yytext; p++; if ('"' == *p) p++; p[strlen(p)-1]='\0'; yylval.sval = ywstrCopy(p); return TK_STRING_LITERAL;}

"..."       {return(TK_ELLIPSIS);}
">>="       {return(TK_RIGHT_ASSIGN);}
"<<="       {return(TK_LEFT_ASSIGN);}
"+="        {return(TK_ADD_ASSIGN);}
"-="        {return(TK_SUB_ASSIGN);}
"*="        {return(TK_MUL_ASSIGN);}
"/="        {return(TK_DIV_ASSIGN);}
"%="        {return(TK_MOD_ASSIGN);}
"&="        {return(TK_AND_ASSIGN);}
"^="        {return(TK_XOR_ASSIGN);}
"|="        {return(TK_OR_ASSIGN);}
">>"        {return(TK_RIGHT_OP);}
"<<"        {return(TK_LEFT_OP);}
"++"        {return(TK_INC_OP);}
"--"        {return(TK_DEC_OP);}
"->"        {return(TK_PTR_OP);}
"&&"        {return(TK_AND_OP);}
"||"        {return(TK_OR_OP);}
"<="        {return(TK_LE_OP);}
">="        {return(TK_GE_OP);}
"=="        {return(TK_EQ_OP);}
"!="        {return(TK_NE_OP);}
";"         {return ';';}
("{"|"<%")  {return '{';}
("}"|"%>")  {return '}';}
","         {return ',';}
":"         {return ':';}
"="         {return '=';}
"("         {return '(';}
")"         {return ')';}
("["|"<:")  {return '[';}
("]"|":>")  {return ']';}
"."         {return '.';}
"&"         {return '&';}
"!"         {return '!';}
"~"         {return '~';}
"-"         {return '-';}
"+"         {return '+';}
"*"         {return '*';}
"/"         {return '/';}
"%"         {return '%';}
"<"         {return '<';}
">"         {return '>';}
"^"         {return '^';}
"|"         {return '|';}
"?"         {return '?';}

[ \t\v\n\f] {continue;}
<<EOF>>     {return TK_EOF;}
.           {error("illeague token\n");}
%%

