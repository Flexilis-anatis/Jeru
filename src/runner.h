#pragma once
#include "../vcvec/vcvec.h"
#include "jerutype.h"
#include "lexer/lex.h"

// Going to store word hashmap here when I'm done
typedef struct {
    vcvec /* JeruType */ *stack;
} VM;

void run(const char *source);
void init_vm(void);
void free_vm(void);
bool run_token(Token token);
