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
            size = (vector_capacity(vm->stack) * sizeof(JeruType)) + (sizeof(size_t) * 2);
            stack_copy = malloc(size);
            memcpy(stack_copy, &((size_t *)vm->stack)[-2], size);
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
            vector_free(vm->stack);
            if (stack_copy) {
                vm->stack = malloc(size);
                memcpy(vm->stack, stack_copy, size);
                // Haha... sinces the size and reserved size are stored in stack[-1] and [-2] as
                // size_t's, I have to cast it to size_t array, get the value that will become [0],
                // then cast it back to a JeruType array
                vm->stack = (JeruType *)&(((size_t *)vm->stack)[2]);

                free(stack_copy);
            } else {
                vm->stack = NULL;
            }
            continue;
        } else {
            free(stack_copy);
        }

        printf("\n[");
        for (size_t i = 0; i+1 < vector_size(vm->stack); ++i) {
            print_jeru_clean(&vm->stack[i]);
            printf(", ");
        }
        // Print last value if there is one
        if (vector_size(vm->stack))
            print_jeru_clean(&vm->stack[vector_size(vm->stack)-1]);

        putchar(']');
    }

    free_vm(vm);
}
