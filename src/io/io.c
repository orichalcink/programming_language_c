#include "io/io.h"
#include <stdio.h>
#include <stdlib.h>

char* read_input()
{
   size_t size   = 50;
   size_t length = 0;
   char *buffer = malloc(size);

   if (!buffer)
      return NULL;
   
   char ch;
   while ((ch = getchar()) != '\n' && ch != EOF)
   {
      if (length + 1 >= size)
      {
         size += size;
         char *new_buffer = realloc(buffer, size);

         if (!new_buffer)
         {
            free(buffer);
            return NULL;
         }
         buffer = new_buffer;
      }

      buffer[length] = ch;
      ++length; 
   }
   buffer[length] = '\0';
   return buffer;
}
