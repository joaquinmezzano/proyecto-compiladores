#define _POSIX_C_SOURCE 200809L

#include "semantics.h"

/*
 * Variables globales para análisis semántico
 */
int semantic_errors = 0;
DataType current_function_return_type = TYPE_VOID;

/*
 * Variable global para el AST (definida en sintaxis.y)
 */
extern Nodo *ast;

/*
 * Función principal del análisis semántico
 */
int semantic_analysis(Nodo *ast_root) {
    if (!ast_root) {
        if (debug_mode) {
            printf("Error: AST vacío para análisis semántico\n");
        }
        return 1;
    }
    
    semantic_errors = 0;
    
    analyze_node(ast_root);
    verify_main_method();
    
    if (semantic_errors > 0) {
        if (debug_mode) {
            printf("\nX Análisis semántico FALLÓ con %d errores\n", semantic_errors);
        }
        return 1;
    } else {
        if (debug_mode) {
            printf("\n✓ Análisis semántico EXITOSO - sin errores\n");
        } else {
            printf("✓ Análisis semántico completado exitosamente.\n");
        }
        return 0;
    }
}

/*
 * Función para reportar errores semánticos
 */
void semantic_error(const char *message, int line) {
    fprintf(stderr, "Error semántico");
    if (line > 0) {
        fprintf(stderr, " en línea %d", line);
    }
    fprintf(stderr, ": %s\n", message);
    semantic_errors++;
}

/*
 * Función para convertir string a DataType
 */
