#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(WIN32)
#include <errno.h>
#endif


FILE *open_input(char *filename)	            /*open filename for reading*/
{	
  FILE *Datei;
  errno = 0;
  Datei = fopen(filename, "r");
  if ( Datei == NULL)   fprintf(stderr,"open of %s for input failed: %s\n", filename, strerror(errno));
  return Datei;
}
			
FILE *open_output(char *filename)	           /*open filename for writing*/
{	
  FILE *Datei;
  errno = 0;
  Datei = fopen(filename, "w");
  if ( Datei == NULL)   fprintf(stderr,"open of %s for output failed: %s\n", filename, strerror(errno));
  return Datei;
}

int close_file(FILE *f)                          /*function that closes a file*/
{	
  int s=0;
  if (f==NULL) return 0;
  errno = 0;
  s=fclose(f);
  if (s==EOF) perror("Close failed");
  return s;
}			
