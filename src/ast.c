#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/*
 * Constructor de un nodo de tipo ID
 */
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

/*
 * Constructor de un nodo de tipo bool
 */
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

/*
 * Constructor de un nodo de tipo int
 */
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

/*
 * Constructor de un nodo de tipo operación binaria
 */
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

/*
 * Constructor de un nodo de tipo return
 */
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

/*
 * Constructor de un nodo de tipo asignación
 */
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

/*
 * Constructor de un nodo de tipo declaración
 */
Nodo *nodo_decl(char *id, Nodo *expr) {
    if (!id) {
        fprintf(stderr, "Error: nodo_decl recibió id NULL\n");
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

/*
 * Constructor de un nodo de tipo metodo
 */
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

/*
 * Constructor de un nodo de tipo llamada a metodo
 */
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

/*
 * Constructor de un nodo de tipo if
 */
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

/*
 * Constructor de un nodo de tipo while
 */
Nodo* nodo_while(Nodo* condicion, Nodo* bloque) {
    Nodo* nodo = (Nodo*)malloc(sizeof(Nodo));
    if (!nodo) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    nodo->padre = NULL;
    nodo->siguiente = NULL;
    nodo->tipo = NODO_WHILE;
    nodo->nombre = strdup("while");
    nodo->while_stmt.cond = condicion;
    nodo->while_stmt.body = bloque;

    if (condicion) condicion->padre = nodo;
    if (bloque) bloque->padre = nodo;
    
    return nodo;
}

/*
 * Función para imprimir un nodo
 */
void imprimir_nodo(Nodo *nodo, int indent) {
    if (!nodo) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (nodo->tipo) {
        case NODO_PROG:
            printf("ID: program\n");
            break;
        case NODO_DECL:
            printf("VAR: %s\n", nodo->assign.id);
            if (nodo->assign.expr) imprimir_nodo(nodo->assign.expr, indent + 1);
            break;
        case NODO_ID:
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
                imprimir_nodo(nodo->method.params, indent + 1);
            }
            if (nodo->method.body) {
                imprimir_nodo(nodo->method.body, indent + 1);
            }
            break;
        case NODO_METHOD_CALL:
            printf("METHOD CALL: %s\n", nodo->method_call.nombre);
            if (nodo->method_call.args) {
                imprimir_nodo(nodo->method_call.args, indent + 1);
            }
            break;
        case NODO_IF:
            printf("IF\n");
            imprimir_nodo(nodo->if_stmt.cond, indent + 1);
            imprimir_nodo(nodo->if_stmt.then_block, indent + 1); 
            if (nodo->if_stmt.else_block) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("ELSE\n");
                imprimir_nodo(nodo->if_stmt.else_block, indent + 2);
            }
            break;
        case NODO_WHILE:
            printf("WHILE\n");
            imprimir_nodo(nodo->while_stmt.cond, indent + 1);
            imprimir_nodo(nodo->while_stmt.body, indent + 1);
            break;
        case NODO_SENT:
            break;
        case NODO_BLOCK:
            break;
        default:
            printf("TIPO_DESCONOCIDO\n");
            break;
    }

    if (nodo->siguiente) {
        imprimir_nodo(nodo->siguiente, indent);
    }
}

/*
 * Funciones y declaraciones para generar el DOT para Graphviz 
 */
static int node_counter = 0;

char* get_unique_node_id() {
    char* id = malloc(10 * sizeof(char));
    sprintf(id, "n%d", node_counter++);
    return id;
}

