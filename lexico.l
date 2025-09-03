%{
    /* Include y definiciones de constantes. */
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "sintaxis.tab.h"
%}

/* Opciones de Flex */

/* Expresiones regulares */
alpha [a-zA-Z]
digit [0-9]
alpha_num ({alpha}|{digit}|\-)
id {alpha}{alpha_num}*
integer_literal {digit}{digit}*

%%
"integer"   { printf("INT (%s)\n", yytext); return INT; }
"bool"      { printf("BOOL (%s)\n", yytext); return BOOL; }
"program"   { printf("PROGRAM (%s)\n", yytext); return PROGRAM; }
"void"      { printf("VOID (%s)\n", yytext); return VOID; }
"extern"    { printf("EXTERN (%s)\n", yytext); return EXTERN; }
"if"        { printf("IF (%s)\n", yytext); return IF; }
"then"      { printf("THEN (%s)\n", yytext); return THEN; }
"else"      { printf("ELSE (%s)\n", yytext); return ELSE; }
"while"     { printf("WHILE (%s)\n", yytext); return WHILE; }
"return"    { printf("RETURN (%s)\n", yytext); return RETURN; }
"true"      { printf("TRUE (%s)\n", yytext); return TRUE; }
"false"     { printf("FALSE (%s)\n", yytext); return FALSE; }

"("         { printf("PARA (%s)\n", yytext); return PARA; }
")"         { printf("PARC (%s)\n", yytext); return PARC; }
"{"         { printf("LLAA (%s)\n", yytext); return LLAA; }
"}"         { printf("LLAC (%s)\n", yytext); return LLAC; }
"["         { printf("CORA (%s)\n", yytext); return CORA; }
"]"         { printf("CORC (%s)\n", yytext); return CORC; }

"-"         { printf("OP_RESTA (%s)\n", yytext); return OP_RESTA; }
"+"         { printf("OP_SUMA (%s)\n", yytext); return OP_SUMA; }
"*"         { printf("OP_MULT (%s)\n", yytext); return OP_MULT; }
"/"         { printf("OP_DIV (%s)\n", yytext); return OP_DIV; }
"%"         { printf("OP_RESTO (%s)\n", yytext); return OP_RESTO; }
"="         { printf("OP_IGUAL (%s)\n", yytext); return OP_IGUAL; }

">"         { printf("OP_MAYOR (%s)\n", yytext); return OP_MAYOR; }
"<"         { printf("OP_MENOR (%s)\n", yytext); return OP_MENOR; }
"=="        { printf("OP_COMP (%s)\n", yytext); return OP_COMP; }
"||"        { printf("OP_OR (%s)\n", yytext); return OP_OR; }
"&&"        { printf("OP_AND (%s)\n", yytext); return OP_AND; }
"!"         { printf("OP_NOT (%s)\n", yytext); return OP_NOT; }

";"         { printf("PYC (%s)\n", yytext); return PYC; }
","         { printf("COMA (%s)\n", yytext); return COMA; }

{id}                { printf("ID (%s)\n", yytext); return ID; }
{integer_literal}   { printf("INTEGER_LITERAL (%s)\n", yytext); return INTEGER_LITERAL; }

[ \t\r\n]+    ;   /* Ignorar espacios en blanco */
.           ;   /* Ignorar caracteres no reconocidos */
%%

/* Función para errores sintácticos */