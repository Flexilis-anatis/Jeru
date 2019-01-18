#pragma once

typedef enum {
    // Custom
    TOK_INT, TOK_DOUBLE, TOK_WORD,
    // Magic
    TOK_ADD, TOK_PRINT,
    // Signals
    SIG_EOF, SIG_ERR
} TokenID;

typedef struct {
    struct {
        const char *string;
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