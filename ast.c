/* Implementación del Árbol Sintáctico Abstracto (AST), tabla de símbolos y evaluador */

/* Definiciones necesarias para compatibilidad POSIX */
#define _POSIX_C_SOURCE 200809L

/* Inclusiones de bibliotecas estándar */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include "ast.h"

/* ------------------ Constructores y destructor de nodos ------------------ */

/* Contador global para asignar IDs únicos a los nodos en la exportación DOT */
static int contador_nodos = 0;

/**
 * Crea un nodo para un identificador (ID).
 * @param nombre El nombre del identificador.
 * @return Puntero al nodo creado.
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
    n->tipo = NODO_ID;
    n->nombre = strdup(nombre);
    return n;
}

/**
 * Crea un nodo para un valor entero.
 * @param val_int El valor entero.
 * @return Puntero al nodo creado.
 */
Nodo *nodo_int(int val_int) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->tipo = NODO_INT;
    n->val_int = val_int;
    return n;
}

/**
 * Crea un nodo para un valor booleano.
 * @param val_bool El valor booleano (0 o 1).
 * @return Puntero al nodo creado.
 */
Nodo *nodo_bool(int val_bool) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->tipo = NODO_BOOL;
    n->val_bool = val_bool;
    return n;
}

/**
 * Crea un nodo para una operación binaria.
 * @param op El tipo de operación.
 * @param izq Nodo izquierdo.
 * @param der Nodo derecho.
 * @return Puntero al nodo creado.
 */
Nodo *nodo_opBin(TipoOP op, Nodo *izq, Nodo *der) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->tipo = NODO_OP;
    n->opBinaria.op = op;
    n->opBinaria.izq = izq;
    n->opBinaria.der = der;
    return n;
}

/**
 * Crea un nodo para una asignación.
 * @param id El identificador a asignar.
 * @param expr La expresión a asignar.
 * @return Puntero al nodo creado.
 */
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
    n->tipo = NODO_ASSIGN;
    n->assign.id = strdup(id);
    n->assign.expr = expr;
    return n;
}

/**
 * Crea un nodo para una sentencia return.
 * @param expr La expresión a retornar (puede ser NULL).
 * @return Puntero al nodo creado.
 */
Nodo *nodo_return(Nodo *expr) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->tipo = NODO_RETURN;
    n->ret_expr = expr;
    return n;
}

/**
 * Crea un nodo para una secuencia de nodos.
 * @param primero El primer nodo de la secuencia.
 * @param resto El resto de la secuencia.
 * @return Puntero al nodo creado.
 */
Nodo *nodo_seq(Nodo *primero, Nodo *resto) {
    Nodo *n = malloc(sizeof(Nodo));
    if (!n) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    n->tipo = NODO_SEQ;
    n->opBinaria.izq = primero;
    n->opBinaria.der = resto;
    return n;
}

/**
 * Crea un nodo para una declaración de variable.
 * @param id El identificador de la variable.
 * @param expr La expresión inicial (puede ser NULL).
 * @return Puntero al nodo creado.
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
    n->tipo = NODO_DECL;
    n->assign.id = strdup(id);
    n->assign.expr = expr;
    return n;
}

/**
 * Libera la memoria de un nodo y sus subnodos recursivamente.
 * @param n El nodo a liberar.
 */
void nodo_libre(Nodo *n) {
    if (!n) return;

    switch (n->tipo) {
        case NODO_SEQ:
        case NODO_OP:
            nodo_libre(n->opBinaria.izq);
            nodo_libre(n->opBinaria.der);
            break;
        case NODO_ASSIGN:
        case NODO_DECL:
            free(n->assign.id);
            nodo_libre(n->assign.expr);
            break;
        case NODO_RETURN:
            nodo_libre(n->ret_expr);
            break;
        case NODO_ID:
            free(n->nombre);
            break;
        default:
            break;
    }
    free(n);
}

/* ------------------ Exportar DOT para Graphviz ------------------ */

/**
 * Escribe recursivamente el nodo en formato DOT.
 * @param f Archivo donde escribir.
 * @param n Nodo a escribir.
 */
