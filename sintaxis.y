%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Declarar variables del lexer */
extern int yylineno;
extern char *yytext;

void yyerror(const char *s);
int yylex(void);

Nodo *ast = NULL; // Variable global para guardar el AST
%}

/* Habilitar mensajes de error detallados */
%define parse.error verbose

/* Unión para los valores de los tokens */
%union {
    int32_t ival;
    char *sval;
    struct Nodo *node;   /* Usar struct Nodo* */
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

/* Declarar tipos de las reglas que producen Nodo* */
%type <node> program decl_list decl var_decl_list var_decl method_decl param_list_opt param_list
%type <node> block statement_list statement else_opt expr_opt arg_list_opt arg_list expr TYPE method_call

%%

program
    : PROGRAM LLAA decl_list LLAC
      {
          Nodo *prog = nodo_ID("program");
          prog->siguiente = $3;  // Enlaza lista de declaraciones (métodos)
          ast = prog;
          $$ = prog;
      }
    ;

decl_list
    : /* empty */ { $$ = NULL; }
    | decl_list decl { $$ = $1 ? $1 : $2; if ($1) $1->siguiente = $2; }
    ;

decl
    : var_decl   { $$ = $1; }
    | method_decl { $$ = $1; }
    ;

var_decl_list
    : /* empty */ { $$ = NULL; }
    | var_decl_list var_decl { $$ = $1 ? $1 : $2; if ($1) $1->siguiente = $2; }
    ;

var_decl
    : TYPE ID OP_IGUAL expr PYC { $$ = nodo_decl($2, $4); }
    ;

method_decl
    : TYPE ID PARA param_list_opt PARC block 
      { 
          $$ = nodo_method($2, $4, $6);  // Método con tipo, params y cuerpo
      }
    | VOID ID PARA param_list_opt PARC block 
      { 
          $$ = nodo_method($2, $4, $6);  // Método void con params y cuerpo
      }
    | TYPE ID PARA param_list_opt PARC EXTERN PYC 
      { 
          $$ = nodo_method($2, $4, NULL);  // Método extern sin cuerpo
      }
    | VOID ID PARA param_list_opt PARC EXTERN PYC 
      { 
          $$ = nodo_method($2, $4, NULL);  // Método void extern sin cuerpo
      }
    ;

param_list_opt
    : /* empty */ { $$ = NULL; }
    | param_list { $$ = $1; }
    ;

param_list
    : TYPE ID { $$ = nodo_ID($2); }  // Un parámetro
    | param_list COMA TYPE ID { $$ = $1; if ($1) $1->siguiente = nodo_ID($4); }  // Lista de parámetros
    ;

block
    : LLAA var_decl_list statement_list LLAC
      {
          $$ = $3;  // Statements como raíz del bloque
          if ($2) {  // Si hay declaraciones de variables, enlazarlas antes
              Nodo *last = $2;
              while (last->siguiente) last = last->siguiente;
              last->siguiente = $3;
              $$ = $2;
          }
      }
    ;

statement_list
    : /* empty */ { $$ = NULL; }
    | statement_list statement { $$ = $1 ? $1 : $2; if ($1) $1->siguiente = $2; }
    ;

statement
    : ID OP_IGUAL expr PYC { $$ = nodo_assign($1, $3); }
    | method_call PYC { $$ = $1; }  // Llamadas a métodos
    | IF PARA expr PARC THEN block else_opt { $$ = nodo_if($3, $6, $7); }  // IF con condición y bloques
    | WHILE expr block { $$ = nodo_ID("while"); }  // Simplificado (puedes expandir si necesitas)
    | RETURN expr_opt PYC { $$ = nodo_return($2); }
    | PYC { $$ = NULL; }
    | block { $$ = $1; }
    ;

else_opt
    : /* empty */ { $$ = NULL; }
    | ELSE block { $$ = $2; }
    ;

expr_opt
    : /* empty */ { $$ = NULL; }
    | expr { $$ = $1; }
    ;

arg_list_opt
    : /* empty */ { $$ = NULL; }
    | arg_list { $$ = $1; }
    ;

arg_list
    : expr { $$ = $1; }
    | arg_list COMA expr { $$ = $1; if ($1) $1->siguiente = $3; }
    ;

expr
    : INTEGER_LITERAL { $$ = nodo_integer($1); }
    | TRUE { $$ = nodo_bool(1); }
    | FALSE { $$ = nodo_bool(0); }
    | ID { $$ = nodo_ID($1); }
    | method_call { $$ = $1; }  // Captura llamadas a métodos como expr
    | PARA expr PARC { $$ = $2; }
    | OP_RESTA expr %prec UMINUS { $$ = nodo_op(TOP_RESTA, nodo_integer(0), $2); }
    | OP_NOT expr { $$ = nodo_op(TOP_NOT, NULL, $2); }
    | expr OP_SUMA expr { $$ = nodo_op(TOP_SUMA, $1, $3); }
    | expr OP_RESTA expr { $$ = nodo_op(TOP_RESTA, $1, $3); }
    | expr OP_MULT expr { $$ = nodo_op(TOP_MULT, $1, $3); }
    | expr OP_DIV expr { $$ = nodo_op(TOP_DIV, $1, $3); }
    | expr OP_RESTO expr { $$ = nodo_op(TOP_RESTO, $1, $3); }
    | expr OP_MAYOR expr { $$ = nodo_op(TOP_MAYOR, $1, $3); }
    | expr OP_MENOR expr { $$ = nodo_op(TOP_MENOR, $1, $3); }
    | expr OP_MAYORIG expr { $$ = nodo_op(TOP_MAYORIG, $1, $3); }
    | expr OP_MENORIG expr { $$ = nodo_op(TOP_MENORIG, $1, $3); }
    | expr OP_DESIGUAL expr { $$ = nodo_op(TOP_DESIGUAL, $1, $3); }
    | expr OP_COMP expr { $$ = nodo_op(TOP_COMP, $1, $3); }
    | expr OP_AND expr { $$ = nodo_op(TOP_AND, $1, $3); }
    | expr OP_OR expr { $$ = nodo_op(TOP_OR, $1, $3); }
    ;

method_call
    : ID PARA arg_list_opt PARC 
      { 
          $$ = nodo_method_call($1, $3);  // Crea nodo para llamada a método
      }
    ;

TYPE
    : INTEGER { $$ = nodo_ID("integer"); }
    | BOOL { $$ = nodo_ID("bool"); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d cerca de '%s': %s\n",
            yylineno, yytext, s);
}

int main(int argc, char **argv) {
    if (yyparse() == 0) {
        printf("Análisis sintáctico completado sin errores.\n");
        imprimir_nodo(ast, 0);
        nodo_libre(ast);
    } else {
        printf("Análisis sintáctico fallido.\n");
    }
    return 0;
}