/*	This program has been written by Christoph Reinhart at the
*	Institue for Research in Construction
*	last changes July 2002
*/
/* program generates a file scale with a constant factor  */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "version.h"
#include "rterror.h"
#include "paths.h"
#include "fropen.h"


int test_header=1;
int num_of_lines=0,line;
int lines_in_1st_batch=0, lines_in_2nd_batch=0;
int number_of_elements=0;
int num_of_columns_specified=0;
int number_of_selected_sensors=0;
int num_header_lines=0;
int *selected_sensor;
int last_element_blank=0;
int MultipleScaling=0;
int MultipleColumnScaling=0;
int SingleScalingFactor=1;
char  *progname;
char  dc_file[200]="";
char  character='\0';
char  dc_file_tmp[200]="";
char  dc_file_out[200]="";
char  befehl[200]="";
float new_transmission;
double scaling1=0, scaling2=0;
char  line_string[1000000]="";
double illuminance;
double scaling_factor=1.0;

FILE *DC_FILE;
FILE *DC_FILE_OUT;

int main( int argc, char  *argv[])
{
	int i=0, j=0, k=0;
	progname = fixargv0(argv[0]);


	if (argc == 1)
	{
		fprintf(stdout, "\n%s: \n", progname);
		printf("Program that scales all daylight coefficients in a dc-file with a constant factor\n");
		printf("The following options are required\n");
		printf("-i <file name> scaled dc input file \n");
		printf("-o <file name> scaled dc output file \n");
		printf("-s <float> scaling factor \n");
		printf("-m n1 s1 n2 s2 : scales first n1 lines with s1 and the following n2 lines with s2\n");
		printf("-n n1 s1 n2 s2 : scales first n1 columns with s1 and the following n2 columns with s2 \n");
		printf("\n");
		exit(0);
	}//end else

/* get the arguments */
	for (i = 1; i < argc; i++) {
		if (argv[i] == NULL || argv[i][0] != '-') {
			sprintf(errmsg, "%s bad option for input arguments", argv[i]);
			error(USER, errmsg);
		}
		if (!strcmp(argv[i], "-version")) {
			puts(VersionID);
			exit(0);
		}
		switch (argv[i][1])
		{
		case 's'://scaling factor
			scaling_factor = atof(argv[++i]);
			break;
		case 'c'://scaling factor
			number_of_selected_sensors = atoi(argv[++i]);
			selected_sensor = (int*)malloc(sizeof(int)*number_of_selected_sensors);
			if (selected_sensor == NULL)
				error(SYSTEM, "out of memory in main");
			for (j = 0; j < number_of_selected_sensors; j++)
				selected_sensor[j] = atoi(argv[++i]);
			break;
		case 'i'://input file
			strcpy(dc_file, argv[++i]);
			strcpy(dc_file_tmp, argv[i]);
			strcat(dc_file_tmp, ".tmp");
			break;
		case 'o'://output file
			strcpy(dc_file_out, argv[++i]);
			break;
		case 'l'://only consider the first num_of_lines
			num_of_lines = atoi(argv[++i]);
			if (num_of_lines == 0)
				error(USER, "number of lines to be considered=0");
			break;
		case 'm'://scales first n1 lines with s1 and the following n2 lines with s2
			MultipleScaling = 1;
			SingleScalingFactor = 0;
			lines_in_1st_batch = atoi(argv[++i]);
			scaling1 = atof(argv[++i]);
			lines_in_2nd_batch = atoi(argv[++i]);
			scaling2 = atof(argv[++i]);
			num_of_lines = lines_in_1st_batch + lines_in_2nd_batch;
			break;
		case 'n'://scales first n1 columns with s1 and the following n2 columns with s2
			MultipleColumnScaling = 1;
			SingleScalingFactor = 0;
			lines_in_1st_batch = atoi(argv[++i]);
			scaling1 = atof(argv[++i]);
			lines_in_2nd_batch = atoi(argv[++i]);
			scaling2 = atof(argv[++i]);
			num_of_columns_specified = lines_in_1st_batch + lines_in_2nd_batch;
			break;



		}//end switch (argv[i][1])
	}

	if (dc_file_tmp[0] == '\0' && argc > 2) {
		strcpy(dc_file_tmp, argv[2]);
		strcat(dc_file_tmp, ".tmp");
	}

i=num_of_lines;
num_of_lines=0;
	DC_FILE=open_input(dc_file);
		while( EOF != fscanf(DC_FILE,"%*[^\n]")){
			num_of_lines++;
			fscanf(DC_FILE,"%*[\n\r]");
		}
	close_file(DC_FILE);
	if(i!=0 && i>num_of_lines)
	{
		sprintf(errmsg, "number of lines specified (%d) larger than number of lines in file (%d)", i, num_of_lines);
		error(USER, errmsg);
	}
if(i>0)
	num_of_lines=i;

DC_FILE=open_input(dc_file);
DC_FILE_OUT=open_output(dc_file_out);

while(test_header){
	/*read in line and write out in string */
	fgets(line_string,1000000,DC_FILE);

	sscanf(line_string,"%c ",&character);
	if( character == '#' ) {
		//	if( !strcmp(character,"#") ){
		fprintf(DC_FILE_OUT,"%s",line_string);
		num_header_lines++;
	}else{
		test_header=0;
	}
}
//fprintf(stderr,"%s\n",line_string);
/*get number of daylight coefficients */
for (i=0;i<1000000;i++){
	//make sure you don't exceed the bounds of the array
	if (line_string[i] == '\n')
	{
			if(last_element_blank==0)
			 	number_of_elements++;
			break; //exits the loop without completing the current iteration
	}
	if(line_string[i] == ' ' || line_string[i] == '\t')
	{
		if(last_element_blank==0)
			number_of_elements++;
		last_element_blank=1;
	 	continue; //goes to the next iteration of the loop without finishing the current iteration
	}else {
		last_element_blank=0;
		continue;
	}

}

	if(num_of_columns_specified > number_of_elements)
	{
		sprintf(errmsg, "number of columns specified (%d) larger than number of elements per line in file (%d)", num_of_columns_specified, number_of_elements);
		error(USER, errmsg);
	}


fprintf(stderr,"number of  header lines: %d\nnumber of data lines: %d\nnumber of elements in each line: %d\n",num_header_lines,num_of_lines,number_of_elements);
close_file(DC_FILE);
DC_FILE=open_input(dc_file);
for (i=0;i<num_of_lines;i++)
{
	if(i< num_header_lines) // skip header lines
	{
		fscanf(DC_FILE,"%*[^\n]");
		fscanf(DC_FILE,"%*[\n\r]");
	}else{ // go through main file
		if(MultipleScaling)
		{
			for(j=0;j<number_of_elements;j++)
			{
				fscanf(DC_FILE,"%lf",&illuminance);
				if(i<lines_in_1st_batch)
				{
					fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling1);
				}else{
					fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling2);
				}
			}
			fprintf(DC_FILE_OUT,"\n");
		}
		if(MultipleColumnScaling)
		{
			for(j=0;j<number_of_elements;j++)
			{
				fscanf(DC_FILE,"%lf",&illuminance);
				if(j<lines_in_1st_batch)
				{
					fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling1);
				}else{
					fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling2);
				}
			}
			fprintf(DC_FILE_OUT,"\n");
		}

		if(SingleScalingFactor)
		{ //Single Scaling Factor
			for(j=0;j<number_of_elements;j++)
			{
				fscanf(DC_FILE,"%lf",&illuminance);
				if (number_of_selected_sensors==0)
				{
			 		fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling_factor);
				}else{
					for(k=0;k<number_of_selected_sensors;k++)
					{
						if(i==(selected_sensor[k]-1))
							fprintf(DC_FILE_OUT,"%e\t", illuminance*scaling_factor);
					}
				}
			}
			//output into file
			if (number_of_selected_sensors==0)
			{
	 			fprintf(DC_FILE_OUT,"\n");
			}else{
				for(k=0;k<number_of_selected_sensors;k++)
				{
					if(i==(selected_sensor[k]-1))
						fprintf(DC_FILE_OUT,"\n");
				}
			}
		}  //end Single Scaling Factor
	}
}

close_file(DC_FILE);
close_file(DC_FILE_OUT);


return 0;

}


