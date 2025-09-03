%{
    /* Definimos estandar POSIX que se usara para compilar el código.*/
    #define _POSIX_C_SOURCE 200809L

    /* Inclusión de bibliotecas estandar de C y el archivo Bison */
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "calc-sintaxis.tab.h"
%}

/* Opciones de Flex */
%option noyywrap
%option yylineno

/* Expresiones regulares */
letra   [a-zA-Z]
digito  [0-9]
numero  {digito}+
id      {letra}({letra}|{digito})*

/* Definicion de tokens */
%%

"int"       { return INT; }
"bool"      { return BOOL; }
"void"      { return VOID; }
"return"    { return RETURN; }
"main"      { return MAIN; }
"&&"        { return OP_AND; }
"||"        { return OP_OR; }
"=="        { return OP_IGUAL; }
"true"      { return TRUE; }
"false"     { return FALSE; }
{numero}    { yylval.num = atoi(yytext); return NUMERO; }
{id}        { yylval.str = strdup(yytext); if (!yylval.str) { perror("strdup"); exit(1); } return ID; }
"-"         { return OP_RESTA; }
"+"         { return OP_SUMA; }
"("         { return PARA; }
")"         { return PARC; }
"{"         { return LLAA; }
"}"         { return LLAC; }
"["         { return CORA; }
"]"         { return CORC; }
"="         { return OP_ASIGN; }
"*"         { return OP_MULT; }
"/"         { return OP_DIV; }
";"         { return PYC; }
","         { return COMA; }
">"         { return OP_MAYOR; }
"<"         { return OP_MENOR; }

[ \t\r\n]+    ;   /* Ignorar espacios en blanco */
.           ;   /* Ignorar caracteres no reconocidos */

%%

/* Función para errores sintácticos */
void yyerror(const char *s) {
    fprintf(stderr, "-> ERROR Sintactico en la linea %d: %s\n", yylineno, s);
}
