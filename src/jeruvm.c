#include "jeruvm.h"
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"

JeruVM *init_vm(void) {
    JeruVM *vm = malloc(sizeof(JeruVM));
    vm->stack = NULL;
    vm->error.exists = false;
    return vm;
}

void free_vm(JeruVM *vm) {
    for (size_t index = 0; index < vector_size(vm->stack); ++index)
        free_jeru_type(&vm->stack[index]);
    vector_free(vm->stack);
    free(vm);
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

JeruBlock *get_block_from(JeruVM *vm, size_t index) {
    if (vector_size(vm->call_stack) <= index)
        return NULL;
    return (vector_end(vm->call_stack)-1)-index;
}

JeruBlock *get_block(JeruVM *vm) {
    return get_block_from(vm, 0);
}

void delete_block(JeruVM *vm) {
    if (vector_size(vm->call_stack) == 0)
        return;
    free_jeru_block(get_block(vm));
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

    return true;
}