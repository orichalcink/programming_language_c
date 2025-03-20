#include "preprocessor/preprocessor.h"
#include "errors/errors.h"
#include "io/io.h"
#include "lexer/lexer.h"
#include "types/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_preprocessor(Preprocessor* preprocessor, Vector* tokens, Catcher* catcher, const char* file)
{
   create_list(&preprocessor->replaceable, 2u, sizeof(Vector*));
   create_set(&preprocessor->included_files, 2u);

   if (file != NULL)
      set_insert(&preprocessor->included_files, file);

   preprocessor->tokens = tokens;
   preprocessor->catcher = catcher;
   preprocessor->mallocated = false;
   preprocessor->index = 0u;
   preprocessor->total_size = preprocessor->tokens->size;
}

void free_preprocessor(Preprocessor* preprocessor)
{
   for (size_t i = 0u; i < preprocessor->replaceable.size; ++i)
   {
      Vector* ptr = *(Vector**)((char*)preprocessor->replaceable.values + i * preprocessor->replaceable.value_size);
      free_tokens_vector(ptr);
      free(ptr);
   }

   free_list(&preprocessor->replaceable);
   free_set(&preprocessor->included_files);

   if (preprocessor->tokens)
   {
      bool free_ptr = (preprocessor->tokens->data != NULL);
      free_tokens_vector(preprocessor->tokens);

      if (free_ptr)
         free(preprocessor->tokens);
   }
   preprocessor->total_size = preprocessor->index = 0u;
}

void preprocessor_process(Preprocessor* preprocessor)
{
   for (; preprocessor->index < preprocessor->total_size; ++preprocessor->index)
   {
      Token* token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);

      if (token->type == MACRO && (!strcmp(token->lexeme, "import") || !strcmp(token->lexeme, "include")))
         preprocessor_handle_import(preprocessor);
      else if (token->type == MACRO && (!strcmp(token->lexeme, "define") || !strcmp(token->lexeme, "def") || !strcmp(token->lexeme, "define_line") || !strcmp(token->lexeme, "defl")))
         preprocessor_handle_define(preprocessor);
      else if (token->type == IDENTIFIER && list_contains(&preprocessor->replaceable, token->lexeme))
         preprocessor_handle_using_define(preprocessor);
      else if (token->type == MACRO && (!strcmp(token->lexeme, "undef") || !strcmp(token->lexeme, "undefine")))
         preprocessor_handle_undefine(preprocessor);
      else if (is_macro_conditional(token))
         preprocessor_handle_conditionals(preprocessor);

      if (!catcher_empty(preprocessor->catcher))
         return;
   }

   Vector* new_vector = malloc(sizeof(Vector));
   create_vector(new_vector, preprocessor->tokens->capacity, preprocessor->tokens->element_size);

   for (size_t i = 0u; i < preprocessor->tokens->size; ++i)
   {
      Token* token = (Token*)vector_at(preprocessor->tokens, i);

      if (token->type != SKIP && token->type != NEWLINE)
         vector_push_back(new_vector, token);
   }

   free_vector(preprocessor->tokens);
   if (preprocessor->mallocated)
      free(preprocessor->tokens);
   preprocessor->tokens = new_vector;
}

