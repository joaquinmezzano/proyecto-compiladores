#define _POSIX_C_SOURCE 200809L

#include "semantics.h"

// Variables globales para análisis semántico
int semantic_errors = 0;
DataType current_function_return_type = TYPE_VOID;

// Variable global para el AST (definida en sintaxis.y)
extern Nodo *ast;

// Función principal del análisis semántico
int semantic_analysis(Nodo *ast_root) {
    if (!ast_root) {
        printf("Error: AST vacío para análisis semántico\n");
        return 1;
    }
    
    printf("\n --- INICIANDO ANÁLISIS SEMÁNTICO ---\n");
    semantic_errors = 0;
    
    // Analizar todo el programa
    analyze_node(ast_root);
    
    if (semantic_errors > 0) {
        printf("\n❌ Análisis semántico FALLÓ con %d errores\n", semantic_errors);
        return 1;
    } else {
        printf("\n✅ Análisis semántico EXITOSO - sin errores\n");
        return 0;
    }
}

// Función para reportar errores semánticos
void semantic_error(const char *message, int line) {
    fprintf(stderr, "Error semántico");
    if (line > 0) {
        fprintf(stderr, " en línea %d", line);
    }
    fprintf(stderr, ": %s\n", message);
    semantic_errors++;
}

// Convertir string de tipo a DataType
DataType get_type_from_string(const char *type_str) {
    if (!type_str) return TYPE_ERROR;
    
    if (strcmp(type_str, "integer") == 0) return TYPE_INTEGER;
    if (strcmp(type_str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(type_str, "void") == 0) return TYPE_VOID;
    if (strstr(type_str, "function") != NULL) return TYPE_FUNCTION;
    
    return TYPE_ERROR;
}

// Convertir DataType a string
const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_INTEGER: return "integer";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_FUNCTION: return "function";
        case TYPE_ERROR: return "error";
        default: return "unknown";
    }
}

// Verificar compatibilidad de tipos
int types_compatible(DataType type1, DataType type2) {
    return type1 == type2;
}

// Crear información de tipo
TypeInfo* create_type_info(DataType type) {
    TypeInfo *info = malloc(sizeof(TypeInfo));
    if (!info) {
        perror("malloc TypeInfo");
        exit(EXIT_FAILURE);
    }
    info->type = type;
    info->is_function = (type == TYPE_FUNCTION);
    info->return_type = TYPE_VOID;
    info->param_count = 0;
    info->param_types = NULL;
    return info;
}

// Liberar información de tipo
void free_type_info(TypeInfo *info) {
    if (info) {
        if (info->param_types) {
            free(info->param_types);
        }
        free(info);
    }
}

// Obtener tipo de retorno de un símbolo de función
DataType get_return_type(Symbol *sym) {
    if (!sym || !sym->type) return TYPE_ERROR;
    if (strncmp(sym->type, "function:", 9) == 0) {
        return get_type_from_string(sym->type + 9);
    }
    return TYPE_ERROR;
}

// Análisis de nodo genérico (dispatcher)
TypeInfo* analyze_node(Nodo *node) {
    if (!node) return NULL;
    
    TypeInfo *current_result = NULL;
    
    switch (node->tipo) {
        case NODO_PROG:
        case NODO_ID:
            // Solo procesar hermanos
            break;
            
        case NODO_INTEGER:
            current_result = create_type_info(TYPE_INTEGER);
            break;
            
        case NODO_BOOL:
            current_result = create_type_info(TYPE_BOOL);
            break;
            
        case NODO_OP:
            current_result = analyze_binary_operation(node);
            break;
            
        case NODO_ASSIGN:
            current_result = analyze_assignment(node);
            break;
            
        case NODO_DECL:
            current_result = analyze_declaration(node);
            break;
            
        case NODO_METHOD:
            current_result = analyze_method(node);
            break;
            
        case NODO_METHOD_CALL:
            current_result = analyze_method_call(node);
            break;
            
        case NODO_IF:
            current_result = analyze_if_statement(node);
            break;
            
        case NODO_WHILE:
            current_result = analyze_while_statement(node);
            break;
            
        case NODO_RETURN:
            current_result = analyze_return_statement(node);
            break;
            
        default:
            // Para otros tipos, no hacer nada especial
            break;
    }
    
    // Procesar nodos hermanos
    if (node->siguiente) {
        analyze_node(node->siguiente);
    }
    
    return current_result;
}

