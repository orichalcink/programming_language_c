#include "lexer/lexer.h"
#include "errors/errors.h"
#include "lexer/keywords.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Set keyword_set;

void create_lexer(Lexer *lexer, char *source, Catcher *catcher)
{
   lexer->catcher = catcher;
   lexer->source = source;
   create_vector(&lexer->tokens, 25u, sizeof(Token));
}

void free_tokens_vector(Vector* vector)
{
   Set freed;
   create_set(&freed, vector->capacity);

   char addr_str[32u];

   for (size_t i = 0u; i < vector->size; ++i)
   {
      Token* token = (Token*)vector_at(vector, i);

      if (token->lexeme != NULL && is_token_mallocated(token->type))
      {
         snprintf(addr_str, sizeof(addr_str), "%p", (void*)token->lexeme);

         if (!set_contains(&freed, addr_str))
         {
            free(token->lexeme);
            token->lexeme = NULL;
            set_insert(&freed, addr_str);
         }
      }
   }
   free_set(&freed);
   free_vector(vector);
}

void lexer_tokenize(Lexer *lexer)
{
   size_t source_size = strlen(lexer->source);

   for (size_t i = 0; i < source_size; ++i)
   {
      char ch = lexer->source[i];

      if (ch == '\n')
         lexer_push_back(lexer, NEWLINE, "\n");
      else if (isspace(ch))
         continue;
      else if (ch == ';')
      {
         if (i + 1 < source_size && lexer->source[i + 1] == ';')
         {
            lexer_push_back(lexer, NEWLINE, "\n");
            ++i;
         }
         else lexer_push_back(lexer, SEMICOLON, ";");
      }
      else if (ch == '=')
      {
         if (i + 1 < source_size && lexer->source[i + 1] == '=')
         {
            lexer_push_back(lexer, EQUALS_EQUALS, "==");
            ++i;
         }
         else lexer_push_back(lexer, EQUALS, "=");
      }
      else if (ch == '<')
      {
         if (i + 1 < source_size && lexer->source[i + 1] == '=')
         {
            lexer_push_back(lexer, SMALLER_EQUAL, "<=");
            ++i;
         }
         else lexer_push_back(lexer, SMALLER, "<");
      }
      else if (ch == '>')
      {
         if (i + 1 < source_size && lexer->source[i + 1] == '=')
         {
            lexer_push_back(lexer, BIGGER_EQUAL, ">=");
            ++i;
         }
         else lexer_push_back(lexer, BIGGER, ">");
      }
      else if (ch == '!')
      {
         if (i + 1 < source_size && lexer->source[i + 1] == '=')
         {
            lexer_push_back(lexer, BANG_EQUALS, "!=");
            ++i;
         }
         else lexer_push_back(lexer, BANG, "!");
      }
      else if (ch == '&' && i + 1 < source_size && lexer->source[i + 1] == '&')
      {
         lexer_push_back(lexer, AND, "&&");
         ++i;
      }
      else if (ch == '|' && i + 1 < source_size && lexer->source[i + 1] == '|')
      {
         lexer_push_back(lexer, OR, "||");
         ++i;
      }
      else if (ch == '(')
         lexer_push_back(lexer, L_PAREN, "(");
      else if (ch == ')')
         lexer_push_back(lexer, R_PAREN, ")");
      else if (ch == ',')
         lexer_push_back(lexer, COMMA, ",");
      else if (ch == '/' && lexer_peek(lexer, i, 1) == '/')
         for (; i < source_size && lexer->source[i] != '\n'; ++i)
            ;
      else if (ch == '/' && lexer_peek(lexer, i, 1) == '*')
      {
         for (; i < source_size && (lexer->source[i] != '*' || lexer_peek(lexer, i, 1) != '/'); ++i)
            ;
         ++i;

         if (i >= source_size)
            catcher_insert(lexer->catcher, err_unterminated_comment);
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

         if (i >= source_size)
         {
            free(string);
            catcher_insert(lexer->catcher, err_unterminated_string);
            return;
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
      else if (isalpha(ch) || ch == '#')
      {
         size_t capacity = 5u;
         size_t size = 0u;
         char *identifier = malloc(capacity);

         bool macro = (ch == '#');
         if (macro) ++i;

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

         bool key = set_contains(&keyword_set, identifier);
         lexer_push_back(lexer, (key ? (macro ? MACRO : KEYWORD) : IDENTIFIER), identifier);
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

bool is_token_mallocated(TType type)
{
   return type == MACRO || type == IDENTIFIER || type == KEYWORD || type == STRING || type == CHARACTER || type == REAL || type == INTEGER;
}