void preprocessor_handle_define(Preprocessor* preprocessor)
{
   Token* token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   bool define_line = (!strcmp(token->lexeme, "define_line") || !strcmp(token->lexeme, "defl"));

   token->type = SKIP;
   free(token->lexeme);
   ++preprocessor->index;

   Token* name_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);

   if (name_token->type != IDENTIFIER)
   {
      catcher_insert(preprocessor->catcher, err_expected_ident_using);
      return;
   }
   name_token->type = SKIP;

   if (list_contains(&preprocessor->replaceable, name_token->lexeme))
   {
      catcher_insert(preprocessor->catcher, err_using_exists);
      free(name_token->lexeme);
      return;
   }
   ++preprocessor->index;

   bool parameter_list = (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type == L_PAREN);

   token = (Token*)vector_at(preprocessor->tokens, preprocessor->index + 1);
   if (parameter_list && (token->type == END_OF_FILE || token->type == R_PAREN))
   {
      catcher_insert(preprocessor->catcher, err_invalid_macro_params);
      free(name_token->lexeme);
      return;
   }

   if (parameter_list)
   {
      ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type = SKIP;
      ++preprocessor->index;

      Vector* values = malloc(sizeof(Vector));
      create_vector(values, 4u, sizeof(Token));

      Token separator = {.type = NEWLINE, .lexeme = "\n"};
      vector_push_back(values, &separator);

      size_t arg_count = 0u;

      while (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type == IDENTIFIER)
      {
         Token* value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         Token new_token = {.type = value_token->type, .lexeme = strdup(value_token->lexeme)};
         vector_push_back(values, &new_token);

         free(value_token->lexeme);
         value_token->type = SKIP;
         ++preprocessor->index;
         ++arg_count;

         value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         if (value_token->type != COMMA && value_token->type != R_PAREN)
         {
            catcher_insert(preprocessor->catcher, err_expected_comma_or_r_paren);
            free(name_token->lexeme);
            return;
         }
         value_token->type = SKIP;
         ++preprocessor->index;
      }
      vector_push_back(values, &separator);

      if (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != EQUALS)
      {
         catcher_insert(preprocessor->catcher, err_expected_equals_using);
         free_vector(values);
         free(values);
         free(name_token->lexeme);
         return;
      }
      ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type = SKIP;
      ++preprocessor->index;

      TType end = (define_line ? NEWLINE : SEMICOLON);
      while (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != end && ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != END_OF_FILE)
      {
         Token* value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         vector_push_back(values, value_token);
         value_token->type = SKIP;
         ++preprocessor->index;
      }

      if (values->size - 2u - arg_count == 0u)
      {
         catcher_insert(preprocessor->catcher, err_invalid_macro_body);
         free(name_token->lexeme);
         free_vector(values);
         free(values);
         return;
      }
      list_insert(&preprocessor->replaceable, name_token->lexeme, &values);
   }
   else
   {
      if (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type == SEMICOLON)
      {
         ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type = SKIP;
         Vector* values = malloc(sizeof(Vector));
         create_vector(values, 0u, sizeof(Token));

         Token fake = {.type = SKIP, .lexeme = ""};
         vector_push_back(values, &fake);

         list_insert(&preprocessor->replaceable, name_token->lexeme, &values);
         free(name_token->lexeme);
         return;
      }

      if (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != EQUALS)
      {
         catcher_insert(preprocessor->catcher, err_expected_equals_using);
         free(name_token->lexeme);
         return;
      }
      ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type = SKIP;
      ++preprocessor->index;

      Vector* values = malloc(sizeof(Vector));
      create_vector(values, 2u, sizeof(Token));

      TType end = (define_line ? NEWLINE : SEMICOLON);
      while (((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != end && ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != END_OF_FILE)
      {
         Token* value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         vector_push_back(values, value_token);
         value_token->type = SKIP;
         ++preprocessor->index;
      }

      if (vector_empty(values))
      {
         catcher_insert(preprocessor->catcher, err_invalid_macro_body);
         free(name_token->lexeme);
         free_vector(values);
         free(values);
         return;
      }
      list_insert(&preprocessor->replaceable, name_token->lexeme, &values);
   }

   if (!define_line && ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type != SEMICOLON)
   {
      catcher_insert(preprocessor->catcher, err_statement_semicolon);
      free(name_token->lexeme);
      return;
   }

   ((Token*)vector_at(preprocessor->tokens, preprocessor->index))->type = SKIP;
   free(name_token->lexeme);
}

void preprocessor_handle_using_define(Preprocessor* preprocessor)
{
   Token* token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   token->type = SKIP;
   Vector* macro_definition = *(Vector**)list_at(&preprocessor->replaceable, token->lexeme);
   free(token->lexeme);

   ++preprocessor->index;
   token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   bool arg_list = (token->type == L_PAREN);

   token = (Token*)vector_at(preprocessor->tokens, preprocessor->index + 1);
   if (arg_list && (token->type == END_OF_FILE || token->type == R_PAREN))
   {
      catcher_insert(preprocessor->catcher, err_invalid_macro_call);
      return;
   }

   if (arg_list)
   {
      Vector copied_definition;
      create_vector(&copied_definition, macro_definition->size, sizeof(Token));

      for (size_t i = 0u; i < macro_definition->size; ++i)
      {
         Token* original = (Token*)vector_at(macro_definition, i);
         Token copy = {.type = original->type, .lexeme = strdup(original->lexeme)};
         vector_push_back(&copied_definition, &copy);
      }

      token = vector_at(preprocessor->tokens, preprocessor->index);
      token->type = SKIP;
      ++preprocessor->index;
      Vector params;
      create_vector(&params, 2u, sizeof(Token));

      while (preprocessor->index < preprocessor->total_size)
      {
         Token* value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         vector_push_back(&params, value_token);
         value_token->type = SKIP;
         ++preprocessor->index;

         value_token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
         if (value_token->type != COMMA && value_token->type != R_PAREN)
         {
            catcher_insert(preprocessor->catcher, err_expected_comma_or_r_paren);
            free_tokens_vector(&params);
            free_vector(&copied_definition);
            return;
         }

         if (value_token->type == R_PAREN)
         {
            value_token->type = SKIP;
            break;
         }

         value_token->type = SKIP;
         ++preprocessor->index;
      }

      List translations;
      create_list(&translations, 2u, sizeof(size_t));

      size_t param_count = 0u;
      for (size_t i = 1u; i < copied_definition.size; ++i)
      {
         Token* token = (Token*)vector_at(&copied_definition, i);
         if (token->type == NEWLINE)
            break;

         ++param_count;
         size_t real_index = i - 1u;
         list_insert(&translations, token->lexeme, &real_index);
         free(token->lexeme);
         token->type = SKIP;
      }

      if (param_count != params.size || (copied_definition.size > 0u && ((Token*)vector_at(&copied_definition, 0u))->type != NEWLINE))
      {
         catcher_insert(preprocessor->catcher, err_invalid_arg_count);
         free_tokens_vector(&params);
         free_list(&translations);
         free_vector(&copied_definition);
         return;
      }

      for (size_t i = param_count + 2u; i < copied_definition.size; ++i)
      {
         Token* token = (Token*)vector_at(&copied_definition, i);

         if (token->type == IDENTIFIER && list_contains(&translations, token->lexeme))
         {
            Token* replacement = (Token*)vector_at(&params, *(size_t*)list_at(&translations, token->lexeme));
            token->type = replacement->type;
            free(token->lexeme);
            token->lexeme = strdup(replacement->lexeme);
         }
      }

      free_tokens_vector(&params);
      free_list(&translations);

      Vector* new_tokens = vector_insert(preprocessor->tokens, &copied_definition, preprocessor->index + 1);

      if (!new_tokens)
      {
         catcher_insert(preprocessor->catcher, err_cannot_handle_using);
         return;
      }
      preprocessor->total_size = new_tokens->size;
      free_vector(preprocessor->tokens);
      free_vector(&copied_definition);

      if (preprocessor->mallocated)
         free(preprocessor->tokens);

      preprocessor->tokens = new_tokens;
      preprocessor->tokens->size = new_tokens->size;
      preprocessor->mallocated = true;

      for (size_t i = 0u; i < preprocessor->tokens->size; ++i)
      {
         Token* token = (Token*)vector_at(preprocessor->tokens, i);

         if (token->type != IDENTIFIER || !list_contains(&preprocessor->replaceable, token->lexeme))
            continue;

         size_t original = preprocessor->index;
         preprocessor->index = i;
         preprocessor_handle_using_define(preprocessor);
         preprocessor->index = original + 1;

         bool separator = false;
         while (preprocessor->index < preprocessor->total_size)
         {
            Token* token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);

            if (token->type == NEWLINE && separator)
            {
               ++preprocessor->index;
               break;
            }
            
            if (token->type == NEWLINE)
               separator = true;
            
            ++preprocessor->index;
            if (is_token_mallocated(token->type))
               free(token->lexeme);
            token->type = SKIP;
         }
      }
   }
   else
   {
      if (((Token*)vector_at(macro_definition, 0u))->type == NEWLINE)
      {
         catcher_insert(preprocessor->catcher, err_invalid_arg_count);
         return;
      }

      Vector* new_tokens = vector_insert(preprocessor->tokens, macro_definition, preprocessor->index);
      --preprocessor->index;

      if (!new_tokens)
      {
         catcher_insert(preprocessor->catcher, err_cannot_handle_using);
         return;
      }

      preprocessor->total_size = new_tokens->size;
      free_vector(preprocessor->tokens);

      if (preprocessor->mallocated)
         free(preprocessor->tokens);

      preprocessor->tokens = new_tokens;
      preprocessor->mallocated = true;
   }
}

