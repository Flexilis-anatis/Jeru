#pragma once
#include "token.h"

typedef struct {
    Token *tokens;
    unsigned long instruct;
} JeruBlock;

JeruBlock init_jeru_block(Token *tokens);
JeruBlock copy_jeru_block(JeruBlock *source);
void free_jeru_block(JeruBlock *block);