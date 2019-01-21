#pragma once

typedef enum {
    TYPE_INT, TYPE_DOUBLE, TYPE_STRING
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