%{
    /* Incluir bibliotecas de C y declarar funciones */ 
    #include <stdio.h>
    #include <stdlib.h>   
%}

/* Definiciones de los tokens */
%token INT, BOOL, PROGRAM, VOID, EXTERN, IF, THEN, ELSE, WHILE, RETURN, TRUE, FALSE
%token PARA, PARC, CORA, CORC, LLAA, LLAC
%token OP_RESTA, OP_SUMA, OP_MULT, OP_DIV, OP_RESTO, OP_IGUAL
%token OP_MAYOR, OP_MENOR, OP_COMP, OP_AND, OP_OR
%token PYC, COMA
%token ID, INTEGER_LITERAL, BOOL_LITERAL

/* Definiciones de precedencia */

%%
program: PROGRAM '{' VAR_DECL* METHOD_DECL* '}'

VAR_DECL: TYPE ID OP_IGUAL E PYC
METHOD_DECL: (TYPE|VOID) ID ([{TYPE ID}+,]) BLOCK
        | (TYPE|VOID) ID ([{TYPE ID}+,]) EXTERN PYC

BLOCK: '{' VAR_DECL* STATEMENT '}'
TYPE: INT | BOOL

STATEMENT: ID OP_IGUAL E PYC
        | METHOD_CALL PYC
        | IF PARA E PARC THEN BLOCK [ELSE BLOCK] PYC
        | WHILE E BLOCK
        | RETURN [E] PYC
        | PYC
        | BLOCK

METHOD_CALL: ID PARA [E+] PARC

E: ID
    | METHOD_CALL
    | LITERAL
    | E BIN_OP E
    | -E
    | !E
    | PARA E PARC

BIN_OP: ARITH_OP | REL_OP | COND_OP
ARITH_OP: OP_SUMA | OP_RESTA | OP_MULT | OP_DIV | OP_RESTO
REL_OP: OP_MAYOR | OP_MENOR | OP_COMP
COND_OP: OP_AND | OP_OR

LITERAL: INTEGER_LITERAL | BOOL_LITERAL

%%

/* Main */