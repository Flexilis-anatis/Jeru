#pragma once
#include "token.h"
#include "block.h"

typedef struct {
    const char *start, *end;
    unsigned long line;
} Scanner;

void set_source(const char *source);
Token next_token(JeruBlock *scope);