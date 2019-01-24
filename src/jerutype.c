#include "jerutype.h"
#include "../vector/vector.h"
#include <stdio.h>
#include <stdlib.h>

JeruType init_jeru_type(JeruTypeID id) {
    JeruType type;
    type.id = id;
    return type;
}
#define INIT_JERU(funcname, type, asname, typename) \
    JeruType init_jeru_##funcname(type value) { \
        JeruType jerutype = init_jeru_type(TYPE_##typename); \
        jerutype.as.asname = value; \
        return jerutype; \
    }

INIT_JERU(double, double, floating, DOUBLE)
INIT_JERU(int, long long, integer, INT)
INIT_JERU(string, char *, string, STRING)
#undef INIT_JERU

JeruType init_jeru_block(Token *tokens) {
    JeruType jerutype = init_jeru_type(TYPE_BLOCK);
    jerutype.as.block.tokens = tokens;
    jerutype.as.block.instruction = 0;
    return jerutype;
}

void free_jeru_type(JeruType *object) {
    if (object->id == TYPE_STRING) {
        free(object->as.string);
    } else if (object->id == TYPE_BLOCK) {
        for (size_t i = 0; i < vector_size(object->as.block.tokens); ++i)
            free(object->as.block.tokens[i].lexeme.string);
        vector_free(object->as.block.tokens);
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