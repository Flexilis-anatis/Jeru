#include "block.h"
#include "../../vector/vector.h"

JeruBlock init_jeru_block(Token *tokens) {
    JeruBlock block;
    block.tokens = tokens;
    block.instruct = 0;
    return block;
}

void free_jeru_block(JeruBlock block) {
    for (size_t item = 0; item < vector_size(block.tokens); ++item)
        free(block.tokens[item].lexeme.string);
    vector_free(block.tokens);
}
