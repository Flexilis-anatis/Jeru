#include "repl.h"
#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define LOGARITHMIC_GROWTH
#include "../vector/vector.h"

#define VECTOR_BOILER_SIZE (sizeof(size_t) * 2)

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
        if (vm->stack) {
            stack_copy = malloc((vector_capacity(vm->stack) * sizeof(JeruType))\
                                 + VECTOR_BOILER_SIZE);

            // Copying capacity and size over
            memcpy(stack_copy, &((size_t *)vm->stack)[-2], VECTOR_BOILER_SIZE);
            // Restoring standard array indexing
            stack_copy = (JeruType *)&((size_t *)stack_copy)[2];
            // Copying over old data
            for (size_t index = 0; index < vector_size(vm->stack); ++index)
                stack_copy[index] = copy_jeru_type(&vm->stack[index]);
        }

        JeruBlock *call_stack_copy = NULL;
        if (vm->call_stack) {
            // Basically the same as previous steps, but with JeruBlock instead
            // of JeruType
            call_stack_copy = malloc((vector_capacity(vm->call_stack) *
                                      sizeof(JeruBlock)) + VECTOR_BOILER_SIZE);

            memcpy(call_stack_copy, &((size_t *)vm->call_stack)[-2],
                   VECTOR_BOILER_SIZE);
            call_stack_copy = (JeruBlock *)&((size_t *)call_stack_copy)[2];
            for (size_t index = 0; index < vector_size(vm->call_stack); ++index)
                call_stack_copy[index] = copy_jeru_block(&vm->call_stack[index]);
        }

        hash_table *word_copy = malloc(sizeof(hash_table));
        hash_table tmp = ht_copy(vm->words);
        memcpy(word_copy, &tmp, sizeof(hash_table));

        if (*input == '?') {
            add_history(input+1);
            set_source(input+1);
        } else {
            set_source(input);
        }

        while (run_next_token(vm, NULL));

        if (vm->error.exists) {
            vm->error.exists = false;
            printf("[line %li] Error: %s\n", vm->error.line, vm->error.message);

            free_vm(vm);
            vm = init_vm();

            // Restore the stacks and word table
            if (stack_copy)
                vm->stack = stack_copy;
            else
                vm->stack = NULL;

            if (call_stack_copy)
                vm->call_stack = call_stack_copy;
            else
                vm->call_stack = NULL;

            vm->words = word_copy;

            continue;
        } else {
            vector_free(stack_copy);
            vector_free(call_stack_copy);
            ht_destroy(word_copy);
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
