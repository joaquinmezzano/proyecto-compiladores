#include "object.h"
#include "intermediate.h"

/* Llevamos la cuenta de la cantidad de registros temporales. */
static int temp_register_count = 0;

/*
 * Inicializamos el buffer resultante.
 */
void object_init(ObjectCode *obj) {
    obj->lines = NULL;
    obj->size = 0;
    obj->capacity = 0;
}

/*
 * Liberamos el buffer (libera la memoria)
 */
void object_free(ObjectCode *obj) {
    if (obj->lines) {
        for (int i = 0; i < obj->size; i++) {
            free(obj->lines[i]);
        }
        free(obj->lines);
        obj->lines = NULL;
    }
    obj->size = 0;
    obj->capacity = 0;
}

/*
 * Inicializador de la tabla de variables, la "symbol table" del stack frame.
 */
void var_table_init(VarTable *table) {
    table->vars = NULL;
    table->count = 0;
    table->capacity = 0;
    table->stack_size = 0;
}

/*
 * Liberamos la tabla de variables.
 */
void var_table_free(VarTable *table) {
    if (table->vars) {
        for (int i = 0; i < table->count; i++) {
            free(table->vars[i].name);
        }
        free(table->vars);
        table->vars = NULL;
    }
    table->count = 0;
    table->capacity = 0;
}

/*
 * Toma una instrucción del IR, la traduce a código objeto y la agrega al resultado.
 */
void object_emit(ObjectCode *obj, const char *line) {
    if (obj->size >= obj->capacity) {
        obj->capacity = (obj->capacity == 0) ? 32 : obj->capacity * 2;
        obj->lines = realloc(obj->lines, obj->capacity * sizeof(char*));
        if (!obj->lines) {
            fprintf(stderr, "Error: no se pudo redimensionar ObjectCode\n");
            exit(1);
        }
    }
    obj->lines[obj->size++] = strdup(line);
}

/*
 * Agrega una variable local a la tabla y le asigna un offset negativo desde %rbp.
 * Cada variable ocupa 8 bytes (qword)
 */
int var_table_add(VarTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->vars[i].name, name) == 0) {
            return table->vars[i].offset;
        }
    }
    
    if (table->count >= table->capacity) {
        table->capacity = (table->capacity == 0) ? 8 : table->capacity * 2;
        table->vars = realloc(table->vars, table->capacity * sizeof(VarInfo));
        if (!table->vars) {
            fprintf(stderr, "Error: no se pudo redimensionar VarTable\n");
            exit(1);
        }
    }
    
    table->stack_size += 8;
    int offset = -table->stack_size;
    
    table->vars[table->count].name = strdup(name);
    table->vars[table->count].offset = offset;
    table->count++;
    
    return offset;
}

/*
 * Busca una variable en la tabla y devuelve cuanto offset tiene.
 */
int var_table_get_offset(VarTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->vars[i].name, name) == 0) {
            return table->vars[i].offset;
        }
    }
    return 0;
}

/*
 * Detecta si una variable es un temporal del IR.
 * Como "t0", "t5", entre otros.
 */
int is_temp_var(const char *name) {
    return name && name[0] == 't' && (name[1] >= '0' && name[1] <= '9');
}

/*
 * Detecta si un nombre es un número.
 */
int is_constant(const char *name) {
    if (!name) return 0;
    return (name[0] == '-' && name[1] >= '0' && name[1] <= '9') || 
           (name[0] >= '0' && name[0] <= '9');
}

/*
 * Detecta si un nombre es una etiqueta.
 * Como "L1", "L2", entre otros. 
 */
int is_label(const char *name) {
    return name && name[0] == 'L' && (name[1] >= '0' && name[1] <= '9');
}

/*
 * Asigna un registro a cada variable temporal.
 */
