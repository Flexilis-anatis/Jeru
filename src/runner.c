#include "runner.h"
#include <stdlib.h>

VM main_vm;
VMError vm_error;

void init_vm() {
    main_vm.stack = vcvec_create(0, sizeof(JeruType), NULL);
}

void free_vm() {
    free(main_vm.stack->data);
    free(main_vm.stack);
}

JeruType *pop() {
    return vcvec_copy_item(main_vm.stack, --main_vm.stack->count);
}

void push(JeruType object) {
    vcvec_push_back(main_vm.stack, &object); // just memcpy's, so it's fine to use a local ref
}

void forget_last(JeruType *copy) {
    if (copy != NULL)
        free_jeru_type(copy);
    --main_vm.stack->count;
}

typedef enum {NOT_FLOAT, IS_FLOAT} IsFloat;
IsFloat promote(JeruType *x, JeruType *y) {
    if (x->id == TYPE_INT && y->id == TYPE_INT) {
        return NOT_FLOAT;
    } else if (x->id == TYPE_DOUBLE && y->id == TYPE_INT) {
        y->id = TYPE_DOUBLE;
        y->as.floating = (double)y->as.integer;
    } else if (y->id == TYPE_DOUBLE && x->id == TYPE_INT) {
        x->id = TYPE_DOUBLE;
        x->as.floating = (double)x->as.integer;
    }

    return IS_FLOAT;
}

#define NUMOP(floatcode, intcode, tofloat) \
    { \
        JeruType *y_ptr = pop();\
         JeruType *x_ptr = pop(); \
        if (tofloat && y_ptr->id == TYPE_INT && x_ptr->id == TYPE_INT) { \
            y_ptr->id = TYPE_DOUBLE; \
            y_ptr->as.floating = (double)y_ptr->as.integer; \
        } \
        if (promote(x_ptr, y_ptr) == IS_FLOAT) { \
            float x = x_ptr->as.floating, y = y_ptr->as.floating; \
            floatcode \
        } else { \
            long long x = x_ptr->as.integer, y = y_ptr->as.integer; \
            intcode \
        } \
        free_jeru_type(x_ptr); \
        free_jeru_type(y_ptr); \
        break; \
    }

bool run_token(Token token) {
    if (token.id == SIG_EOF) return false;
    switch (token.id) {
        case TOK_DOUBLE:
            push(init_jeru_double(strtod(token.lexeme.string, NULL)));
            break;
        case TOK_INT:
            push(init_jeru_int(strtoll(token.lexeme.string, NULL, 10)));
            break;
        case TOK_ADD: 
            NUMOP(push(init_jeru_double(x + y));, push(init_jeru_int(x + y));, false)
        case TOK_SUB: 
            NUMOP(push(init_jeru_double(x - y));, push(init_jeru_int(x - y));, false)
        case TOK_MUL: 
            NUMOP(push(init_jeru_double(x * y));, push(init_jeru_int(x * y));, false)
        case TOK_DIV: 
            NUMOP(push(init_jeru_double(x / y));, push(init_jeru_int(x / y));, true)

        case TOK_PRINT: {
            JeruType *value = pop();
            print_jeru_type(value);
            free_jeru_type(value);
            break;
        }
    }

    free(token.lexeme.string);
    return true;
}

void run(const char *source) {
    init_vm();
    set_source(source);
    while (run_token(next_token()));
    free_vm();
}

