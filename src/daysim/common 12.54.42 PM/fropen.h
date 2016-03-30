#ifndef FROPEN_H
#define FROPEN_H

#include <stdio.h>

void trim(char* string,char* trimmed_string);

int getWord(char* word, FILE* f, const int max, char delim);

int close_file(FILE *f);	/*function that closes a file*/
int check_if_file_exists(char *filename);
int copy_file(char *original_file, char *copied_file);

FILE *open_output(char *filename);	/*open filename for writing*/
FILE *open_input(char *filename);	/*open filename for reading*/
int does_file_exist(char *keyword, char *filename);	/*test whether filename exsists*/
int number_of_lines_in_file( char *filename);	/* returns the number of lines in file */
int length_of_file(char *filename); /* return length of file */

/**
 * prepend the specified path to all the tokens in within the string
 * allocates memory which has to be released by the caller.
 */
char* prepend_path( char* path, char* str );

/**
 * Same as prepend_path except that content of the string itself is changed.
 */
char* prepend_path_m( char* path, char* str, int size );

#endif