static void escribir_dot_rec(FILE *f, Nodo *n) {
    if (!n) return;

    int mi_id = contador_nodos++;

    switch (n->tipo) {
        case NODO_SEQ:
            fprintf(f, "    nodo%d [label=\"SEQ\", style=filled, fillcolor=lightgray];\n", mi_id);
            if (n->opBinaria.izq) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->opBinaria.izq);
            }
            if (n->opBinaria.der) {
                fprintf(f, "    nodo%d -> nodo%d [style=dashed];\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->opBinaria.der);
            }
            break;
        case NODO_DECL:
            fprintf(f, "    nodo%d [label=\"DECL %s\"];\n", mi_id, n->assign.id);
            if (n->assign.expr) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->assign.expr);
            }
            break;
        case NODO_OP:
            fprintf(f, "    nodo%d [label=\"%s\"];\n", mi_id,
                    n->opBinaria.op == TOP_SUMA ? "+" :
                    n->opBinaria.op == TOP_RESTA ? "-" :
                    n->opBinaria.op == TOP_MULT ? "*" :
                    n->opBinaria.op == TOP_DIV ? "/" :
                    n->opBinaria.op == TOP_IGUAL ? "==" :
                    n->opBinaria.op == TOP_OR ? "or" :
                    n->opBinaria.op == TOP_AND ? "and" :
                    n->opBinaria.op == TOP_MAYOR ? ">" :
                    n->opBinaria.op == TOP_MENOR ? "<" : "?");
            if (n->opBinaria.izq) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->opBinaria.izq);
            }
            if (n->opBinaria.der) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->opBinaria.der);
            }
            break;
        case NODO_ID:
            fprintf(f, "    nodo%d [label=\"%s\"];\n", mi_id, n->nombre);
            break;
        case NODO_INT:
            fprintf(f, "    nodo%d [label=\"%d\"];\n", mi_id, n->val_int);
            break;
        case NODO_RETURN:
            fprintf(f, "    nodo%d [label=\"RETURN\", style=filled, fillcolor=lightpink];\n", mi_id);
            if (n->ret_expr) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->ret_expr);
            }
            break;
        case NODO_ASSIGN:
            fprintf(f, "    nodo%d [label=\"ASSIGN %s\"];\n", mi_id, n->assign.id);
            if (n->assign.expr) {
                fprintf(f, "    nodo%d -> nodo%d;\n", mi_id, contador_nodos);
                escribir_dot_rec(f, n->assign.expr);
            }
            break;
        default:
            fprintf(stderr, "Warning: tipo de nodo desconocido %d en escribir_dot_rec\n", n->tipo);
            break;
    }
}

/**
 * Exporta el AST a un archivo DOT y genera una imagen PNG usando Graphviz.
 * @param nodo Raíz del AST.
 * @param filename Nombre base del archivo (sin extensión).
 */
void exportar_dot(Nodo *nodo, const char *filename) {
    char dot_file[PATH_MAX];
    char png_file[PATH_MAX];
    char command[PATH_MAX * 2 + 80];

    snprintf(dot_file, sizeof(dot_file), "%s.dot", filename);
    snprintf(png_file, sizeof(png_file), "%s.png", filename);

    FILE *f = fopen(dot_file, "w");
    if (!f) {
        perror("Error al crear archivo DOT");
        return;
    }

    fprintf(f, "digraph AST {\n");
    fprintf(f, "    node [shape=box, fontname=\"Arial\"];\n");

    contador_nodos = 0;
    escribir_dot_rec(f, nodo);

    fprintf(f, "}\n");
    fclose(f);

    snprintf(command, sizeof(command), "dot -Tpng %s -o %s 2>/dev/null", dot_file, png_file);
    system(command);
}

/* ------------------ Tabla de símbolos simple ------------------ */

/* Tamaño máximo de la tabla de símbolos */
#define MAX_SIMBOLOS 1024

/**
 * Estructura para un símbolo en la tabla.
 */
typedef struct {
    char *id;           /* Nombre del símbolo */
    int valor;          /* Valor (entero o booleano 0/1) */
    int es_bool;        /* 0 = int, 1 = bool */
    int inicializado;   /* 0 = no inicializado, 1 = inicializado */
} Simbolo;

/* Tabla de símbolos estática */
static Simbolo tabla[MAX_SIMBOLOS];
static int tabla_size = 0;

/**
 * Busca un símbolo en la tabla por su ID.
 * @param id El identificador a buscar.
 * @return Puntero al símbolo si existe, NULL en caso contrario.
 */
static Simbolo* buscar_simbolo(const char *id) {
    for (int i = 0; i < tabla_size; ++i) {
        if (strcmp(tabla[i].id, id) == 0) return &tabla[i];
    }
    return NULL;
}

