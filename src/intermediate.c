#include "intermediate.h"
#include <stdio.h>

static int temp_count = 0;
static int label_count = 0;

void ir_init(IRList *list) {
    list->codes = NULL;
    list->size = 0;
    list->capacity = 0;
}

void ir_emit(IRList *list, IRInstr op, IRSymbol *arg1, IRSymbol *arg2, IRSymbol *result) {
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