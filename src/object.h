#ifndef OBJECT_H
#define OBJECT_H

typedef enum {
    O_LOAD,
    O_STORE,
    O_ADD,
    O_SUB,
    O_UMINUS,
    O_MUL,
    O_DIV,
    O_MOD,
    O_AND,
    O_OR,
    O_NOT,
    O_EQ,
    O_NEQ,
    O_LT,
    O_LE,
    O_GT,
    O_GE,
    O_LABEL,
    O_GOTO,
    O_IF_FALSE,
    O_IF_TRUE,
    O_RETURN,
    O_CALL,
    O_METHOD,
    O_EXTERN,
    O_PARAM,
    O_CALL_PARAM
} OInstr;

// PROLOGO
//     CODIGO
//     Registros de 8 en 8
// EPILOGO

void o_print();
void o_save_to_file();

#endif