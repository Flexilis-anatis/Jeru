#pragma once
#include "jerutype.h"
#include "lexer/lex.h"
#include <stdbool.h>

typedef struct {
    JeruType *stack;
    struct {
        bool exists;
        unsigned long line;
        char *message;
    } error;
} VM;

bool run(const char *source);
void init_vm(void);
void free_vm(void);
bool run_token(Token token, JeruType *parent_block);