void preprocessor_handle_undefine(Preprocessor* preprocessor)
{
   Token* token = vector_at(preprocessor->tokens, preprocessor->index);
   token->type = SKIP;
   free(token->lexeme);
   ++preprocessor->index;

   token = vector_at(preprocessor->tokens, preprocessor->index);

   if (token->type != IDENTIFIER)
   {
      catcher_insert(preprocessor->catcher, err_invalid_undefine_macro);
      return;
   }

   if (list_contains(&preprocessor->replaceable, token->lexeme))
   {
      Vector* ptr = *(Vector**)list_at(&preprocessor->replaceable, token->lexeme);
      free_tokens_vector(ptr);
      free(ptr);
      list_remove(&preprocessor->replaceable, token->lexeme);
   }
   token->type = SKIP;
   free(token->lexeme);
   ++preprocessor->index;

   token = vector_at(preprocessor->tokens, preprocessor->index);

   if (token->type != SEMICOLON)
   {
      catcher_insert(preprocessor->catcher, err_statement_semicolon);
      return;
   }
   token->type = SKIP;
}

void preprocessor_handle_import(Preprocessor* preprocessor)
{
   Token* token = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   bool include_guard = (!strcmp("import", token->lexeme));

   token->type = SKIP;
   free(token->lexeme);
   ++preprocessor->index;

   Token* next = (Token*)vector_at(preprocessor->tokens, preprocessor->index);

   if (next->type == IDENTIFIER && list_contains(&preprocessor->replaceable, next->lexeme))
   {
      preprocessor_handle_using_define(preprocessor);
      ++preprocessor->index;
      next = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   }

   if (next->type != STRING)
   {
      catcher_insert(preprocessor->catcher, err_expected_file);
      return;
   }
   next->type = SKIP;
   char* file = next->lexeme;

   Vector files;
   create_vector(&files, 2u, sizeof(char**));

   vector_push_back(&files, &file);
   ++preprocessor->index;
   next = (Token*)vector_at(preprocessor->tokens, preprocessor->index);

   while (next->type == COMMA)
   {
      next->type = SKIP;
      ++preprocessor->index;

      next = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
      if (next->type != STRING)
      {
         catcher_insert(preprocessor->catcher, err_expected_file);
         for (size_t i = 0u; i < files.size; ++i)
            free(*(char**)vector_at(&files, i));
         free_vector(&files);
         return;
      }
      next->type = SKIP;
      file = next->lexeme;
      vector_push_back(&files, &file);
      ++preprocessor->index;
   }

   next = (Token*)vector_at(preprocessor->tokens, preprocessor->index);
   if (next->type != SEMICOLON)
   {
      catcher_insert(preprocessor->catcher, err_statement_semicolon);
      for (size_t i = 0u; i < files.size; ++i)
         free(*(char**)vector_at(&files, i));
      free_vector(&files);
      return;
   }
   next->type = SKIP;

   for (size_t i = 0u; i < files.size; ++i)
      preprocessor_handle_file(preprocessor, *(char**)vector_at(&files, i), include_guard);

   free_vector(&files);
}

