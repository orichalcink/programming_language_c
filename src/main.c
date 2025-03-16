#include "errors/errors.h"
#include "lexer/keywords.h"
#include "lexer/lexer.h"
#include "io/io.h"
#include "preprocessor/preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
   printf("REPL for an interpreted programming language written in C.\n");
   Catcher catcher;
   create_catcher(&catcher);

   initialize_keyword_set();

   while (true)
   {
      printf("> ");
      char* input = read_input();

      if (!input || !strcmp(input, ""))
      {
         catcher_insert(&catcher, err_invalid_input);
         catcher_display(&catcher);

         if (input)
            free(input);
         continue;
      }

      char* file_name = NULL;

      if (is_file(input))
      {
         file_name = strdup(input);
         char* file = read_file(&catcher, input);

         if (catcher_display(&catcher))
         {
            free(input);
            free(file_name);
            continue;
         }
         input = strdup(file);
         free(file);
      }

      if (!strcmp(input, "quit"))
      {
         printf("Quitting...\n");
         free(input);
         if (file_name)
            free(file_name);

         free_catcher(&catcher);
         free_keyword_set();
         return 0;
      }

      Lexer lexer;
      create_lexer(&lexer, input, &catcher);
      lexer_tokenize(&lexer);

      if (catcher_display(&catcher))
      {
         free_lexer(&lexer);
         free(input);
         if (file_name)
            free(file_name);
         continue;
      }

      Preprocessor preprocessor;
      create_preprocessor(&preprocessor, &lexer.tokens, &catcher, file_name);
      preprocessor_process(&preprocessor);

      if (catcher_display(&catcher))
      {
         free_lexer(&lexer);
         free_preprocessor(&preprocessor);
         free(input);
         if (file_name)
            free(file_name);
         continue;
      }

      for (size_t i = 0; i < preprocessor.tokens->size; ++i)
      {
         Token *token = (Token*)vector_at(preprocessor.tokens, i);
         printf("PR: %d: '%s'\n", token->type, token->lexeme);
      }

      free_lexer(&lexer);
      free_preprocessor(&preprocessor);
      free(input);
      if (file_name)
         free(file_name);
   }
}
