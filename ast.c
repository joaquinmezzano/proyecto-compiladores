#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Nodo *nodo_ID(char *nombre) {
    if (!nombre) {
        fprintf(stderr, "Error: nodo_ID recibió nombre NULL\n");
        exit(EXIT_FAILURE);
    }

    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_ID;
    n->nombre = strdup(nombre);
    
    return n;
}

Nodo *nodo_bool(int val_bool) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_BOOL;
    n->val_bool = val_bool;

    return n;
}

Nodo *nodo_integer(int val_int) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_INTEGER;
    n->val_int = val_int;
    
    return n;
}

Nodo *nodo_op(TipoOP op, Nodo *izq, Nodo *der) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_OP;
    n->opBinaria.op = op;
    n->opBinaria.izq = izq;
    n->opBinaria.der = der;

    if (izq) izq->padre = n;
    if (der) der->padre = n;
    
    return n;
}

Nodo *nodo_return(Nodo *ret_expr) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_RETURN;
    n->ret_expr = ret_expr;

    if (ret_expr) ret_expr->padre = n;
    
    return n;
}

Nodo *nodo_assign(char *id, Nodo *expr) {
    if (!id) {
        fprintf(stderr, "Error: nodo_assign recibió id NULL\n");
        exit(EXIT_FAILURE);
    }
    if (!expr) {
        fprintf(stderr, "Error: nodo_assign recibió expr NULL\n");
        exit(EXIT_FAILURE);
    }

    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_ASSIGN;
    n->assign.expr = expr;
    n->assign.id = strdup(id);

    if (expr) expr->padre = n;
    
    return n;
}

Nodo *nodo_decl(char *id, Nodo *expr) {
    if (!id) {
        fprintf(stderr, "Error: nodo_decl recibió id NULL\n");
        exit(EXIT_FAILURE);
    }
    if (!expr) {
        fprintf(stderr, "Error: nodo_decl recibió expr NULL\n");
        exit(EXIT_FAILURE);
    }

    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_DECL;
    n->assign.expr = expr;
    n->assign.id = strdup(id);

    if (expr) expr->padre = n;
    
    return n;
}

void imprimir_nodo(Nodo *nodo, int indent) {
    if (!nodo) return;

    for (int i = 0; i < indent; i++) printf("    ");

    switch(nodo->tipo) {
        case NODO_DECL:
            printf("%-10s%s\n", "DECL", nodo->assign.id);
            if (nodo->assign.expr)
                imprimir_nodo(nodo->assign.expr, indent + 1);
            break;
        case NODO_OP:
            printf("%-10s\n",
                   nodo->opBinaria.op == TOP_SUMA ? "SUMA" :
                   nodo->opBinaria.op == TOP_RESTA ? "RESTA" :
                   nodo->opBinaria.op == TOP_MULT ? "MULT" :
                   nodo->opBinaria.op == TOP_DIV ? "DIV" :
                   nodo->opBinaria.op == TOP_RESTO ? "RESTO" :
                   nodo->opBinaria.op == TOP_IGUAL ? "IGUAL" :
                   nodo->opBinaria.op == TOP_OR ? "OR" :
                   nodo->opBinaria.op == TOP_AND ? "AND" :
                   nodo->opBinaria.op == TOP_COMP ? "COMP" :
                   nodo->opBinaria.op == TOP_DESIGUAL ? "DESIGUAL" :
                   nodo->opBinaria.op == TOP_MAYORIG ? "MAYORIG" :
                   nodo->opBinaria.op == TOP_MENORIG ? "MENORIG" :
                   nodo->opBinaria.op == TOP_MAYOR ? "MAYOR" :                   
                   nodo->opBinaria.op == TOP_MENOR ? "MENOR" : "OP_UNKNOWN");
            imprimir_nodo(nodo->opBinaria.izq, indent + 1);
            imprimir_nodo(nodo->opBinaria.der, indent + 1);
            break;
        case NODO_ID:
            printf("%-10s%s\n", "ID", nodo->nombre);
            break;
        case NODO_INTEGER:
            printf("%-10s%d\n", "INTEGER", nodo->val_int);
            break;
        case NODO_BOOL:
            printf("%-10s%s\n", "BOOL", nodo->val_bool ? "true" : "false");
            break;
        case NODO_ASSIGN:
            printf("%-10s%s\n", "ASSIGN", nodo->assign.id);
            imprimir_nodo(nodo->assign.expr, indent + 1);
            break;
        case NODO_RETURN:
            printf("%-10s\n", "RETURN");
            if (nodo->ret_expr)
                imprimir_nodo(nodo->ret_expr, indent + 1);
            break;
        default:
            printf("%-10s%d\n", "UNKNOWN", nodo->tipo);
            break;
    }

}

void nodo_libre(Nodo *nodo) {
    if (!nodo) return;

    switch (nodo->tipo) {
        case NODO_OP:
            nodo_libre(nodo->opBinaria.izq);
            nodo_libre(nodo->opBinaria.der);
            break;
        case NODO_ASSIGN:
        case NODO_DECL:
            free(nodo->assign.id);
            nodo_libre(nodo->assign.expr);
            break;
        case NODO_RETURN:
            nodo_libre(nodo->ret_expr);
            break;
        case NODO_ID:
            free(nodo->nombre);
            break;
        default:
            break;
    }
    free(nodo);
}