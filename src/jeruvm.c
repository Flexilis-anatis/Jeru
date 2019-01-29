#include "jeruvm.h"
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"
#include <stdio.h>

JeruVM *init_vm(void) {
    JeruVM *vm = malloc(sizeof(JeruVM));
    vm->stack = NULL;
    vm->call_stack = NULL;
    vm->error.exists = false;
    vm->words = malloc(sizeof(hash_table));
    // Not sure what best load factor is
    ht_init(vm->words, 0.3);
    return vm;
}

void free_vm(JeruVM *vm) {
    for (size_t index = 0; index < vector_size(vm->stack); ++index)
        free_jeru_type(&vm->stack[index]);
    for (size_t index = 0; index < vector_size(vm->call_stack); ++index)
        free_jeru_block(vm->call_stack[index]);
    vector_free(vm->call_stack);
    vector_free(vm->stack);
    ht_destroy(vm->words);
    free(vm);
}

void print_stack(JeruVM *vm) {
    printf("\n[");
    for (size_t i = 0; i+1 < vector_size(vm->stack); ++i) {
        print_jeru_clean(&vm->stack[i]);
        printf(", ");
    }
    // Print last value if there is one
    if (vector_size(vm->stack))
        print_jeru_clean(&vm->stack[vector_size(vm->stack)-1]);
    printf("], %lu", vector_size(vm->call_stack));
}

void push_data(JeruVM *vm, JeruType item) {
    vector_push_back(vm->stack, item);
}

JeruType *get_back_from(JeruVM *vm, size_t index) {
    if (vector_size(vm->stack) <= index)
        return NULL;
    return ((vector_end(vm->stack)-1)-index);
}
JeruType *get_back(JeruVM *vm) {
    return get_back_from(vm, 0);
}

bool delete_back(JeruVM *vm) {
    if (!vector_size(vm->stack))
        return false;

    free_jeru_type(get_back(vm));
    vector_pop_back(vm->stack);
    return true;
}

void push_block(JeruVM *vm, JeruBlock block) {
    vector_push_back(vm->call_stack, block);
}

JeruBlock *get_block(JeruVM *vm) {
    return vector_end(vm->call_stack)-1;
}

// This one's naive and assumes you know what you're doing
JeruBlock pop_block(JeruVM *vm) {
    JeruBlock block = copy_jeru_block(get_block(vm));
    vector_pop_back(vm->call_stack);
    return block;
}

void delete_block(JeruVM *vm) {
    if (vector_size(vm->call_stack) == 0)
        return;
    free_jeru_block(*get_block(vm));
    vector_pop_back(vm->call_stack);
}

bool check_type(JeruVM *vm, JeruTypeID expected, size_t index) {
    JeruTypeID actual_id = get_back_from(vm, index)->id;
    return (actual_id & expected) != 0;
}

bool stack_has_types(JeruVM *vm, JeruTypeID *types) {
    size_t index = 0;
    for (;*types;++types,++index) {
        if (!check_type(vm, *types, index))
            return false;
    }
    free(types-index);

    return true;
}
