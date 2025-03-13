#ifndef SET_H
#define SET_H

#include "types/boolean.h"
#include <stddef.h>

typedef struct
{
   char **keys;
   size_t size;
   size_t capacity;
} Set;

void create_set(Set *set, size_t capacity);
void free_set(Set *set);

void set_insert(Set *set, const char *key);
bool set_contains(Set *set, const char *key);
bool set_empty(Set *set);
void set_clear(Set *set);

#endif // SET_H
