#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Verifica si un número es una potencia de 2.
 */
bool is_power_of_two(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/*
 * Calcula el logaritmo base 2 de un entero.
 * Se usa para convertir divisiones por potencias de 2 en shifteos.
 */
int log2_int(int n) {
    int result = 0;
    while (n > 1) {
        n >>= 1;
        result++;
    }
    return result;
}

/*
 * Determina si un símbolo representa un número.
 */
bool is_constant_symbol(IRSymbol *sym) {
    if (!sym || !sym->name) return false;
    return sym->type == IR_SYM_CONST || 
           (sym->name[0] >= '0' && sym->name[0] <= '9') ||
           (sym->name[0] == '-' && sym->name[1] >= '0' && sym->name[1] <= '9');
}

/*
 * Extrae el valor numérico de un símbolo constante.
 */
int get_constant_value(IRSymbol *sym) {
    if (!sym || !sym->name) return 0;
    return atoi(sym->name);
}

/*
 * Reemplaza una instrucción completa en la lista de código intermedio.
 */
void replace_instruction(IRList *list, int index, IRInstr new_op,
                        IRSymbol *new_arg1, IRSymbol *new_arg2, IRSymbol *new_result) {
    if (index < 0 || index >= list->size) return;
    
    IRCode *code = &list->codes[index];
    code->op = new_op;
    code->arg1 = new_arg1;
    code->arg2 = new_arg2;
    code->result = new_result;
}

/*
 * Marca una instrucción como no-operación (NOP)
 * Convierte la instrucción a LABEL sin argumentos para eliminarla efectivamente.
 */
void mark_instruction_as_nop(IRList *list, int index) {
    if (index < 0 || index >= list->size) return;
    list->codes[index].op = IR_LABEL;
    list->codes[index].arg1 = NULL;
    list->codes[index].arg2 = NULL;
    list->codes[index].result = NULL;
}

/*
 * Optimización peephole principal
 * Transforma patrones específicos:
 * - x / 2^n  →  x >> n
 * - x * 2^n  →  x << n
 * - x + 0    →  x
 * - x * 1    →  x
 * - x * 0    →  0
 */
void optimize_peephole(IRList *list) {
    int optimizations = 0;
    
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        
        // Optimización: DIV por potencias de 2 → SHIFT derecha
        if (code->op == IR_DIV && is_constant_symbol(code->arg2)) {
            int divisor = get_constant_value(code->arg2);
            if (is_power_of_two(divisor)) {
                int shift_amount = log2_int(divisor);
                
                // Cambiar DIV por un comentario que luego se traduce a SAR
                printf("  [PEEPHOLE] Línea %d: DIV por %d → puede usar SAR %d\n", 
                       i, divisor, shift_amount);
                optimizations++;
            }
        }
        
        // Optimización: MUL por potencias de 2 → SHIFT izquierda
        else if (code->op == IR_MUL && is_constant_symbol(code->arg2)) {
            int multiplier = get_constant_value(code->arg2);
            if (is_power_of_two(multiplier)) {
                int shift_amount = log2_int(multiplier);
                
                printf("  [PEEPHOLE] Línea %d: MUL por %d → puede usar SAL %d\n", 
                       i, multiplier, shift_amount);
                optimizations++;
            }
        }
        
        // Optimización: x + 0 = x
        else if (code->op == IR_ADD && is_constant_symbol(code->arg2)) {
            int value = get_constant_value(code->arg2);
            if (value == 0) {
                // Cambiar ADD por LOAD (copiar arg1 a result)
                replace_instruction(list, i, IR_LOAD, code->arg1, NULL, code->result);
                printf("  [PEEPHOLE] Línea %d: x + 0 → x\n", i);
                optimizations++;
            }
        }
        
        // Optimización: x * 1 = x
        else if (code->op == IR_MUL && is_constant_symbol(code->arg2)) {
            int value = get_constant_value(code->arg2);
            if (value == 1) {
                replace_instruction(list, i, IR_LOAD, code->arg1, NULL, code->result);
                printf("  [PEEPHOLE] Línea %d: x * 1 → x\n", i);
                optimizations++;
            }
        }
        
        // Optimización: x * 0 = 0
        else if (code->op == IR_MUL && is_constant_symbol(code->arg2)) {
            int value = get_constant_value(code->arg2);
            if (value == 0) {
                IRSymbol *zero = new_const_symbol(0, 0);
                replace_instruction(list, i, IR_LOAD, zero, NULL, code->result);
                printf("  [PEEPHOLE] Línea %d: x * 0 → 0\n", i);
                optimizations++;
            }
        }
        
        // Optimización: x - 0 = x
        else if (code->op == IR_SUB && is_constant_symbol(code->arg2)) {
            int value = get_constant_value(code->arg2);
            if (value == 0) {
                replace_instruction(list, i, IR_LOAD, code->arg1, NULL, code->result);
                printf("  [PEEPHOLE] Línea %d: x - 0 → x\n", i);
                optimizations++;
            }
        }
        
        // Optimización: 0 - x = -x (UMINUS)
        else if (code->op == IR_SUB && is_constant_symbol(code->arg1)) {
            int value = get_constant_value(code->arg1);
            if (value == 0) {
                replace_instruction(list, i, IR_UMINUS, code->arg2, NULL, code->result);
                printf("  [PEEPHOLE] Línea %d: 0 - x → -x\n", i);
                optimizations++;
            }
        }
    }
    
    if (optimizations > 0) {
        printf("✓ Optimización peephole: %d transformaciones aplicadas\n", optimizations);
    }
}

