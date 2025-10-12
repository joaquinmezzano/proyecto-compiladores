#ifndef OBJECT_H
#define OBJECT_H

typedef struct {
    char **lines;
    int size;
    int capacity;
} ObjectCode;

typedef struct {
    char *name;
    int offset;
} VarInfo;

typedef struct {
    VarInfo *vars;
    int count;
    int capacity;
    int stack_size;
} VarTable;

#endif