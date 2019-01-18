#pragma once

typedef enum {
    TYPE_INT, TYPE_DOUBLE
} JeruTypeID;

typedef struct {
    JeruTypeID id;
    union {
        long long integer;
        double floating;
    } as;
} JeruType;

JeruType *init_jeru_type(JeruTypeID id);
JeruType *init_jeru_int(long long value);
JeruType *init_jeru_double(double value);
void free_jeru_type(JeruType *object);

void print_jeru_type(JeruType *object);