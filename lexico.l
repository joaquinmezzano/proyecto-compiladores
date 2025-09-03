%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sintaxis.tab.h"

extern YYSTYPE yylval;
%}

%option noyywrap

/* Opciones de Flex */

/* Expresiones regulares */
alpha [a-zA-Z]
digit [0-9]
alpha_num ({alpha}|{digit}|\-)
id {alpha}{alpha_num}*
integer_literal {digit}{digit}*

%%
[ \t\r\n]+                   { /* ignorar espacios en blanco */ }

"//".*                       { /* comentario de línea */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* comentario multilínea */ }

"int"                        { return INT; }
"bool"                       { return BOOL; }
"program"                    { return PROGRAM; }
"void"                       { return VOID; }
"extern"                     { return EXTERN; }
"if"                         { return IF; }
"then"                       { return THEN; }
"else"                       { return ELSE; }
"while"                      { return WHILE; }
"return"                     { return RETURN; }
"true"                       { return TRUE; }
"false"                      { return FALSE; }

"("                          { return PARA; }
")"                          { return PARC; }
"["                          { return CORA; }
"]"                          { return CORC; }
"{"                          { return LLAA; }
"}"                          { return LLAC; }

"+"                          { return OP_SUMA; }
"-"                          { return OP_RESTA; }
"*"                          { return OP_MULT; }
"/"                          { return OP_DIV; }
"%"                          { return OP_RESTO; }
"="                          { return OP_IGUAL; }

">"                          { return OP_MAYOR; }
"<"                          { return OP_MENOR; }
"=="                         { return OP_COMP; }

"&&"                         { return OP_AND; }
"||"                         { return OP_OR; }
"!"                          { return OP_NOT; }

";"                          { return PYC; }
","                          { return COMA; }

[0-9]+                       { yylval.ival = atoi(yytext); return INTEGER_LITERAL; }
[a-zA-Z_][a-zA-Z0-9_]*       { yylval.sval = strdup(yytext); return ID; }

.                            { printf("Caracter inesperado: %s\n", yytext); }

%%
