#pragma once

typedef enum {
    // Types
    TOK_INT, TOK_DOUBLE, TOK_STRING, TOK_WORD,
    // Magic
    TOK_ADD, TOK_SUB, TOK_MUL, TOK_DIV, TOK_PRINT, TOK_EXEC, TOK_COPY, TOK_POP,
    TOK_GT, TOK_LT,
    // "Statements" (in most langs)
    TOK_IF, TOK_IFELSE, TOK_WHILE,
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