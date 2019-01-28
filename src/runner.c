#include "runner.h"
#include "jerutype.h"
#include "../vector/vector.h"
#include <stdlib.h>
#include <string.h>

#define SET_ERROR(msg) \
    { \
        vm->error.exists = true; \
        vm->error.line = token.line; \
        vm->error.message = msg; \
        return false; \
    }
#define STACK_MSG "Not enough space on stack for "

JeruBlock parse_block(bool *eof_error, JeruBlock *scope) {
    *eof_error = false;

    Token *tok_list = NULL;
    size_t nest_level = 1;
    while (nest_level > 0 && !*eof_error) {
        Token token = next_token(scope);
        switch (token.id) {
            case SIG_EOF:
                if (nest_level)
                    *eof_error = true;
                break;
            case TOK_BLOCK_START:
                ++nest_level; break;
            case TOK_BLOCK_END:
                --nest_level; break;
            default: break;
        }

        vector_push_back(tok_list, token);
    }

    vector_pop_back(tok_list); // get rid of final ]
    JeruBlock block = init_jeru_block(tok_list);
    return copy_jeru_block(&block);
}

bool jeru_exec(JeruVM *vm, JeruBlock *scope) {
    while (scope->instruct < vector_size(scope->tokens))
        if (!run_next_token(vm, scope))
            return false;
    return true;
}

#define NUMOP(name, floatcode, intcode) \
    { \
        if (vector_size(vm->stack) < 2) \
            SET_ERROR(STACK_MSG name) \
        if (!stack_has_types(vm, jeru_id_list(2, TYPE_NUM, TYPE_NUM))) \
            SET_ERROR("Incorrect types on stack for " name) \
        JeruType *x_ptr = get_back_from(vm, 1),\
                 *y_ptr = get_back(vm);\
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
#define TYPELESS_NUMOP(name, code) \
    NUMOP(name, code, code)

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
            push_data(vm, jeru_type_int(strtoll(token.lexeme.string, NULL, 10)));
            break;

        case TOK_STRING: {
            char *new_string = malloc(token.lexeme.length + 1);
            strcpy(new_string, token.lexeme.string);
            push_data(vm, jeru_type_string(new_string));
            return true;
        }

        case TOK_DOUBLE:
            push_data(vm, jeru_type_double(strtod(token.lexeme.string, NULL)));
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

        case TOK_COPY:
            push_data(vm, copy_jeru_type(get_back(vm)));
            break;

        case TOK_ADD:
            NUMOP("addition", push_data(vm, jeru_type_double(x + y));, push_data(vm, jeru_type_int(x + y));)
        case TOK_SUB:
            NUMOP("subtraction", push_data(vm, jeru_type_double(x - y));, push_data(vm, jeru_type_int(x - y));)
        case TOK_MUL:
            NUMOP("multiplication", push_data(vm, jeru_type_double(x * y));, push_data(vm, jeru_type_int(x * y));)
        case TOK_DIV:
            if (vector_size(vm->stack))
                morph_type(get_back(vm), ToFloat);
            NUMOP("division", push_data(vm, jeru_type_double(x / y));, (void)x;(void)y;)
        case TOK_LT:
            TYPELESS_NUMOP("less than operation",
                push_data(vm, jeru_type_int(x < y ? 1 : 0));
            )
        case TOK_GT:
            TYPELESS_NUMOP("greater than operation",
                push_data(vm, jeru_type_int(x > y ? 1 : 0));
            )


        // Everything below this point is based around code blocks

        case TOK_BLOCK_START: {
            bool eof_error;
            push_block(vm, parse_block(&eof_error, scope));
            if (eof_error) {
                delete_block(vm);
                SET_ERROR("Unmatched '['");
            }
            break;
        }
        case TOK_BLOCK_END:
            SET_ERROR("Unmatched ']'")

        case TOK_EXEC:
        case TOK_RUN: {
            if (!vector_size(vm->call_stack))
                SET_ERROR("Nothing to execute");

            JeruBlock block;
            if (token.id == TOK_EXEC)
                block = pop_block(vm);
            else
                block = copy_jeru_block(get_block(vm));

            if (!jeru_exec(vm, &block))
                return false;

            free_jeru_block(block);

            break;
        }

        // Control flow

        case TOK_IF: {
            if (!vector_size(vm->call_stack))
                SET_ERROR("Nothing to execute in if statement");
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "if statement");
            if (!stack_has_types(vm, jeru_id_list(1, TYPE_BOOL)))
                SET_ERROR("Need a boolean-convertible type on stack for if statement");

            JeruType *cond = get_back(vm);
            if (jeru_true(cond)) {
                delete_back(vm);
                JeruBlock block = pop_block(vm);

                if (!jeru_exec(vm, &block))
                    return false;

                free_jeru_block(block);
            } else {
                delete_back(vm);
            }
            break;
        }

        case TOK_IFELSE: {
            if (vector_size(vm->call_stack) < 2)
                SET_ERROR("Not enough blocks to execute if-else statement");
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "if-else statement");
            if (!stack_has_types(vm, jeru_id_list(1, TYPE_BOOL)))
                SET_ERROR("Need a boolean-convertible type on stack for if-else statement");

            JeruType *cond = get_back(vm);
            JeruBlock block_false = pop_block(vm);
            JeruBlock block_true = pop_block(vm);

            if (jeru_true(cond)) {
                delete_back(vm);
                if (!jeru_exec(vm, &block_true)) {
                    free_jeru_block(block_false);
                    free_jeru_block(block_true);
                    return false;
                }
            } else {
                delete_back(vm);
                if (!jeru_exec(vm, &block_false)) {
                    free_jeru_block(block_false);
                    free_jeru_block(block_true);
                    return false;
                }
            }

            free_jeru_block(block_false);
            free_jeru_block(block_true);

            break;
        }

        case TOK_WHILE: {
            if (!vector_size(vm->call_stack))
                SET_ERROR("Nothing to execute in if statement");
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "if statement");

            JeruBlock block = pop_block(vm);
            JeruType *cond = NULL;

            do {
                if (cond)
                    delete_back(vm);
                block.instruct = 0;
                if (!jeru_exec(vm, &block)) {
                    free_jeru_block(block);
                    return false;
                }
                if (!stack_has_types(vm, jeru_id_list(1, TYPE_BOOL)))
                    SET_ERROR("Need a boolean-convertible type on stack for if statement");
                cond = get_back(vm);
            } while (jeru_true(cond));

            delete_back(vm);
            free_jeru_block(block);

            break;
        }

        case TOK_WORD_CALL: case TOK_WORD:
            SET_ERROR("Words are unimplemented");
    }

    if (!scope)
        free(token.lexeme.string);

    return true;
}
