%{
    /* Incluir bibliotecas de C y declarar funciones */ 
    #include <stdio.h>
    #include <stdlib.h>

    int yylex(void);
    void yyerror(const char *s);
%}

/* Definiciones de los tokens */
%token INT BOOL PROGRAM VOID EXTERN IF THEN ELSE WHILE RETURN TRUE FALSE
%token PARA PARC CORA CORC LLAA LLAC
%token OP_RESTA OP_SUMA OP_MULT OP_DIV OP_RESTO OP_IGUAL
%token OP_MAYOR OP_MENOR OP_COMP OP_AND OP_OR OP_NOT
%token PYC COMA
%token ID INTEGER_LITERAL

/* Definiciones de precedencia */
%left OP_OR
%left OP_AND
%left OP_MAYOR OP_MENOR OP_COMP
%left OP_SUMA OP_RESTA
%left OP_MULT OP_DIV OP_RESTO
%right OP_NOT

%%
program
    :PROGRAM LLAA var_decl_list method_decl_list LLAC
    ;

var_decl_list
    : /* empty */
    | var_decl_list var_decl
    ;

var_decl
    :TYPE ID OP_IGUAL expr PYC
    ;

method_decl_list
    : /* empty */
    | method_decl_list method_decl
    ;

method_decl
    : TYPE ID PARA param_list_opt PARC block
    | VOID ID PARA param_list_opt PARC block
    | TYPE ID PARA param_list_opt PARC EXTERN PYC
    | VOID ID PARA param_list_opt PARC EXTERN PYC
    ;

param_list_opt
    : /* empty */
    | param_list
    ;

param_list
    : TYPE ID
    | param_list COMA TYPE ID
    ;

block
    : LLAA var_decl_list statement_list LLAC
    ;

statement_list
    : /* empty */
    | statement_list statement
    ;

statement
    : ID OP_IGUAL expr PYC
    | method_call PYC
    | IF PARA expr PARC THEN block else_opt
    | WHILE expr block
    | RETURN expr_opt PYC
    | PYC
    | block
    ;

else_opt
    : /* empty */
    | ELSE block
    ;

expr_opt
    : /* empty */
    | expr
    ;

method_call
    : ID PARA arg_list_opt PARC
    ;

arg_list_opt
    : /* empty */
    | arg_list
    ;

arg_list
    : expr
    | arg_list COMA expr
    ;

expr
    : ID
    | method_call
    | literal
    | expr bin_op expr
    | OP_RESTA expr
    | OP_NOT expr
    | PARA expr PARC
    ;

bin_op
    : arith_op
    | rel_op
    | cond_op
    ;

arith_op
    : OP_SUMA
    | OP_RESTA
    | OP_MULT
    | OP_DIV
    | OP_RESTO
    ;

rel_op
    : OP_MAYOR
    | OP_MENOR
    | OP_COMP
    ;

cond_op
    : OP_AND
    | OP_OR
    ;

literal
    : INTEGER_LITERAL
    | TRUE
    | FALSE
    ;

TYPE
    : INT
    | BOOL
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis: %s\n", s);
}

/* Main */