DataType get_type_from_string(const char *type_str) {
    if (!type_str) return TYPE_ERROR;
    
    if (strcmp(type_str, "integer") == 0) return TYPE_INTEGER;
    if (strcmp(type_str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(type_str, "void") == 0) return TYPE_VOID;
    if (strstr(type_str, "function") != NULL) return TYPE_FUNCTION;
    
    return TYPE_ERROR;
}

/*
 * Función para convertir DataType a string
 */
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

/*
 * Función para verificar compatibilidad de tipos entre dos datos distintos 
 */
int types_compatible(DataType type1, DataType type2) {
    return type1 == type2;
}

/*
 * Función para crear información de tipo
 */
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

/*
 * Función para liberar información de tipo 
 */
void free_type_info(TypeInfo *info) {
    if (info) {
        if (info->param_types) {
            free(info->param_types);
        }
        free(info);
    }
}

/*
 * Función para verificar la existencia de un método main en el programa
 */
int verify_main_method() {
    Symbol *main_symbol = NULL;
    
    for (int i = 0; i < global_table->num_symbols; i++) {
        if (strcmp(global_table->symbols[i].name, "main") == 0) {
            main_symbol = &global_table->symbols[i];
            break;
        }
    }
    
    if (!main_symbol) {
        semantic_error("Programa debe contener un método main", 0);
        return 0;
    }
    
    if (strncmp(main_symbol->type, "function:", 9) != 0) {
        semantic_error("main debe ser una función", 0);
        return 0;
    }
    
    const char *return_type = main_symbol->type + 9;
    
    if (strcmp(return_type, "void") != 0 && strcmp(return_type, "integer") != 0) {
        semantic_error("main debe retornar void o integer", 0);
        return 0;
    }
    
    SymbolTable *main_scope = get_function_scope("main");
    if (!main_scope) {
        semantic_error("No se encontró el scope de la función main", 0);
        return 0;
    }
    
    int param_count = 0;
    for (int i = 0; i < main_scope->num_symbols; i++) {
        Symbol *sym = &main_scope->symbols[i];
        if (sym->is_param) {
            param_count++;
        }
    }
    
    if (param_count > 0) {
        semantic_error("El método 'main' no debe tener parámetros", 0);
        return 0;
    }
    
    if (debug_mode) {
        printf("Debug: main válido con tipo %s, parámetros = %d\n",
               return_type, param_count);
    }
    
    return 1;
}

/*
 * Función para obtener el tipo de retorno de un símbolo de una función
 */
DataType get_return_type(Symbol *sym) {
    if (!sym || !sym->type) return TYPE_ERROR;
    if (strncmp(sym->type, "function:", 9) == 0) {
        return get_type_from_string(sym->type + 9);
    }
    return TYPE_ERROR;
}

/*
 * Función para analizar nodos genéricos (dispatcher)
 */
TypeInfo* analyze_node(Nodo *node) {
    if (!node) return NULL;
    
    TypeInfo *current_result = NULL;
    
    switch (node->tipo) {
        case NODO_PROG:
        case NODO_ID:
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
            break;
    }
    
    if (node->siguiente) {
        analyze_node(node->siguiente);
    }
    
    return current_result;
}

/*
 * Función para analizar nodos de operaciones binarias
 */
TypeInfo* analyze_binary_operation(Nodo *op_node) {
    if (!op_node || op_node->tipo != NODO_OP) return NULL;
    
    TypeInfo *left_type = NULL;
    TypeInfo *right_type = NULL;
    
    if (op_node->opBinaria.izq) {
        left_type = analyze_expression(op_node->opBinaria.izq);
    }
    
    if (op_node->opBinaria.der) {
        right_type = analyze_expression(op_node->opBinaria.der);
    }
    
    TypeInfo *result = create_type_info(TYPE_ERROR);
    
    switch (op_node->opBinaria.op) {
        case TOP_SUMA:
        case TOP_RESTA:
        case TOP_MULT:
        case TOP_DIV:
        case TOP_RESTO:
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
            if (left_type && right_type && 
                left_type->type == TYPE_INTEGER && right_type->type == TYPE_INTEGER) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Comparación requiere operandos enteros", yylineno);
            }
            break;
            
        case TOP_COMP:
        case TOP_DESIGUAL:
            if (left_type && right_type && types_compatible(left_type->type, right_type->type)) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Comparación de igualdad requiere tipos compatibles", yylineno);
            }
            break;
            
        case TOP_AND:
        case TOP_OR:
            if (left_type && right_type && 
                left_type->type == TYPE_BOOL && right_type->type == TYPE_BOOL) {
                result->type = TYPE_BOOL;
            } else {
                semantic_error("Operación lógica requiere operandos booleanos", yylineno);
            }
            break;
            
        case TOP_NOT:
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
    
    free_type_info(left_type);
    free_type_info(right_type);
    
    return result;
}

/*
 * Función para analizar nodos de expresiones
 */
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

/*
 * Función para analizar nodos de asignaciones
 */