/*
 * Constant folding: evaluar operaciones con constantes en tiempo de compilación
 * Ejemplo: 2 + 3 → 5
 */
void optimize_constant_folding(IRList *list) {
    int optimizations = 0;
    
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        
        if (!code->arg1 || !code->arg2) continue;
        
        // Solo procesar si ambos argumentos son constantes
        if (is_constant_symbol(code->arg1) && is_constant_symbol(code->arg2)) {
            int val1 = get_constant_value(code->arg1);
            int val2 = get_constant_value(code->arg2);
            int result_value = 0;
            bool can_fold = true;
            
            switch (code->op) {
                case IR_ADD:
                    result_value = val1 + val2;
                    break;
                case IR_SUB:
                    result_value = val1 - val2;
                    break;
                case IR_MUL:
                    result_value = val1 * val2;
                    break;
                case IR_DIV:
                    if (val2 != 0) {
                        result_value = val1 / val2;
                    } else {
                        can_fold = false;
                    }
                    break;
                case IR_MOD:
                    if (val2 != 0) {
                        result_value = val1 % val2;
                    } else {
                        can_fold = false;
                    }
                    break;
                case IR_LT:
                    result_value = val1 < val2 ? 1 : 0;
                    break;
                case IR_LE:
                    result_value = val1 <= val2 ? 1 : 0;
                    break;
                case IR_GT:
                    result_value = val1 > val2 ? 1 : 0;
                    break;
                case IR_GE:
                    result_value = val1 >= val2 ? 1 : 0;
                    break;
                case IR_EQ:
                    result_value = val1 == val2 ? 1 : 0;
                    break;
                case IR_NEQ:
                    result_value = val1 != val2 ? 1 : 0;
                    break;
                case IR_AND:
                    result_value = (val1 && val2) ? 1 : 0;
                    break;
                case IR_OR:
                    result_value = (val1 || val2) ? 1 : 0;
                    break;
                default:
                    can_fold = false;
            }
            
            if (can_fold) {
                IRSymbol *const_result = new_const_symbol(result_value, 0);
                replace_instruction(list, i, IR_LOAD, const_result, NULL, code->result);
                printf("  [FOLDING] Línea %d: %d op %d → %d\n", i, val1, val2, result_value);
                optimizations++;
            }
        }
    }
    
    if (optimizations > 0) {
        printf("✓ Constant folding: %d operaciones evaluadas\n", optimizations);
    }
}

/*
 * Propagación de constantes
 * Si t1 = 5 y luego usamos t1, reemplazar por 5 directamente
 */
void optimize_constant_propagation(IRList *list) {
    int optimizations = 0;
    
    // Tabla para rastrear qué temporales tienen valores constantes
    #define MAX_TEMPS 256
    int temp_values[MAX_TEMPS];
    bool temp_is_const[MAX_TEMPS];
    
    for (int i = 0; i < MAX_TEMPS; i++) {
        temp_is_const[i] = false;
    }
    
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        
        // Detectar asignaciones de constantes: t1 = 5
        if (code->op == IR_LOAD && is_constant_symbol(code->arg1)) {
            if (code->result && code->result->name && code->result->name[0] == 't') {
                int temp_num = atoi(code->result->name + 1);
                if (temp_num >= 0 && temp_num < MAX_TEMPS) {
                    temp_values[temp_num] = get_constant_value(code->arg1);
                    temp_is_const[temp_num] = true;
                }
            }
        }
        
        // Propagar constantes en los usos
        if (code->arg1 && code->arg1->name && code->arg1->name[0] == 't') {
            int temp_num = atoi(code->arg1->name + 1);
            if (temp_num >= 0 && temp_num < MAX_TEMPS && temp_is_const[temp_num]) {
                code->arg1 = new_const_symbol(temp_values[temp_num], 0);
                optimizations++;
            }
        }
        
        if (code->arg2 && code->arg2->name && code->arg2->name[0] == 't') {
            int temp_num = atoi(code->arg2->name + 1);
            if (temp_num >= 0 && temp_num < MAX_TEMPS && temp_is_const[temp_num]) {
                code->arg2 = new_const_symbol(temp_values[temp_num], 0);
                optimizations++;
            }
        }
        
        // Invalidar temporales que son redefinidos
        if (code->result && code->result->name && code->result->name[0] == 't') {
            int temp_num = atoi(code->result->name + 1);
            if (temp_num >= 0 && temp_num < MAX_TEMPS) {
                if (code->op != IR_LOAD || !is_constant_symbol(code->arg1)) {
                    temp_is_const[temp_num] = false;
                }
            }
        }
    }
    
    if (optimizations > 0 && debug_mode) {
        printf("✓ Propagación de constantes: %d reemplazos\n", optimizations);
    }
}

