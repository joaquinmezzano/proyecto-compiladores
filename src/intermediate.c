#include "intermediate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const char *ir_names[] = {
    "LOAD", "STORE", "ADD", "SUB", "UMINUS", "MUL", "DIV", "MOD",
    "AND", "OR", "NOT", "EQ", "NEQ", "LT", "LE", "GT", "GE", "LABEL",
    "GOTO", "IF_FALSE", "IF_TRUE", "RETURN", "CALL", "METHOD", "EXTERN",
    "PARAM", "LOAD_PARAM"
};

static int temp_count = 0;
static int label_count = 0;

void ir_init(IRList *list) {
    list->codes = NULL;
    list->size = 0;
    list->capacity = 0;
}

void ir_emit(IRList *list, IRInstr op, IRSymbol *arg1, IRSymbol *arg2, IRSymbol *result) {
    // Redimensionar si no hay espacio
    if (list->size >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 8 : list->capacity * 2;
        list->codes = realloc(list->codes, list->capacity * sizeof(IRCode));
        if (!list->codes) {
            fprintf(stderr, "Error: no se pudo redimensionar la lista de código intermedio\n");
            exit(1);
        }
    }
    
    IRCode *code = &list->codes[list->size++];
    code->op = op;
    code->arg1 = arg1;
    code->arg2 = arg2;
    code->result = result;
}

