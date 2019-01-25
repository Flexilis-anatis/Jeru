#include "src/lexer/lex.h"
#include "vector/vector.h"
#include "src/jeruvm.h"
#include "src/jerutype.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    JeruVM *vm = init_vm();

    push(vm, jeru_type_double(3.7));
    push(vm, jeru_type_int(123456789));

    // "hello\n\t\"world!\""
    set_source("\"hello\\n\\t\\\"world!\\\"\"");
    push(vm, jeru_type_string(next_token(NULL).lexeme.string));

    printf("Stack: ");
    while (vector_size(vm->stack)) {
        JeruType *back = get_back(vm);
        if (back == NULL)
            break;
        print_jeru_type(back);
        putchar(' ');
        delete_back(vm);
    }
    putchar('\n');

    free_vm(vm);

    return 0;
}
