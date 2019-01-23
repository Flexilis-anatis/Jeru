#pragma once
#include "lexer/token.h"
#include <stdbool.h>

typedef enum {
    TYPE_INT    = 1 << 0, // 00001
    TYPE_DOUBLE = 1 << 1, // 00010
    TYPE_STRING = 1 << 2, // 00100
    TYPE_BLOCK  = 1 << 3, // 01000
    TYPE_END    = 1 << 4, // 10000

    TYPE_NUM = TYPE_INT|TYPE_DOUBLE, // 00011
    TYPE_VAL = TYPE_NUM|TYPE_STRING, // 00111
    TYPE_ALL = TYPE_VAL|TYPE_BLOCK,  // 01111
    TYPE_BOOL = TYPE_VAL // resolvable to boolean
} JeruTypeID;

typedef struct {
    JeruTypeID id;
    union {
        long long integer;
        double floating;
        char *string;
        struct {
            Token *tokens;
            unsigned long instruction;
        } block;
    } as;
} JeruType;

JeruType init_jeru_type(JeruTypeID id);
JeruType init_jeru_int(long long value);
JeruType init_jeru_double(double value);
JeruType init_jeru_string(char *string);
JeruType init_jeru_block(Token *block);
bool jeru_true(JeruType *object);
void free_jeru_type(JeruType *object);

void print_jeru_type(JeruType *object);