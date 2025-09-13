#ifndef AST_H
#define AST_H

#include <stdio.h>

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
    NODO_BLOCK
} TipoNodo;

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

typedef struct Nodo {
    TipoNodo tipo;
    struct Nodo *padre;
    struct Nodo *siguiente;  // Para listas (params, args, statements, etc.)

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

        // METHOD
        struct {
            char *nombre;
            struct Nodo *params;  // Lista de NODO_ID enlazados por siguiente
            struct Nodo *body;    // El block (statements enlazados)
        } method;

        // METHOD_CALL
        struct {
            char *nombre;
            struct Nodo *args;    // Lista de expr enlazados por siguiente
        } method_call;

        // IF
        struct {
            struct Nodo *cond;
            struct Nodo *then_block;
            struct Nodo *else_block;
        } if_stmt;
    };
} Nodo;

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

void imprimir_nodo(Nodo *nodo, int indent);
void nodo_libre(Nodo *nodo);
void generar_png_ast(Nodo *ast);

#endif