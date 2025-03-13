#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_vector(Vector *vector, size_t capacity, size_t element_size)
{
   vector->data = malloc(element_size * capacity);
   vector->capacity = capacity;
   vector->element_size = element_size;
   vector->size = 0;
}

void free_vector(Vector *vector)
{
   if (vector->data)
   {
      free(vector->data);
      vector->data = NULL;
   }
   vector->capacity = vector->element_size = vector->size = 0u;
}

void free_mallocated_vector(Vector *vector)
{
   if (vector->data)
   {
      for (size_t i = 0; i < vector->size; ++i)
      {
         void* ptr = (char*)vector->data + i * vector->element_size;
         if (ptr && *(void**)ptr)
            free(*(void**)ptr);
      }

      free(vector->data);
      vector->data = NULL;
   }
   vector->capacity = vector->element_size = vector->size = 0u;
}

void* vector_at(Vector *vector, size_t index)
{
   if (index >= vector->size)
      return NULL;
   return (char*)vector->data + index * vector->element_size;
}

void vector_push_back(Vector *vector, void *element)
{
   if (vector->size >= vector->capacity)
   {
      vector->capacity += (vector->capacity ? vector->capacity : 1);
      void* new_data = realloc(vector->data, vector->capacity * vector->element_size);

      if (!new_data)
      {
         printf("Vector memory reallocation failed.\n");
         exit(1);
      }
      vector->data = new_data;
   }
   memcpy((char*)vector->data + vector->size * vector->element_size, element, vector->element_size);
   ++vector->size;
}

bool vector_empty(Vector *vector)
{
   return vector->size == 0u;
}

void vector_clear(Vector *vector)
{
   vector->size = 0;
}