const char* get_register_for_temp(const char *temp_name) {
    static const char* registers[] = {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9"};
    static int temp_to_reg[256];
    static int initialized = 0;
    
    if (!initialized) {
        memset(temp_to_reg, -1, sizeof(temp_to_reg));
        initialized = 1;
    }
    
    if (!is_temp_var(temp_name)) return "%rax";
    
    int temp_num = atoi(temp_name + 1);
    if (temp_num < 0 || temp_num >= 256) return "%rax";
    
    if (temp_to_reg[temp_num] == -1) {
        temp_to_reg[temp_num] = temp_register_count % 8;
        temp_register_count++;
    }
    
    return registers[temp_to_reg[temp_num]];
}

/*
 * Genera el prologo de una función.
 */
void translate_prologue(ObjectCode *obj, const char *func_name, VarTable *vars) {
    char line[256];
    
    snprintf(line, sizeof(line), ".globl %s", func_name);
    object_emit(obj, line);
    snprintf(line, sizeof(line), ".type %s, @function", func_name);
    object_emit(obj, line);
    snprintf(line, sizeof(line), "%s:", func_name);
    object_emit(obj, line);
    
    if (vars->stack_size > 0) {
        snprintf(line, sizeof(line), "\tenter\t$%d, $0", vars->stack_size);
        object_emit(obj, line);
    } else {
        object_emit(obj, "\tenter\t$0, $0");
    }
}

/*
 * Genera el epilogo de una función.
 */
void translate_epilogue(ObjectCode *obj) {
    object_emit(obj, "\tleave");
    object_emit(obj, "\tret");
}

/*
 * Traduce de una instrucción IR a su equivalente en código objeto.
 */
void translate_ir_instruction(ObjectCode *obj, IRCode *code, VarTable *vars) {
    char line[512];
    
    switch (code->op) {
        case IR_LOAD: {
            const char *src_name = code->arg1 ? code->arg1->name : "0";
            const char *dst_reg = get_register_for_temp(code->result->name);
            
            if (is_constant(src_name)) {
                snprintf(line, sizeof(line), "\tmovq\t$%s, %s", src_name, dst_reg);
            } else {
                int offset = var_table_get_offset(vars, src_name);
                if (offset != 0) {
                    snprintf(line, sizeof(line), "\tmovq\t%d(%%rbp), %s", offset, dst_reg);
                } else {
                    snprintf(line, sizeof(line), "\tmovq\t%s, %s", src_name, dst_reg);
                }
            }
            object_emit(obj, line);
            break;
        }
        
        case IR_STORE: {
            const char *src_reg = get_register_for_temp(code->arg1->name);
            const char *dst_name = code->result->name;
            
            int offset = var_table_add(vars, dst_name);
            snprintf(line, sizeof(line), "\tmovq\t%s, %d(%%rbp)", src_reg, offset);
            object_emit(obj, line);
            break;
        }
        
        case IR_ADD: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            if (strcmp(reg1, result_reg) != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%s, %s", reg1, result_reg);
                object_emit(obj, line);
            }
            snprintf(line, sizeof(line), "\taddq\t%s, %s", reg2, result_reg);
            object_emit(obj, line);
            break;
        }
        
        case IR_SUB: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            if (strcmp(reg1, result_reg) != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%s, %s", reg1, result_reg);
                object_emit(obj, line);
            }
            snprintf(line, sizeof(line), "\tsubq\t%s, %s", reg2, result_reg);
            object_emit(obj, line);
            break;
        }
        
        case IR_MUL: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            if (strcmp(reg1, result_reg) != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%s, %s", reg1, result_reg);
                object_emit(obj, line);
            }
            snprintf(line, sizeof(line), "\timulq\t%s, %s", reg2, result_reg);
            object_emit(obj, line);
            break;
        }
        
        case IR_DIV: {
            object_emit(obj, "\tmovq\t%rax, %r10");
            object_emit(obj, "\tmovq\t%rdx, %r11");
            
            const char *dividend_reg = get_register_for_temp(code->arg1->name);
            const char *divisor_reg = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tmovq\t%s, %%rax", dividend_reg);
            object_emit(obj, line);
            object_emit(obj, "\tcqto");
            snprintf(line, sizeof(line), "\tidivq\t%s", divisor_reg);
            object_emit(obj, line);
            
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            
            object_emit(obj, "\tmovq\t%r11, %rdx");
            object_emit(obj, "\tmovq\t%r10, %rax");
            break;
        }
        
        case IR_MOD: {
            object_emit(obj, "\tmovq\t%rax, %r10");
            object_emit(obj, "\tmovq\t%rdx, %r11");
            
            const char *dividend_reg = get_register_for_temp(code->arg1->name);
            const char *divisor_reg = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tmovq\t%s, %%rax", dividend_reg);
            object_emit(obj, line);
            object_emit(obj, "\tcqto");
            snprintf(line, sizeof(line), "\tidivq\t%s", divisor_reg);
            object_emit(obj, line);
            
            if (strcmp(result_reg, "%rdx") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rdx, %s", result_reg);
                object_emit(obj, line);
            }
            
            object_emit(obj, "\tmovq\t%r11, %rdx");
            object_emit(obj, "\tmovq\t%r10, %rax");
            break;
        }
        
        case IR_LABEL: {
            snprintf(line, sizeof(line), "%s:", code->result->name);
            object_emit(obj, line);
            break;
        }
        
        case IR_GOTO: {
            snprintf(line, sizeof(line), "\tjmp\t%s", code->result->name);
            object_emit(obj, line);
            break;
        }
        
        case IR_IF_FALSE: {
            const char *cond_reg = get_register_for_temp(code->arg1->name);
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", cond_reg);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tje\t%s", code->result->name);
            object_emit(obj, line);
            break;
        }
        
        case IR_RETURN: {
            if (code->arg1) {
                const char *ret_reg = get_register_for_temp(code->arg1->name);
                if (strcmp(ret_reg, "%rax") != 0) {
                    snprintf(line, sizeof(line), "\tmovq\t%s, %%rax", ret_reg);
                    object_emit(obj, line);
                }
            } else {
                object_emit(obj, "\tmovq\t$0, %rax");
            }
            translate_epilogue(obj);
            break;
        }
        
        case IR_METHOD: {
            temp_register_count = 0;
            translate_prologue(obj, code->result->name, vars);
            break;
        }
        
        case IR_EQ: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsete\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_NEQ: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetne\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_LT: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetl\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_LE: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetle\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_GT: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetg\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_GE: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t%s, %s", reg2, reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetge\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_AND: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetne\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", reg2);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetne\t%%dl");
            object_emit(obj, line);
            object_emit(obj, "\tandb\t%dl, %al");
            object_emit(obj, "\tmovzbl\t%al, %eax");
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_OR: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *reg2 = get_register_for_temp(code->arg2->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetne\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", reg2);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsetne\t%%dl");
            object_emit(obj, line);
            object_emit(obj, "\torb\t%dl, %al");
            object_emit(obj, "\tmovzbl\t%al, %eax");
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_NOT: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", reg1);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tsete\t%%al");
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tmovzbl\t%%al, %%eax");
            object_emit(obj, line);
            if (strcmp(result_reg, "%rax") != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                object_emit(obj, line);
            }
            break;
        }
        
        case IR_UMINUS: {
            const char *reg1 = get_register_for_temp(code->arg1->name);
            const char *result_reg = get_register_for_temp(code->result->name);
            
            if (strcmp(reg1, result_reg) != 0) {
                snprintf(line, sizeof(line), "\tmovq\t%s, %s", reg1, result_reg);
                object_emit(obj, line);
            }
            snprintf(line, sizeof(line), "\tnegq\t%s", result_reg);
            object_emit(obj, line);
            break;
        }
        
        case IR_IF_TRUE: {
            const char *cond_reg = get_register_for_temp(code->arg1->name);
            snprintf(line, sizeof(line), "\tcmpq\t$0, %s", cond_reg);
            object_emit(obj, line);
            snprintf(line, sizeof(line), "\tjne\t%s", code->result->name);
            object_emit(obj, line);
            break;
        }
        
        case IR_CALL: {
            const char *func_name = code->arg1->name;
            snprintf(line, sizeof(line), "\tcall\t%s", func_name);
            object_emit(obj, line);

            if (code->result) {
                const char *result_reg = get_register_for_temp(code->result->name);
                if (strcmp(result_reg, "%rax") != 0) {
                    snprintf(line, sizeof(line), "\tmovq\t%%rax, %s", result_reg);
                    object_emit(obj, line);
                }
            }
            break;
        }
        
        case IR_CALL_PARAM: {
            const char *param_name = code->arg1->name;
            if (is_constant(param_name)) {
                snprintf(line, sizeof(line), "\tmovq\t$%s, %%rdi", param_name);
            } else if (is_temp_var(param_name)) {
                const char *param_reg = get_register_for_temp(param_name);
                if (strcmp(param_reg, "%rdi") != 0) {
                    snprintf(line, sizeof(line), "\tmovq\t%s, %%rdi", param_reg);
                } else {
                    break;
                }
            } else {
                int offset = var_table_get_offset(vars, param_name);
                if (offset != 0) {
                    snprintf(line, sizeof(line), "\tmovq\t%d(%%rbp), %%rdi", offset);
                } else {
                    snprintf(line, sizeof(line), "\tmovq\t%s, %%rdi", param_name);
                }
            }
            object_emit(obj, line);
            break;
        }
        
        case IR_PARAM: {
            const char *param_name = code->arg1->name;
            snprintf(line, sizeof(line), "\t# Parameter: %s", param_name);
            object_emit(obj, line);
            break;
        }
        
        default:
            snprintf(line, sizeof(line), "\t# Instrucción no implementada: %d", code->op);
            object_emit(obj, line);
            break;
    }
}

