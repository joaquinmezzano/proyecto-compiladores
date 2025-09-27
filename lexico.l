%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include "sintaxis.tab.h"

extern YYSTYPE yylval;

void reportar_error(const char *msg, const char *text, int lineno);
%}

/* Opciones de Flex */
%option noyywrap
%option yylineno

/* Definiciones */
DIGIT       [0-9]
ALPHA       [a-zA-Z_]
ALPHA_NUM   [a-zA-Z0-9_]

%%

[ \t\r\n]+                      { /* Ignorar espacios en blanco, tabulaciones y saltos de línea */ }

"//".*                          { /* Ignorar comentario de línea */ }
"/*"([^*]|\n|(\*+[^*/]))*"*/"   { /* ignorar comentario multilínea */ }
"/*"([^*]|\n|(\*+[^*/]))*       { reportar_error("Comentario sin cerrar", yytext, yylineno); }

"integer"                    { return INTEGER; }
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
">="                         { return OP_MAYORIG; }
"<="                         { return OP_MENORIG; }
"!="                         { return OP_DESIGUAL; }
"=="                         { return OP_COMP; }

"&&"                         { return OP_AND; }
"||"                         { return OP_OR; }
"!"                          { return OP_NOT; }

";"                          { return PYC; }
","                          { return COMA; }

[-]?{DIGIT}+                 {
                                errno = 0;
                                long long val = strtol(yytext, NULL, 10);

                                if (errno == ERANGE || val < INT32_MIN || val > INT32_MAX) {
                                    reportar_error("Literal entero fuera de rango (se espera int32 con signo)", yytext, yylineno);
                                } else {
                                    yylval.ival = (int)val;
                                    return INTEGER_LITERAL;
                                }
                             }

{ALPHA}{ALPHA_NUM}*          { yylval.sval = strdup(yytext); return ID; }

.                            { reportar_error("Caracter inesperado", yytext, yylineno); }

%%

/* Función para reportar un error */
void reportar_error(const char *msg, const char *text, int lineno) {
    fprintf(stderr, "Error léxico en línea %d: %s: '%s'\n", lineno, msg, text);
}