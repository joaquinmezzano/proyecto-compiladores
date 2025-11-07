#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "intermediate.h"
#include <stdbool.h>

/*
 * Variable global para controlar si las optimizaciones están habilitadas
 */
extern int optimizer_enabled;

/*
 * Estructura para análisis de uso de variables
 */
typedef struct {
    char *var_name;
    int last_use;       // Índice de la última instrucción que usa esta variable
    int def_count;      // Cantidad de definiciones
    int use_count;      // Cantidad de usos
    bool is_dead;       // Marca si la variable está muerta
} VarUsage;

typedef struct {
    VarUsage *vars;
    int count;
    int capacity;
} VarUsageTable;

/*
 * Funciones principales de optimización
 */
// Funciones para instrucciones IR
void optimize_peephole(IRList *list);
void optimize_constant_folding(IRList *list);
void optimize_constant_propagation(IRList *list);
void optimize_dead_code_elimination(IRList *list);
void optimize_algebraic_simplification(IRList *list);

// Funciones para AST
Nodo *optimize_ast_constant_folding(Nodo *node);
Nodo *optimize_ast_algebraic_simplification(Nodo *node);

// Funciones para ejecutar las optimizaciones
void optimize_ir_code(IRList *list);
Nodo *optimize_ast(Nodo *ast_root);

/*
 * Funciones auxiliares
 */
bool is_power_of_two(int n);
int log2_int(int n);
bool is_constant_symbol(IRSymbol *sym);
int get_constant_value(IRSymbol *sym);
void replace_instruction(IRList *list, int index, IRInstr new_op, 
                        IRSymbol *new_arg1, IRSymbol *new_arg2, IRSymbol *new_result);
void mark_instruction_as_nop(IRList *list, int index);
void compact_ir_list(IRList *list);

#endif