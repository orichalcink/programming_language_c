#include "types/set.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_set(Set *set, size_t capacity)
{
   set->keys = malloc(capacity * sizeof(char*));
   set->size = 0u;
   set->capacity = capacity;
}

void free_set(Set *set)
{
   if (set->keys)
   {
      for (size_t i = 0u; i < set->size; ++i)
         free(set->keys[i]);
      free(set->keys);
      set->keys = NULL;
   }
   set->size = set->capacity = 0u;
}

void set_insert(Set *set, const char *key)
{
   if (set->size >= set->capacity)
   {
      set->capacity += (set->capacity ? set->capacity : 1u);
      char **new_keys = realloc(set->keys, set->capacity * sizeof(char*));

      if (!new_keys)
      {
         printf("Set key memory reallocation failed.\n");
         exit(1);
      }
      set->keys = new_keys;
   }
   set->keys[set->size] = strdup(key);
   ++set->size;
}

bool set_contains(Set *set, const char *key)
{
   for (size_t i = 0u; i < set->size; ++i)
      if (!strcmp(set->keys[i], key))
         return true;
   return false;
}

bool set_empty(Set *set)
{
   return set->size == 0u;
}

void set_clear(Set *set)
{
   for (size_t i = 0u; i < set->size; ++i)
      free(set->keys[i]);
   set->size = 0u;
}
