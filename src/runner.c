#include "runner.h"
#include "jerutype.h"
#include <stdlib.h>

bool run_next_token(JeruVM *vm, JeruBlock *scope) {
    Token token = next_token(scope);
    if (token.id == SIG_EOF) {
        return vm->error.exists = false;
    } else if (token.id == SIG_ERR) {
        vm->error.message = token.lexeme.string;
        vm->error.exists = true;
        vm->error.line = token.line;
        return false;
    }

    switch(token.id) {
        case TOK_INT:
            push(vm, jeru_type_int(strtoll(token.lexeme.string, NULL, 10)));
            break;

        case TOK_PRINT:
            print_jeru_type(get_back(vm));
            break;
    }

    if (!scope)
        free(token.lexeme.string);

    return true;
}