/**
 * Declara un nuevo símbolo en la tabla.
 * @param id Identificador.
 * @param valor Valor inicial.
 * @param es_bool Tipo (0=int, 1=bool).
 * @param inicializado Estado de inicialización.
 */
static void declarar_simbolo(const char *id, int valor, int es_bool, int inicializado) {
    if (buscar_simbolo(id)) {
        fprintf(stderr, "Error: variable '%s' ya declarada\n", id);
        exit(EXIT_FAILURE);
    }
    if (tabla_size >= MAX_SIMBOLOS) {
        fprintf(stderr, "Error: tabla de símbolos llena\n");
        exit(EXIT_FAILURE);
    }
    tabla[tabla_size].id = strdup(id);
    tabla[tabla_size].valor = valor;
    tabla[tabla_size].es_bool = es_bool;
    tabla[tabla_size].inicializado = inicializado;
    tabla_size++;
}

/**
 * Asigna un valor a un símbolo existente.
 * @param id Identificador.
 * @param valor Nuevo valor.
 */
static void asignar_simbolo(const char *id, int valor) {
    Simbolo *s = buscar_simbolo(id);
    if (!s) {
        fprintf(stderr, "Error: asignación a variable no declarada '%s'\n", id);
        exit(EXIT_FAILURE);
    }
    s->valor = valor;
    s->inicializado = 1;
}

/**
 * Libera la memoria de la tabla de símbolos.
 */
static void liberar_tabla_simbolos(void) {
    for (int i = 0; i < tabla_size; ++i) {
        free(tabla[i].id);
    }
    tabla_size = 0;
}

/* ------------------ Evaluador / intérprete ------------------ */

/**
 * Evalúa un nodo del AST y retorna su valor.
 * @param n Nodo a evaluar.
 * @return Valor resultante de la evaluación.
 */
int eval_nodo(Nodo *n) {
    if (!n) return 0;

    switch (n->tipo) {
        case NODO_INT:
            return n->val_int;
        case NODO_BOOL:
            return n->val_bool ? 1 : 0;
        case NODO_ID: {
            Simbolo *s = buscar_simbolo(n->nombre);
            if (!s) {
                fprintf(stderr, "Error: variable '%s' no declarada\n", n->nombre);
                exit(EXIT_FAILURE);
            }
            if (!s->inicializado) {
                fprintf(stderr, "Error: variable '%s' usada sin inicializar\n", n->nombre);
                exit(EXIT_FAILURE);
            }
            return s->valor;
        }
        case NODO_DECL: {
            int inicializado = 0;
            int valor = 0;
            if (n->assign.expr) {
                valor = eval_nodo(n->assign.expr);
                inicializado = 1;
            }
            declarar_simbolo(n->assign.id, valor, 0, inicializado);
            return 0;
        }
        case NODO_ASSIGN: {
            int valor = eval_nodo(n->assign.expr);
            asignar_simbolo(n->assign.id, valor);
            return valor;
        }
        case NODO_OP: {
            int izq = eval_nodo(n->opBinaria.izq);
            int der = eval_nodo(n->opBinaria.der);
            switch (n->opBinaria.op) {
                case TOP_SUMA:  return izq + der;
                case TOP_RESTA: return izq - der;
                case TOP_MULT:  return izq * der;
                case TOP_DIV:
                    if (der == 0) {
                        fprintf(stderr, "Error: división por cero\n");
                        exit(EXIT_FAILURE);
                    }
                    return izq / der;
                case TOP_IGUAL: return izq == der;
                case TOP_OR:    return (izq != 0) || (der != 0);
                case TOP_AND:   return (izq != 0) && (der != 0);
                case TOP_MAYOR: return izq > der;
                case TOP_MENOR: return izq < der;
                default:
                    fprintf(stderr, "Error: operación binaria desconocida %d\n", n->opBinaria.op);
                    exit(EXIT_FAILURE);
            }
        }
        case NODO_SEQ: {
            int resultado = 0;
            if (n->opBinaria.izq) {
                resultado = eval_nodo(n->opBinaria.izq);
                if (n->opBinaria.izq->tipo == NODO_RETURN) return resultado;
            }
            if (n->opBinaria.der) {
                resultado = eval_nodo(n->opBinaria.der);
                if (n->opBinaria.der->tipo == NODO_RETURN) return resultado;
            }
            return resultado;
        }
        case NODO_RETURN: {
            if (!n->ret_expr) return 0;
            return eval_nodo(n->ret_expr);
        }
        default:
            fprintf(stderr, "Error: eval_nodo no soporta tipo de nodo %d\n", n->tipo);
            exit(EXIT_FAILURE);
    }
}

