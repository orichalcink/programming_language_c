#include "types/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_list(List *list, size_t capacity, size_t element_size)
{
   list->keys = malloc(capacity * sizeof(char*));
   list->values = malloc(capacity * element_size);
   list->value_size = element_size;
   list->size = 0u;
   list->capacity = capacity;
}

void free_list(List *list)
{
   if (list->keys)
   {
      for (size_t i = 0u; i < list->size; ++i)
         free(list->keys[i]);

      free(list->keys);
      list->keys = NULL;
   }
   if (list->values)
   {
      free(list->values);
      list->values = NULL;
   }
   list->value_size = list->size = list->capacity = 0u;
}

void* list_at(List *list, const char *key)
{
   for (size_t i = 0u; i < list->size; ++i)
      if (!strcmp(list->keys[i], key))
         return (char*)list->values + i * list->value_size;
   return NULL;
}

void list_insert(List *list, const char *key, void *value)
{
   if (list->size >= list->capacity)
   {
      list->capacity += (list->capacity ? list->capacity : 1u);
      void *new_values = realloc(list->values, list->capacity * list->value_size);

      if (!new_values)
      {
         printf("List value memory reallocation failed.\n");
         exit(1);
      }
      list->values = new_values;

      char **new_keys = realloc(list->keys, list->capacity * sizeof(char*));

      if (!new_keys)
      {
         printf("List key memory reallocation failed.\n");
         exit(1);
      }
      list->keys = new_keys;
   }

   memcpy((char*)list->values + list->size * list->value_size, value, list->value_size);
   list->keys[list->size] = strdup(key);
   ++list->size;
}

bool list_contains(List *list, const char *key)
{
   for (size_t i = 0u; i < list->size; ++i)
      if (!strcmp(list->keys[i], key))
         return true;
   return false;
}

bool list_empty(List *list)
{
   return list->size == 0u;
}

void list_clear(List *list)
{
   for (size_t i = 0u; i < list->size; ++i)
      free(list->keys[i]);
   list->size = 0u;
}