/*
 * Eliminación de código muerto.
 */
void optimize_dead_code_elimination(IRList *list) {
    int optimizations = 0;
    bool *is_used = calloc(list->size, sizeof(bool));
    
    // Marcar todas las instrucciones esenciales (saltos, returns, stores, calls)
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        
        if (code->op == IR_STORE || code->op == IR_RETURN || 
            code->op == IR_CALL || code->op == IR_LABEL ||
            code->op == IR_GOTO || code->op == IR_IF_FALSE || code->op == IR_IF_TRUE ||
            code->op == IR_METHOD || code->op == IR_EXTERN || code->op == IR_PARAM ||
            code->op == IR_CALL_PARAM) {
            is_used[i] = true;
        }
    }
    
    // Propagar uso hacia atrás
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = list->size - 1; i >= 0; i--) {
            if (!is_used[i]) continue;
            
            IRCode *code = &list->codes[i];
            
            // Si esta instrucción está marcada como usada, marcar sus dependencias
            if (code->arg1 && code->arg1->name && code->arg1->name[0] == 't') {
                // Buscar la definición de arg1
                for (int j = i - 1; j >= 0; j--) {
                    if (list->codes[j].result && 
                        strcmp(list->codes[j].result->name, code->arg1->name) == 0) {
                        if (!is_used[j]) {
                            is_used[j] = true;
                            changed = true;
                        }
                        break;
                    }
                }
            }
            
            if (code->arg2 && code->arg2->name && code->arg2->name[0] == 't') {
                // Buscar la definición de arg2
                for (int j = i - 1; j >= 0; j--) {
                    if (list->codes[j].result && 
                        strcmp(list->codes[j].result->name, code->arg2->name) == 0) {
                        if (!is_used[j]) {
                            is_used[j] = true;
                            changed = true;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // Eliminar instrucciones no usadas
    for (int i = 0; i < list->size; i++) {
        if (!is_used[i]) {
            IRCode *code = &list->codes[i];
            // Solo eliminar operaciones aritméticas/lógicas con resultado temporal
            if (code->result && code->result->name && code->result->name[0] == 't') {
                if (code->op == IR_ADD || code->op == IR_SUB || code->op == IR_MUL ||
                    code->op == IR_DIV || code->op == IR_MOD || code->op == IR_AND ||
                    code->op == IR_OR || code->op == IR_NOT || code->op == IR_UMINUS ||
                    code->op == IR_EQ || code->op == IR_NEQ || code->op == IR_LT ||
                    code->op == IR_LE || code->op == IR_GT || code->op == IR_GE ||
                    (code->op == IR_LOAD && code->arg1->type == IR_SYM_CONST)) {
                    mark_instruction_as_nop(list, i);
                    if (debug_mode) {
                        printf("  [DEAD CODE] Línea %d: instrucción eliminada (resultado no usado)\n", i);
                    }
                    optimizations++;
                }
            }
        }
    }
    
    free(is_used);
    
    if (optimizations > 0 && debug_mode) {
        printf("✓ Eliminación de código muerto: %d instrucciones eliminadas\n", optimizations);
    }
}

/*
 * Simplificación algebraica adicional.
 */
void optimize_algebraic_simplification(IRList *list) {
    int optimizations = 0;
    
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        
        // x - x = 0
        if (code->op == IR_SUB && code->arg1 && code->arg2 &&
            strcmp(code->arg1->name, code->arg2->name) == 0) {
            IRSymbol *zero = new_const_symbol(0, 0);
            replace_instruction(list, i, IR_LOAD, zero, NULL, code->result);
            printf("  [ALGEBRAIC] Línea %d: x - x → 0\n", i);
            optimizations++;
        }
        
        // x / x = 1 (si no es 0)
        else if (code->op == IR_DIV && code->arg1 && code->arg2 &&
                 strcmp(code->arg1->name, code->arg2->name) == 0) {
            IRSymbol *one = new_const_symbol(1, 0);
            replace_instruction(list, i, IR_LOAD, one, NULL, code->result);
            printf("  [ALGEBRAIC] Línea %d: x / x → 1\n", i);
            optimizations++;
        }
    }
    
    if (optimizations > 0) {
        printf("✓ Simplificación algebraica: %d simplificaciones\n", optimizations);
    }
}

/*
 * Función principal que ejecuta todas las optimizaciones en orden
 */
void optimize_ir_code(IRList *list) {
    if (debug_mode) {
        printf("\n=== INICIANDO OPTIMIZACIONES ===\n");
    }
    
    // Fase 1: Constant Folding
    optimize_constant_folding(list);
    
    // Fase 2: Constant Propagation
    optimize_constant_propagation(list);
    
    // Fase 3: Peephole (después de propagación)
    optimize_peephole(list);
    
    // Fase 4: Simplificación algebraica
    optimize_algebraic_simplification(list);
    
    // Fase 5: Dead Code Elimination (al final)
    optimize_dead_code_elimination(list);
    
    if (debug_mode) {
        printf("=== OPTIMIZACIONES COMPLETADAS ===\n\n");
    }
}