// Análisis de operaciones binarias
TypeInfo* analyze_binary_operation(Nodo *op_node) {
    if (!op_node || op_node->tipo != NODO_OP) return NULL;
    
    TypeInfo *left_type = NULL;
    TypeInfo *right_type = NULL;
    
    // Analizar operandos
    if (op_node->opBinaria.izq) {
        left_type = analyze_expression(op_node->opBinaria.izq);
    }
    
    if (op_node->opBinaria.der) {
        right_type = analyze_expression(op_node->opBinaria.der);
    }
    
    TypeInfo *result = create_type_info(TYPE_ERROR);
    
    // Verificar compatibilidad según el operador
    switch (op_node->opBinaria.op) {
        case TOP_SUMA:
        case TOP_RESTA:
        case TOP_MULT:
        case TOP_DIV:
        case TOP_RESTO:
            // Operaciones aritméticas: requieren enteros
            if (left_type && right_type && 
                left_type->type == TYPE_INTEGER && right_type->type == TYPE_INTEGER) {
                result->type = TYPE_INTEGER;
            } else {
                semantic_error("Operación aritmética requiere operandos enteros", yylineno);
            }
            break;
            
        case TOP_MAYOR:
        case TOP_MENOR:
        case TOP_MAYORIG:
        case TOP_MENORIG:
            // Comparaciones: enteros -> bool
            if (left_type && right_type && 
                left_type->type == TYPE_INTEGER && right_type->type == TYPE_INTEGER) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Comparación requiere operandos enteros", yylineno);
            }
            break;
            
        case TOP_COMP:
        case TOP_DESIGUAL:
            // Igualdad/desigualdad: tipos compatibles -> bool
            if (left_type && right_type && types_compatible(left_type->type, right_type->type)) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Comparación de igualdad requiere tipos compatibles", yylineno);
            }
            break;
            
        case TOP_AND:
        case TOP_OR:
            // Operaciones lógicas: bool -> bool
            if (left_type && right_type && 
                left_type->type == TYPE_BOOL && right_type->type == TYPE_BOOL) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Operación lógica requiere operandos booleanos", yylineno);
            }
            break;
            
        case TOP_NOT:
            // Negación: solo operando derecho, debe ser bool
            if (right_type && right_type->type == TYPE_BOOL) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Negación lógica requiere operando booleano", yylineno);
            }
            break;
            
        default:
            semantic_error("Operador desconocido", yylineno);
            break;
    }
    
    // Liberar tipos temporales
    free_type_info(left_type);
    free_type_info(right_type);
    
    return result;
}

// Análisis de expresiones
TypeInfo* analyze_expression(Nodo *expr) {
    if (!expr) return NULL;
    
    switch (expr->tipo) {
        case NODO_INTEGER:
            return create_type_info(TYPE_INTEGER);
            
        case NODO_BOOL:
            return create_type_info(TYPE_BOOL);
            
        case NODO_ID: {
            Symbol *sym = search_symbol(expr->nombre);
            if (!sym) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), 
                         "Variable '%s' no declarada", expr->nombre);
                semantic_error(error_msg, yylineno);
                return create_type_info(TYPE_ERROR);
            }
            
            DataType var_type = get_type_from_string(sym->type);
            return create_type_info(var_type);
        }
        
        case NODO_OP:
            return analyze_binary_operation(expr);
            
        case NODO_METHOD_CALL:
            return analyze_method_call(expr);
            
        default:
            return create_type_info(TYPE_ERROR);
    }
}

// Análisis de asignaciones
TypeInfo* analyze_assignment(Nodo *assign_node) {
    if (!assign_node || assign_node->tipo != NODO_ASSIGN) return NULL;
    
    // Verificar que la variable existe
    Symbol *var_sym = search_symbol(assign_node->assign.id);
    if (!var_sym) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Variable '%s' no declarada en asignación", assign_node->assign.id);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    // Analizar expresión del lado derecho
    TypeInfo *expr_type = analyze_expression(assign_node->assign.expr);
    if (!expr_type) {
        semantic_error("Error en expresión de asignación", yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    // Verificar compatibilidad de tipos
    DataType var_type = get_type_from_string(var_sym->type);
    if (!types_compatible(var_type, expr_type->type)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Tipos incompatibles en asignación: %s := %s", 
                 type_to_string(var_type), type_to_string(expr_type->type));
        semantic_error(error_msg, yylineno);
    }
    
    TypeInfo *result = create_type_info(var_type);
    free_type_info(expr_type);
    return result;
}

// Análisis de declaraciones
TypeInfo* analyze_declaration(Nodo *decl_node) {
    if (!decl_node || decl_node->tipo != NODO_DECL) return NULL;
    
    // Si hay valor inicial, verificar compatibilidad
    if (decl_node->assign.expr) {
        Symbol *var_sym = search_symbol(decl_node->assign.id);
        if (!var_sym) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                     "Variable '%s' no encontrada en tabla de símbolos", decl_node->assign.id);
            semantic_error(error_msg, yylineno);
            return create_type_info(TYPE_ERROR);
        }
        
        TypeInfo *init_type = analyze_expression(decl_node->assign.expr);
        if (init_type) {
            DataType var_type = get_type_from_string(var_sym->type);
            if (!types_compatible(var_type, init_type->type)) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), 
                         "Tipos incompatibles en inicialización: %s := %s", 
                         type_to_string(var_type), type_to_string(init_type->type));
                semantic_error(error_msg, yylineno);
            }
            free_type_info(init_type);
        }
    }
    
    return create_type_info(TYPE_VOID);
}

