#include "lex.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

static Scanner scanner;

void set_source(const char *source) {
    scanner.start = scanner.end = source;
    scanner.line = 1;
}

void advance() {
    ++scanner.end;
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

// Checks to see if a full string matches. Takes token_type to shorten
// short-circuit returns. It returns TOK_WORDif not equal.
TokenID matches(const char *string, unsigned int length, TokenID type) {
    // Sees if the string matches what's in the scanner
    if (scanner.end - scanner.start == 1 + length &&
        memcmp(scanner.start + 1, string, length) == 0)
        return type; // found
    return TOK_WORD; // not found
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
        case 'p':
            return make_token(matches("rint", 4, TOK_PRINT));
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
