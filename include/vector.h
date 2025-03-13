#ifndef VECTOR_H
#define VECTOR_H

#include "boolean.h"
#include <stddef.h>

typedef struct
{
   void* data;
   size_t element_size;
   size_t size;
   size_t capacity;
} Vector;

void create_vector(Vector *vector, size_t capacity, size_t element_size);
void free_vector(Vector *vector);
void free_mallocated_vector(Vector *vector);

void* vector_at(Vector *vector, size_t index);
void  vector_push_back(Vector *vector, void* element);
bool  vector_empty(Vector *vector);
void  vector_clear(Vector *vector);

#endif // VECTOR_H
