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

        bool stack_is_null = true;
        JeruVM copy;
        if (vm->stack) {
            copy = *vm;
            stack_is_null = false;
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
            if (!stack_is_null) {
                size_t size = vector_capacity(&copy) * sizeof(JeruType) + 2 * sizeof(size_t);
                vm->stack = malloc(size);
                memcpy(&((size_t *)vm->stack)[-2], &((size_t *)&copy)[-2], size);
            } else {
                vm->stack = NULL;
            }
            continue;
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
