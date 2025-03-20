#include "types/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_stack(Stack* stack, size_t capacity, size_t element_size)
{
   stack->data = malloc(element_size * capacity);
   stack->capacity = capacity;
   stack->element_size = element_size;
   stack->size = 0u;
}

void free_stack(Stack* stack)
{
   if (stack->data)
   {
      free(stack->data);
      stack->data = NULL;
   }
   stack->capacity = stack->element_size = stack->size = 0u;
}

void* stack_top(Stack* stack)
{
   if (stack->size == 0u)
      return NULL;
   return (char*)stack->data + (stack->size - 1u) * stack->element_size;
}

void stack_push(Stack* stack, void* element)
{
   if (stack->size >= stack->capacity)
   {
      stack->capacity += (stack->capacity ? stack->capacity : 1u);
      void* new_data = realloc(stack->data, stack->capacity * stack->element_size);

      if (!new_data)
      {
         printf("Stack memory reallocation failed.\n");
         exit(1);
      }
      stack->data = new_data;
   }
   memcpy((char*)stack->data + stack->size * stack->element_size, element, stack->element_size);
   ++stack->size;
}

void stack_pop(Stack* stack)
{
   if (stack->size != 0u)
      --stack->size;
}

bool stack_empty(Stack* stack)
{
   return stack->size == 0u;
}

void stack_clear(Stack* stack)
{
   stack->size = 0u;
}
