#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "errors/catcher.h"
#include "lexer/tokens.h"
#include "types/list.h"
#include "types/set.h"

typedef enum
{
   CT_TRUE, CT_FALSE, CT_EVALUATED, CT_EVALUATING
} CType;

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
void preprocessor_handle_undefine(Preprocessor* preprocessor);
void preprocessor_handle_import(Preprocessor* preprocessor);
void preprocessor_handle_file(Preprocessor* preprocessor, char* file, bool include_guard);

void preprocessor_handle_conditionals(Preprocessor* preprocessor);
bool preprocessor_handle_boolean_expressions(Preprocessor* preprocessor, bool if_defined, bool negative);

bool is_macro_conditional(Token* token);
bool is_macro_conditional_else_if(Token* token);
int  get_precedence(TType type);
bool has_higher_precedence(TType first, TType second);

#endif // PREPROCESSOR_H
