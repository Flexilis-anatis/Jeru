#include "jerutype.h"
#include <stdio.h>
#include <stdlib.h>

JeruType *init_jeru_type(JeruTypeID id) {
    JeruType *type = malloc(sizeof type);
    type->id = id;
    return type;
}

JeruType *init_jeru_int(long long value) {
    JeruType *type = init_jeru_type(TYPE_INT);
    type->as.integer = value;
    return type;
}

JeruType *init_jeru_double(double value) {
    JeruType *type = init_jeru_type(TYPE_DOUBLE);
    type->as.floating = value;
    return type;
}

void print_jeru_type(JeruType *object) {
    switch (object->id) {
        case TYPE_DOUBLE:
            printf("%f", object->as.floating);
            break;
        case TYPE_INT:
            printf("%lld", object->as.integer);
            break;
    }
}