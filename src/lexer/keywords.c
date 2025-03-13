#include "lexer/keywords.h"

void initialize_keyword_set()
{
   create_set(&keyword_set, 2u);
   set_insert(&keyword_set, "let");
   set_insert(&keyword_set, "mut");
}

void free_keyword_set()
{
   free_set(&keyword_set);
}
