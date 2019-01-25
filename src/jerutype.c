#include "jerutype.h"
#include "../vector/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

JeruType jeru_type(JeruTypeID id) {
    JeruType type;
    type.id = id;
    return type;
}
#define INIT_JERU(funcname, type, asname, typename) \
    JeruType jeru_type_##funcname(type value) { \
        JeruType jerutype = jeru_type(TYPE_##typename); \
        jerutype.as.asname = value; \
        return jerutype; \
    }

INIT_JERU(double, double, floating, DOUBLE)
INIT_JERU(int, long long, integer, INT)
INIT_JERU(string, char *, string, STRING)
#undef INIT_JERU

JeruType jeru_type_block(Token *tokens) {
    JeruType jerutype = jeru_type(TYPE_BLOCK);
    jerutype.as.block = init_jeru_block(tokens);
    return jerutype;
}

void free_jeru_type(JeruType *object) {
    if (object->id == TYPE_STRING) {
        free(object->as.string);
    } else if (object->id == TYPE_BLOCK) {
        free_jeru_block(object->as.block);
    }
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
        case TYPE_BLOCK:
            printf("{...}");
            break;
    }
}

bool jeru_true(JeruType *object) {
    switch (object->id) {
        case TYPE_DOUBLE:
        case TYPE_INT:
            return object->as.floating != 0;
        case TYPE_STRING:
            return *object->as.string != '\0';
    }

    return false;
}

JeruTypeID *jeru_id_list(size_t items, ...) {
    JeruTypeID *list = malloc(sizeof(JeruTypeID) * (items + 1));
    va_list args;
    va_start(args, items);

    for (size_t item = 0; item < items; ++item) 
        list[item] = va_arg(args, JeruTypeID);
    list[items] = TYPE_NULL;

    va_end(args);

    return list;
}