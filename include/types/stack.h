#ifndef STACK_H
#define STACK_H

#include "types/boolean.h"
#include <stddef.h>

typedef struct
{
   void* data;
   size_t element_size;
   size_t size;
   size_t capacity;
} Stack;

void create_stack(Stack* stack, size_t capacity, size_t element_size);
void free_stack(Stack* stack);

void* stack_top(Stack* stack);
void  stack_push(Stack* stack, void* element);
void  stack_pop(Stack* stack);
bool  stack_empty(Stack* stack);
void  stack_clear(Stack* stack);

#endif // STACK_H
