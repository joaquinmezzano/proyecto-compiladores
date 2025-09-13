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
    if (!id || !expr) {
        fprintf(stderr, "Error: nodo_assign recibió id o expr NULL\n");
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
    if (!id || !expr) {
        fprintf(stderr, "Error: nodo_decl recibió id o expr NULL\n");
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

Nodo *nodo_method(char *nombre, Nodo *params, Nodo *body) {
    if (!nombre) {
        fprintf(stderr, "Error: nodo_method recibió nombre NULL\n");
        exit(EXIT_FAILURE);
    }

    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_METHOD;
    n->method.nombre = strdup(nombre);
    n->method.params = params;
    n->method.body = body;

    if (params) params->padre = n;
    if (body) body->padre = n;
    
    return n;
}

Nodo *nodo_method_call(char *nombre, Nodo *args) {
    if (!nombre) {
        fprintf(stderr, "Error: nodo_method_call recibió nombre NULL\n");
        exit(EXIT_FAILURE);
    }

    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_METHOD_CALL;
    n->method_call.nombre = strdup(nombre);
    n->method_call.args = args;

    if (args) args->padre = n;
    
    return n;
}

Nodo *nodo_if(Nodo *cond, Nodo *then_block, Nodo *else_block) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    n->padre = NULL;
    n->siguiente = NULL;
    n->tipo = NODO_IF;
    n->if_stmt.cond = cond;
    n->if_stmt.then_block = then_block;
    n->if_stmt.else_block = else_block;

    if (cond) cond->padre = n;
    if (then_block) then_block->padre = n;
    if (else_block) else_block->padre = n;
    
    return n;
}

void imprimir_nodo(Nodo *nodo, int indent) {
    if (!nodo) return;

    // Indentación según el nivel
    for (int i = 0; i < indent; i++) printf("  ");

    // Mostrar según tipo de nodo
    switch (nodo->tipo) {
        case NODO_PROG:
            printf("ID: program\n");
            break;
        case NODO_DECL:
            printf("VAR: %s\n", nodo->assign.id);
            if (nodo->assign.expr) imprimir_nodo(nodo->assign.expr, indent + 1);
            break;
        case NODO_ID:
            // Detecta si es parámetro (hijo de METHOD)
            if (nodo->padre && nodo->padre->tipo == NODO_METHOD) {
                printf("PARAM: %s\n", nodo->nombre);
            } else {
                printf("ID: %s\n", nodo->nombre);
            }
            break;
        case NODO_INTEGER:
            printf("INTEGER: %d\n", nodo->val_int);
            break;
        case NODO_BOOL:
            printf("BOOL: %s\n", nodo->val_bool ? "TRUE" : "FALSE");
            break;
        case NODO_ASSIGN:
            printf("ASSIGN: %s\n", nodo->assign.id);
            imprimir_nodo(nodo->assign.expr, indent + 1);
            break;
        case NODO_RETURN:
            printf("RETURN\n");
            if (nodo->ret_expr) imprimir_nodo(nodo->ret_expr, indent + 1);
            break;
        case NODO_OP:
            switch (nodo->opBinaria.op) {
                case TOP_SUMA:     printf("OP: TOP_SUMA\n"); break;
                case TOP_RESTA:    printf("OP: TOP_RESTA\n"); break;
                case TOP_MULT:     printf("OP: TOP_MULT\n"); break;
                case TOP_DIV:      printf("OP: TOP_DIV\n"); break;
                case TOP_RESTO:    printf("OP: TOP_RESTO\n"); break;
                case TOP_IGUAL:    printf("OP: TOP_IGUAL\n"); break;
                case TOP_MAYOR:    printf("OP: TOP_MAYOR\n"); break;
                case TOP_MENOR:    printf("OP: TOP_MENOR\n"); break;
                case TOP_MAYORIG:  printf("OP: TOP_MAYORIG\n"); break;
                case TOP_MENORIG:  printf("OP: TOP_MENORIG\n"); break;
                case TOP_DESIGUAL: printf("OP: TOP_DESIGUAL\n"); break;
                case TOP_COMP:     printf("OP: TOP_COMP\n"); break;
                case TOP_AND:      printf("OP: TOP_AND\n"); break;
                case TOP_OR:       printf("OP: TOP_OR\n"); break;
                case TOP_NOT:      printf("OP: TOP_NOT\n"); break;
                default:           printf("OP: UNKNOWN\n"); break;
            }
            if (nodo->opBinaria.izq) imprimir_nodo(nodo->opBinaria.izq, indent + 1);
            if (nodo->opBinaria.der) imprimir_nodo(nodo->opBinaria.der, indent + 1);
            break;
        case NODO_METHOD:
            printf("METHOD: %s\n", nodo->method.nombre);
            if (nodo->method.params) {
                imprimir_nodo(nodo->method.params, indent + 1);  // PARAM: x
            }
            if (nodo->method.body) {
                imprimir_nodo(nodo->method.body, indent + 1);  // Statements dentro
            }
            break;
        case NODO_METHOD_CALL:
            printf("METHOD CALL: %s\n", nodo->method_call.nombre);
            if (nodo->method_call.args) {
                imprimir_nodo(nodo->method_call.args, indent + 1);  // Args como hijos
            }
            break;
        case NODO_IF:
            printf("IF\n");
            imprimir_nodo(nodo->if_stmt.cond, indent + 1);  // Condición
            imprimir_nodo(nodo->if_stmt.then_block, indent + 1);  // Then block
            if (nodo->if_stmt.else_block) {
                // Indentación correcta para ELSE
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("ELSE\n");
                imprimir_nodo(nodo->if_stmt.else_block, indent + 2);  // Contenido del else con +2 indent
            }
            break;
        case NODO_SENT:
            // Caso para eliminar warning (no usado actualmente)
            break;
        case NODO_BLOCK:
            // Caso para eliminar warning (no usado actualmente)
            // Si tuviera statements, los imprimiríamos aquí
            break;
        default:
            printf("TIPO_DESCONOCIDO\n");
            break;
    }

    // Imprimir nodo siguiente si existe (hermanos al mismo nivel)
    if (nodo->siguiente) {
        imprimir_nodo(nodo->siguiente, indent);
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
        case NODO_METHOD:
            free(nodo->method.nombre);
            nodo_libre(nodo->method.params);
            nodo_libre(nodo->method.body);
            break;
        case NODO_METHOD_CALL:
            free(nodo->method_call.nombre);
            nodo_libre(nodo->method_call.args);
            break;
        case NODO_IF:
            nodo_libre(nodo->if_stmt.cond);
            nodo_libre(nodo->if_stmt.then_block);
            nodo_libre(nodo->if_stmt.else_block);
            break;
        case NODO_ID:
            free(nodo->nombre);
            break;
        case NODO_INTEGER:
        case NODO_BOOL:
        case NODO_SENT:
        case NODO_PROG:
        case NODO_BLOCK:
            // Casos vacíos para eliminar warnings
            break;
        default:
            // Para cualquier otro tipo futuro
            break;
    }

    free(nodo);
}