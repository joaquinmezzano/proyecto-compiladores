%{
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ast.h"
#include "symtab.h"
#include "semantics.h"
#include "intermediate.h"
#include "object.h"
#include "optimizer.h"

/*
 * Declarar variables del lexer
 */
extern int yylineno;
extern char *yytext;

void yyerror(const char *s);
int yylex(void);

Nodo *ast = NULL;
int debug_mode = 0;
int optimizer_enabled = 0;
typedef enum {
    TARGET_SEMANTIC,    // Hasta análisis semántico (incluye AST + optimizaciones)
    TARGET_IR,          // Hasta código intermedio
    TARGET_OBJECT,      // Hasta código objeto (completo)
    TARGET_ALL          // Alias para TARGET_OBJECT
} target_stage_t;

target_stage_t target_stage = TARGET_ALL;
%}

/*
 * Habilitar mensajes de error detallados
 */
%define parse.error verbose

/*
 * Unión para los valores de los tokens
 */
%union {
    int32_t ival;
    char *sval;
    struct Nodo *node;
}

/*
 * Tokens con tipo
 */
%token <sval> ID
%token <ival> INTEGER_LITERAL
%token <ival> BOOL_LITERAL
%token INTEGER BOOL PROGRAM VOID EXTERN IF THEN ELSE WHILE RETURN TRUE FALSE
%token PARA PARC CORA CORC LLAA LLAC
%token OP_RESTA OP_SUMA OP_MULT OP_DIV OP_RESTO OP_IGUAL
%token OP_MAYOR OP_MENOR OP_MAYORIG OP_MENORIG OP_DESIGUAL OP_COMP OP_AND OP_OR OP_NOT
%token PYC COMA

/*
 * Precedencias
 */
%left OP_OR
%left OP_AND
%left OP_MAYOR OP_MENOR OP_MAYORIG OP_MENORIG OP_DESIGUAL OP_COMP
%left OP_SUMA OP_RESTA
%left OP_MULT OP_DIV OP_RESTO
%right OP_NOT
%right UMINUS

/*
 * Declarar tipos de las reglas que producen Nodo*
 */
%type <node> program decl_list decl var_decl_list var_decl method_decl param_list_opt param_list
%type <node> block statement_list statement else_opt expr_opt arg_list_opt arg_list expr method_call
%type <node> TYPE method_body

%%

program
    : PROGRAM LLAA decl_list LLAC
      {
          Nodo *prog = nodo_ID("program");
          prog->siguiente = $3;
          ast = prog;
          $$ = prog;
      }
    ;

decl_list
    : /* empty */ { $$ = NULL; }
    | decl_list decl
      {
          if ($1) {
              Nodo *last = $1;
              while (last->siguiente) last = last->siguiente;
              last->siguiente = $2;
              $$ = $1;
          } else {
              $$ = $2;
          }
      }
    | decl_list statement
      {
          if ($1) {
              Nodo *last = $1;
              while (last->siguiente) last = last->siguiente;
              last->siguiente = $2;
              $$ = $1;
          } else {
              $$ = $2;
          }
      }
    ;

decl
    : var_decl    { $$ = $1; }
    | method_decl { $$ = $1; }
    ;

var_decl_list
    : /* empty */            { $$ = NULL; }
    | var_decl_list var_decl { 
        $$ = $1 ? $1 : $2; 
        if ($1) {
            Nodo *last = $1;
            while (last->siguiente) last = last->siguiente;
            last->siguiente = $2;
        }
    }
    ;

var_decl
    : TYPE ID OP_IGUAL expr PYC
      {
          if ($1 && $1->tipo == NODO_ID) {
              insert_symbol($2, $1->nombre, 0);
              nodo_libre($1);
          } else {
              insert_symbol($2, "unknown", 0);
          }
          $$ = nodo_decl($2, $4);
      }
    | TYPE ID PYC
      {
          if ($1 && $1->tipo == NODO_ID) {
              insert_symbol($2, $1->nombre, 0);
              nodo_libre($1);
          } else {
              insert_symbol($2, "unknown", 0);
          }
          $$ = nodo_decl($2, NULL);
      }
    ;

method_decl
    : TYPE ID PARA
      {
          if (strcmp($2, "main") == 0 && strcmp($1->nombre, "integer") != 0 && strcmp($1->nombre, "void") != 0) {
              fprintf(stderr, "Error semántico en línea %d: main debe retornar integer o void\n", yylineno);
              semantic_errors++;
          }
          char func_type[100];
          sprintf(func_type, "function:%s", $1->nombre);
          insert_symbol($2, func_type, 0);
          nodo_libre($1);
          push_scope_for_function($2);
      }
      param_list_opt PARC method_body
      {
          $$ = nodo_method($2, $5, $7);
          if ($7 != NULL) pop_scope();
      }
    | VOID ID PARA
      {
          insert_symbol($2, "function:void", 0);
          push_scope_for_function($2);
      }
      param_list_opt PARC method_body
      {
          $$ = nodo_method($2, $5, $7);
          if ($7 != NULL) pop_scope();
      }
    ;

