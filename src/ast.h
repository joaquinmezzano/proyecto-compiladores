#ifndef AST_H
#define AST_H

#include <stdio.h>

/* Variable global para controlar el modo debug */
extern int debug_mode;

/*
 * Tipos de nodos 
 */
typedef enum {
    NODO_PROG,
    NODO_DECL,
    NODO_SENT,
    NODO_ASSIGN,
    NODO_RETURN,
    NODO_OP,
    NODO_INTEGER,
    NODO_BOOL,
    NODO_ID,
    NODO_METHOD,
    NODO_METHOD_CALL,
    NODO_IF,
    NODO_WHILE,
    NODO_BLOCK
} TipoNodo;

/*
 * Tipos de operaciones binarias 
 */
typedef enum {
    TOP_SUMA,
    TOP_RESTA,
    TOP_MULT,
    TOP_DIV,
    TOP_RESTO,
    TOP_IGUAL,
    TOP_MAYOR,
    TOP_MENOR,
    TOP_MAYORIG,
    TOP_MENORIG,
    TOP_DESIGUAL,
    TOP_COMP,
    TOP_AND,
    TOP_OR,
    TOP_NOT
} TipoOP;

/*
 * Definición de nodo 
 */
typedef struct Nodo {
    TipoNodo tipo;
    struct Nodo *padre;
    struct Nodo *siguiente;

    union {
        int val_int;
        int val_bool;
        char *nombre;

        struct {
            struct Nodo *izq;
            struct Nodo *der;
            TipoOP op;
        } opBinaria;

        struct {
            char *id;
            struct Nodo *expr;
        } assign;

        struct Nodo *ret_expr;

        struct {
            char *nombre;
            struct Nodo *params;
            struct Nodo *body;
        } method;

        struct {
            char *nombre;
            struct Nodo *args;
        } method_call;

        struct {
            struct Nodo *cond;
            struct Nodo *then_block;
            struct Nodo *else_block;
        } if_stmt;

        struct {
            struct Nodo *cond;
            struct Nodo *body;
        } while_stmt;
    };
} Nodo;

/*
 * Declaraciones de funciones a definir 
 */
Nodo *nodo_ID(char *nombre);
Nodo *nodo_bool(int val_bool);
Nodo *nodo_integer(int val_int);
Nodo *nodo_op(TipoOP op, Nodo *izq, Nodo *der);
Nodo *nodo_return(Nodo *ret_expr);
Nodo *nodo_assign(char *id, Nodo *expr);
Nodo *nodo_decl(char *id, Nodo *expr);
Nodo *nodo_method(char *nombre, Nodo *params, Nodo *body);
Nodo *nodo_method_call(char *nombre, Nodo *args);
Nodo *nodo_if(Nodo *cond, Nodo *then_block, Nodo *else_block);
Nodo *nodo_while(Nodo *cond, Nodo *body);  // <-- Agregar declaración

void imprimir_nodo(Nodo *nodo, int indent);
void nodo_libre(Nodo *nodo);
void generar_png_ast(Nodo *ast);

#endif