#ifndef AST_H
#define AST_H

#include <stdio.h>

/* ------------------ Definiciones de tipos ------------------ */

/* Tipos de nodos del AST */
typedef enum {
    NODO_PROG,
    NODO_DECL,
    NODO_SENT,
    NODO_ASSIGN,
    NODO_RETURN,
    NODO_OP,
    NODO_INT,
    NODO_BOOL,
    NODO_ID,
    NODO_SEQ
} TipoNodo;

/* Tipos de operaciones binarias */
typedef enum {
    TOP_SUMA,
    TOP_RESTA,
    TOP_MULT,
    TOP_DIV,
    TOP_ASSIGN,
    TOP_IGUAL,
    TOP_OR,
    TOP_AND,
    TOP_MAYOR,
    TOP_MENOR
} TipoOP;

/* Estructura del nodo del AST */
typedef struct Nodo {
    TipoNodo tipo;
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
    };
} Nodo;

/* ------------------ Prototipos de funciones ------------------ */

/* Constructores de nodos */
Nodo *nodo_ID(char *nombre);
Nodo *nodo_int(int val_int);
Nodo *nodo_bool(int val_bool);
Nodo *nodo_opBin(TipoOP op, Nodo *izq, Nodo *der);
Nodo *nodo_assign(char *id, Nodo *expr);
Nodo *nodo_return(Nodo *expr);
Nodo *nodo_seq(Nodo *primero, Nodo *resto);
Nodo *nodo_decl(char *id, Nodo *expr);

/* Funciones de manejo del AST */
void imprimir_nodo(Nodo *nodo, int indent); /* Imprimir AST básico */
void print_ast(Nodo *nodo, int indent);     /* Imprimir AST con formato mejorado */
void nodo_libre(Nodo *nodo);                /* Liberar memoria del AST */
void exportar_dot(Nodo *nodo, const char *filename); /* Exportar a Graphviz */


/* Funciones del intérprete */
int eval_nodo(Nodo *n);                /* Evalúa un nodo del AST y devuelve su valor */
int interpretar_programa(Nodo *programa);
void ast_liberar_recursos(void);

/* --- Codegen a seudo-assembly --- */
void generar_asm(Nodo *programa, const char *filename);

#endif /* AST_H */