void preprocessor_handle_file(Preprocessor* preprocessor, char* file, bool include_guard)
{
   if (!is_file(file))
   {
      catcher_insert(preprocessor->catcher, err_import_invalid_file);
      free(file);
      return;
   }

   if (include_guard)
   {
      if (set_contains(&preprocessor->included_files, file))
      {
         free(file);
         return;
      }
      set_insert(&preprocessor->included_files, file);
   }

   char* code = read_file(preprocessor->catcher, file);

   if (!catcher_empty(preprocessor->catcher))
      return;

   Lexer lexer;
   create_lexer(&lexer, code, preprocessor->catcher);
   lexer_tokenize(&lexer);

   if (!catcher_empty(preprocessor->catcher))
   {
      free_vector(&lexer.tokens);
      free(code);
      return;
   }

   ((Token*)vector_at(&lexer.tokens, lexer.tokens.size - 1))->type = SKIP;
   Vector* new_tokens = vector_insert(preprocessor->tokens, &lexer.tokens, preprocessor->index + 1);

   if (!new_tokens)
   {
      free_vector(&lexer.tokens);
      free(code);
      catcher_insert(preprocessor->catcher, err_cannot_import);
      return;
   }

   preprocessor->total_size = new_tokens->size;
   free_vector(preprocessor->tokens);

   if (preprocessor->mallocated)
      free(preprocessor->tokens);

   preprocessor->tokens = new_tokens;
   free_vector(&lexer.tokens);
   free(code);
   preprocessor->mallocated = true;
}

