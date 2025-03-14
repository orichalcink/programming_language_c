#ifndef ERRORS_H
#define ERRORS_H

static char *err_invalid_input = "Invalid input.";

static char *err_unexpected_char = "Unexpected character while lexing.";
static char *err_cannot_resize_ident = "Could not resize identifier while lexing.";
static char *err_cannot_resize_string = "Could not resize string while lexing.";
static char *err_cannot_resize_number = "Could not resize number while lexing.";
static char *err_invalid_character = "Invalid character while lexing, single quotes \"'\" can only contain one character (escape characters are an exception, as they require 2 characters instead).";
static char *err_invalid_escape_code = "Invalid escape code while lexing.";
static char *err_invalid_quotes = "Invalid quote placement in a number while lexing.";
static char *err_invalid_real = "Invalid real number with multiple commas while lexing.";

#endif // ERRORS_H
