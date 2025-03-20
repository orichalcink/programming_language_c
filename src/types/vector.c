#include "types/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_vector(Vector* vector, size_t capacity, size_t element_size)
{
   vector->data = malloc(element_size * capacity);
   vector->capacity = capacity;
   vector->element_size = element_size;
   vector->size = 0u;
}

void free_vector(Vector* vector)
{
   if (vector->data)
   {
      free(vector->data);
      vector->data = NULL;
   }
   vector->capacity = vector->element_size = vector->size = 0u;
}

void* vector_at(Vector* vector, size_t index)
{
   if (index >= vector->size)
      return NULL;
   return (char*)vector->data + index * vector->element_size;
}

void vector_push_back(Vector* vector, void* element)
{
   if (vector->size >= vector->capacity)
   {
      vector->capacity += (vector->capacity ? vector->capacity : 1u);
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

bool vector_empty(Vector* vector)
{
   return vector->size == 0u;
}

void vector_clear(Vector* vector)
{
   vector->size = 0u;
}

Vector* vector_insert(Vector* original, Vector* inserted, size_t index)
{
   if (original->element_size != inserted->element_size || index > original->size)
      return NULL;
   size_t total_size = original->size + inserted->size;

   Vector* vector = malloc(sizeof(Vector));
   create_vector(vector, total_size, original->element_size);

   for (size_t i = 0u; i < index; ++i)
      vector_push_back(vector, vector_at(original, i));
   
   for (size_t i = 0u; i < inserted->size; ++i)
      vector_push_back(vector, vector_at(inserted, i));
   
   for (size_t i = 0u; i < original->size - index; ++i)
      vector_push_back(vector, vector_at(original, i + index));
   return vector;
}
