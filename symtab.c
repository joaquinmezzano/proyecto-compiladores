#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *current_table = NULL;
SymbolTable *global_table = NULL;

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
    current_table = global_table;
}

void push_scope() {
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
    
    // Solo agregar como hijo si no es el scope global
    if (current_table != global_table) {
        current_table->children = realloc(current_table->children, 
                                          (current_table->num_children + 1) * sizeof(SymbolTable *));
        if (!current_table->children) {
            perror("realloc children");
            exit(EXIT_FAILURE);
        }
        current_table->children[current_table->num_children++] = new_scope;
    } else {
        // Para scopes directos del global, agregarlos directamente
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

void pop_scope() {
    if (current_table == global_table) {
        fprintf(stderr, "Warning: intentando pop del scope global\n");
        return;
    }
    
    current_table = current_table->parent;
}

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

// Función auxiliar para calcular el nivel del scope actual (0 = global, 1 = primer nivel, etc.)
static int get_current_scope_level() {
    int level = 0;
    SymbolTable *scope = current_table;
    while (scope != global_table) {
        scope = scope->parent;
        level++;
    }
    return level;
}

void insert_symbol(char *name, char *type) {
    // Verificar si ya existe en el scope actual
    for (int i = 0; i < current_table->num_symbols; i++) {
        if (strcmp(current_table->symbols[i].name, name) == 0) {
            fprintf(stderr, "Warning: redeclaración de '%s' en scope actual\n", name);
            return;
        }
    }
    
    // Redimensionar array de símbolos
    current_table->symbols = realloc(current_table->symbols, 
                                   (current_table->num_symbols + 1) * sizeof(Symbol));
    if (!current_table->symbols) {
        perror("realloc symbols");
        exit(EXIT_FAILURE);
    }
    
    // Agregar nuevo símbolo
    Symbol *new_sym = &current_table->symbols[current_table->num_symbols];
    new_sym->name = strdup(name);
    new_sym->type = strdup(type);
    new_sym->scope_level = get_current_scope_level();  // Nivel correcto
    
    current_table->num_symbols++;
}

// Función auxiliar para liberar un scope y todos sus descendientes
static void free_scope(SymbolTable *scope) {
    if (!scope) return;
    
    // Liberar símbolos
    for (int i = 0; i < scope->num_symbols; i++) {
        free(scope->symbols[i].name);
        free(scope->symbols[i].type);
    }
    free(scope->symbols);
    
    // Liberar scopes hijos
    for (int i = 0; i < scope->num_children; i++) {
        free_scope(scope->children[i]);
    }
    free(scope->children);
    
    // Liberar scope actual
    free(scope);
}

// Función para liberar toda la estructura de scopes
void free_symtab() {
    if (global_table) {
        free_scope(global_table);
        global_table = NULL;
        current_table = NULL;
    }
}

static void print_scope(SymbolTable *scope, int level) {
    // Solo imprimir scopes que tengan símbolos o sean scopes de función
    if (scope->num_symbols == 0 && scope->num_children == 0) {
        return; // No imprimir scopes completamente vacíos
    }
    
    printf("=== SCOPE LEVEL %d ===\n", level);
    
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
    
    // Recursivamente imprimir scopes hijos solo si tienen contenido
    for (int i = 0; i < scope->num_children; i++) {
        print_scope(scope->children[i], level + 1);
    }
}

void print_symtab() {
    printf("TABLA DE SÍMBOLOS COMPLETA:\n");
    print_scope(global_table, 0);
    printf("==================\n");
}

// Función de debug para mostrar estado actual de scopes
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