#include "src/lexer/lex.h"
#include "vector/vector.h"
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    set_source("3.7 copy print [ pop 0 + ] exec Ident");
    Token token, *list = NULL;
    while ((token = next_token(NULL)).id != SIG_EOF) {
        printf("Token id: %d\tValue: '%s'\n", token.id, token.lexeme.string);
        vector_push_back(list, token);
    }
    JeruBlock *scope = init_jeru_block(list);
    while ((token = next_token(scope)).id != SIG_EOF) {
        printf("Token id: %d\tValue: '%s'\n", token.id, token.lexeme.string);
    }
    free_jeru_block(scope);

    return 0;
}
