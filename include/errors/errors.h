#ifndef ERRORS_H
#define ERRORS_H

static char* err_invalid_input = "Invalid input.";
static char* err_cannot_open_file = "Could not open the given file.";
static char* err_cannot_resize_input = "Could not resize input to read the file.";

static char* err_unexpected_char = "Unexpected character while lexing.";
static char* err_cannot_resize_ident = "Could not resize identifier while lexing.";
static char* err_cannot_resize_string = "Could not resize string while lexing.";
static char* err_cannot_resize_number = "Could not resize number while lexing.";
static char* err_invalid_character = "Invalid character while lexing, single quotes \"'\" can only contain one character (escape characters are an exception, as they require 2 characters instead).";
static char* err_invalid_escape_code = "Invalid escape code while lexing.";
static char* err_invalid_quotes = "Invalid quote placement in a number while lexing.";
static char* err_invalid_real = "Invalid real number with multiple commas while lexing.";
static char* err_unterminated_comment = "Unterminated comment.";
static char* err_unterminated_string = "Unterminated string.";

static char* err_expected_file = "Expected file after keyword 'import' while processing imports.";
static char* err_cannot_import = "Could not import file.";
static char* err_import_invalid_file = "Tried to import a file that does not exist.";
static char* err_expected_ident_using = "Expected an identifier after keyword 'define'/'define_line' while processing macros.";
static char* err_expected_equals_using = "Expected an '=' or '(' after keyword 'define'/'define_line' followed up with an identifier while processing macros.";
static char* err_invalid_using_value = "Expected a keyword as a value for the 'define'/'define_line' statement.";
static char* err_using_exists = "Tried to define a macro that already exists using 'define'/'define_line' keyword.";
static char* err_statement_semicolon = "Expected statement to end with a semicolon.";
static char* err_cannot_handle_using = "Could not replace a macro call while processing.";
static char* err_expected_comma_or_r_paren = "Expected a ',' or ')' after a parameter in macro arguments.";
static char* err_invalid_arg_count = "Invalid argument count in macro call.";
static char* err_invalid_macro_call = "Invalid macro call, either use no parentheses for no arguments, or parentheses with values as arguments.";
static char* err_invalid_macro_params = "Invalid macro parameter definition, either use no parentheses for no parameters, or parentheses with identifiers as parameters.";
static char* err_invalid_macro_body = "Invalid macro body, expected atleast one token.";
static char* err_invalid_backslash = "Invalid backslash, either next line does not exist or last token isn't a newline token.";
static char* err_mismatched_parentheses = "Mismatched parentheses in macro conditional.";
static char* err_invalid_macro_conditional = "Unfinished boolean expression in macro conditional, expected '#then' after boolean expression.";
static char* err_unexpected_token_mcond = "Unexpected token/operator in macro conditional boolean expression.";
static char* err_cannot_convert_to_ld = "Could not convert from string to long double.";
static char* err_invalid_macro_in_mcond = "Invalid macro in macro conditional boolean expression, either does not exist or isn't exactly one token in size.";
static char* err_mcond_did_not_end_with_endif = "Macro conditional statement did not end with the '#endif' keyword.";
static char* err_invalid_bool_expr_mcond = "Invalid boolean expression in macro condition, invalid amount of operands.";
static char* err_invalid_undefine_macro = "Invalid 'undefine' macro, expected an identifier of a macro after the keyword.";
static char* err_invalid_mcond_start = "Expected a macro conditional to start with '#if'/'#ifn'/'#ifdef'/'#ifndef'.";

#endif // ERRORS_H
