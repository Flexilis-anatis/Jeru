#include "repl.h"
#include "runner.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>

extern VM main_vm;

void run_repl() {
    init_vm();
    printf("Welcome to the Jeru REPL");
    while (1) {
        char *input = readline("\n>>> ");
        if (*input == '\0')
            continue;

        set_source(input);
        while (run_token(next_token()));
        add_history(input);

        printf("\n[");
        for (JeruType *item = vcvec_begin(main_vm.stack);
               item != vcvec_end(main_vm.stack);
               item = vcvec_next(main_vm.stack, item)) {
            print_jeru_type(item);
            putchar(',');
        }
        putchar(']');
    }
}