#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "custom_types.h"

SP_font SP_load_font(char *filename);
void SP_free_font(SP_font* font);

#endif