#include "lex.h"
#include <stdbool.h>
#include <ctype.h>

static Scanner scanner;

void set_source(const char *source) {
    scanner.start = scanner.end = source;
    scanner.line = 1;
}

char peek() {
    return scanner.end[1];
}

void advance() {
    ++scanner.end;
}

bool match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

bool scanner_at_end() {
    return *scanner.end == '\0';
}

char current() {
    return *scanner.end;
}

Token make_token(TokenID id) {
    Token tok;
    tok.id = id;
    tok.lexeme.start = scanner.start;
    tok.lexeme.length = (scanner.end - scanner.start);
    tok.line = scanner.line;
    scanner.start = scanner.end;
    return tok;
}

Token make_signal(TokenID signal) {
    Token tok;
    tok.id = signal;
    return tok;
}

Token parse_word() {
    do
        advance();
    while (current() && !(isdigit(current()) || isblank(current())));

    switch (*scanner.start) {
        case '+':
            if (scanner.start+1 == scanner.end) // one char
                return make_token(TOK_ADD);
            break;
    }

    return make_token(TOK_WORD);
}

Token parse_number() {
    bool is_double = false;
    while (isdigit(current()) || current() == '.') {
        if (current() == '.') {
            if (is_double) // make sure only one decimal is found
                return make_token(SIG_ERR);
            is_double = true;
        }

        advance();
    }

    if (is_double)
        return make_token(TOK_DOUBLE);
    return make_token(TOK_INT);
}

Token next_token() {
    if (scanner_at_end())
        return make_signal(SIG_EOF);

    while (isblank(current()))
        scanner.start = ++scanner.end;

    if (isdigit(current()) || current() == '.') {
        return parse_number();
    } else {
        return parse_word();
    }
}
