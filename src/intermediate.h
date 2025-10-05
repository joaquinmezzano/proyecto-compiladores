#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

typedef enum {
    IR_LOAD,
    IR_STORE,
    IR_ADD,
    IR_SUB,
    IR_UMINUS,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_AND,
    IR_OR,
    IR_NOT,
    IR_EQ,
    IR_NEQ,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_LABEL,
    IR_GOTO,
    IR_IF_FALSE,
    IR_IF_TRUE,
    IR_RETURN,
    IR_CALL,
    IR_METHOD,
    IR_EXTERN,
    IR_PARAM
} IRInstr;

typedef struct IRSymbol {
    char *name;
    enum {
        IR_SYM_VAR,
        IR_SYM_TEMP,
        IR_SYM_LABEL,
        IR_SYM_CONST,
        IR_SYM_FUNC
    } type;
    union {
        int int_val;
        int bool_val;
    } value;
} IRSymbol;

typedef struct IRCode {
    IRInstr op;
    IRSymbol *arg1;
    IRSymbol *arg2;
    IRSymbol *result;
} IRCode;

#endif