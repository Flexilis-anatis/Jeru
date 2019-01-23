#include "runner.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"

VM main_vm;

void init_vm() {
    main_vm.stack = NULL;
}

void free_vm() {
    vector_free(main_vm.stack);
}

void forget_last(JeruType *copy) {
    if (copy != NULL)
        free_jeru_type(copy);
    vector_set_size(main_vm.stack, vector_size(main_vm.stack) - 1);
}

JeruType *pop() {
    if (main_vm.stack == NULL)
        return NULL;
    forget_last(NULL);
    JeruType *copy = malloc(sizeof(JeruType));
    memcpy(copy, vector_end(main_vm.stack), sizeof(JeruType));
    return copy;
}

void push(JeruType object) {
    vector_push_back(main_vm.stack, object);
}

JeruType parse_block(JeruType *parent_block) {
    unsigned long nest_level = 1;
    Token token, *block = NULL;
    while (nest_level > 0) {
        if (parent_block)
            token = parent_block->as.block.tokens[parent_block->as.block.instruction++];
        else
            token = next_token();

        if (token.id == TOK_BLOCK_START)
            ++nest_level;
        else if (token.id == TOK_BLOCK_END)
            --nest_level;

        vector_push_back(block, token);
    }
    return init_jeru_block(block);
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

#define RAISE_BOILER \
    main_vm.error.line = token.line; \
    main_vm.error.exists = true; \
    free(token.lexeme.string); \
    return false;


#define STACK_REQUIRE(op, req_length, ...) \
    do { \
        if (vector_size(main_vm.stack) < req_length) \
            main_vm.error.message = "Not enough space on stack for " op; \
        else if (!stack_ok(req_length, ## __VA_ARGS__)) \
            main_vm.error.message = "Datatypes on stack incorrect for " op; \
        else \
            break; \
        RAISE_BOILER \
    } while (0)

#define NUMOP(name, floatcode, intcode, tofloat) \
    { \
        STACK_REQUIRE(name, 2, TYPE_NUM, TYPE_NUM); \
        JeruType *y_ptr = pop(), \
                 *x_ptr = pop(); \
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
#define GENNUMOP(name, code, tofloat) \
    NUMOP(name, code, code, tofloat)

bool jeru_exec(JeruType *block) {
    while(block->as.block.instruction < vector_size(block->as.block.tokens)) {
        if (!run_token(block->as.block.tokens[block->as.block.instruction++], block)) {
            free_jeru_type(block);
            return false;
        }
    }
    return true;
}

/* Given size and list of wanted datatypes, return true if the stack matches the list.
 * OR types together to get mixes, e.g. TYPE_DOUBLE|TYPE_INT will match both.
 *
 * Predefined mixes:
 * TYPE_VAL = all values (int, string, etc.)
 * TYPE_NUM = TYPE_DOUBLE|TYPE_INT
 * TYPE_ALL = anything
 */
#define backwards_index(vector, index) ((vector)[vector_size(vector)-index-1])
bool stack_ok(size_t req_length, ...) {
    if (vector_size(main_vm.stack) < req_length)
        return false;
    else if (req_length == 0)
        return true;

    va_list args;
    va_start(args, req_length);

    // Really ugly. Iterates backwards a lot
    for (size_t i = req_length; i-- > 0;) {
        JeruTypeID req_id = va_arg(args, JeruTypeID);

        bool is_good = false;
        for (size_t type = 1; type < TYPE_END; type <<= 1) {
            if ((type & req_id) == 0)
                continue;
            if ((backwards_index(main_vm.stack, i).id & type) == type) {
                is_good = true;
                break;
            }
        }
        if (!is_good)
            return false;
    }

    va_end(args);
    return true;
}

bool run_token(Token token, JeruType *parent_block) {
    if (token.id == SIG_EOF)
        return main_vm.error.exists = false;

    switch (token.id) {
        case TOK_DOUBLE:
            push(init_jeru_double(strtod(token.lexeme.string, NULL)));
            break;
        case TOK_INT:
            push(init_jeru_int(strtoll(token.lexeme.string, NULL, 10)));
            break;
            
        case TOK_ADD: 
            GENNUMOP("addition", push(init_jeru_int(x + y));, false)
        case TOK_SUB: 
            GENNUMOP("subtraction", push(init_jeru_int(x - y));, false)
        case TOK_MUL:
            GENNUMOP("multiplication", push(init_jeru_int(x * y));, false)
        case TOK_DIV:
            GENNUMOP("division", push(init_jeru_int(x / y));, true)
        case TOK_GT:
            GENNUMOP("greater than", {
                if (x > y)
                    push(init_jeru_int(1));
                else
                    push(init_jeru_int(0));
            }, false)
        case TOK_LT:
            GENNUMOP("greater than", {
                if (x < y)
                    push(init_jeru_int(1));
                else
                    push(init_jeru_int(0));
            }, false)

        case TOK_PRINT: {
            STACK_REQUIRE("printing", 1, TYPE_VAL);
            JeruType *value = pop();
            print_jeru_type(value);
            free_jeru_type(value);
            break;
        }

        case TOK_BLOCK_START:
            push(parse_block(parent_block));
            break;

        case TOK_EXEC: {
            STACK_REQUIRE("executing", 1, TYPE_BLOCK);
            JeruType *block = pop();
            if (!jeru_exec(block))
                return false;
            free_jeru_type(block);
            break;
        }

        case TOK_COPY:
            STACK_REQUIRE("copying", 1, TYPE_ALL);
            push(*(vector_end(main_vm.stack)-1));
            break;

        case TOK_POP:
            STACK_REQUIRE("popping", 1, TYPE_ALL);
            free_jeru_type(pop());
            break;

        case TOK_IF: {
            STACK_REQUIRE("if statement", 2, TYPE_BLOCK, TYPE_BOOL);
            JeruType *cond = pop(), *block = pop();
            if (jeru_true(cond))
                if (!jeru_exec(block))
                    return false;
            free_jeru_type(cond);
            free_jeru_type(block);
            break;
        }

        case TOK_IFELSE: {
            STACK_REQUIRE("if-else statement", 3, TYPE_BLOCK, TYPE_BLOCK, TYPE_BOOL);
            JeruType *cond = pop(), *block_false = pop(), *block_true = pop();
            if (jeru_true(cond)) {
                if (!jeru_exec(block_true))
                    return false;
            }
            else {
                if (!jeru_exec(block_false))
                    return false;
            }

            free_jeru_type(cond);
            free_jeru_type(block_false);
            free_jeru_type(block_true);
            break;
        }
    }

    free(token.lexeme.string);
    return true;
}

void run(const char *source) {
    init_vm();
    set_source(source);
    while (run_token(next_token(), NULL));
    if (main_vm.error.exists)
        printf("[line %li] Error: %s\n", main_vm.error.line, main_vm.error.message);
    
    free_vm();
}

