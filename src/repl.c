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

        add_history(input);
        set_source(input);

        JeruType *stack_copy = NULL;
        size_t size = main_vm.stack ? sizeof(size_t)*2 + vector_get_size(main_vm.stack) * sizeof(JeruType) : 0;
        if (main_vm.stack) {
            printf("Orig size: %li\n", vector_get_size(main_vm.stack));
            stack_copy = malloc(size);
            memcpy(stack_copy, &vector_get_size(main_vm.stack), size);
            printf("New size: %li\n", *(size_t *)stack_copy);
        }

        while (run_token(next_token()));

        if (main_vm.error.exists) {
            printf("[line %li] Error: %s\n", main_vm.error.line, main_vm.error.message);
            if (main_vm.stack)
                vector_free(main_vm.stack);
            if (stack_copy) {
                printf("Allocating...");
                ((size_t *)main_vm.stack)[-2] = malloc(size);
                printf("Copying...\n");
                memcpy(((size_t *)main_vm.stack)[-2], stack_copy, size);
                printf("Newer size: %li\n", vector_get_size(main_vm.stack));
                //vector_free(&stack_copy[2]);
            } else {
                main_vm.stack = NULL;
            }
            continue;
        } else if (stack_copy) {
            //vector_free(stack_copy);
        }

        printf("\n[");
        for (size_t i = 0; i < vector_get_size(main_vm.stack); ++i) {
            print_jeru_type(&main_vm.stack[i]);
            putchar(',');
        }
        putchar(']');
    }
}