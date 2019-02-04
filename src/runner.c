#include "runner.h"
#include "jerutype.h"
#include "../vector/vector.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

    // get rid of final ]
    if (!scope) free(tok_list[vector_size(tok_list)-1].lexeme.string);
    vector_pop_back(tok_list); 

    // copy and return
    JeruBlock block = init_jeru_block(tok_list);
    JeruBlock safeblock = copy_jeru_block(&block);
    if (!scope) free_jeru_block(&block);
    else vector_free(block.tokens);
    return safeblock;
}

bool jeru_exec(JeruVM *vm, JeruBlock *scope) {
    while (scope->instruct < vector_size(scope->tokens))
        if (!run_next_token(vm, scope, false))
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
            if (!nopop) {delete_back(vm); delete_back(vm);} \
            floatcode \
        } else { \
            long long x = x_ptr->as.integer, y = y_ptr->as.integer; \
            if (!nopop) {delete_back(vm); delete_back(vm);} \
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
            if (item->id == TYPE_DOUBLE)
                break;
            item->id = TYPE_DOUBLE;
            item->as.floating = (double)item->as.integer;
            break;
        case ToInt:
            if (item->id == TYPE_INT)
                break;
            item->id = TYPE_INT;
            item->as.integer = (long long)item->as.floating;
            break;
        case Normal:
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

void concat_jeru_strings(JeruType *string1, JeruType *string2) {
    string1->as.string = realloc(string1->as.string,
                         strlen(string1->as.string)+strlen(string2->as.string)+1);
    strcat(string1->as.string, string2->as.string);
}

bool run_next_token(JeruVM *vm, JeruBlock *scope, bool nopop) {
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

        case TOK_NOPOP:
            return run_next_token(vm, scope, true);

        case TOK_PRINT:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "printing");
            print_jeru_type(get_back(vm));
            break;

        case TOK_STACKLOG:
            print_stack(vm);
            break;

        case TOK_SWAPTOP:
            if (vector_size(vm->stack) < 2)
                SET_ERROR(STACK_MSG "swapping");
            JeruType tmp = *get_back(vm);
            vm->stack[vector_size(vm->stack)-1] = *get_back_from(vm, 1);
            vm->stack[vector_size(vm->stack)-2] = tmp;
            break;

        case TOK_POP:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "popping")
            delete_back(vm);
            break;

        case TOK_COPY:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "copying")
            push_data(vm, copy_jeru_type(get_back(vm)));
            break;

        case TOK_ADD:
            if (vector_size(vm->stack) >= 2 &&
                stack_has_types(vm, jeru_id_list(2, TYPE_STRING, TYPE_STRING))) {
                JeruType *string2 = get_back(vm), *string1 = get_back_from(vm, 1);
                concat_jeru_strings(string1, string2);
                delete_back(vm); // second string
                break;
            } else {
                NUMOP("addition", push_data(vm, jeru_type_double(x + y));, push_data(vm, jeru_type_int(x + y));)
            }
        case TOK_SUB:
            NUMOP("subtraction", push_data(vm, jeru_type_double(x - y));, push_data(vm, jeru_type_int(x - y));)
        case TOK_MUL:
            if (vector_size(vm->stack) >= 2) {
                JeruType *number, *string;
                if (get_back(vm)->id == TYPE_STRING &&
                    get_back_from(vm, 1)->id == TYPE_INT) {
                    string = get_back(vm);
                    number = get_back_from(vm, 1);
                } else if (get_back(vm)->id == TYPE_INT &&
                           get_back_from(vm, 1)->id == TYPE_STRING) {
                    string = get_back_from(vm, 1);
                    number = get_back(vm);
                } else {
                    NUMOP("multiplication",
                        push_data(vm, jeru_type_double(x * y));,
                        push_data(vm, jeru_type_int(x * y));)
                }

                long long times = number->as.integer;
                if (times < 0)
                    SET_ERROR("Must multiply strings by a value >= 1");

                char *orig = string->as.string;
                string->as.string = malloc(strlen(string->as.string)*times+1);
                string->as.string[0] = '\0';
                while (times--)
                    strcat(string->as.string, orig);
                free(orig);
                vector_set_size(vm->stack, vector_size(vm->stack)-2);
                free_jeru_type(number);
                push_data(vm, *string);
                break;
            }

            NUMOP("multiplication",
                push_data(vm, jeru_type_double(x * y));,
                push_data(vm, jeru_type_int(x * y));)
            break;
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
        case TOK_LTE:
            TYPELESS_NUMOP("less-equal to operation",
                push_data(vm, jeru_type_int(x <= y ? 1 : 0));
            )
        case TOK_GTE:
            TYPELESS_NUMOP("greater-equal to operation",
                push_data(vm, jeru_type_int(x >= y ? 1 : 0));
            )
        case TOK_EQUALS:
            if (vector_size(vm->stack) < 2)
                SET_ERROR(STACK_MSG "equality test")
            JeruType *x = get_back(vm), *y = get_back_from(vm, 1);
            bool result = false;
            if (x->id == TYPE_STRING && y->id == TYPE_STRING) {
                result = strcmp(x->as.string, y->as.string) == 0;
            } else if (((x->id & TYPE_NUM) != 0) && ((y->id & TYPE_NUM) != 0)) {
                if (promote(x, y, Normal) == ToFloat) {
                    result = x->as.floating == y->as.floating;
                } else {
                    result = x->as.integer == y->as.integer;
                }
            }
            if (!nopop) {
                delete_back(vm);
                delete_back(vm);
            }
            push_data(vm, jeru_type_int(result));
            break;

        case TOK_FLOOR:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "floor operation");
            if (!(get_back(vm)->id == TYPE_DOUBLE))
                SET_ERROR("Floor only works on floats");
            get_back(vm)->as.integer = (long long)get_back(vm)->as.floating;
            get_back(vm)->id = TYPE_INT;
            break;

        case TOK_CEIL:
            if (!vector_size(vm->stack))
                SET_ERROR(STACK_MSG "floor operation");
            if (!stack_has_types(vm, jeru_id_list(1, TYPE_DOUBLE)))
                SET_ERROR("Ceil only works on floats");
            get_back(vm)->as.integer = ceil(get_back(vm)->as.floating);
            get_back(vm)->id = TYPE_INT;
            break;

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

            free_jeru_block(&block);

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
            JeruBlock block = pop_block(vm);
            if (jeru_true(cond)) {
                delete_back(vm);
                if (!jeru_exec(vm, &block))
                    return false;
            } else {
                delete_back(vm);
            }
            free_jeru_block(&block);
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
                    free_jeru_block(&block_false);
                    free_jeru_block(&block_true);
                    return false;
                }
            } else {
                delete_back(vm);
                if (!jeru_exec(vm, &block_false)) {
                    free_jeru_block(&block_false);
                    free_jeru_block(&block_true);
                    return false;
                }
            }

            free_jeru_block(&block_false);
            free_jeru_block(&block_true);

            break;
        }

        case TOK_WHILE: {
            if (!vector_size(vm->call_stack))
                SET_ERROR("Nothing to execute in while statement");

            JeruBlock block = pop_block(vm);
            JeruType *cond = NULL;

            do {
                if (cond)
                    delete_back(vm);
                block.instruct = 0;
                if (!jeru_exec(vm, &block)) {
                    free_jeru_block(&block);
                    return false;
                }
                if (!stack_has_types(vm, jeru_id_list(1, TYPE_BOOL)))
                    SET_ERROR("Need a boolean-convertible type on stack for if statement");
                cond = get_back(vm);
            } while (jeru_true(cond));

            delete_back(vm);
            free_jeru_block(&block);

            break;
        }

        case TOK_WORD: {
            // free the 'word' lexeme
            if (!scope)
                free(token.lexeme.string);

            if (!vector_size(vm->call_stack))
                SET_ERROR("No code block to pop into word");
            token = next_token(scope);
            if (token.id != TOK_WORD_CALL)
                SET_ERROR("No word name after 'word' keyword");

            JeruBlock tmp = copy_jeru_block(get_block(vm));
            ht_insert(vm->words, token.lexeme.string, token.lexeme.length, &tmp,
                      sizeof(JeruBlock));
            delete_block(vm);
            break;
        }

        case TOK_WORD_CALL: {
            JeruBlock *block = ht_get(vm->words, token.lexeme.string, token.lexeme.length,
                                      NULL);
            if (block == NULL)
                SET_ERROR("Unrecognized word");

            JeruBlock tmp = copy_jeru_block(block);
            if (!jeru_exec(vm, &tmp)) {
                free_jeru_block(&tmp);
                return false;
            }
            free_jeru_block(&tmp);

            break;
        }
        default: break; // shut up -Wswitch
    }

    if (!scope)
        free(token.lexeme.string);

    return true;
}
