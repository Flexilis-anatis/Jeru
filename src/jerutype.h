#pragma once
#include "lexer/token.h"
#include "lexer/block.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TYPE_NULL   = 0,      // 0000
    TYPE_INT    = 1 << 0, // 0001
    TYPE_DOUBLE = 1 << 1, // 0010
    TYPE_STRING = 1 << 2, // 0100
    TYPE_END    = TYPE_STRING, 

    TYPE_NUM = TYPE_INT|TYPE_DOUBLE, // 0011
    TYPE_VAL = TYPE_NUM|TYPE_STRING, // 0111
    TYPE_ALL = TYPE_VAL,             // 0111
    TYPE_BOOL = TYPE_VAL // resolvable to boolean
} JeruTypeID;

typedef struct {
    JeruTypeID id;
    union {
        long long integer;
        double floating;
        char *string;
    } as;
} JeruType;

JeruType jeru_type(JeruTypeID id);
JeruType jeru_type_int(long long value);
JeruType jeru_type_double(double value);
JeruType jeru_type_string(char *string);
bool jeru_true(JeruType *object);
void free_jeru_type(JeruType *object);

JeruType copy_jeru_type(JeruType *source);

void print_jeru_type(JeruType *object);
// For stack output. Prints all types. Translates newlines -> \n, etc.
void print_jeru_clean(JeruType *object);

// Composes a reversed, 0-terminated array of JeruTypeID's
JeruTypeID *jeru_id_list(size_t items, ...);
