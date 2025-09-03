%{
    /* Include y definiciones de constantes. */
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
%}

/* Opciones de Flex */

/* Expresiones regulares */
alpha [a-ZA-Z]
digit [0-9]
alpha_num ({alpha}|{digit}|{_})
id {alpha}{alpha_num}*
integer_literal {digit}{digit}*

%%
"integer"   { return INT; }
"bool"      { return BOOL; }
"Program"   { return PROGRAM; }
"void"      { return VOID; }
"extern"    { return EXTERN; }
"if"        { return IF; }
"then"      { return THEN; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"return"    { return RETURN; }
"true"      { return TRUE; }
"false"     { return FALSE; }

"("         { return PARA; }
")"         { return PARC; }
"{"         { return LLAA; }
"}"         { return LLAC; }
"["         { return CORA; }
"]"         { return CORC; }

"-"         { return OP_RESTA; }
"+"         { return OP_SUMA; }
"*"         { return OP_MULT; }
"/"         { return OP_DIV; }
"%"         { return OP_RESTO; }
"="         { return OP_IGUAL; }

">"         { return OP_MAYOR; }
"<"         { return OP_MENOR; }
"=="        { return OP_COMP; }
"||"        { return OP_OR; }
"&&"        { return OP_AND; }

";"         { return PYC; }
","         { return COMA; }

{id}                { return ID; }
{integer_literal}   { return INTEGER_LITERAL; }
("true"|"false"}    { return BOOL_LITERAL; }

[ \t\r\n]+    ;   /* Ignorar espacios en blanco */
.           ;   /* Ignorar caracteres no reconocidos */
%%

/* Función para errores sintácticos */