void generar_dot_ast(Nodo *nodo, FILE *dot_file, char* parent_id) {
    if (!nodo) return;

    char* my_id = get_unique_node_id();
    char label[256] = {0};

    switch (nodo->tipo) {
        case NODO_PROG:
            strcpy(label, "ID: program");
            break;
        case NODO_DECL:
            snprintf(label, sizeof(label), "VAR: %s", nodo->assign.id);
            break;
        case NODO_ID: {
            if (nodo->padre && nodo->padre->tipo == NODO_METHOD) {
                snprintf(label, sizeof(label), "PARAM: %s", nodo->nombre);
            } else {
                snprintf(label, sizeof(label), "ID: %s", nodo->nombre);
            }
            break;
        }
        case NODO_INTEGER:
            snprintf(label, sizeof(label), "INTEGER: %d", nodo->val_int);
            break;
        case NODO_BOOL:
            snprintf(label, sizeof(label), "BOOL: %s", nodo->val_bool ? "TRUE" : "FALSE");
            break;
        case NODO_ASSIGN:
            snprintf(label, sizeof(label), "ASSIGN: %s", nodo->assign.id);
            break;
        case NODO_RETURN:
            strcpy(label, "RETURN");
            break;
        case NODO_OP: {
            const char* op_str;
            switch (nodo->opBinaria.op) {
                case TOP_SUMA: op_str = "TOP_SUMA"; break;
                case TOP_RESTA: op_str = "TOP_RESTA"; break;
                case TOP_MULT: op_str = "TOP_MULT"; break;
                case TOP_DIV: op_str = "TOP_DIV"; break;
                case TOP_RESTO: op_str = "TOP_RESTO"; break;
                case TOP_IGUAL: op_str = "TOP_IGUAL"; break;
                case TOP_MAYOR: op_str = "TOP_MAYOR"; break;
                case TOP_MENOR: op_str = "TOP_MENOR"; break;
                case TOP_MAYORIG: op_str = "TOP_MAYORIG"; break;
                case TOP_MENORIG: op_str = "TOP_MENORIG"; break;
                case TOP_DESIGUAL: op_str = "TOP_DESIGUAL"; break;
                case TOP_COMP: op_str = "TOP_COMP"; break;
                case TOP_AND: op_str = "TOP_AND"; break;
                case TOP_OR: op_str = "TOP_OR"; break;
                case TOP_NOT: op_str = "TOP_NOT"; break;
                default: op_str = "UNKNOWN"; break;
            }
            snprintf(label, sizeof(label), "OP: %s", op_str);
            break;
        }
        case NODO_METHOD:
            snprintf(label, sizeof(label), "METHOD: %s", nodo->method.nombre);
            break;
        case NODO_METHOD_CALL:
            snprintf(label, sizeof(label), "METHOD CALL: %s", nodo->method_call.nombre);
            break;
        case NODO_IF:
            strcpy(label, "IF");
            break;
        case NODO_WHILE:
            strcpy(label, "WHILE");
            break;
        case NODO_SENT:
        case NODO_BLOCK:
            strcpy(label, "BLOCK/SENT");
            break;
        default:
            strcpy(label, "UNKNOWN");
            break;
    }

    fprintf(dot_file, "  \"%s\" [label=\"%s\"];\n", my_id, label);

    if (parent_id) {
        fprintf(dot_file, "  \"%s\" -> \"%s\";\n", parent_id, my_id);
    }

    switch (nodo->tipo) {
        case NODO_DECL:
            if (nodo->assign.expr) generar_dot_ast(nodo->assign.expr, dot_file, my_id);
            break;
        case NODO_ASSIGN:
            if (nodo->assign.expr) generar_dot_ast(nodo->assign.expr, dot_file, my_id);
            break;
        case NODO_RETURN:
            if (nodo->ret_expr) generar_dot_ast(nodo->ret_expr, dot_file, my_id);
            break;
        case NODO_OP:
            if (nodo->opBinaria.izq) generar_dot_ast(nodo->opBinaria.izq, dot_file, my_id);
            if (nodo->opBinaria.der) generar_dot_ast(nodo->opBinaria.der, dot_file, my_id);
            break;
        case NODO_METHOD:
            if (nodo->method.params) generar_dot_ast(nodo->method.params, dot_file, my_id);
            if (nodo->method.body) generar_dot_ast(nodo->method.body, dot_file, my_id);
            break;
        case NODO_METHOD_CALL:
            if (nodo->method_call.args) generar_dot_ast(nodo->method_call.args, dot_file, my_id);
            break;
        case NODO_IF:
            if (nodo->if_stmt.cond) generar_dot_ast(nodo->if_stmt.cond, dot_file, my_id);
            if (nodo->if_stmt.then_block) generar_dot_ast(nodo->if_stmt.then_block, dot_file, my_id);
            if (nodo->if_stmt.else_block) {
                generar_dot_ast(nodo->if_stmt.else_block, dot_file, my_id);
            }
            break;
        case NODO_WHILE: 
            if (nodo->while_stmt.cond) generar_dot_ast(nodo->while_stmt.cond, dot_file, my_id);
            if (nodo->while_stmt.body) generar_dot_ast(nodo->while_stmt.body, dot_file, my_id);
            break;
        default:
            break;
    }

    if (nodo->siguiente) {
        generar_dot_ast(nodo->siguiente, dot_file, parent_id);
    }

    free(my_id);
}

/*
 * Función para generar el PNG del AST
 */
void generar_png_ast(Nodo *ast) {
    if (!ast) return;

    FILE *dot_file = fopen("ast.dot", "w");
    if (!dot_file) {
        perror("No se pudo abrir ast.dot");
        return;
    }

    node_counter = 0;
    fprintf(dot_file, "digraph AST {\n");
    fprintf(dot_file, "  rankdir=TB;\n");
    fprintf(dot_file, "  node [shape=box, style=filled, fillcolor=lightblue];\n");

    generar_dot_ast(ast, dot_file, NULL);

    fprintf(dot_file, "}\n");
    fclose(dot_file);

    int ret = system("dot -Tpng ast.dot -o ast_tree.png");
    if (ret != 0) {
        if (debug_mode) {
            fprintf(stderr, "Error: No se pudo generar PNG. Asegúrate de tener Graphviz instalado y 'dot' en PATH.\n");
        }
    } else {
        if (debug_mode) {
            printf("\nAST generado como 'ast_tree.png'.\n");
        }
    }
}

/*
 * Función para liberar el nodo cuando ya no sea necesario 
 */
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
        case NODO_WHILE:
            nodo_libre(nodo->while_stmt.cond);
            nodo_libre(nodo->while_stmt.body);
            break;
        case NODO_ID:
            free(nodo->nombre);
            break;
        case NODO_INTEGER:
        case NODO_BOOL:
        case NODO_SENT:
        case NODO_PROG:
        case NODO_BLOCK:
            break;
        default:
            break;
    }

    free(nodo);
}