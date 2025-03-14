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
      if (token.type == IDENTIFIER || token.type == KEYWORD || token.type == STRING || token.type == CHARACTER || token.type == REAL || token.type == INTEGER)
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
      else if (ch == '/' && lexer_peek(lexer, i, 1) == '/')
         for (; i < source_size && lexer->source[i] != '\n'; ++i)
            ;
      else if (ch == '/' && lexer_peek(lexer, i, 1) == '*')
      {
         for (; i < source_size && (lexer->source[i] != '*' || lexer_peek(lexer, i, 1) != '/'); ++i)
            ;
         ++i;
      }
      else if (ch == '"')
      {
         ++i;
         size_t capacity = 5u;
         size_t size = 0u;
         char *string = malloc(capacity);

         for (; i < source_size; ++i)
         {
            ch = lexer->source[i];

            if (size + 1 >= capacity)
            {
               capacity += capacity;
               char *new_string = realloc(string, capacity);

               if (!new_string)
               {
                  catcher_insert(lexer->catcher, err_cannot_resize_string);
                  return;
               }
               string = new_string;
            }

            if (ch == '"')
               break;
            
            if (ch == '\\')
            {
               ++i;
               char next = lexer->source[i]; 
               if (next == 'n')
                  ch = '\n';
               else if (next == 'r')
                  ch = '\r';
               else if (next == 't')
                  ch = '\t';
               else if (next == '\'')
                  ch = '\'';
               else if (next == '"')
                  ch = '"';
               else if (next == '\\')
                  ch = '\\';
               else
                  catcher_insert(lexer->catcher, err_invalid_escape_code);
            }

            string[size] = ch;
            ++size;
         }

         string[size] = '\0';
         lexer_push_back(lexer, STRING, string);
      }
      else if (ch == '\'')
      {
         ++i;
         ch = lexer->source[i];
         ++i;
         char next = lexer->source[i];

         if (ch == '\\')
         {
            if (next == 'n')
               ch = '\n';
            else if (next == 'r')
               ch = '\r';
            else if (next == 't')
               ch = '\t';
            else if (next == '\'')
               ch = '\'';
            else if (next == '"')
               ch = '"';
            else if (next == '\\')
               ch = '\\';
            else
               catcher_insert(lexer->catcher, err_invalid_escape_code);

            ++i;
            next = lexer->source[i];
         }
         
         if (next != '\'')
            catcher_insert(lexer->catcher, err_invalid_character);

         char *ptr = malloc(2u);
         ptr[0] = ch;
         ptr[1] = '\0';

         lexer_push_back(lexer, CHARACTER, ptr);
      }
      else if (isalpha(ch))
      {
         size_t capacity = 5u;
         size_t size = 0u;
         char *identifier = malloc(capacity);

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
         size_t capacity = 5u;
         size_t size = 0u;
         char *number = malloc(capacity);

         bool floating = false;
         bool last_quote = false;

         for (; i < source_size; ++i)
         {
            ch = lexer->source[i];

            if (size >= capacity)
            {
               capacity += capacity;
               char *new_number = realloc(number, capacity);

               if (!new_number)
               {
                  catcher_insert(lexer->catcher, err_cannot_resize_number);
                  free(number);
                  return;
               }
               number = new_number;
            }

            if (ch == '\'')
            {
               if (last_quote)
                  catcher_insert(lexer->catcher, err_invalid_quotes);
               last_quote = true;
               continue;
            }
            last_quote = false;

            if (ch == '.' && !floating)
            {
               floating = true;
               number[size] = ch;
               ++size;
               continue;
            }
            else if (ch == '.')
            {
               catcher_insert(lexer->catcher, err_invalid_real);
               free(number);
               return;
            }

            if (!isdigit(ch))
            {
               --i;
               break;
            }

            number[size] = ch;
            ++size;
         }

         if (last_quote)
            catcher_insert(lexer->catcher, err_invalid_quotes);

         number[size] = '\0';
         lexer_push_back(lexer, (floating ? REAL : INTEGER), number);
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

char lexer_peek(Lexer *lexer, size_t index, size_t by)
{
   if (index + by >= strlen(lexer->source))
      return 0;
   return lexer->source[index + by];
}