TypeInfo* analyze_assignment(Nodo *assign_node) {
    if (!assign_node || assign_node->tipo != NODO_ASSIGN) return NULL;
    
    Symbol *var_sym = search_symbol(assign_node->assign.id);
    if (!var_sym) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Variable '%s' no declarada en asignación", assign_node->assign.id);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    TypeInfo *expr_type = analyze_expression(assign_node->assign.expr);
    if (!expr_type) {
        semantic_error("Error en expresión de asignación", yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
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

/*
 * Función para analizar nodos de declaraciones
 */
TypeInfo* analyze_declaration(Nodo *decl_node) {
    if (!decl_node || decl_node->tipo != NODO_DECL) return NULL;
    
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

/*
 * Función para analizar nodos de llamadas a métodos
*/
TypeInfo* analyze_method_call(Nodo *call_node) {
    if (!call_node || call_node->tipo != NODO_METHOD_CALL) return NULL;
    
    Symbol *func_sym = search_symbol(call_node->method_call.nombre);
    if (!func_sym) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Función '%s' no declarada", call_node->method_call.nombre);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }
    
    if (get_type_from_string(func_sym->type) != TYPE_FUNCTION) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "'%s' no es una función", call_node->method_call.nombre);
        semantic_error(error_msg, yylineno);
        return create_type_info(TYPE_ERROR);
    }

    SymbolTable *func_scope = get_function_scope(call_node->method_call.nombre);
    if (func_scope) {
        int param_count = 0;
        for (int i = 0; i < func_scope->num_symbols; i++) {
            Symbol *sym = &func_scope->symbols[i];
            if (sym->is_param) param_count++;
        }
        
        int arg_count = 0;
        for (Nodo *tmp = call_node->method_call.args; tmp; tmp = tmp->siguiente) {
            arg_count++;
        }
        
        if (arg_count != param_count) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg),
                     "Cantidad de argumentos incorrecta en llamada a '%s': esperado %d, recibido %d",
                     call_node->method_call.nombre, param_count, arg_count);
            semantic_error(error_msg, yylineno);
        }

        Nodo *arg = call_node->method_call.args;
        int param_idx = 0;

        for (int i = 0; i < func_scope->num_symbols; i++) {
            Symbol *param_sym = &func_scope->symbols[i];
            if (!param_sym->is_param) continue;

            if (!arg) break;  // ya controlaste la cantidad antes

            TypeInfo *arg_type = analyze_expression(arg);
            DataType param_type = get_type_from_string(param_sym->type);

            if (!types_compatible(param_type, arg_type->type)) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg),
                         "Tipo de argumento %d incorrecto en llamada a '%s': esperado %s, recibido %s",
                         param_idx + 1, call_node->method_call.nombre,
                         type_to_string(param_type), type_to_string(arg_type->type));
                semantic_error(error_msg, yylineno);
            }

            free_type_info(arg_type);
            arg = arg->siguiente;
            param_idx++;
        }
    }
    
    return create_type_info(get_return_type(func_sym));
}

 /*
 * Función para analizar nodos de métodos
 */
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
    
    if (method_node->method.body) {
        analyze_node(method_node->method.body);
    }
    
    current_table = old_current;
    current_function_return_type = old_return_type;
    
    return create_type_info(TYPE_FUNCTION);
}

/*
 * Función para analizar nodos if
 */
TypeInfo* analyze_if_statement(Nodo *if_node) {
    if (!if_node || if_node->tipo != NODO_IF) return NULL;
    
    TypeInfo *cond_type = analyze_expression(if_node->if_stmt.cond);
    if (cond_type) {
        if (cond_type->type != TYPE_BOOL) {
            semantic_error("Condición de if debe ser booleana", yylineno);
        }
        free_type_info(cond_type);
    }
    
    if (if_node->if_stmt.then_block) {
        analyze_node(if_node->if_stmt.then_block);
    }
    
    if (if_node->if_stmt.else_block) {
        analyze_node(if_node->if_stmt.else_block);
    }
    
    return create_type_info(TYPE_VOID);
}

/*
 * Función para analizar nodos while
 */
TypeInfo* analyze_while_statement(Nodo *while_node) {
    if (!while_node || while_node->tipo != NODO_WHILE) return NULL;
    
    TypeInfo *cond_type = analyze_expression(while_node->while_stmt.cond);
    if (cond_type) {
        if (cond_type->type != TYPE_BOOL) {
            semantic_error("Condición de while debe ser booleana", yylineno);
        }
        free_type_info(cond_type);
    }
    
    if (while_node->while_stmt.body) {
        analyze_node(while_node->while_stmt.body);
    }
    
    return create_type_info(TYPE_VOID);
}

/*
 * Función para analizar nodos de return
 */
TypeInfo* analyze_return_statement(Nodo *return_node) {
    if (!return_node || return_node->tipo != NODO_RETURN) return NULL;
    
    if (return_node->ret_expr) {
        TypeInfo *return_type = analyze_expression(return_node->ret_expr);
        if (return_type) {
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
        if (current_function_return_type != TYPE_VOID) {
            semantic_error("Función no-void debe retornar un valor", yylineno);
        }
    }
    
    return create_type_info(TYPE_VOID);
}