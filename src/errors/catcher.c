#include "errors/catcher.h"
#include <stdio.h>

void create_catcher(Catcher *catcher)
{
   create_vector(&catcher->errors, 2u, sizeof(char**));
}

void free_catcher(Catcher *catcher)
{
   free_vector(&catcher->errors);
}

void catcher_insert(Catcher *catcher, char *error)
{
   vector_push_back(&catcher->errors, &error);
}

bool catcher_empty(Catcher* catcher)
{
   return catcher->errors.size == 0u;
}

bool catcher_display(Catcher* catcher)
{
   if (catcher_empty(catcher))
      return false;

   printf("\n%zu error%c occurred:\n", catcher->errors.size, (catcher->errors.size == 1u ? 0 : 's'));
   for (size_t i = 0u; i < catcher->errors.size; ++i)
      printf("%s\n", *(char**)vector_at(&catcher->errors, i));

   printf("\n");
   vector_clear(&catcher->errors);
   return true;
}
