#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "errors/catcher.h"
#include "types/list.h"
#include "types/set.h"

typedef struct
{
   Vector* tokens;
   Catcher* catcher;
   List replaceable;
   Set included_files;
   bool mallocated;
   size_t total_size;
   size_t index;
} Preprocessor;

void create_preprocessor(Preprocessor* preprocessor, Vector* tokens, Catcher* catcher, const char* file);
void free_preprocessor(Preprocessor* preprocessor);

void preprocessor_process(Preprocessor* preprocessor);
void preprocessor_handle_using_define(Preprocessor* preprocessor);
void preprocessor_handle_define(Preprocessor* preprocessor);
void preprocessor_handle_import(Preprocessor* preprocessor);
void preprocessor_handle_file(Preprocessor* preprocessor, char* file);

#endif // PREPROCESSOR_H
