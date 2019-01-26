#include "runner.h"
#include "jerutype.h"
#include "../vector/vector.h"
#include <stdlib.h>

#define SET_ERROR(msg) \
    { \
        vm->error.exists = true; \
        vm->error.line = token.line; \
        vm->error.message = msg; \
        return false; \
    }
#define STACK_MSG "Not enough space on stack for "

#define NUMOP(name, floatcode, intcode) \
    { \
        if (vector_size(vm->stack) < 2) \
            SET_ERROR(STACK_MSG name) \
        JeruType *x_ptr = get_back_from(vm, 1),\
                 *y_ptr = get_back(vm);\
        if (!stack_has_types(vm, jeru_id_list(2, TYPE_NUM, TYPE_NUM))) \
            SET_ERROR("Incorrect types on stack for " name) \
        if (promote(x_ptr, y_ptr, Normal) == ToFloat) { \
            double x = x_ptr->as.floating, y = y_ptr->as.floating; \
            delete_back(vm); delete_back(vm); \
            floatcode \
        } else { \
            long long x = x_ptr->as.integer, y = y_ptr->as.integer; \
            delete_back(vm); delete_back(vm); \
            intcode \
        } \
        break; \
    }

typedef enum {ToFloat, ToInt, Normal} TypePromo;
void morph_type(JeruType *item, TypePromo type) {
    switch (type) {
        case ToFloat:
            item->id = TYPE_DOUBLE;
            item->as.floating = (double)item->as.integer;
            break;
        case ToInt:
            item->id = TYPE_INT;
            item->as.integer = (long long)item->as.floating;
            break;
    }
}

TypePromo promote(JeruType *x, JeruType *y, TypePromo promotion_type) {
    switch (promotion_type) {
        case ToFloat:
            morph_type(x, ToFloat);
            morph_type(y, ToFloat);
            break;
        case ToInt:
            morph_type(x, ToInt);
            morph_type(y, ToInt);
            return ToInt;
        case Normal:
            if (x->id == TYPE_DOUBLE)
                morph_type(y, ToFloat);
            else if (y->id == TYPE_DOUBLE)
                morph_type(x, ToFloat);
            else
                return ToInt;
    }
    return ToFloat;
}

bool run_next_token(JeruVM *vm, JeruBlock *scope) {
    Token token = next_token(scope);
    if (token.id == SIG_EOF) {
        return vm->error.exists = false;
    } else if (token.id == SIG_ERR) {
        vm->error.message = token.lexeme.string;
        vm->error.exists = true;
        vm->error.line = token.line;
        return false;
    }

    switch(token.id) {
        case TOK_INT:
            push(vm, jeru_type_int(strtoll(token.lexeme.string, NULL, 10)));
            break;

        case TOK_PRINT:
            if (!vector_size(vm->stack)) 
                SET_ERROR(STACK_MSG "printing");
            print_jeru_type(get_back(vm));
            break;

        case TOK_POP:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "popping")
            delete_back(vm);
            break;

        case TOK_ADD:
            NUMOP("addition", push(vm, jeru_type_double(x + y));, push(vm, jeru_type_int(x + y));)
    }

    if (!scope)
        free(token.lexeme.string);

    return true;
}