void preprocessor_handle_conditionals(Preprocessor* preprocessor)
{
   Token* token = vector_at(preprocessor->tokens, preprocessor->index);
   bool if_defined = (!strcmp(token->lexeme, "ifdef") || !strcmp(token->lexeme, "ifndef") || !strcmp(token->lexeme, "elifdef") || !strcmp(token->lexeme, "elifndef"));
   bool negative = (!strcmp(token->lexeme, "ifn") || !strcmp(token->lexeme, "ifndef") || !strcmp(token->lexeme, "elifn") || !strcmp(token->lexeme, "elifndef"));
   
   token->type = SKIP;
   free(token->lexeme);
   ++preprocessor->index;

   bool result = preprocessor_handle_boolean_expressions(preprocessor, if_defined, negative);
   CType current_type = (result ? CT_TRUE : CT_FALSE);
   Token* current = vector_at(preprocessor->tokens, preprocessor->index);

   while (current->type != END_OF_FILE && (current->type != MACRO || strcmp(current->lexeme, "endif")))
   {
      if (is_macro_conditional_else_if(current))
      {
         if (current_type == CT_FALSE)
         {
            preprocessor_handle_conditionals(preprocessor);
            return;
         }
         else
            current_type = CT_EVALUATED;
      }

      if (!strcmp(current->lexeme, "else"))
      {
         current_type = (current_type == CT_FALSE ? CT_TRUE : CT_EVALUATED);
         if (current_type == CT_TRUE)
         {
            free(current->lexeme);
            current->type = SKIP;
         }
      }

      if (current_type == CT_TRUE)
      {
         if (current->type == MACRO && (!strcmp(current->lexeme, "import") || !strcmp(current->lexeme, "include")))
            preprocessor_handle_import(preprocessor);
         else if (current->type == MACRO && (!strcmp(current->lexeme, "define") || !strcmp(current->lexeme, "def") || !strcmp(current->lexeme, "define_line") || !strcmp(current->lexeme, "defl")))
            preprocessor_handle_define(preprocessor);
         else if (current->type == IDENTIFIER && list_contains(&preprocessor->replaceable, current->lexeme))
            preprocessor_handle_using_define(preprocessor);
         else if (token->type == MACRO && (!strcmp(token->lexeme, "undef") || !strcmp(token->lexeme, "undefine")))
            preprocessor_handle_undefine(preprocessor);
         else if (is_macro_conditional(current))
            preprocessor_handle_conditionals(preprocessor);
         ++preprocessor->index;
      }
      else
      {
         if (is_token_mallocated(current->type))
            free(current->lexeme);
         current->type = SKIP;
         ++preprocessor->index;
      }
      current = vector_at(preprocessor->tokens, preprocessor->index);
   }

   if (current->type == END_OF_FILE)
   {
      catcher_insert(preprocessor->catcher, err_mcond_did_not_end_with_endif);
      return;
   }
   current->type = SKIP;
   free(current->lexeme);
}

