#pragma once

typedef enum {
    // Custom
    TOK_INT, TOK_DOUBLE, TOK_WORD,
    // Magic
    TOK_ADD, TOK_SUB, TOK_MUL, TOK_DIV, TOK_PRINT, TOK_EXEC,
    // Block
    TOK_BLOCK_START, TOK_BLOCK_END,
    // Signals
    SIG_EOF, SIG_ERR
} TokenID;

typedef struct {
    struct {
        char *string;
        unsigned long length;
    } lexeme;

    TokenID id;
    unsigned long line;
} Token;