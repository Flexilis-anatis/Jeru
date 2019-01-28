#include "lex.h"
#include "../../vector/vector.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // sprintf

static Scanner scanner;

void set_source(const char *source) {
    scanner.start = scanner.end = source;
    scanner.line = 1;
}

char advance() {
    return *(scanner.end++);
}

bool scanner_at_end() {
    return *scanner.end == '\0';
}

char peek() {
    if (*scanner.end == '\0')
        return '\0';
    return *(scanner.end+1);
}

char current() {
    return *scanner.end;
}

char start_offset(unsigned short offset) {
    if (scanner.end - scanner.start < offset)
        return '\0';
    return *(scanner.start + offset);
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

Token make_signal(TokenID signal, char *message) {
    Token tok;
    tok.id = signal;
    tok.lexeme.string = message;
    tok.line = scanner.line;
    return tok;
}

// Checks to see if a full string matches. Takes token_type to shorten
// short-circuit returns. It returns TOK_WORD_CALL if not equal.
TokenID matches(const char *string, unsigned int length, TokenID type) {
    // Sees if the string matches what's in the scanner
    if (scanner.end - scanner.start == 1 + length &&
        memcmp(scanner.start + 1, string, length) == 0)
        return type; // found
    return TOK_WORD_CALL; // not found
}

Token parse_word() {
    do
        advance();
    while (current() && !(isdigit(current()) || isspace(current())));

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
            case '[':
                return make_token(TOK_BLOCK_START);
            case ']':
                return make_token(TOK_BLOCK_END);
            case '>':
                return make_token(TOK_GT);
            case '<':
                return make_token(TOK_LT);
        }
    }

    // Parses 2+ length keywords
    switch (*scanner.start) {
        case 'p':
            if (start_offset(1) == 'o') 
                return make_token(matches("op", 2, TOK_POP));
            return make_token(matches("rint", 4, TOK_PRINT));
        case 'e':
            return make_token(matches("xec", 3, TOK_EXEC));
        case 'c':
            return make_token(matches("opy", 3, TOK_COPY));
        case 'i':
            if (start_offset(2) == 'e')
                return make_token(matches("felse", 5, TOK_IFELSE));
            return make_token(matches("f", 1, TOK_IF));
        case 'w':
            if (start_offset(1) == 'o')
                return make_token(matches("ord", 3, TOK_WORD));
            return make_token(matches("hile", 4, TOK_WHILE));
        case 'r':
            return make_token(matches("un", 2, TOK_RUN));
    }

    return make_token(TOK_WORD_CALL);
}

Token parse_number() {
    bool is_double = false;
    while (isdigit(current()) || current() == '.') {
        if (current() == '.') {
            if (is_double) // make sure only one decimal is found
                return make_signal(SIG_ERR, "Two decimal points in floating point literal");
            is_double = true;
        }

        advance();
    }

    if (is_double)
        return make_token(TOK_DOUBLE);

    return make_token(TOK_INT);
}

bool parse_out_comment() {
    do
        advance();
    while (current() != '#' && !scanner_at_end());
    
    if (scanner_at_end())
        return false;

    scanner.start = ++scanner.end;
    return true;
}

Token parse_string() {
    char last = '\0';
    do
        last = advance();
    while ((current() != '"' || last == '\\') && !scanner_at_end());

    if (scanner_at_end())
        return make_signal(SIG_ERR, "string not terminated");

    /* this entire next section parses out escape codes */
    size_t size = (size_t)((--scanner.end) - (++scanner.start));
    char *string = malloc(size + 2);

    // index is the index into the actual string, string_index is the index into the new string
    size_t string_index = 0;
    for (size_t index = 0; index <= size; ++index, ++string_index) {
        char character = start_offset(index);
        if (character == '\\') {
            character = start_offset(++index);
            if (index > size)
                return make_signal(SIG_ERR, "string ends with escape sequence");

            switch (character) {
                case '\\':
                    string[string_index] = '\\';
                    break;
                case 'n':
                    string[string_index] = '\n';
                    break;
                case 't':
                    string[string_index] = '\t';
                    break;
                case '"':
                    string[string_index] = '"';
                    break;

                default: {
                    #define STRING "unrecognized escape sequence '\\%c'"
                    string = malloc(sizeof STRING - 1);
                    sprintf(string, STRING, character);
                    return make_signal(SIG_ERR, string);
                    #undef STRING
                }
            }
        } else {
            string[string_index] = character;
        }
    }
    string[string_index] = '\0';

    Token tok;
    tok.id = TOK_STRING;
    tok.lexeme.string = string;
    tok.lexeme.length = scanner.end-scanner.start+1;
    tok.line = scanner.line;
    scanner.end += 2;
    scanner.start = scanner.end;
    return tok;
}

Token next_token(JeruBlock *scope) {
    if (scope) {
        if (scope->instruct == vector_size(scope->tokens))
            return make_signal(SIG_EOF, "EOF");
        return scope->tokens[scope->instruct++];
    }

    while (isspace(current())) {
        if (*scanner.start == '\n') 
            ++scanner.line;
        scanner.start = ++scanner.end;
    }

    if (scanner_at_end())
        return make_signal(SIG_EOF, "EOF");

    if (isdigit(current()) || current() == '.') {
        return parse_number();
    } else if (current() == '#') {
        if (!parse_out_comment())
            return make_signal(SIG_ERR, "unterminated comment");
        return next_token(scope);
    } else if (current() == '"') {
        return parse_string();
    }
    return parse_word();
}
