#include "runner.h"
#include <stdlib.h>

static VM main_vm;

void init_vm() {
    main_vm.stack = vcvec_create(0, sizeof(JeruType), NULL);
}

JeruType *pop() {
    return (JeruType *)vcvec_pop(main_vm.stack);
}

void push(JeruType *object) {
    vcvec_push_back(main_vm.stack, object);
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

bool run_next_instruct() {
    Token token = next_token();
    if (token.id == SIG_EOF) return false;
    switch (token.id) {
        case TOK_DOUBLE:
            push(init_jeru_double(strtod(token.lexeme.string, NULL)));
            break;
        case TOK_INT:
            push(init_jeru_int(strtoll(token.lexeme.string, NULL, 10)));
            break;
        case TOK_ADD: {
            JeruType *x = pop(), *y = pop();
            if (promote(x, y) == IS_FLOAT) {
                push(init_jeru_double(x->as.floating + y->as.floating));
            } else {
                push(init_jeru_int(x->as.integer + y->as.integer));
            }
            free_jeru_type(x);
            free_jeru_type(y);
            break;
        }

        case TOK_PRINT:
            print_jeru_type(pop());
            break;
    }
    return true;
}

void run(const char *source) {
    init_vm();
    set_source(source);
    while (run_next_instruct());
}

