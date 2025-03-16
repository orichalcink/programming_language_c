#include "lexer/keywords.h"

void initialize_keyword_set()
{
   create_set(&keyword_set, 4u);
   set_insert(&keyword_set, "let");
   set_insert(&keyword_set, "mut");
   set_insert(&keyword_set, "define");
   set_insert(&keyword_set, "define_line");
   set_insert(&keyword_set, "import");
}

void free_keyword_set()
{
   free_set(&keyword_set);
}
