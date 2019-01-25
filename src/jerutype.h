#pragma once
#include "lexer/token.h"
#include "lexer/block.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TYPE_NULL   = 0,      // 00000
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
        JeruBlock *block;
    } as;
} JeruType;

JeruType jeru_type(JeruTypeID id);
JeruType jeru_type_int(long long value);
JeruType jeru_type_double(double value);
JeruType jeru_type_string(char *string);
JeruType jeru_type_block(Token *block);
bool jeru_true(JeruType *object);
void free_jeru_type(JeruType *object);

void print_jeru_type(JeruType *object);

// Composes a NULL terminated list of type ID's
JeruTypeID *jeru_id_list(size_t items, ...);
