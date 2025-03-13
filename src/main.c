#include "errors/errors.h"
#include "lexer/keywords.h"
#include "lexer/lexer.h"
#include "io/io.h"
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
      char *input = read_input();

      if (!input || !strcmp(input, ""))
      {
         catcher_insert(&catcher, err_invalid_input);
         catcher_display(&catcher);

         if (input)
            free(input);
         continue;
      }

      if (!strcmp(input, "quit") || !strcmp(input, "exit"))
      {
         printf("Quitting...\n");
         free(input);
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
         continue;
      }

      for (size_t i = 0; i < lexer.tokens.size; ++i)
      {
         Token *token = (Token*)vector_at(&lexer.tokens, i);
         printf("%d: '%s'\n", token->type, token->lexeme);
      }

      free_lexer(&lexer);
      free(input);
   }
}
