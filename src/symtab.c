#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *current_table = NULL;
SymbolTable *global_table = NULL;

/*
 * Función para inicializar la tabla de simbolos con el nivel 0
 */
void init_symtab() {
    global_table = malloc(sizeof(SymbolTable));
    if (!global_table) {
        perror("malloc global_table");
        exit(EXIT_FAILURE);
    }
    global_table->parent = NULL;
    global_table->symbols = NULL;
    global_table->num_symbols = 0;
    global_table->children = NULL;
    global_table->num_children = 0;
    global_table->function_name = NULL;
    current_table = global_table;
}

/*
 * Función para pushear una scope (subir 1 nivel) para una función en especifico 
 */
void push_scope_for_function(char *function_name) {
    SymbolTable *new_scope = malloc(sizeof(SymbolTable));
    if (!new_scope) {
        perror("malloc new_scope");
        exit(EXIT_FAILURE);
    }
    new_scope->parent = current_table;
    new_scope->symbols = NULL;
    new_scope->num_symbols = 0;
    new_scope->children = NULL;
    new_scope->num_children = 0;
    new_scope->function_name = function_name ? strdup(function_name) : NULL;  // NUEVO
    
    // Agregar como hijo al scope padre
    if (current_table != global_table) {
        current_table->children = realloc(current_table->children, 
                                          (current_table->num_children + 1) * sizeof(SymbolTable *));
        if (!current_table->children) {
            perror("realloc children");
            exit(EXIT_FAILURE);
        }
        current_table->children[current_table->num_children++] = new_scope;
    } else {
        global_table->children = realloc(global_table->children, 
                                         (global_table->num_children + 1) * sizeof(SymbolTable *));
        if (!global_table->children) {
            perror("realloc children");
            exit(EXIT_FAILURE);
        }
        global_table->children[global_table->num_children++] = new_scope;
    }
    
    current_table = new_scope;
}

/*
 * Función para pushear un scope (subir 1 nivel) sin ninguna función especificada
 */
void push_scope() {
    push_scope_for_function(NULL);
}

/*
 * Función para popear un scope (bajar 1 nivel)
 */
void pop_scope() {
    if (current_table == global_table) {
        fprintf(stderr, "Warning: intentando pop del scope global\n");
        return;
    }
    
    current_table = current_table->parent;
}

/*
 * Función para buscar un simbolo en toda la SymbolTable 
 */
Symbol* search_symbol(char *name) {
    SymbolTable *scope = current_table;
    while (scope) {
        for (int i = 0; i < scope->num_symbols; i++) {
            if (strcmp(scope->symbols[i].name, name) == 0) {
                return &scope->symbols[i];
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

/*
 * Función auxiliar para calcular el nivel del scope actual (0 = global, 1 = primer nivel, etc)
 */
int get_current_scope_level() {
    int level = 0;
    SymbolTable *scope = current_table;
    while (scope != global_table) {
        scope = scope->parent;
        level++;
    }
    return level;
}

/*
 * Función para insertar un simbolo en el scope actual
 */
void insert_symbol(char *name, char *type, int isparam) {
    for (int i = 0; i < current_table->num_symbols; i++) {
        if (strcmp(current_table->symbols[i].name, name) == 0) {
            fprintf(stderr, "Warning: redeclaración de '%s' en scope actual\n", name);
            exit(EXIT_FAILURE);
        }
    }
    
    current_table->symbols = realloc(current_table->symbols, 
                                   (current_table->num_symbols + 1) * sizeof(Symbol));
    if (!current_table->symbols) {
        perror("realloc symbols");
        exit(EXIT_FAILURE);
    }
    
    Symbol *new_sym = &current_table->symbols[current_table->num_symbols];
    new_sym->name = strdup(name);
    new_sym->type = strdup(type);
    new_sym->is_param = isparam;
    new_sym->scope_level = get_current_scope_level();
    
    current_table->num_symbols++;
}

/*
 * Función auxiliar para liberar un scope y todos sus descendientes
 */
static void free_scope(SymbolTable *scope) {
    if (!scope) return;
    
    for (int i = 0; i < scope->num_symbols; i++) {
        free(scope->symbols[i].name);
        free(scope->symbols[i].type);
    }
    free(scope->symbols);
    
    if (scope->function_name) {
        free(scope->function_name);
    }
    
    for (int i = 0; i < scope->num_children; i++) {
        free_scope(scope->children[i]);
    }
    free(scope->children);
    
    free(scope);
}

/*
 * Función para liberar toda la estructura de scopes
 */
void free_symtab() {
    if (global_table) {
        free_scope(global_table);
        global_table = NULL;
        current_table = NULL;
    }
}

/*
 * Función para imprimir el scope
 */
static void print_scope(SymbolTable *scope, int level) {
    if (scope->num_symbols == 0 && scope->num_children == 0) {
        return;
    }
    
    if (scope->function_name) {
        printf("--- SCOPE LEVEL %d (%s) ---\n", level, scope->function_name);
    } else {
        printf("--- SCOPE LEVEL %d ---\n", level);
    }
    
    if (scope->num_symbols == 0) {
        printf("  (scope de función - sin variables locales)\n");
    } else {
        for (int i = 0; i < scope->num_symbols; i++) {
            printf("  %s: %s (nivel %d)\n", 
                   scope->symbols[i].name, 
                   scope->symbols[i].type,
                   scope->symbols[i].scope_level);
        }
    }
    
    for (int i = 0; i < scope->num_children; i++) {
        print_scope(scope->children[i], level + 1);
    }
}

/*
 * Función para imprimir la tabla de simbolos
 */
void print_symtab() {
    printf("\n ------------------------");
    printf("\n| Tabla de Simbolos (TS) |");
    printf("\n ------------------------\n");
    print_scope(global_table, 0);
    printf("\n");
}

/*
 * Función para mostrar el estado actual de los scopes 
 */
void debug_print_scopes() {
    printf("\n--- DEBUG: Estado actual de scopes ---\n");
    SymbolTable *scope = current_table;
    int level = get_current_scope_level();
    while (scope) {
        printf("Nivel %d: %d símbolos", level, scope->num_symbols);
        if (scope->num_symbols > 0) {
            printf(" (");
            for (int i = 0; i < scope->num_symbols; i++) {
                if (i > 0) printf(", ");
                printf("%s", scope->symbols[i].name);
            }
            printf(")");
        }
        printf("\n");
        scope = scope->parent;
        level--;
    }
    printf("------------------------------------\n\n");
}

/*
 * Función para obtener el scope la función especificada como parametro
 */
SymbolTable* get_function_scope(const char* name) {
    if (!name) return NULL;
    
    for (int i = 0; i < global_table->num_children; i++) {
        SymbolTable* child = global_table->children[i];
        if (child->function_name && strcmp(child->function_name, name) == 0) {
            return child;
        }
    }
    return NULL;
}