#ifndef OBJECT_H
#define OBJECT_H

#include "intermediate.h"

/*
 * Estructuras necesarias.
 */
typedef struct {
    char **lines;
    int size;
    int capacity;
} ObjectCode;

typedef struct {
    char *name;
    int offset;
} VarInfo;

typedef struct {
    VarInfo *vars;
    int count;
    int capacity;
    int stack_size;
} VarTable;

/*
 * Declaraciones de funciones a definir.
 */
void object_init(ObjectCode *obj);
void object_free(ObjectCode *obj);
void var_table_init(VarTable *table);
void var_table_free(VarTable *table);

void object_emit(ObjectCode *obj, const char *line);
int var_table_add(VarTable *table, const char *name);
int var_table_get_offset(VarTable *table, const char *name);
int is_temp_var(const char *name);
int is_constant(const char *name);
int is_label(const char *name);
const char* get_register_for_temp(const char *temp_name);

void translate_prologue(ObjectCode *obj, const char *func_name, VarTable *vars);
void translate_epilogue(ObjectCode *obj);
void translate_ir_instruction(ObjectCode *obj, IRCode *code, VarTable *vars);
int generate_object_code(const char *ir_filename, const char *output_filename);

#endif