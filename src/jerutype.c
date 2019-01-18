#include "jerutype.h"
#include <stdio.h>
#include <stdlib.h>

JeruType *init_jeru_type(JeruTypeID id) {
    JeruType *type = malloc(sizeof type);
    type->id = id;
    return type;
}
#define INIT_JERU(funcname, type, asname, typename) \
    JeruType *init_jeru_##funcname(type value) { \
        JeruType *jerutype = init_jeru_type(TYPE_##typename); \
        jerutype->as.asname = value; \
        return jerutype; \
    }

INIT_JERU(double, double, floating, DOUBLE)
INIT_JERU(int, long long, integer, INT)
INIT_JERU(string, const char *, string, STRING)
#undef INIT_JERU

void free_jeru_type(JeruType *object) {
    if (object->id == TYPE_STRING)
        free(object->as.string);
    free(object);
}

void print_jeru_type(JeruType *object) {
    switch (object->id) {
        case TYPE_DOUBLE:
            printf("%f", object->as.floating);
            break;
        case TYPE_INT:
            printf("%lld", object->as.integer);
            break;
        case TYPE_STRING:
            printf("%s", object->as.string);
            break;
    }
}