method_body
    : block { $$ = $1; }
    | EXTERN PYC { 
        pop_scope();
        $$ = NULL; 
      }
    ;

param_list_opt
    : /* empty */ { $$ = NULL; }
    | param_list  { $$ = $1; }
    ;

param_list
    : TYPE ID
      {
          if ($1 && $1->tipo == NODO_ID) {
              insert_symbol($2, $1->nombre, 1);
              nodo_libre($1);
          } else {
              insert_symbol($2, "unknown", 1);
          }
          $$ = nodo_ID($2);
      }
    | param_list COMA TYPE ID
      {
          if ($3 && $3->tipo == NODO_ID) {
              insert_symbol($4, $3->nombre, 1);
              nodo_libre($3);
          } else {
              insert_symbol($4, "unknown", 1);
          }
          Nodo *last = $1;
          while (last && last->siguiente) {
              last = last->siguiente;
          }
          Nodo *new_param = nodo_ID($4);
          if (last) {
              last->siguiente = new_param;
          }
          $$ = $1;
      }
    ;

block
    : LLAA var_decl_list statement_list LLAC 
      { 
          Nodo *decls = $2;
          Nodo *stmts = $3;
          
          if (decls && stmts) {
              Nodo *last_decl = decls;
              while (last_decl->siguiente) last_decl = last_decl->siguiente;
              last_decl->siguiente = stmts;
              $$ = decls;
          } else if (decls) {
              $$ = decls;
          } else {
              $$ = stmts;
          }
      }
    ;

statement_list
    : /* empty */              { $$ = NULL; }
    | statement_list statement { 
        $$ = $1 ? $1 : $2; 
        if ($1) {
            Nodo *last = $1;
            while (last->siguiente) last = last->siguiente;
            last->siguiente = $2;
        }
    }
    ;

statement
    : ID OP_IGUAL expr PYC
      {
          Symbol *s = search_symbol($1);
          if (!s) {
              fprintf(stderr, "Error semántico en línea %d: variable '%s' no declarada.\n", yylineno, $1);
          }
          $$ = nodo_assign($1, $3);
          free($1);
      }
    | method_call PYC { $$ = $1; }
    | IF PARA expr PARC THEN block else_opt { $$ = nodo_if($3, $6, $7); }
    | WHILE PARA expr PARC block { $$ = nodo_while($3, $5); }
    | RETURN expr_opt PYC { $$ = nodo_return($2); }
    | PYC { $$ = NULL; }
    | block { $$ = $1; }
    ;

else_opt
    : /* empty */ { $$ = NULL; }
    | ELSE block  { $$ = $2; }
    ;

expr_opt
    : /* empty */ { $$ = NULL; }
    | expr        { $$ = $1; }
    ;

arg_list_opt
    : /* empty */ { $$ = NULL; }
    | arg_list    { $$ = $1; }
    ;

arg_list
    : expr               
      { 
          $$ = $1; 
      }
    | arg_list COMA expr 
      { 
          if ($1) {
              // Encontrar el último nodo de la lista
              Nodo *last = $1;
              while (last->siguiente) {
                  last = last->siguiente;
              }
              // Enlazar el nuevo nodo al final
              last->siguiente = $3;
              $$ = $1;
          } else {
              $$ = $3;
          }
      }
    ;

expr
    : INTEGER_LITERAL { $$ = nodo_integer($1); }
    | TRUE { $$ = nodo_bool(1); }
    | FALSE { $$ = nodo_bool(0); }
    | ID { 
          char *name = $1;
          Symbol *s = search_symbol(name);
          if (!s) {
              fprintf(stderr, "Error semántico en línea %d: identificador '%s' no declarado.\n", yylineno, name);
          }
          $$ = nodo_ID(name);
          free(name);
      }
    | method_call { $$ = $1; }
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
          Symbol *s = search_symbol($1);
          (void)s;
          $$ = nodo_method_call($1, $3);
          free($1);
      }
    ;

TYPE
    : INTEGER { $$ = nodo_ID("integer"); }
    | BOOL    { $$ = nodo_ID("bool"); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d cerca de '%s': %s\n",
            yylineno, yytext ? yytext : "unknown", s);
}