IRSymbol *new_temp_symbol() {
    IRSymbol *sym = malloc(sizeof(IRSymbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para símbolo temporal\n");
        exit(1);
    }

    char buf[32];
    sprintf(buf, "t%d", temp_count++);
    sym->name = strdup(buf);
    sym->type = IR_SYM_TEMP;
    return sym;
}

IRSymbol *new_label_symbol() {
    IRSymbol *sym = malloc(sizeof(IRSymbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para etiqueta\n");
        exit(1);
    }
    
    char buf[32];
    sprintf(buf, "L%d", label_count++);
    sym->name = strdup(buf);
    sym->type = IR_SYM_LABEL;
    return sym;
}

IRSymbol *new_const_symbol(int value, int is_bool) {
    IRSymbol *sym = malloc(sizeof(IRSymbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para constante\n");
        exit(1);
    }
    
    char buf[32];
    sprintf(buf, "%d", value);
    sym->name = strdup(buf);
    sym->type = IR_SYM_CONST;
    
    if (is_bool) {
        sym->value.bool_val = value;
    } else {
        sym->value.int_val = value;
    }
    return sym;
}

IRSymbol *new_var_symbol(const char *name) {
    IRSymbol *sym = malloc(sizeof(IRSymbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para variable\n");
        exit(1);
    }
    
    sym->name = strdup(name);
    sym->type = IR_SYM_VAR;
    return sym;
}

IRSymbol *new_func_symbol(const char *name) {
    IRSymbol *sym = malloc(sizeof(IRSymbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para función\n");
        exit(1);
    }
    
    sym->name = strdup(name);
    sym->type = IR_SYM_FUNC;
    return sym;
}

void free_ir_symbol(IRSymbol *sym) {
    if (sym) {
        if (sym->name) {
            free(sym->name);
            sym->name = NULL;
        }
        free(sym);
    }
}

IRSymbol *gen_code(Nodo *node, IRList *list) {
    if (!node) return NULL;

    switch (node->tipo) {
        case NODO_INTEGER: {
            IRSymbol *const_sym = new_const_symbol(node->val_int, 0);
            IRSymbol *temp = new_temp_symbol();
            ir_emit(list, IR_LOAD, const_sym, NULL, temp);
            return temp;
        }

        case NODO_BOOL: {
            IRSymbol *const_sym = new_const_symbol(node->val_bool, 1);
            IRSymbol *temp = new_temp_symbol();
            ir_emit(list, IR_LOAD, const_sym, NULL, temp);
            return temp;
        }

        case NODO_ID: {
            if (!node->nombre) {
                fprintf(stderr, "Error: nodo_ID sin nombre\n");
                return NULL;
            }
            
            IRSymbol *var_sym = new_var_symbol(node->nombre);
            IRSymbol *temp = new_temp_symbol();
            ir_emit(list, IR_LOAD, var_sym, NULL, temp);
            return temp;
        }

        case NODO_OP: {
            IRSymbol *temp = new_temp_symbol();
            
            // Para operador unario NOT
            if (node->opBinaria.op == TOP_NOT) {
                IRSymbol *operand = gen_code(node->opBinaria.der, list);
                ir_emit(list, IR_NOT, operand, NULL, temp);
            } 
            // Para menos unario (si izq es 0 y der existe, es menos unario)
            else if (node->opBinaria.op == TOP_RESTA && 
                     node->opBinaria.izq && 
                     node->opBinaria.izq->tipo == NODO_INTEGER && 
                     node->opBinaria.izq->val_int == 0) {
                IRSymbol *operand = gen_code(node->opBinaria.der, list);
                ir_emit(list, IR_UMINUS, operand, NULL, temp);
            }
            // Operadores binarios
            else {
                IRSymbol *left = gen_code(node->opBinaria.izq, list);
                IRSymbol *right = gen_code(node->opBinaria.der, list);
                
                switch (node->opBinaria.op) {
                    case TOP_SUMA:
                        ir_emit(list, IR_ADD, left, right, temp);
                        break;
                    case TOP_RESTA:
                        ir_emit(list, IR_SUB, left, right, temp);
                        break;
                    case TOP_MULT:
                        ir_emit(list, IR_MUL, left, right, temp);
                        break;
                    case TOP_DIV:
                        ir_emit(list, IR_DIV, left, right, temp);
                        break;
                    case TOP_RESTO:
                        ir_emit(list, IR_MOD, left, right, temp);
                        break;
                    case TOP_COMP:
                        ir_emit(list, IR_EQ, left, right, temp);
                        break;
                    case TOP_DESIGUAL:
                        ir_emit(list, IR_NEQ, left, right, temp);
                        break;
                    case TOP_MENOR:
                        ir_emit(list, IR_LT, left, right, temp);
                        break;
                    case TOP_MAYOR:
                        ir_emit(list, IR_GT, left, right, temp);
                        break;
                    case TOP_MENORIG:
                        ir_emit(list, IR_LE, left, right, temp);
                        break;
                    case TOP_MAYORIG:
                        ir_emit(list, IR_GE, left, right, temp);
                        break;
                    case TOP_AND:
                        ir_emit(list, IR_AND, left, right, temp);
                        break;
                    case TOP_OR:
                        ir_emit(list, IR_OR, left, right, temp);
                        break;
                    default:
                        fprintf(stderr, "Operador binario no soportado: %d\n", node->opBinaria.op);
                        break;
                }
            }
            return temp;
        }

        case NODO_ASSIGN: {
            if (!node->assign.id) {
                fprintf(stderr, "Error: nodo_assign sin id\n");
                return NULL;
            }
            
            IRSymbol *rhs = gen_code(node->assign.expr, list);
            if (!rhs) {
                fprintf(stderr, "Error: no se pudo generar código para la expresión en assign\n");
                return NULL;
            }
            
            IRSymbol *var_sym = new_var_symbol(node->assign.id);
            ir_emit(list, IR_STORE, rhs, NULL, var_sym);
            return var_sym;
        }

        case NODO_DECL: {
            if (!node->assign.id) {
                fprintf(stderr, "Error: nodo_decl sin id\n");
                break;
            }
            
            // Si hay inicialización, generar código para la expresión
            if (node->assign.expr) {
                IRSymbol *rhs = gen_code(node->assign.expr, list);
                if (rhs) {
                    IRSymbol *var_sym = new_var_symbol(node->assign.id);
                    ir_emit(list, IR_STORE, rhs, NULL, var_sym);
                }
            }
            break;
        }

        case NODO_METHOD: {
            if (!node->method.nombre) {
                fprintf(stderr, "Error: método sin nombre\n");
                break;
            }
            
            // Método externo
            if (node->method.body == NULL) {
                IRSymbol *func_sym = new_func_symbol(node->method.nombre);
                ir_emit(list, IR_EXTERN, NULL, NULL, func_sym);
            } else {
                // Método normal
                IRSymbol *func_sym = new_func_symbol(node->method.nombre);
                ir_emit(list, IR_METHOD, NULL, NULL, func_sym);
                
                // Generar parámetros
                Nodo *param = node->method.params;
                while (param) {
                    if (param->nombre) {
                        IRSymbol *param_sym = new_var_symbol(param->nombre);
                        ir_emit(list, IR_PARAM, NULL, NULL, param_sym);
                    }
                    param = param->siguiente;
                }
                
                // Generar cuerpo del método - procesar todas las sentencias
                Nodo *stmt = node->method.body;
                while (stmt) {
                    gen_code(stmt, list);
                    stmt = stmt->siguiente;
                }
            }
            break;
        }

        case NODO_METHOD_CALL: {
            if (!node->method_call.nombre) {
                fprintf(stderr, "Error: nodo_method_call sin nombre\n");
                return NULL;
            }
            
            if (node->method_call.args) {
                Nodo *arg = node->method_call.args;
                while (arg) {
                    IRSymbol *arg_temp = gen_code(arg, list);
                    if (arg_temp) {
                        ir_emit(list, IR_CALL_PARAM, arg_temp, NULL, NULL);
                    }
                    arg = arg->siguiente;
                }
            }
            
            IRSymbol *func_sym = new_func_symbol(node->method_call.nombre);
            IRSymbol *temp = new_temp_symbol();
            
            ir_emit(list, IR_CALL, func_sym, NULL, temp);
            return temp;
        }

        case NODO_IF: {
            if (!node->if_stmt.cond) {
                fprintf(stderr, "Error: nodo IF sin condición\n");
                break;
            }
            
            IRSymbol *cond = gen_code(node->if_stmt.cond, list);
            if (!cond) {
                fprintf(stderr, "Error: no se pudo generar código para condición IF\n");
                break;
            }
            
            IRSymbol *label_end = new_label_symbol();
            
            // Si hay else
            if (node->if_stmt.else_block) {
                IRSymbol *label_else = new_label_symbol();
                ir_emit(list, IR_IF_FALSE, cond, NULL, label_else);
                
                // Procesar todas las sentencias del then
                Nodo *stmt = node->if_stmt.then_block;
                while (stmt) {
                    gen_code(stmt, list);
                    stmt = stmt->siguiente;
                }
                
                ir_emit(list, IR_GOTO, NULL, NULL, label_end);
                ir_emit(list, IR_LABEL, NULL, NULL, label_else);
                
                // Procesar todas las sentencias del else
                stmt = node->if_stmt.else_block;
                while (stmt) {
                    gen_code(stmt, list);
                    stmt = stmt->siguiente;
                }
            } else {
                // Solo if
                ir_emit(list, IR_IF_FALSE, cond, NULL, label_end);
                
                // Procesar todas las sentencias del then
                Nodo *stmt = node->if_stmt.then_block;
                while (stmt) {
                    gen_code(stmt, list);
                    stmt = stmt->siguiente;
                }
            }
            
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODO_WHILE: {
            if (!node->while_stmt.cond) {
                fprintf(stderr, "Error: nodo WHILE sin condición\n");
                break;
            }
            
            IRSymbol *label_start = new_label_symbol();
            IRSymbol *label_end = new_label_symbol();
            
            ir_emit(list, IR_LABEL, NULL, NULL, label_start);
            IRSymbol *cond = gen_code(node->while_stmt.cond, list);
            if (!cond) {
                fprintf(stderr, "Error: no se pudo generar código para condición WHILE\n");
                break;
            }
            
            ir_emit(list, IR_IF_FALSE, cond, NULL, label_end);
            
            // Procesar todas las sentencias del cuerpo del while
            Nodo *stmt = node->while_stmt.body;
            while (stmt) {
                gen_code(stmt, list);
                stmt = stmt->siguiente;
            }
            
            ir_emit(list, IR_GOTO, NULL, NULL, label_start);
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODO_RETURN: {
            if (node->ret_expr) {
                IRSymbol *ret_val = gen_code(node->ret_expr, list);
                ir_emit(list, IR_RETURN, ret_val, NULL, NULL);
            } else {
                ir_emit(list, IR_RETURN, NULL, NULL, NULL);
            }
            break;
        }

        case NODO_PROG: {
            // Para el nodo programa, procesar todos los hijos en la lista
            Nodo *current = node->siguiente;
            while (current) {
                gen_code(current, list);
                current = current->siguiente;
            }
            break;
        }

        case NODO_BLOCK:
        case NODO_SENT:
            // Estos nodos no generan código por sí mismos
            break;

        default: {
            fprintf(stderr, "Advertencia: Tipo de nodo no manejado en gen_code: %d\n", node->tipo);
            break;
        }
    }
    
    return NULL;
}

void ir_print(IRList *list) {
    printf("\n--- CÓDIGO INTERMEDIO ---\n");
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        printf("%s", ir_names[code->op]);
        
        switch (code->op) {
            case IR_LOAD:
            case IR_STORE:
                if (code->arg1) printf(" %s", code->arg1->name);
                if (code->result) printf(", %s", code->result->name);
                break;
                
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
            case IR_MOD:
            case IR_AND:
            case IR_OR:
            case IR_EQ:
            case IR_NEQ:
            case IR_LT:
            case IR_LE:
            case IR_GT:
            case IR_GE:
                if (code->arg1) printf(" %s", code->arg1->name);
                if (code->arg2) printf(", %s", code->arg2->name);
                if (code->result) printf(", %s", code->result->name);
                break;
                
            case IR_NOT:
            case IR_UMINUS:
                if (code->arg1) printf(" %s", code->arg1->name);
                if (code->result) printf(", %s", code->result->name);
                break;
                
            case IR_LABEL:
            case IR_METHOD:
            case IR_EXTERN:
            case IR_PARAM:
                if (code->result) printf(" %s", code->result->name);
                break;
                
            case IR_GOTO:
                if (code->result) printf(" %s", code->result->name);
                break;
                
            case IR_IF_FALSE:
            case IR_IF_TRUE:
                if (code->arg1) printf(" %s", code->arg1->name);
                if (code->result) printf(", %s", code->result->name);
                break;
                
            case IR_RETURN:
                if (code->arg1) printf(" %s", code->arg1->name);
                break;
                
            case IR_CALL:
                if (code->arg1) printf(" %s", code->arg1->name);
                if (code->result) printf(", %s", code->result->name);
                break;
            
            case IR_CALL_PARAM:
                if (code->arg1) printf(" %s", code->arg1->name);
                break;

            default:
                break;
        }
        printf("\n");
    }
    printf("--- FIN CÓDIGO INTERMEDIO ---\n\n");
}

void ir_save_to_file(IRList *list, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: no se pudo abrir el archivo %s para escritura\n", filename);
        return;
    }
    
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        fprintf(file, "%s", ir_names[code->op]);
        
        switch (code->op) {
            case IR_LOAD:
            case IR_STORE:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;
                
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
            case IR_MOD:
            case IR_AND:
            case IR_OR:
            case IR_EQ:
            case IR_NEQ:
            case IR_LT:
            case IR_LE:
            case IR_GT:
            case IR_GE:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->arg2) fprintf(file, ", %s", code->arg2->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;
                
            case IR_NOT:
            case IR_UMINUS:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;
                
            case IR_LABEL:
                if (code->result) fprintf(file, " %s:", code->result->name);
                break;
                
            case IR_METHOD:
                if (code->result) fprintf(file, " %s:", code->result->name);
                break;
                
            case IR_EXTERN:
                if (code->result) fprintf(file, " %s", code->result->name);
                break;
                
            case IR_PARAM:
                if (code->result) fprintf(file, " %s", code->result->name);
                break;
                
            case IR_GOTO:
                if (code->result) fprintf(file, " %s", code->result->name);
                break;
                
            case IR_IF_FALSE:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;
                
            case IR_IF_TRUE:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;
                
            case IR_RETURN:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                break;
                
            case IR_CALL:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                if (code->result) fprintf(file, ", %s", code->result->name);
                break;

            case IR_CALL_PARAM:
                if (code->arg1) fprintf(file, " %s", code->arg1->name);
                break;
                
            default:
                break;
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("Código intermedio guardado en: %s\n", filename);
}

void ir_free(IRList *list) {
    if (list && list->codes) {
        free(list->codes);
        list->codes = NULL;
        list->size = 0;
        list->capacity = 0;
    }
}

int generate_intermediate_code(Nodo *ast) {
    if (!ast) {
        fprintf(stderr, "Error: AST es NULL\n");
        return 1;
    }
    
    IRList ir_list;
    ir_init(&ir_list);
    
    printf("\n=======================================");
    printf("\n| INICIANDO GENERACIÓN CÓDIGO INTERMEDIO |");
    printf("\n=======================================\n");
    
    // Generar código intermedio recorriendo el AST
    // Comenzar desde el primer hijo del nodo program
    if (ast->tipo == NODO_ID && strcmp(ast->nombre, "program") == 0) {
        // Procesar todos los nodos hermanos (declaraciones y métodos)
        Nodo *current = ast->siguiente;
        while (current) {
            gen_code(current, &ir_list);
            current = current->siguiente;
        }
    } else {
        gen_code(ast, &ir_list);
    }
    
    // Imprimir código intermedio
    ir_print(&ir_list);
    
    // Guardar a archivo
    ir_save_to_file(&ir_list, "inter.s");
    
    // Liberar memoria
    ir_free(&ir_list);
    
    printf("✅ Generación de código intermedio completada exitosamente.\n\n");
    return 0;
}
