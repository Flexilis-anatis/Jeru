#include "lex.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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

    // Parsing out the string
    char *string = malloc(scanner.end - scanner.start + 1);
    for (const char *cur = scanner.start; cur < scanner.end; ++cur)
        string[cur - scanner.start] = scanner.start[cur - scanner.start];
    string[scanner.end-scanner.start] = '\0';
    // Yuck...
    
    tok.lexeme.string = string;
    tok.lexeme.length = scanner.end-scanner.start;
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

    // Parses 1 length keywords
    if (scanner.end - scanner.start == 1) {
        switch(*scanner.start) {
            case '+':
                return make_token(TOK_ADD);
            case '-':
                return make_token(TOK_SUB);
            case '*':
                return make_token(TOK_MUL);
            case '/':
                return make_token(TOK_DIV);
        }
    }

    // Parses 2+ length keywords
    switch (*scanner.start) {
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
