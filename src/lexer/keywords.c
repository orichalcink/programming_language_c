#include "lexer/keywords.h"

void initialize_keyword_set()
{
   create_set(&keyword_set, 21u);

   // Keywords
   set_insert(&keyword_set, "let");
   set_insert(&keyword_set, "mut");

   // Macros
   set_insert(&keyword_set, "def");
   set_insert(&keyword_set, "defl");
   set_insert(&keyword_set, "define");
   set_insert(&keyword_set, "define_line");
   set_insert(&keyword_set, "undef");
   set_insert(&keyword_set, "undefine");
   set_insert(&keyword_set, "import");
   set_insert(&keyword_set, "include");
   set_insert(&keyword_set, "if");
   set_insert(&keyword_set, "ifn");
   set_insert(&keyword_set, "ifdef");
   set_insert(&keyword_set, "ifndef");
   set_insert(&keyword_set, "elif");
   set_insert(&keyword_set, "elifn");
   set_insert(&keyword_set, "elifdef");
   set_insert(&keyword_set, "elifndef");
   set_insert(&keyword_set, "else");
   set_insert(&keyword_set, "endif");
   set_insert(&keyword_set, "then");
}

void free_keyword_set()
{
   free_set(&keyword_set);
}
