#pragma once

#include <errno.h>

FILE *open_input(char *);
FILE *open_output(char *);
int close_file(FILE *);
