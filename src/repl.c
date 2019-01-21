#include "repl.h"
#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../vector/vector.h"

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
            size = vector_size(main_vm.stack) * sizeof(JeruType) + sizeof(size_t) * 2;
            stack_copy = malloc(size);
            memcpy(stack_copy, &vector_get_size(main_vm.stack), size);
            stack_copy = &stack_copy[1];
        }

        add_history(input);
        set_source(input);

        while (run_token(next_token()));

        if (main_vm.error.exists) {
            printf("[line %li] Error: %s\n", main_vm.error.line, main_vm.error.message);

            vector_free(main_vm.stack);
            if (stack_copy) {
                main_vm.stack = malloc(size);
                memcpy(main_vm.stack, &vector_get_size(stack_copy), size);
                main_vm.stack = &main_vm.stack[1];
                vector_free(stack_copy);
            } else {
                main_vm.stack = NULL;
            }
            continue;
        } else {
            vector_free(stack_copy);
        }

        printf("\n[");
        for (size_t i = 0; i+1 < vector_size(main_vm.stack); ++i) {
            print_jeru_type(&main_vm.stack[i]);
            printf(", ");
        }
        print_jeru_type(&main_vm.stack[vector_size(main_vm.stack)-1]);
        putchar(']');
    }
}
