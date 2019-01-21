#pragma once

typedef enum {
    TYPE_INT    = 1 << 0, // 001
    TYPE_DOUBLE = 1 << 1, // 010
    TYPE_STRING = 1 << 2, // 100

    TYPE_NUM = TYPE_INT|TYPE_DOUBLE, // 011
    TYPE_VAL = TYPE_NUM|TYPE_STRING  // 111
} JeruTypeID;

typedef struct {
    JeruTypeID id;
    union {
        long long integer;
        double floating;
        char *string;
    } as;
} JeruType;

JeruType init_jeru_type(JeruTypeID id);
JeruType init_jeru_int(long long value);
JeruType init_jeru_double(double value);
JeruType init_jeru_string(char *string);
void free_jeru_type(JeruType *object);

void print_jeru_type(JeruType *object);