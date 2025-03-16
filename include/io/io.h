#ifndef IO_H
#define IO_H

#include "errors/catcher.h"

char* read_input();

bool is_file(const char* input);
char* read_file(Catcher* catcher, char* input);

#endif // IO_H
