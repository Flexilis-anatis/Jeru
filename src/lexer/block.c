#include "block.h"
#include "../../vector/vector.h"
#include <string.h>
#include <stdlib.h>

JeruBlock init_jeru_block(Token *tokens) {
    JeruBlock block;
    block.tokens = tokens;
    block.instruct = 0;
    return block;
}

JeruBlock copy_jeru_block(JeruBlock *source) {
    size_t size = vector_capacity(source->tokens) * sizeof(Token);
    Token *copied_list = malloc(size + sizeof(size_t) * 2);
    
    // Copy size & capacity
    memcpy(copied_list, &((size_t *)source->tokens)[-2], sizeof(size_t) * 2);
    copied_list = (Token *)&((size_t *)copied_list)[2];

    // Copy ID's, line no.s, and strings
    for (size_t index = 0; index < vector_size(source->tokens); ++index) {
        copied_list[index] = source->tokens[index];
        copied_list[index].lexeme.string = malloc(source->tokens[index].lexeme.length + 1);
        strcpy(copied_list[index].lexeme.string, source->tokens[index].lexeme.string);
    }

    // Return the new block!
    return init_jeru_block(copied_list);
}

void free_jeru_block(JeruBlock *block) {
    for (size_t item = 0; item < vector_size(block->tokens); ++item)
        free(block->tokens[item].lexeme.string);
    vector_free(block->tokens);
}
