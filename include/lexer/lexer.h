#ifndef LEXER_H
#define LEXER_H

#include "errors/catcher.h"
#include "lexer/tokens.h"

typedef struct
{
   char* source;
   Vector tokens;
   Catcher* catcher;
} Lexer;

void create_lexer(Lexer *lexer, char *source, Catcher *catcher);
void free_tokens_vector(Vector* vector);

void lexer_tokenize(Lexer *lexer);
void lexer_push_back(Lexer *lexer, TType type, char *lexeme);
char lexer_peek(Lexer *lexer, size_t index, size_t by);
bool is_token_mallocated(TType type);

#endif // LEXER_H
