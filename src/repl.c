#include "repl.h"
#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"

#include <assert.h>

extern VM main_vm;

void run_repl() {
    init_vm();
    printf("Welcome to the Jeru REPL");
    bool first_iter = true;
    char *input;
    while (1) {
        if (first_iter)
            first_iter = false;
        else
            free(input);

        input = readline("\n>>> ");

        JeruType *stack_copy = NULL;
        size_t size = 0;
        if (main_vm.stack) {
            size = (vector_capacity(main_vm.stack) * sizeof(JeruType)) + (sizeof(size_t) * 2);
            stack_copy = malloc(size);
            memcpy(stack_copy, &((size_t *)main_vm.stack)[-2], size);
        }

        add_history(input);
        set_source(input);

        while (run_token(next_token(), NULL));

        if (main_vm.error.exists) {
            main_vm.error.exists = false;
            printf("[line %li] Error: %s\n", main_vm.error.line, main_vm.error.message);

            vector_free(main_vm.stack);
            if (stack_copy) {
                main_vm.stack = malloc(size);
                memcpy(main_vm.stack, stack_copy, size);
                // Haha... sinces the size and reserved size are stored in stack[-1] and [-2] as
                // size_t's, I have to cast it to size_t *, get the value that will become [0], 
                // then cast it back to a JeruType *
                main_vm.stack = (JeruType *)&(((size_t *)main_vm.stack)[2]);

                free(stack_copy);
            } else {
                main_vm.stack = NULL;
            }
            continue;
        } else {
            free(stack_copy);
        }

        printf("\n[");
        for (size_t i = 0; i+1 < vector_size(main_vm.stack); ++i) {
            print_jeru_type(&main_vm.stack[i]);
            printf(", ");
        }
        // Print last value if there is one
        if (vector_size(main_vm.stack))
            print_jeru_type(&main_vm.stack[vector_size(main_vm.stack)-1]);

        putchar(']');
    }

    free_vm();
}
