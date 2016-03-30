#pragma once

FILE *win_popen(char *command, char *type);
int win_pclose(FILE *p);