bool preprocessor_handle_boolean_expressions(Preprocessor* preprocessor, bool if_defined, bool negative)
{
   Token* next = vector_at(preprocessor->tokens, preprocessor->index);
   Stack operator_stack;
   Stack output_stack;

   create_stack(&operator_stack, 3u, sizeof(Token*));
   create_stack(&output_stack, 3u, sizeof(Token*));

   while ((next->type != MACRO || strcmp(next->lexeme, "then")) && next->type != END_OF_FILE)
   {
      if (get_precedence(next->type))
      {
         while (!stack_empty(&operator_stack))
         {
            Token* top = *(Token**)stack_top(&operator_stack);

            if (!has_higher_precedence(next->type, top->type))
            {
               stack_pop(&operator_stack);
               stack_push(&output_stack, &top);
            }
            else break;
         }
         stack_push(&operator_stack, &next);
      }
      else if (next->type == L_PAREN)
         stack_push(&operator_stack, &next);
      else if (next->type == R_PAREN)
      {
         while (operator_stack.size != 0)
         {
            Token* element = *(Token**)stack_top(&operator_stack);
            
            if (element->type == L_PAREN)
            {
               stack_pop(&operator_stack);
               break;
            }
            stack_push(&output_stack, &element);
            stack_pop(&operator_stack);
         }
      }
      else stack_push(&output_stack, &next);

      ++preprocessor->index;
      next = vector_at(preprocessor->tokens, preprocessor->index);
   }

   if (next->type == END_OF_FILE)
   {
      catcher_insert(preprocessor->catcher, err_invalid_macro_conditional);
      free_stack(&output_stack);
      free_stack(&operator_stack);
      return false;
   }
   next->type = SKIP;
   free(next->lexeme);
   ++preprocessor->index;

   Stack reversed;
   create_stack(&reversed, 3u, sizeof(Token*));

   while (!stack_empty(&operator_stack))
   {
      Token* element = *(Token**)stack_top(&operator_stack);
      if (element->type == L_PAREN)
      {
         catcher_insert(preprocessor->catcher, err_mismatched_parentheses);
         free_stack(&output_stack);
         free_stack(&operator_stack);
         free_stack(&reversed);
         return false;
      }
      stack_pop(&operator_stack);
      stack_push(&output_stack, &element);
   }

   while (!stack_empty(&output_stack))
   {
      Token* element = *(Token**)stack_top(&output_stack);
      stack_pop(&output_stack);
      stack_push(&reversed, &element);
   }

   Stack eval_stack;
   create_stack(&eval_stack, 3u, sizeof(long double));

   while (!stack_empty(&reversed))
   {
      Token* element = *(Token**)stack_top(&reversed);
      stack_pop(&reversed);

      if (element->type == IDENTIFIER)
      {
         if (if_defined)
         {
            long double defined = (long double)list_contains(&preprocessor->replaceable, element->lexeme);
            stack_push(&eval_stack, &defined);
         }
         else
         {
            if (!list_contains(&preprocessor->replaceable, element->lexeme))
            {
               catcher_insert(preprocessor->catcher, err_invalid_macro_in_mcond);
               free_stack(&eval_stack);
               free_stack(&operator_stack);
               free_stack(&output_stack);
               free_stack(&reversed);
               return false;
            }

            Vector* macro_body = *(Vector**)list_at(&preprocessor->replaceable, element->lexeme);

            if (macro_body->size != 1)
            {
               catcher_insert(preprocessor->catcher, err_invalid_macro_in_mcond);
               free_stack(&eval_stack);
               free_stack(&operator_stack);
               free_stack(&output_stack);
               free_stack(&reversed);
               return false;
            }

            Token* token = vector_at(macro_body, 0u);
            stack_push(&reversed, &token);
         }
      }
      else if (element->type == REAL || element->type == INTEGER)
      {
         char* endptr;
         long double a = strtold(element->lexeme, &endptr);

         if (*endptr != '\0')
         {
            catcher_insert(preprocessor->catcher, err_cannot_convert_to_ld);
            free_stack(&eval_stack);
            free_stack(&operator_stack);
            free_stack(&output_stack);
            free_stack(&reversed);
            return false;
         }
         stack_push(&eval_stack, &a);
      }
      else if (element->type == BANG)
      {
         if (eval_stack.size > 1u)
         {
            catcher_insert(preprocessor->catcher, err_invalid_bool_expr_mcond);
            free_stack(&eval_stack);
            free_stack(&operator_stack);
            free_stack(&output_stack);
            free_stack(&reversed);
            return false;
         }

         long double a = *(long double*)stack_top(&eval_stack);
         long double result = (a == 0.0 ? 1.0 : 0.0);
         stack_pop(&eval_stack);
         stack_push(&eval_stack, &result);
      }
      else
      {
         if (eval_stack.size > 2u)
         {
            catcher_insert(preprocessor->catcher, err_invalid_bool_expr_mcond);
            free_stack(&eval_stack);
            free_stack(&operator_stack);
            free_stack(&output_stack);
            free_stack(&reversed);
            return false;
         }

         long double b = *(long double*)stack_top(&eval_stack);
         stack_pop(&eval_stack);
         long double a = *(long double*)stack_top(&eval_stack);
         stack_pop(&eval_stack);

         long double result;
         switch (element->type)
         {
         case AND:
            result = (long double)(a && b);
            break;
         case OR:
            result = (long double)(a || b);
            break;
         case EQUALS_EQUALS:
            result = (long double)(a == b);
            break;
         case BANG_EQUALS:
            result = (long double)(a != b);
            break;
         case SMALLER:
            result = (long double)(a < b);
            break;
         case SMALLER_EQUAL:
            result = (long double)(a <= b);
            break;
         case BIGGER:
            result = (long double)(a > b);
            break;
         case BIGGER_EQUAL:
            result = (long double)(a >= b);
            break;
         default:
            result = false;
            catcher_insert(preprocessor->catcher, err_unexpected_token_mcond);
         }
         stack_push(&eval_stack, &result);
      }

      if (is_token_mallocated(element->type))
         free(element->lexeme);
      element->type = SKIP;
   }

   bool result = *(long double*)stack_top(&eval_stack);
   free_stack(&eval_stack);
   free_stack(&operator_stack);
   free_stack(&output_stack);
   free_stack(&reversed);
   return (negative ? !result : result);
}

bool is_macro_conditional(Token* token)
{
   return token->type == MACRO && (!strcmp(token->lexeme, "if") || !strcmp(token->lexeme, "ifn") || !strcmp(token->lexeme, "ifdef") || !strcmp(token->lexeme, "ifndef"));
}

bool is_macro_conditional_else_if(Token* token)
{
   return token->type == MACRO && (!strcmp(token->lexeme, "elif") || !strcmp(token->lexeme, "elifn") || !strcmp(token->lexeme, "elifdef") || !strcmp(token->lexeme, "elifndef"));
}

int get_precedence(TType type)
{
   switch (type)
   {
   case BANG:
      return 5;
   case SMALLER: case SMALLER_EQUAL: case BIGGER: case BIGGER_EQUAL:
      return 4;
   case EQUALS_EQUALS: case BANG_EQUALS:
      return 3;
   case AND:
      return 2;
   case OR:
      return 1;
   default:
      return 0;
   };
}

bool has_higher_precedence(TType first, TType second)
{
   return get_precedence(first) >= get_precedence(second);
}
