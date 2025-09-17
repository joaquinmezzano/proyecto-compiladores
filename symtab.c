#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *current_table = NULL;

void init_symtab() {
    // Caso en que se vuelva a llamar
    while (current_table != NULL) {
        pop_scope();
    }

    SymbolTable *global = (SymbolTable *) malloc(sizeof(SymbolTable));
    if (!global) {
        fprintf(stderr, "Error: can't assign memory for global scope.");
        exit(1);
    }
    global->level = 0;
    global->symbols = NULL;
    global->previous = NULL;
    current_table = global;
}

void push_scope() {
    // Caso en el que se llame a la función sin previamente inicializar la tabla global.
    if (current_table == NULL) {
        init_symtab();
        return;
    }
    
    SymbolTable *new = (SymbolTable *) malloc(sizeof(SymbolTable));
    if(!new) {
        fprintf(stderr, "Error: can't assign memory for new symbol table.");
        exit(1);
    }

    new->level = (current_table ? current_table->level+1 : 0);
    new->symbols = NULL;
    new->previous = current_table;
    current_table = new;
}

void pop_scope() {
    if(!current_table) return;

    Symbol *s = current_table->symbols;
    while(s) {
        Symbol *next = s->next;
        free(s->name);
        free(s->type);
        free(s);
        s = next;
    }

    SymbolTable *previous = current_table->previous;
    free(current_table);
    current_table = previous;
}

void insert_symbol(char *name, char *type) {
    if (!current_table) {
        fprintf(stderr, "Error: NON active scope found to insert symbol %s\n", name);
        return;
    }

    // Verifica si existe el simbolo en el scoup actual
    for(Symbol *s = current_table->symbols; s != NULL; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            fprintf(stderr, "Error: symbol %s already declared on current scope.\n", name);
            return;
        }
    }

    Symbol *new = (Symbol *) malloc(sizeof(Symbol));
    if (!new) {
        fprintf(stderr, "Error: Can't assign memory for the symbol.\n");
        exit(1);
    }
    new->name = strdup(name);
    new->type = strdup(type);
    new->level = current_table->level;

    // Insertar al principio de la lista
    new->next = current_table->symbols;
    current_table->symbols = new;
}

Symbol *search_symbol(char *name) {
    for (SymbolTable *t = current_table; t != NULL; t = t->previous) {
        for (Symbol *s = t->symbols; s != NULL; s = s->next) {
            if (strcmp(s->name, name) == 0) {
                return s;
            }
        }
    }

    return NULL;
}

void print_symtab() {
    printf("\n--- Tablas de símbolos ---\n");
    for (SymbolTable *t = current_table; t != NULL; t = t->previous) {
        printf("Nivel %d:\n", t->level);
        for (Symbol *s = t->symbols; s != NULL; s = s->next) {
            printf(" %s : %s\n", s->name, s->type);
        }
    }
    printf("---------------------------\n");
}