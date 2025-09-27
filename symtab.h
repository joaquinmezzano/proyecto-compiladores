#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Symbol {
    char *name;
    char *type;
    int scope_level;  // Nivel de scope donde fue declarado
} Symbol;

typedef struct SymbolTable {
    struct SymbolTable *parent;
    Symbol *symbols;
    int num_symbols;
    struct SymbolTable **children;
    int num_children;
    char *function_name;  // NUEVO: nombre de la función
} SymbolTable;

extern SymbolTable *current_table;
extern SymbolTable *global_table;

void init_symtab(void);
void push_scope(void);
void push_scope_for_function(char *function_name);
void pop_scope(void);
Symbol* search_symbol(char *name);
void insert_symbol(char *name, char *type);
void free_symtab(void);
void print_symtab(void);
void debug_print_scopes(void);
SymbolTable* get_function_scope(const char *name);
int get_current_scope_level(void);

#endif