/**
 * Imprime un divisor con título para separar secciones en la salida.
 * @param title Título del divisor.
 */
static void print_divider(const char *title) {
    printf("┌──────────────────────────────────────────────────┐\n");
    printf("│ %-48s │\n", title);
    printf("└──────────────────────────────────────────────────┘\n");
}

/**
 * Imprime el AST con indentación recursiva.
 * @param nodo Nodo a imprimir.
 * @param indent Nivel de indentación.
 */
void imprimir_nodo(Nodo *nodo, int indent) {
    if (!nodo) return;

    for (int i = 0; i < indent; i++) printf("    ");

    switch (nodo->tipo) {
        case NODO_SEQ:
            printf("%-10s\n", "SEQ");
            imprimir_nodo(nodo->opBinaria.izq, indent + 1);
            imprimir_nodo(nodo->opBinaria.der, indent + 1);
            break;
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
                   nodo->opBinaria.op == TOP_IGUAL ? "IGUAL" :
                   nodo->opBinaria.op == TOP_OR ? "OR" :
                   nodo->opBinaria.op == TOP_AND ? "AND" :
                   nodo->opBinaria.op == TOP_MAYOR ? "MAYOR" :
                   nodo->opBinaria.op == TOP_MENOR ? "MENOR" : "OP_UNKNOWN");
            imprimir_nodo(nodo->opBinaria.izq, indent + 1);
            imprimir_nodo(nodo->opBinaria.der, indent + 1);
            break;
        case NODO_ID:
            printf("%-10s%s\n", "ID", nodo->nombre);
            break;
        case NODO_INT:
            printf("%-10s%d\n", "INT", nodo->val_int);
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

/* ------------------ Intérprete / wrapper de ejecución ------------------ */

/**
 * Interpreta un programa completo, imprimiendo detalles.
 * @param programa Raíz del AST del programa.
 * @return Resultado de la ejecución.
 */
int interpretar_programa(Nodo *programa) {
    liberar_tabla_simbolos();

    printf("\n");
    print_divider("EJECUCION DEL PROGRAMA");

    printf("Árbol Sintáctico Abstracto (AST):\n");
    printf("----------------------------------------\n");
    if (programa)
        imprimir_nodo(programa, 0);
    else
        printf("(AST vacío)\n");
    printf("\n");

    int resultado = 0;
    if (programa)
        resultado = eval_nodo(programa);

    printf("Tabla de Símbolos:\n");
    printf("----------------------------------------\n");
    if (tabla_size > 0) {
        int max_len = 8; // Ancho mínimo para "Variable"
        for (int i = 0; i < tabla_size; ++i) {
            int len = (int)strlen(tabla[i].id);
            if (len > max_len) max_len = len;
        }
        printf("%-*s %-10s %-15s\n", max_len, "Variable", "Valor", "Estado");
        printf("----------------------------------------\n");
        for (int i = 0; i < tabla_size; ++i) {
            printf("%-*s %-10d %-15s\n",
                   max_len, tabla[i].id, tabla[i].valor,
                   tabla[i].inicializado ? "Inicializado" : "No inicializado");
        }
    } else {
        printf("(Tabla vacía)\n");
    }
    printf("\n");

    print_divider("FIN DE LA EJECUCION");

    return resultado;
}

/**
 * Libera recursos globales del AST.
 */
void ast_liberar_recursos(void) {
    liberar_tabla_simbolos();
}

/* ------------------ Generador de seudo-assembly ------------------ */

/* Contador para etiquetas únicas */
static int gen_label_counter = 0;

/**
 * Genera una nueva etiqueta única.
 * @return Número de etiqueta.
 */
static int nueva_etiqueta(void) { return gen_label_counter++; }

/**
 * Emite una línea de código assembly al archivo.
 * @param f Archivo de salida.
 * @param fmt Formato de la línea.
 * @param ... Argumentos variables.
 */
static void emit(FILE *f, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    fprintf(f, "\n");
    va_end(ap);
}

/**
 * Obtiene el mnemónico para una operación binaria.
 * @param op Tipo de operación.
 * @return Cadena con el mnemónico.
 */
static const char *op_mnemonic(TipoOP op) {
    switch (op) {
        case TOP_SUMA:  return "ADD";
        case TOP_RESTA: return "SUB";
        case TOP_MULT:  return "MUL";
        case TOP_DIV:   return "DIV";
        case TOP_IGUAL: return "CMPEQ";
        case TOP_MAYOR: return "CMPGT";
        case TOP_MENOR: return "CMPLT";
        default:        return "??";
    }
}

/* Declaraciones forward */
static void gen_stmt(FILE *f, Nodo *n);
static void gen_expr(FILE *f, Nodo *n);

/**
 * Genera código para una expresión (deja valor en la pila).
 * @param f Archivo de salida.
 * @param n Nodo de la expresión.
 */
static void gen_expr(FILE *f, Nodo *n) {
    if (!n) { emit(f, "PUSH 0"); return; }

    switch (n->tipo) {
        case NODO_INT:
            emit(f, "PUSH %d", n->val_int);
            break;
        case NODO_BOOL:
            emit(f, "PUSH %d", n->val_bool ? 1 : 0);
            break;
        case NODO_ID:
            emit(f, "LOAD %s", n->nombre);
            break;
        case NODO_OP: {
            if (n->opBinaria.op == TOP_AND) {
                int L_false = nueva_etiqueta();
                int L_end = nueva_etiqueta();
                gen_expr(f, n->opBinaria.izq);
                emit(f, "JZ L%d", L_false);
                gen_expr(f, n->opBinaria.der);
                emit(f, "JZ L%d", L_false);
                emit(f, "PUSH 1");
                emit(f, "JMP L%d", L_end);
                emit(f, "LABEL L%d", L_false);
                emit(f, "PUSH 0");
                emit(f, "LABEL L%d", L_end);
            } else if (n->opBinaria.op == TOP_OR) {
                int L_true = nueva_etiqueta();
                int L_end = nueva_etiqueta();
                gen_expr(f, n->opBinaria.izq);
                emit(f, "JNZ L%d", L_true);
                gen_expr(f, n->opBinaria.der);
                emit(f, "JNZ L%d", L_true);
                emit(f, "PUSH 0");
                emit(f, "JMP L%d", L_end);
                emit(f, "LABEL L%d", L_true);
                emit(f, "PUSH 1");
                emit(f, "LABEL L%d", L_end);
            } else {
                gen_expr(f, n->opBinaria.izq);
                gen_expr(f, n->opBinaria.der);
                emit(f, "%s", op_mnemonic(n->opBinaria.op));
            }
            break;
        }
        default:
            gen_stmt(f, n);
            emit(f, "PUSH 0");
            break;
    }
}

/**
 * Genera código para una sentencia.
 * @param f Archivo de salida.
 * @param n Nodo de la sentencia.
 */
static void gen_stmt(FILE *f, Nodo *n) {
    if (!n) return;

    switch (n->tipo) {
        case NODO_SEQ:
            gen_stmt(f, n->opBinaria.izq);
            gen_stmt(f, n->opBinaria.der);
            break;
        case NODO_DECL:
            emit(f, "DECL %s", n->assign.id);
            if (n->assign.expr) {
                gen_expr(f, n->assign.expr);
                emit(f, "STORE %s", n->assign.id);
            }
            break;
        case NODO_ASSIGN:
            gen_expr(f, n->assign.expr);
            emit(f, "STORE %s", n->assign.id);
            break;
        case NODO_RETURN:
            if (n->ret_expr) {
                gen_expr(f, n->ret_expr);
            } else {
                emit(f, "PUSH 0");
            }
            emit(f, "RET");
            break;
        case NODO_OP:
        case NODO_INT:
        case NODO_BOOL:
        case NODO_ID:
            gen_expr(f, n);
            break;
        default:
            fprintf(stderr, "Codegen: nodo no soportado en stmt %d\n", n->tipo);
            break;
    }
}

/**
 * Genera código seudo-assembly para el programa.
 * @param programa Raíz del AST.
 * @param filename Nombre del archivo de salida (NULL para stdout).
 */
void generar_asm(Nodo *programa, const char *filename) {
    FILE *f = filename ? fopen(filename, "w") : stdout;
    if (!f) {
        perror("fopen generar_asm");
        return;
    }
    emit(f, "; ---------- PSEUDO-ASM GENERADO ----------");
    emit(f, "BEGIN");

    if (programa) gen_stmt(f, programa);

    emit(f, "HALT");
    if (filename) fclose(f);
}