#include "repl.h"
#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"

void run_repl() {
    printf("Welcome to the Jeru REPL");
    bool first_iter = true;
    char *input;

    JeruVM *vm = init_vm();

    while (1) {
        if (first_iter)
            first_iter = false;
        else
            free(input);

        input = readline("\n>>> ");

        JeruType *stack_copy = NULL;
        size_t size = 0;
        if (vm->stack) {
            size = (sizeof(size_t) * 2);
            stack_copy = malloc((vector_capacity(vm->stack) * sizeof(JeruType)) + size);
            memcpy(stack_copy, &((size_t *)vm->stack)[-2], size);
            stack_copy = (JeruType *)&((size_t *)stack_copy)[2];
            for (size_t index = 0; index < vector_size(vm->stack); ++index)
                stack_copy[index] = copy_jeru_type(&vm->stack[index]);
        }

        JeruBlock *call_stack_copy = NULL;
        size_t call_size = 0;
        if (vm->call_stack) {
            call_size = (sizeof(size_t) * 2);
            call_stack_copy = malloc((vector_capacity(vm->call_stack) * sizeof(JeruBlock)) + call_size);
            memcpy(call_stack_copy, &((size_t *)vm->call_stack)[-2], call_size);
            call_stack_copy = (JeruType *)&((size_t *)call_stack_copy)[2];
            for (size_t index = 0; index < vector_size(vm->call_stack); ++index)
                call_stack_copy[index] = copy_jeru_block(&vm->call_stack[index]);
        }

        add_history(input);
        set_source(input);

        while (run_next_token(vm, NULL));

        if (vm->error.exists) {
            vm->error.exists = false;
            printf("[line %li] Error: %s\n", vm->error.line, vm->error.message);

            free_vm(vm);
            vm = init_vm();

            // Restore the stack
            if (stack_copy)
                vm->stack = stack_copy;
            else
                vm->stack = NULL;

            if (call_stack_copy)
                vm->call_stack = call_stack_copy;
            else
                vm->call_stack = NULL;

            continue;
        } else {
            vector_free(stack_copy);
            vector_free(call_stack_copy);
        }

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

    free_vm(vm);
}
