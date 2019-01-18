#pragma once
#include "../vcvec/vcvec.h"
#include "jerutype.h"
#include "lexer/lex.h"

// Going to store word hashmap here when I'm done
typedef struct {
    vcvec /* JeruType */ *stack;
} VM;

typedef struct {
    TokenID opcode;
    JeruType *jeru;
} Instruct;

Instruct next_instruct();
void run_instruct();
void free_instruct();
void free_vm();