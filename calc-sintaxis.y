/* Importamos las definiciones de las funciones y estructuras */
%code requires {
    #include "ast.h"
}

/* Incluimos bibliotecas de C y declaramos funciones*/
%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex(void);
extern void yyerror(const char *s);
%}

/* Definición de la union */
%union {
    Nodo *nodo;
    char *str;
    int num;
}

/* Definición de los tokens */
%token <str> ID
%token <num> NUMERO
%token INT BOOL VOID TRUE FALSE RETURN MAIN
%token PARA PARC LLAA LLAC CORA CORC PYC COMA
%token OP_RESTA OP_SUMA OP_MAYOR OP_MENOR OP_AND OP_OR OP_DIV OP_MULT OP_IGUAL OP_ASIGN

/* Asociamos los tipos de los nodos con las producciones */
%type <nodo> prog CODIGO SENTENCIA E EB DECLARACION VAR VARS

/* Definimos precedencia de operadores */
%left OP_OR
%left OP_AND
%left OP_SUMA OP_RESTA
%left OP_MULT OP_DIV

/* Reglas de producción */
%%
prog:
    TIPOM MAIN PARA PARC LLAA CODIGO LLAC {
        /* Exporta el AST a DOT y genera PNG */
        exportar_dot($6, "ast_tree");

        /* Genera seudo-assembly */
        printf("\n----------------------------------------\n");
        generar_asm($6, "programa.sasm");
        printf("Seudo-assembly escrito en 'programa.sasm'\n");
        printf("----------------------------------------\n");

        /* Evalúa directamente el nodo raíz */
        int resultado_directo = eval_nodo($6);
        printf("Resultado (eval_nodo): %d\n", resultado_directo);

        /* Interpreta el programa completo */
        int resultado_completo = interpretar_programa($6);

        /* Libera memoria del AST */
        nodo_libre($6);

        /* Libera tabla de símbolos */
        ast_liberar_recursos();

        /* Imprime resultado final */
        printf("┌───────────────────────────────┐\n");
        printf("│ Resultado del Programa        │\n");
        printf("└───────────────────────────────┘\n");
        printf("     %d\n", resultado_completo);
        printf("     ✔ No hay errores\n\n");
    }
;

TIPOM:
    INT
  | BOOL
  | VOID
;

CODIGO:
    %empty              { $$ = NULL; }
  | DECLARACION CODIGO  {
        $$ = nodo_seq($1, $2);
    }
  | SENTENCIA CODIGO    {
        $$ = nodo_seq($1, $2);
    }
;

DECLARACION:
    TIPO VARS PYC { $$ = $2; }
;

VARS:
    VAR             { $$ = $1; }
  | VAR COMA VARS   { $$ = nodo_seq($1, $3); }
;

VAR:
    ID                    { $$ = nodo_decl($1, NULL); free($1); }
  | ID OP_ASIGN E        { $$ = nodo_decl($1, $3); free($1); }
  | ID OP_ASIGN EB       { $$ = nodo_decl($1, $3); free($1); }
;

TIPO:
    INT
  | BOOL
;

SENTENCIA:
    ID OP_ASIGN E PYC     { $$ = nodo_assign($1, $3); free($1); }
  | ID OP_ASIGN EB PYC    { $$ = nodo_assign($1, $3); free($1); }
  | RETURN PYC            { $$ = nodo_return(NULL); }
  | RETURN E PYC          { $$ = nodo_return($2); }
  | RETURN EB PYC         { $$ = nodo_return($2); }
;

E:
    E OP_SUMA E     { $$ = nodo_opBin(TOP_SUMA, $1, $3); }
  | E OP_RESTA E    { $$ = nodo_opBin(TOP_RESTA, $1, $3); }
  | E OP_MULT E     { $$ = nodo_opBin(TOP_MULT, $1, $3); }
  | E OP_DIV E      { $$ = nodo_opBin(TOP_DIV, $1, $3); }
  | PARA E PARC     { $$ = $2; }
  | ID              { $$ = nodo_ID($1); free($1); }
  | NUMERO          { $$ = nodo_int($1); }
;

EB:
    EB OP_OR EB     { $$ = nodo_opBin(TOP_OR, $1, $3); }
  | EB OP_AND EB    { $$ = nodo_opBin(TOP_AND, $1, $3); }
  | E OP_IGUAL E    { $$ = nodo_opBin(TOP_IGUAL, $1, $3); }
  | E OP_MAYOR E    { $$ = nodo_opBin(TOP_MAYOR, $1, $3); }
  | E OP_MENOR E    { $$ = nodo_opBin(TOP_MENOR, $1, $3); }
  | PARA EB PARC    { $$ = $2; }
  | TRUE            { $$ = nodo_bool(1); }
  | FALSE           { $$ = nodo_bool(0); }
;

%%

int main(int argc, char *argv[]) {
    extern FILE *yyin;
    ++argv, --argc;

    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;

    yyparse();
    return 0;
}