%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declarar variables del lexer */
extern int yylineno;   // Línea actual
extern char *yytext;   // Token actual

void yyerror(const char *s);
int yylex(void);
%}

/* Habilitar mensajes de error detallados */
%define parse.error verbose

/* Unión para los valores de los tokens */
%union {
    int32_t ival;
    char *sval;
}

/* Tokens con tipo */
%token <ival> INTEGER_LITERAL
%token <sval> ID
%token INTEGER BOOL PROGRAM VOID EXTERN IF THEN ELSE WHILE RETURN TRUE FALSE
%token PARA PARC CORA CORC LLAA LLAC
%token OP_RESTA OP_SUMA OP_MULT OP_DIV OP_RESTO OP_IGUAL
%token OP_MAYOR OP_MENOR OP_MAYORIG OP_MENORIG OP_DESIGUAL OP_COMP OP_AND OP_OR OP_NOT
%token PYC COMA

/* Precedencias */
%left OP_OR
%left OP_AND
%left OP_MAYOR OP_MENOR OP_MAYORIG OP_MENORIG OP_DESIGUAL OP_COMP
%left OP_SUMA OP_RESTA
%left OP_MULT OP_DIV OP_RESTO
%right OP_NOT
%right UMINUS

%%

program
    : PROGRAM LLAA decl_list LLAC
    ;

decl_list
    : /* empty */
    | decl_list decl
    ;

decl
    : var_decl
    | method_decl
    ;

var_decl_list
    : /* empty */
    | var_decl_list var_decl
    ;

var_decl
    : TYPE ID OP_IGUAL expr PYC
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
    | ID PARA arg_list_opt PARC PYC   /* method call */
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

arg_list_opt
    : /* empty */
    | arg_list
    ;

arg_list
    : expr
    | arg_list COMA expr
    ;

expr
    : INTEGER_LITERAL
    | TRUE
    | FALSE
    | ID
    | ID PARA arg_list_opt PARC   /* method_call */
    | PARA expr PARC
    | OP_RESTA expr %prec UMINUS
    | OP_NOT expr
    | expr OP_SUMA expr
    | expr OP_RESTA expr
    | expr OP_MULT expr
    | expr OP_DIV expr
    | expr OP_RESTO expr
    | expr OP_MAYOR expr
    | expr OP_MENOR expr
    | expr OP_MAYORIG expr
    | expr OP_MENORIG expr
    | expr OP_DESIGUAL expr
    | expr OP_COMP expr
    | expr OP_AND expr
    | expr OP_OR expr
    ;

TYPE
    : INTEGER
    | BOOL
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d cerca de '%s': %s\n",
            yylineno, yytext, s);
}

int main(int argc, char **argv) {
    if (yyparse() == 0) {
        printf("Análisis sintáctico completado sin errores.\n");
    } else {
        printf("Análisis sintáctico fallido.\n");
    }
    return 0;
}