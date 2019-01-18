#include "src/lexer/lex.h"
#include <stdio.h>

void print_token(Token *token) {
    const char *current = token->lexeme.start;
    while ((current - token->lexeme.start) < token->lexeme.length)
        putchar(*current++);
}

int main(void) {
    set_source("5 + 5.3");
    Token tok;
    size_t iter_limit = 20;
    while ((tok = next_token()).id != SIG_EOF) {
        putchar('\'');
        print_token(&tok);
        printf("'\tID: %lu\n", tok.id);
        if (--iter_limit == 0)
            break;
    }

    return 0;
}