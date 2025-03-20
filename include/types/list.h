#ifndef LIST_H
#define LIST_H

#include "types/boolean.h"
#include <stddef.h>

typedef struct
{
   char** keys;
   void* values;
   size_t value_size;
   size_t size;
   size_t capacity;
} List;

void create_list(List* list, size_t capacity, size_t element_size);
void free_list(List* list);

void* list_at(List* list, const char* key);
void  list_insert(List* list, const char* key, void* value);
void  list_remove(List* list, const char* key);
bool  list_contains(List* list, const char* key);
bool  list_empty(List* list);
void  list_clear(List* list);

#endif // LIST_H