// Análisis de llamadas a métodos
TypeInfo* analyze_method_call(Nodo *call_node) {
    if (!call_node || call_node->tipo != NODO_METHOD_CALL) return NULL;
    
    // Verificar que la función existe
    Symbol *func_sym = search_symbol(call_node->method_call.nombre);
    if (!func_sym) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Función '%s' no declarada", call_node->method_call.nombre);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    // Verificar que es una función
    if (get_type_from_string(func_sym->type) != TYPE_FUNCTION) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "'%s' no es una función", call_node->method_call.nombre);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    // Analizar argumentos (simplificado)
    if (call_node->method_call.args) {
        analyze_node(call_node->method_call.args);
    }
    
    // Retornar el tipo de retorno real de la función
    return create_type_info(get_return_type(func_sym));
}

// Análisis de definiciones de métodos
TypeInfo* analyze_method(Nodo *method_node) {
    if (!method_node || method_node->tipo != NODO_METHOD) return NULL;
    
    Symbol *func_sym = search_symbol(method_node->method.nombre);
    if (!func_sym) {
        semantic_error("Función no encontrada en TS", yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    DataType return_type = get_return_type(func_sym);
    
    DataType old_return_type = current_function_return_type;
    current_function_return_type = return_type;
    
    SymbolTable *old_current = current_table;
    SymbolTable *func_scope = get_function_scope(method_node->method.nombre);
    if (func_scope) {
        current_table = func_scope;
    } else {
        semantic_error("Scope de función no encontrado", yylineno);
    }
    
    // Analizar cuerpo de la función si existe
    if (method_node->method.body) {
        analyze_node(method_node->method.body);
    }
    
    // Restaurar
    current_table = old_current;
    current_function_return_type = old_return_type;
    
    return create_type_info(TYPE_FUNCTION);
}

// Análisis de sentencias if
TypeInfo* analyze_if_statement(Nodo *if_node) {
    if (!if_node || if_node->tipo != NODO_IF) return NULL;
    
    // Analizar condición
    TypeInfo *cond_type = analyze_expression(if_node->if_stmt.cond);
    if (cond_type) {
        if (cond_type->type != TYPE_BOOL) {
            semantic_error("Condición de if debe ser booleana", yylineno);
        }
        free_type_info(cond_type);
    }
    
    // Analizar bloque then
    if (if_node->if_stmt.then_block) {
        analyze_node(if_node->if_stmt.then_block);
    }
    
    // Analizar bloque else si existe
    if (if_node->if_stmt.else_block) {
        analyze_node(if_node->if_stmt.else_block);
    }
    
    return create_type_info(TYPE_VOID);
}

// Análisis de sentencias while
TypeInfo* analyze_while_statement(Nodo *while_node) {
    if (!while_node || while_node->tipo != NODO_WHILE) return NULL;
    
    // Analizar condición
    TypeInfo *cond_type = analyze_expression(while_node->while_stmt.cond);
    if (cond_type) {
        if (cond_type->type != TYPE_BOOL) {
            semantic_error("Condición de while debe ser booleana", yylineno);
        }
        free_type_info(cond_type);
    }
    
    // Analizar cuerpo del while
    if (while_node->while_stmt.body) {
        analyze_node(while_node->while_stmt.body);
    }
    
    return create_type_info(TYPE_VOID);
}

// Análisis de sentencias return
TypeInfo* analyze_return_statement(Nodo *return_node) {
    if (!return_node || return_node->tipo != NODO_RETURN) return NULL;
    
    if (return_node->ret_expr) {
        TypeInfo *return_type = analyze_expression(return_node->ret_expr);
        if (return_type) {
            // Verificar compatibilidad con tipo de retorno de función actual
            if (!types_compatible(current_function_return_type, return_type->type)) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), 
                         "Tipo de retorno incompatible: esperado %s, obtenido %s", 
                         type_to_string(current_function_return_type), 
                         type_to_string(return_type->type));
                semantic_error(error_msg, yylineno);
            }
            free_type_info(return_type);
        }
    } else {
        // Return vacío
        if (current_function_return_type != TYPE_VOID) {
            semantic_error("Función no-void debe retornar un valor", yylineno);
        }
    }
    
    return create_type_info(TYPE_VOID);
}