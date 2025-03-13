#include "lexer/lexer.h"
#include "errors/errors.h"
#include "lexer/keywords.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Set keyword_set;

void create_lexer(Lexer *lexer, char *source, Catcher *catcher)
{
   lexer->catcher = catcher;
   lexer->source = source;
   create_vector(&lexer->tokens, 25u, sizeof(Token));
}

void free_lexer(Lexer *lexer)
{
   for (size_t i = 0u; i < lexer->tokens.size; ++i)
   {
      Token token = *(Token*)vector_at(&lexer->tokens, i); 
      if (token.type == IDENTIFIER || token.type == KEYWORD)
         free(token.lexeme);
   }
   free_vector(&lexer->tokens);
}

void lexer_tokenize(Lexer *lexer)
{
   size_t source_size = strlen(lexer->source);

   for (size_t i = 0; i < source_size; ++i)
   {
      char ch = lexer->source[i];

      if (isspace(ch))
         continue;

      if (ch == '=')
         lexer_push_back(lexer, EQUALS, "=");
      else if (isalpha(ch))
      {
         size_t capacity = 5u;
         size_t size = 0u;
         char* identifier = malloc(capacity);

         for (; i < source_size; ++i)
         {
            ch = lexer->source[i];

            if (size + 1 >= capacity)
            {
               capacity += capacity;
               void *new_identifier = realloc(identifier, capacity);

               if (!new_identifier)
               {
                  catcher_insert(lexer->catcher, err_cannot_resize_ident);
                  free(identifier);
                  return;
               }
               identifier = new_identifier;
            }

            if (!isalnum(ch) && ch != '_')
            {
               --i;
               break;
            }

            identifier[size] = ch;
            ++size;
         }

         identifier[size] = '\0';
         lexer_push_back(lexer, (set_contains(&keyword_set, identifier) ? KEYWORD : IDENTIFIER), identifier);
      }
      else if (isdigit(ch) || ch == '.')
      {

      }
      else
         catcher_insert(lexer->catcher, err_unexpected_char);
   }
   lexer_push_back(lexer, END_OF_FILE, "eof");
}

void lexer_push_back(Lexer *lexer, TType type, char *lexeme)
{
   Token token;
   token.type = type;
   token.lexeme = lexeme;
   vector_push_back(&lexer->tokens, &token);
}
