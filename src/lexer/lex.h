#pragma once

typedef enum {
    TOK_INT, TOK_DOUBLE, TOK_ADD, TOK_WORD,
    SIG_EOF, SIG_ERR
} TokenID;

typedef struct {
    struct {
        const char *start;
        unsigned long length;
    } lexeme;

    TokenID id;
    unsigned long line;
} Token;

typedef struct {
    const char *start, *end;
    unsigned long line;
} Scanner;

void set_source(const char *source);
Token next_token();