/*
 * Hace todo el proceso de traducción; abre el archivo .ir, inicializa las estructuras,
 * agrega .text al inicio del output, lee el archivo .ir linea por linea, parsea los
 * argumentos y crea una estructura IRCode, usa translate_ir_instruction() para traducir,
 * por último emite .section .note.GNU-stack y guarda el archivo resultante como output.s
 */
int generate_object_code(const char *ir_filename, const char *output_filename) {
    FILE *ir_file = fopen(ir_filename, "r");
    if (!ir_file) {
        fprintf(stderr, "Error: no se pudo abrir %s\n", ir_filename);
        return 1;
    }
    
    ObjectCode obj;
    VarTable vars;
    object_init(&obj);
    var_table_init(&vars);
    object_emit(&obj, ".text");
    
    char line[512];
    char current_function[256] = "";
    int in_function = 0;
    
    while (fgets(line, sizeof(line), ir_file)) {
        line[strcspn(line, "\n\r")] = 0;
        
        if (strlen(line) == 0) continue;
        
        if (strncmp(line, "METHOD ", 7) == 0) {
            char func_name[256];
            if (sscanf(line, "METHOD %s", func_name) == 1) {
                char *colon = strchr(func_name, ':');
                if (colon) *colon = '\0';
                strcpy(current_function, func_name);
                translate_prologue(&obj, func_name, &vars);
                in_function = 1;
            }
        }
        else if (strncmp(line, "EXTERN ", 7) == 0) {
            continue;
        }
        else if (strncmp(line, "LOAD ", 5) == 0) {
            char src[256], dst[256];
            if (sscanf(line, "LOAD %[^,], %s", src, dst) == 2) {
                IRSymbol src_sym = {strdup(src), IR_SYM_VAR, {0}};
                IRSymbol dst_sym = {strdup(dst), IR_SYM_TEMP, {0}};
                IRCode code = {IR_LOAD, &src_sym, NULL, &dst_sym};
                
                if (!is_temp_var(src) && !is_constant(src)) {
                    var_table_add(&vars, src);
                }
                if (!is_temp_var(dst)) {
                    var_table_add(&vars, dst);
                }
                
                translate_ir_instruction(&obj, &code, &vars);
                
                free(src_sym.name);
                free(dst_sym.name);
            }
        }
        else if (strncmp(line, "STORE ", 6) == 0) {
            char src[256], dst[256];
            if (sscanf(line, "STORE %[^,], %s", src, dst) == 2) {
                IRSymbol src_sym = {strdup(src), IR_SYM_TEMP, {0}};
                IRSymbol dst_sym = {strdup(dst), IR_SYM_VAR, {0}};
                IRCode code = {IR_STORE, &src_sym, NULL, &dst_sym};
                
                if (!is_temp_var(src)) {
                    var_table_add(&vars, src);
                }
                if (!is_temp_var(dst)) {
                    var_table_add(&vars, dst);
                }
                
                translate_ir_instruction(&obj, &code, &vars);
                
                free(src_sym.name);
                free(dst_sym.name);
            }
        }
        else if (strncmp(line, "ADD ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "ADD %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_ADD, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "SUB ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "SUB %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_SUB, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "MUL ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "MUL %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_MUL, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "DIV ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "DIV %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_DIV, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "MOD ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "MOD %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_MOD, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "AND ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "AND %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_AND, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "OR ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "OR %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_OR, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "NOT ", 4) == 0) {
            char arg1[256], result[256];
            if (sscanf(line, "NOT %[^,], %s", arg1, result) == 2) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_NOT, &arg1_sym, NULL, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "UMINUS ", 7) == 0) {
            char arg1[256], result[256];
            if (sscanf(line, "UMINUS %[^,], %s", arg1, result) == 2) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_UMINUS, &arg1_sym, NULL, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "EQ ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "EQ %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_EQ, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "LE ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "LE %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_LE, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "NEQ ", 4) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "NEQ %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_NEQ, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "LT ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "LT %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_LT, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "GT ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "GT %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_GT, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "GE ", 3) == 0) {
            char arg1[256], arg2[256], result[256];
            if (sscanf(line, "GE %[^,], %[^,], %s", arg1, arg2, result) == 3) {
                IRSymbol arg1_sym = {strdup(arg1), IR_SYM_TEMP, {0}};
                IRSymbol arg2_sym = {strdup(arg2), IR_SYM_TEMP, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_GE, &arg1_sym, &arg2_sym, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(arg1_sym.name);
                free(arg2_sym.name);
                free(result_sym.name);
            }
        }
        else if (strncmp(line, "IF_FALSE ", 9) == 0) {
            char cond[256], label[256];
            if (sscanf(line, "IF_FALSE %[^,], %s", cond, label) == 2) {
                IRSymbol cond_sym = {strdup(cond), IR_SYM_TEMP, {0}};
                IRSymbol label_sym = {strdup(label), IR_SYM_LABEL, {0}};
                IRCode code = {IR_IF_FALSE, &cond_sym, NULL, &label_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(cond_sym.name);
                free(label_sym.name);
            }
        }
        else if (strncmp(line, "IF_TRUE ", 8) == 0) {
            char cond[256], label[256];
            if (sscanf(line, "IF_TRUE %[^,], %s", cond, label) == 2) {
                IRSymbol cond_sym = {strdup(cond), IR_SYM_TEMP, {0}};
                IRSymbol label_sym = {strdup(label), IR_SYM_LABEL, {0}};
                IRCode code = {IR_IF_TRUE, &cond_sym, NULL, &label_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(cond_sym.name);
                free(label_sym.name);
            }
        }
        else if (strncmp(line, "GOTO ", 5) == 0) {
            char label[256];
            if (sscanf(line, "GOTO %s", label) == 1) {
                IRSymbol label_sym = {strdup(label), IR_SYM_LABEL, {0}};
                IRCode code = {IR_GOTO, NULL, NULL, &label_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(label_sym.name);
            }
        }
        else if (strncmp(line, "LABEL ", 6) == 0) {
            char label[256];
            if (sscanf(line, "LABEL %s", label) == 1) {
                char *colon = strchr(label, ':');
                if (colon) *colon = '\0';
                IRSymbol label_sym = {strdup(label), IR_SYM_LABEL, {0}};
                IRCode code = {IR_LABEL, NULL, NULL, &label_sym};
                translate_ir_instruction(&obj, &code, &vars);
                
                free(label_sym.name);
            }
        }
        else if (strncmp(line, "RETURN ", 7) == 0) {
            char value[256];
            if (sscanf(line, "RETURN %s", value) == 1) {
                IRSymbol value_sym = {strdup(value), IR_SYM_TEMP, {0}};
                IRCode code = {IR_RETURN, &value_sym, NULL, NULL};
                translate_ir_instruction(&obj, &code, &vars);
                free(value_sym.name);
            } else {
                IRCode code = {IR_RETURN, NULL, NULL, NULL};
                translate_ir_instruction(&obj, &code, &vars);
            }
        }
        else if (strncmp(line, "CALL ", 5) == 0) {
            char func[256], result[256];
            if (sscanf(line, "CALL %[^,], %s", func, result) == 2) {
                IRSymbol func_sym = {strdup(func), IR_SYM_FUNC, {0}};
                IRSymbol result_sym = {strdup(result), IR_SYM_TEMP, {0}};
                IRCode code = {IR_CALL, &func_sym, NULL, &result_sym};
                translate_ir_instruction(&obj, &code, &vars);
                free(func_sym.name);
                free(result_sym.name);
            } else {
                char func[256];
                if (sscanf(line, "CALL %s", func) == 1) {
                    IRSymbol func_sym = {strdup(func), IR_SYM_FUNC, {0}};
                    IRCode code = {IR_CALL, &func_sym, NULL, NULL};
                    translate_ir_instruction(&obj, &code, &vars);
                    free(func_sym.name);
                }
            }
        }
        else if (strncmp(line, "LOAD_PARAM ", 11) == 0) {
            char param[256];
            if (sscanf(line, "LOAD_PARAM %s", param) == 1) {
                IRSymbol param_sym = {strdup(param), IR_SYM_TEMP, {0}};
                IRCode code = {IR_CALL_PARAM, &param_sym, NULL, NULL};
                translate_ir_instruction(&obj, &code, &vars);
                free(param_sym.name);
            }
        }
        else if (strncmp(line, "PARAM ", 6) == 0) {
            char param[256];
            if (sscanf(line, "PARAM %s", param) == 1) {
                IRSymbol param_sym = {strdup(param), IR_SYM_VAR, {0}};
                IRCode code = {IR_PARAM, &param_sym, NULL, NULL};
                translate_ir_instruction(&obj, &code, &vars);
                free(param_sym.name);
            }
        }
    }
    
    if (in_function) {
        translate_epilogue(&obj);
    }
    
    object_emit(&obj, ".section\t.note.GNU-stack,\"\",@progbits");
    fclose(ir_file);
    
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        fprintf(stderr, "Error: no se pudo crear %s\n", output_filename);
        object_free(&obj);
        var_table_free(&vars);
        return 1;
    }
    
    for (int i = 0; i < obj.size; i++) {
        fprintf(output, "%s\n", obj.lines[i]);
    }
    
    fclose(output);
    
    if (debug_mode) {
        printf("Código objeto guardado en: %s\n", output_filename);
    }
    
    object_free(&obj);
    var_table_free(&vars);
    
    return 0;
}