/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <paths.h>
#include "rterror.h"


#ifndef PATH_SIZE
#define PATH_SIZE 1024
#endif


/*
 * function trim()
 * parameters -> trims the begining and the end of a string
 */
void trim(char* string,char* trimmed_string)
{
	size_t start_pos, end_pos;

	// find last non " "-position in string
	for( end_pos= strlen(string) - 1; end_pos >= 0 && isspace(string[end_pos]); end_pos-- );

	// find first non " "-position in string
	for( start_pos= 0; start_pos < end_pos && isspace(string[start_pos]); start_pos++ );

	//	strncpy( trimmed_string, &string[start_pos], end_pos - start_pos + 1 );
	memmove( trimmed_string, &string[start_pos], end_pos - start_pos + 1 );
	trimmed_string[end_pos - start_pos + 1]= '\0';
}


//function getWord()
//parameters -> word[] is where the word that we're reading in will be stored
//              f is the file we're reading from
//              max is the maximum length of the word we're reading
//              delim is the character you want to use as a delimiter
//Note: By default a new line character is a delimiter as well
//This function is completely portable
int getWord(char* word, FILE* f, const int max, char delim)
{

	int delimiter = delim;
	char new_line = '\n';
	char *untrimed_word;
    	int size = 0;
	int my_letter = getc(f);

    if (f == NULL)
    {
		error(WARNING, "null file");
		return 0;
	}

    untrimed_word=(char *)malloc(sizeof(char)*max);
	if (untrimed_word == NULL)
		error(SYSTEM, "out of memory in getWord");

    strcpy(untrimed_word,"");

    if(!feof(f))
    {
        if(my_letter == new_line)
                my_letter = getc(f);
    }

	while (!feof(f) && size < max-1 && my_letter != delimiter && my_letter != new_line)
	{
		word[size] = my_letter;
		size++;
		my_letter = getc(f);
	}
	word[size] = '\0'; //NULL;
	strcpy(untrimed_word,word);
	trim(word,untrimed_word);
	strcpy(word,untrimed_word);
	//printf("trim: \'%s\' \'%s\'\n", untrimed_word,word);
    if (size >= max-1)
	{
		error(WARNING, "word may be too large, taking first 200 characters only"); //TODO doesn't it take max characters?
		while (!feof(f) && my_letter != delimiter && my_letter != new_line)
			my_letter = getc(f);
	}

	if (feof(f))
        return EOF;
	return size;
}



int close_file(FILE *f)	/*function that closes a file*/
{	int s=0;
	if (f==NULL) return 0;
	errno =0;
	s=fclose(f);
	if (s==EOF) perror("Close failed");
	return s;
}

int check_if_file_exists(char *filename)
{	FILE *Datei;
	errno =0;
	Datei = fopen(filename, "r");
	if (Datei == NULL)
		return 0;
	close_file(Datei);
	return 1;
}


int length_of_file(char *filename)
{	FILE *Datei;
	int		end=0;
	errno =0;
	Datei = fopen(filename, "r");
	if ( Datei )
	{
		fseek (Datei, 0, SEEK_END);
		end = ftell (Datei);
		close_file(Datei);
	}
	return(end);


}


FILE *open_output(char *filename)	/*open filename for writing*/
{	FILE *Datei;
	errno =0;
	Datei = fopen(filename, "w");
	if (Datei == NULL) {
		sprintf(errmsg, "open of '%s' for output failed: %s", filename, strerror(errno));
		error(USER, errmsg);
	}
	return Datei;
}

FILE *open_input(char *filename)	/*open file for reading*/
{	FILE *Datei;
	errno =0;
	Datei = fopen(filename, "r");
	if (Datei == NULL) {
		sprintf(errmsg, "open of '%s' for input failed: %s", filename, strerror(errno));
		error(USER, errmsg);
	}
	return Datei;
}


int does_file_exist(char *keyword, char *filename)	/*test whether file exists*/
{	FILE *Datei;
	errno =0;
	Datei = fopen(filename, "r");
	if ( Datei == NULL){
		sprintf(errmsg, "open of %s = %s for output failed: %s", keyword, filename, strerror(errno));
		error(WARNING, errmsg);
		return 0;
	}
	close_file(Datei);
	return 1;
}

int number_of_lines_in_file( char *filename)	/* returns the number of lines in file */
{
	FILE *Datei;
	int lines=0;
	errno =0;

	if( (Datei = fopen(filename, "r")) == NULL ) {
		sprintf(errmsg, "in number_of_lines_in_file(): cannot open file %s", filename);
		error(WARNING, errmsg);
		return 0;
	}
	while( EOF != fscanf(Datei,"%*[^\n]")){
		lines++;
		fscanf(Datei,"%*[\n\r]");
	}
	close_file(Datei);

	return lines;
 }



//function copies the content of a file into another
// returns '1' if successful
int copy_file(char *original_file, char *copied_file)
{
	FILE *ORIGINAL_FILE;
	FILE *COPIED_FILE;
	char ch;
	errno =0;
	ORIGINAL_FILE = fopen(original_file, "r");
	if ( ORIGINAL_FILE == NULL){
		sprintf(errmsg, "open of %s for output failed %s", original_file, strerror(errno));
		error(WARNING, errmsg);
		return 0;
	}
	COPIED_FILE = fopen(copied_file, "w");
	if ( COPIED_FILE == NULL){
		sprintf(errmsg, "open of %s for input failed %s", copied_file, strerror(errno));
		error(WARNING, errmsg);
		close_file(ORIGINAL_FILE);
		return 0;
	}
	// copy content character by character
	while(1)
    {
     	ch = getc(ORIGINAL_FILE);
    	if(ch==EOF)
     		break;
     	putc(ch,COPIED_FILE);
    }
	close_file(ORIGINAL_FILE);
	close_file(COPIED_FILE);
	return 1;
}


/*
 *
 */
char* prepend_path( char* path, char* str ) {
	size_t	offset = 0;
	int		pos;
	char 	token[PATH_SIZE];
	char 	p[PATH_SIZE];
	char* 	mpath= NULL;
	size_t	mpath_c = 0;		/* capacity */
	size_t	mpath_l = 0;		/* len */
	size_t	path_l;

	if( path == NULL )
		return NULL;
	path_l= strlen(path);
	if( path_l >= PATH_SIZE - 1 ) {
		error(WARNING, "file path to long");
		return NULL;
	}
	strncpy( p, path, PATH_SIZE );
	if( path_l && !ISDIRSEP( p[path_l - 1] ) ) {
		p[path_l++]= DIRSEP;
		p[path_l]= '\0';
	}

	while( sscanf( str + offset, "%s%n", token, &pos ) == 1 ) {
		size_t n = strlen(token);
		offset += pos;

		while( mpath_l + n + path_l + 1 > mpath_c ) {
			mpath_c= mpath_c ? 2*mpath_c : PATH_SIZE;
			mpath= (char*)realloc( mpath, mpath_c );
			if( mpath == NULL ) {
				perror( "failed to allocate memory" );
				return NULL;
			}
		}

		if( mpath_l )
			mpath[mpath_l++]= ' ';
		mpath[mpath_l]= '\0';

		strcpy( &mpath[mpath_l], p );
		strcpy( &mpath[mpath_l + path_l], token );
		mpath_l+= path_l + n;
	}

	return mpath;
}

/*
 *
 */
char* prepend_path_m( char* path, char* str, int size ) {
	char* p= prepend_path( path, str );
	if( p ) {
		if( strlen(p) < size )
			memcpy( str, p, strlen(p) + 1 );
		free(p);
		return str;
	}
	return NULL;
}
