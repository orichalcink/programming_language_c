#ifndef CATCHER_H
#define CATCHER_H

#include "types/vector.h"

typedef struct
{
   Vector errors;
} Catcher;

void create_catcher(Catcher *catcher);
void free_catcher(Catcher *catcher);

void catcher_insert(Catcher *catcher, char* error);
bool catcher_empty(Catcher *catcher);
bool catcher_display(Catcher *catcher);

#endif // CATCHER_H
