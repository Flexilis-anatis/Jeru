#include "jerutype.h"
#include "../vector/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

// Does NOT free the given object itself
void free_jeru_type(JeruType *object) {
    if (object->id == TYPE_STRING) {
        free(object->as.string);
    }
}

JeruType copy_jeru_type(JeruType *source) {
    JeruType new;
    new.id = source->id;
    if (source->id == TYPE_STRING) {
        new.as.string = malloc(strlen(source->as.string) + 1);
        strcpy(new.as.string, source->as.string);
    } else {
        // Just to copy the bits over
        new.as.floating = source->as.floating;
    }
    return new;
}

JeruType *alloc_jeru_copy(JeruType *source) {
    JeruType *new = malloc(sizeof(JeruType));
    new->id = source->id;
    if (source->id == TYPE_STRING) {
        new->as.string = malloc(strlen(source->as.string) + 1);
        strcpy(new->as.string, source->as.string);
    } else {
        // Just to copy the bits over
        new->as.floating = source->as.floating;
    }
    return new;
}

void print_jeru_type(JeruType *object) {
    switch (object->id) {
        case TYPE_DOUBLE:
            printf("%f", object->as.floating);
            break;
        case TYPE_INT:
            printf("%lld",object->as.integer);
            break;
        case TYPE_STRING:
            printf("%s", object->as.string);
            break;
    }
}

void print_sanatized_string(char *string) {
    putchar('"');
    for(; *string; ++string) {
        switch (*string) {
            case '\\':
                printf("\\\\");
                break;
            case '\n':
                printf("\\n");
                break;
            case '\t':
                printf("\\t");
                break;
            case '"':
                printf("\\\"");
                break;
            default:
                putchar(*string);
                break;
        }
    }
    putchar('"');
}

void print_jeru_clean(JeruType *object) {
    if (object->id == TYPE_STRING)
        print_sanatized_string(object->as.string);
    else
        print_jeru_type(object);
}

bool jeru_true(JeruType *object) {
    switch (object->id) {
        case TYPE_DOUBLE:
        case TYPE_INT:
            return object->as.floating != 0;
        case TYPE_STRING:
            return *object->as.string != '\0';
        default:
            return false;
    }
}

JeruTypeID *jeru_id_list(size_t items, ...) {
    JeruTypeID *list = malloc(sizeof(JeruTypeID) * (items + 1));
    va_list args;
    va_start(args, items);

    for (size_t item = 0; item < items; ++item)
        list[items-1-item] = va_arg(args, JeruTypeID);
    list[items] = TYPE_NULL;

    va_end(args);

    return list;
}