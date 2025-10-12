#include "object.h"
#include "intermediate.h"

/* CONSTRUCTORS AND DESTRUCTORS */
void object_init(ObjectCode *obj) {
    obj->lines = NULL;
    obj->size = 0;
    obj->capacity = 0;
}

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

void var_table_init(VarTable *table) {
    table->vars = NULL;
    table->count = 0;
    table->capacity = 0;
    table->stack_size = 0;
}

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