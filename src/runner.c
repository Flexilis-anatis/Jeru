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

// Forgets - but doesn't free - the end of the stack.
void forget_last() {
    vector_set_size(main_vm.stack, vector_size(main_vm.stack) - 1);
}

// Pops the end of the stack and returns a copy. Returns NULL on failure.
JeruType *pop() {
    if (main_vm.stack == NULL)
        return NULL;
    forget_last(); // Keep's it alloced, as long as you don't do anything with the vector
    JeruType *copy = malloc(sizeof(JeruType));
    // Copies over the item
    if (memcpy(copy, vector_end(main_vm.stack), sizeof(JeruType)) == NULL) 
        return NULL;
    return copy;
}

// Pushes an item onto the stack
void push(JeruType object) {
    vector_push_back(main_vm.stack, object);
}

// Give this a token to make it set the .line and .exists properties of the error substruct
#define RAISE_BOILER(line_) \
    main_vm.error.line = line_; \
    main_vm.error.exists = true;

// Ooh boy... pushes a block onto the stack. If the parent block is NULL, reads from lexer
JeruType parse_block(JeruType *parent_block) {
    // when the nest level drops to zero, we've reached the end (same amount of opens and closes)
    unsigned long nest_level = 1;
    Token token, *block = NULL;
    bool is_first_iter = true;
    unsigned long first_line;
    while (nest_level > 0) {
        if (parent_block) {
            // This lovely bit of code get's the next instruction and increases the IP
            token = parent_block->as.block.tokens[parent_block->as.block.instruction++];

            // If the instruction exceeds the size, set an error
            if (parent_block->as.block.instruction > vector_size(parent_block->as.block.tokens)) {
                main_vm.error.message = "No closing brace in code block";
                RAISE_BOILER(parent_block->as.block.tokens[0].line);
                free_jeru_type(parent_block);
                break;
            }
        } else {
            token = next_token();
            // Set the line of the first token for errors
            if (is_first_iter) {
                first_line = token.line;
                is_first_iter = false;
            }

            // If we've reached the end, they forgot to close their brace
            if (token.id == SIG_EOF) {
                main_vm.error.message = "No closing brace in code block";
                RAISE_BOILER(first_line);
                break;
            }
        }

        if (token.id == TOK_BLOCK_START)
            ++nest_level;
        else if (token.id == TOK_BLOCK_END)
            --nest_level;

        vector_push_back(block, token);
    }

    // Get rid of closing brace
    vector_pop_back(block);
    return init_jeru_block(block);
}

// Promote types to float if neccessary
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

// STACK_REQUIRE("command name", 3, TYPE_NUM, TYPE_STRING, TYPE_ALL)
// ^ makes sure that there are at least 3 items on the stack with the top being anything, the
// second one being a string, and the third one being a float or int
#define STACK_REQUIRE(op, req_length, ...) \
    do { \
        if (vector_size(main_vm.stack) < (req_length)) /* make sure ther's enough items */ \
            main_vm.error.message = "Not enough space on stack for " op; \
        else if (!stack_ok((req_length), ## __VA_ARGS__)) /* check types */ \
            main_vm.error.message = "Datatypes on stack incorrect for " op; \
        else \
            break; \
        RAISE_BOILER(token.line) /* should have broken out of loop now */ \
        free(token.lexeme.string); \
        return false; \
    } while (0)

// For stuff like equality and arithmatic. It needs the name for errors.
// Use the variables 'x' and 'y' for the values
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
            double x = x_ptr->as.floating, y = y_ptr->as.floating; \
            floatcode \
        } else { \
            long long x = x_ptr->as.integer, y = y_ptr->as.integer; \
            intcode \
        } \
        free_jeru_type(x_ptr); \
        free_jeru_type(y_ptr); \
        break; \
    }

// When your floatcode and intcode are the same
#define GENNUMOP(name, code, tofloat) \
    NUMOP(name, code, code, tofloat)

// Executes a block of code. Returns failure or success
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

// Runs a single token.
bool run_token(Token token, JeruType *parent_block) {
    if (token.id == SIG_EOF)
        return main_vm.error.exists = false;
    else if (token.id == SIG_ERR) {
        main_vm.error.message = token.lexeme.string;
        main_vm.error.exists = true;
        return false;
    }

    switch (token.id) {
        case TOK_DOUBLE:
            push(init_jeru_double(strtod(token.lexeme.string, NULL)));
            break;
        case TOK_INT:
            push(init_jeru_int(strtoll(token.lexeme.string, NULL, 10)));
            break;
            
        case TOK_ADD: 
            NUMOP("addition", push(init_jeru_double(x + y));, push(init_jeru_int(x + y));, false)
        case TOK_SUB: 
            NUMOP("subtraction", push(init_jeru_double(x - y));, push(init_jeru_int(x - y));, false)
        case TOK_MUL: 
            NUMOP("multiplication", push(init_jeru_double(x * y));, push(init_jeru_int(x * y));, false)
        case TOK_DIV:
            NUMOP("division", push(init_jeru_double(x / y));, push(init_jeru_int(x / y));, true)

        case TOK_GT:
            GENNUMOP("greater than", {
                if (x > y)
                    push(init_jeru_int(1));
                else
                    push(init_jeru_int(0));
            }, false)
        case TOK_LT:
            GENNUMOP("less than", {
                if (x < y)
                    push(init_jeru_int(1));
                else
                    push(init_jeru_int(0));
            }, false)

        case TOK_PRINT: {
            STACK_REQUIRE("printing", 1, TYPE_VAL);
            print_jeru_type(vector_end(main_vm.stack)-1);
            break;
        }

        case TOK_BLOCK_START:
            push(parse_block(parent_block));
            if (main_vm.error.exists) // the parser set's an error upon failure
                return false;
            break;

        case TOK_BLOCK_END:
            RAISE_BOILER(token.line);
            main_vm.error.message = "Unmatched ]";
            return false;

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
            /*
            sinces else-if's aren't possible, you can fake them like this:
            In C-like languages:
                if (cond1) {if-code}
                else if (cond2) {else-if-code}
                else {else-code}

            In Jeru:
                [if-code]
                [
                    [else-if-code]
                    [else-code]
                    cond2 if
                ]
                cond1 if
            */
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

        case TOK_WORD:
            #define STRING "Unrecognized word '"
            main_vm.error.message = malloc(sizeof(STRING)+token.lexeme.length);
            sprintf(main_vm.error.message, STRING"%s'", token.lexeme.string);
            RAISE_BOILER(token.line)
            free(token.lexeme.string);
            return false;
            #undef STRING

        case TOK_WHILE: {
            STACK_REQUIRE("while loop", 1, TYPE_BLOCK);
            JeruType *block = pop(), *top = NULL;
            do {
                if (top)
                    free_jeru_type(top);

                if (!jeru_exec(block))
                    return false;
                block->as.block.instruction = 0;

                top = vector_size(main_vm.stack) ? pop() : NULL;
            } while (vector_size(main_vm.stack) && jeru_true(top));
            if (top)
                free_jeru_type(top);
            free_jeru_type(block);
            break;
        }
    }

    if (parent_block == NULL)
        free(token.lexeme.string);
    return true;
}

bool run(const char *source) {
    init_vm();
    set_source(source);
    while (run_token(next_token(), NULL));
    free_vm();
    if (main_vm.error.exists) {
        printf("[line %li] Error: %s\n", main_vm.error.line, main_vm.error.message);
        return false;
    }
    
    return true;
}

