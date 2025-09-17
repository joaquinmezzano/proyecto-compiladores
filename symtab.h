#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

typedef struct Symbol {
    char *name;            // identificador
    char *type;            // int, bool
    int level;             // nivel del scope
    struct Symbol next;    // Lista enlazada dentro de la tabla
} Symbol;

typedef struct SymbolTable {
    int level;                      // 0 = global, 1..n = funciones
    Symbol *symbols;                // lista de simbolos en este nivel
    struct SymbolTable *previous;   // apunta al nivel anterior (stack de tablas)
} SymbolTable;

extern SymbolTable *current_table;

void init_symtab();                         // Inicializa la tabla global
void push_scope();                          // Entra en un nuevo nivel
void pop_scope();                           // Sale del nivel y libera los simbolos del mismo
void insert_symbol(char *name, char *type); // Inserta en la tabla actual (la del tope)
Symbol search_symbol(char *name);           // Busca desde la tabla actual hacia atras (subiendo en la pila)
void print_symtab();

#endif