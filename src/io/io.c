#include "io/io.h"
#include "errors/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_input()
{
   size_t capacity   = 50;
   size_t size = 0;
   char* buffer = malloc(capacity);

   if (!buffer)
      return NULL;
   
   char ch;
   while ((ch = getchar()) != '\n' && ch != EOF)
   {
      if (size + 1 >= capacity)
      {
         capacity += capacity;
         char* new_buffer = realloc(buffer, capacity);

         if (!new_buffer)
         {
            free(buffer);
            return NULL;
         }
         buffer = new_buffer;
      }

      buffer[size] = ch;
      ++size; 
   }
   buffer[size] = '\0';
   return buffer;
}

bool is_file(const char* input)
{
   FILE* file = fopen(input, "r");
   if (!file)
      return false;
   
   fclose(file);
   return true;
}

char* read_file(Catcher* catcher, char* input)
{
   FILE* file = fopen(input, "r");
   if (!file)
   {
      catcher_insert(catcher, err_cannot_open_file);
      return NULL;
   }

   size_t size = 0u;
   size_t capacity = 100u;
   char* contents = malloc(capacity);
   char ch;

   while ((ch = fgetc(file)) != EOF)
   {
      if (size + 1 >= capacity)
      {
         capacity += capacity;
         char* new_contents = realloc(contents, capacity);

         if (!new_contents)
         {
            catcher_insert(catcher, err_cannot_resize_input);
            free(contents);
            return NULL;
         }
         contents = new_contents;
      }
      contents[size] = ch;
      ++size;
   }

   contents[size] = '\0';
   fclose(file);
   free(input);
   return contents;
}
