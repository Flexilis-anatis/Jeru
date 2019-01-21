#pragma once
#include "jerutype.h"
#include "lexer/lex.h"
#include <stdbool.h>

typedef struct {
    JeruType *stack;
    struct {
        bool exists;
        unsigned long line;
        const char *message;
    } error;
} VM;

void run(const char *source);
void init_vm(void);
void free_vm(void);
bool run_token(Token token);
