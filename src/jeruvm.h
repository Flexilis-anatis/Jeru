#pragma once
#include "jerutype.h"
#include "lexer/lex.h"
#include "lexer/block.h"
#include <stdbool.h>

typedef struct {
    JeruType *stack;
    JeruBlock *call_stack;
    struct {
        bool exists;
        unsigned long line;
        char *message;
    } error;
} JeruVM;

JeruVM *init_vm(void);
void free_vm(JeruVM *vm);

void push_data(JeruVM *vm, JeruType item);

// Returns the last item of the stack with reverse indexing (e.g. 0 is the last
// item, 1 is the second to last, etc.)
JeruType *get_back_from(JeruVM *vm, size_t index);
// Returns the last item of the stack WITHOUT deallocating it
JeruType *get_back(JeruVM *vm);
// Deallocates the last item of the stack. Returns false if there's nothing to delete
bool delete_back(JeruVM *vm);

// These map to the last 4 functions, but with the call stack
void push_block(JeruVM *vm, JeruBlock block);
JeruBlock get_block_from(JeruVM *vm, size_t index);
JeruBlock get_block(JeruVM *vm);
void delete_block(JeruVM *vm);

// This one's a whopper... given a 0-terminated list of types, make sure the stack has
// matching types, but backwards. As in, if the input is [TYPE_INT, TYPE_FLOAT, NULL], and
// the stack is [TYPE_STRING, TYPE_BLOCK, TYPE_INT, TYPE_FLOAT], this will return true
// Note that the variadic function jeru_id_list(len,...) is available for manual lists
// (jerutype.h)
bool stack_has_types(JeruVM *vm, JeruTypeID *types);