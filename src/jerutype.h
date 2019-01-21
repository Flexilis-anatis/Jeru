#pragma once

typedef enum {
    TYPE_INT    = 1 << 0, // 0001
    TYPE_DOUBLE = 1 << 1, // 0010
    TYPE_STRING = 1 << 2, // 0100
    TYPE_END    = 1 << 3, // 1000

    TYPE_NUM = TYPE_INT|TYPE_DOUBLE, // 0011
    TYPE_VAL = TYPE_NUM|TYPE_STRING, // 0111
    TYPE_ALL = TYPE_VAL              // 0111 (for now)
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