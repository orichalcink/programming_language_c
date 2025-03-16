#ifndef TOKENS_H
#define TOKENS_H

typedef enum
{
   KEYWORD, IDENTIFIER, INTEGER, REAL, STRING, CHARACTER,
   EQUALS, SEMICOLON, L_PAREN, R_PAREN, COMMA,
   NEWLINE, SKIP, END_OF_FILE
} TType;

typedef struct
{
   TType type;
   char* lexeme;
} Token;

#endif // TOKENS_H