int main(int argc, char **argv) {
    // Parsear argumentos de línea de comandos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-debug") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-optimizer") == 0) {
            optimizer_enabled = 1;
        } else if (strcmp(argv[i], "-target") == 0) {
            if (i + 1 < argc) {
                i++; // Avanzar al siguiente argumento
                if (strcmp(argv[i], "syntax") == 0 || strcmp(argv[i], "semantic") == 0) {
                    target_stage = TARGET_SEMANTIC;
                } else if (strcmp(argv[i], "ir") == 0) {
                    target_stage = TARGET_IR;
                } else if (strcmp(argv[i], "object") == 0 || strcmp(argv[i], "all") == 0) {
                    target_stage = TARGET_OBJECT;
                } else {
                    fprintf(stderr, "Error: etapa desconocida '%s'\n", argv[i]);
                    fprintf(stderr, "Etapas válidas:\n");
                    fprintf(stderr, "  syntax/semantic - Análisis sintáctico y semántico + AST (con optimizaciones)\n");
                    fprintf(stderr, "  ir              - Hasta código intermedio (incluye optimizaciones IR)\n");
                    fprintf(stderr, "  object          - Compilación completa hasta código objeto\n");
                    fprintf(stderr, "  all             - Alias para 'object' (por defecto)\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -target requiere una etapa (syntax, semantic, ir, object, all)\n");
                return 1;
            }
        }
    }
    
    init_symtab();

    if (yyparse() == 0) {
        if (debug_mode) {
            printf("Análisis sintáctico completado sin errores.\n");
        } else {
            printf("✓ Análisis sintáctico completado exitosamente.\n");
        }
        
        if (debug_mode) {
            printf(" ------------------------------");
            printf("\n| INICIANDO ANÁLISIS SEMÁNTICO |");
            printf("\n ------------------------------\n");
        }
        
        // Aplicar optimizaciones al AST solo si están habilitadas
        if (optimizer_enabled) {
            if (debug_mode) {
                printf("✓ Optimizaciones del AST habilitadas.\n");
            }
            ast = optimize_ast(ast);
        } else {
            if (debug_mode) {
                printf("✓ Optimizaciones del AST deshabilitadas.\n");
            }
        }
        
        if (debug_mode) {
            printf("\n ----------------------------------");
            printf("\n| Árbol Sintáctico Abstracto (AST) |");
            if (optimizer_enabled) {
                printf("\n|        (POST-OPTIMIZACIÓN)       |");
            }
            printf("\n ----------------------------------\n");

            imprimir_nodo(ast, 0);
            generar_png_ast(ast);

            print_symtab();
        } else {
            generar_png_ast(ast);
        }
        
        int semantic_result = semantic_analysis(ast);
        
        if (semantic_result == 0) {
            // Si solo queremos análisis semántico
            if (target_stage == TARGET_SEMANTIC) {
                if (debug_mode) {
                    printf("\n==> Compilación detenida en etapa: SEMANTIC\n");
                } else {
                    printf("✓ Compilación completada hasta: análisis semántico + AST optimizado.\n");
                }
                nodo_libre(ast);
                free_symtab();
                return 0;
            }
            
            int ir_result = generate_intermediate_code(ast);
            
            if (ir_result == 0) {
                // Si solo queremos código intermedio
                if (target_stage == TARGET_IR) {
                    if (debug_mode) {
                        printf("\n==> Compilación detenida en etapa: IR\n");
                    } else {
                        printf("✓ Compilación completada hasta: código intermedio.\n");
                    }
                    nodo_libre(ast);
                    free_symtab();
                    return 0;
                }
                
                if (debug_mode) {
                    printf(" ------------------------- ");
                    printf("\n| GENERANDO CÓDIGO OBJETO |");
                    printf("\n ------------------------- \n");
                }
                
                int obj_result = generate_object_code("inter.ir", "output.s");
                
                if (obj_result == 0) {
                    if (debug_mode) {
                        printf("✓ Generación de código objeto completado exitosamente.\n\n");
                    } else {
                        printf("✓ Generación de código objeto completado exitosamente.\n");
                    }
                } else {
                    printf("X ERROR en la generación de código objeto.\n\n");
                }
                
                nodo_libre(ast);
                free_symtab();
                return obj_result;
            } else {
                nodo_libre(ast);
                free_symtab();
                return ir_result;
            }
        } else {
            if (debug_mode) {
                printf("X COMPILACIÓN FALLIDA: Errores en análisis semántico.\n\n");
            } else {
                printf("✗ Compilación fallida: errores en análisis semántico.\n");
            }
            nodo_libre(ast);
            free_symtab();
            return semantic_result;
        }
    } else {
        if (debug_mode) {
            printf("Análisis sintáctico fallido.\n");
        } else {
            printf("✗ Análisis sintáctico fallido.\n");
        }
        free_symtab();
        return 1;
    }
}