#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

typedef enum {
    TYPE_INTEGER,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_FUNCTION,
    TYPE_ERROR
} DataType;

typedef struct {
    DataType type;
    int is_function;
    DataType return_type;
    int param_count;
    DataType *param_types;
} TypeInfo;

extern int yylineno;
extern int semantic_errors;

int semantic_analysis(Nodo *ast_root);
void semantic_error(const char *message, int line);
DataType get_type_from_string(const char *type_str);
const char* type_to_string(DataType type);
int types_compatible(DataType type1, DataType type2);
TypeInfo* create_type_info(DataType type);
void free_type_info(TypeInfo *info);
DataType get_return_type(Symbol *sym);
TypeInfo* analyze_node(Nodo *node);
TypeInfo* analyze_binary_operation(Nodo *op_node);
TypeInfo* analyze_expression(Nodo *expr);
TypeInfo* analyze_assignment(Nodo *assign_node);
TypeInfo* analyze_declaration(Nodo *decl_node);
TypeInfo* analyze_method_call(Nodo *call_node);
TypeInfo* analyze_method(Nodo *method_node);
TypeInfo* analyze_if_statement(Nodo *if_node);
TypeInfo* analyze_while_statement(Nodo *while_node);
TypeInfo* analyze_return_statement(Nodo *return_node);

#endif