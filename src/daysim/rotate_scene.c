/*  Copyright (c) 2003
 *  National Research Council Canada
 *  written by Christoph Reinhart
 */

// radfiles2daysim:
// DAYSIM GUI subprogram that rotates RADIANCE sources files and the sensor point file
// around the z-axis

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include  "fropen.h"
#include  "read_in_header.h"

char header_file[200]="";
char  *progname;


int main( int argc, char  *argv[])
{
	int i;
	float x,y,z,x_or,y_or,z_or,x_rot,y_rot;
	char sensor_file_rotated[1024]="";
	char befehl1[1024]="";
	FILE *COMMAND;
	FILE *TEST_FILE;
	FILE  *TEST_FILE2;

	progname = argv[0];

	if (argc < 2){
		fprintf(stderr,"rotate_scene: no input file specified\n");
		fprintf(stderr,"start program with\n");
		fprintf(stderr,"rotate_scene <file>.hea \n\n");
		exit(1);
	}
	strcpy(header_file,argv[1]);

	for (i = 2; i < argc; i++)
		if (argv[i][0] == '-' )
			switch (argv[i][1])
			{
			case 'x':
					break;
			}//end switch (argv[i][1])

//===================================
// read in DAYSIM project header file
//===================================
	read_in_header(header_file);

//==============================================================
// if rotation angle not zero, generate a new sensor point file
//==============================================================
	//printf("rotation_angle %s\n",material_file);
	printf("rotation_angle %f\n",rotation_angle);
	if(rotation_angle!=0.0)
	{
		sprintf(sensor_file_rotated,"%s.rotated.pts",sensor_file);
		TEST_FILE=open_input(sensor_file);
		TEST_FILE2=open_output(sensor_file_rotated);
		for (i=0 ; i<( number_of_sensors) ; i++)
		{
			fscanf(TEST_FILE,"%f %f %f %f %f %f",&x,&y,&z,&x_or,&y_or,&z_or);
			x_rot=1.0*cos((M_PI/180.)*(-1.0)*rotation_angle)*x+1.0*sin((M_PI/180.)*(-1.0)*rotation_angle)*y;
			y_rot=-1.0*sin((M_PI/180.)*(-1.0)*rotation_angle)*x + 1.0*cos((M_PI/180.)*(-1.0)*rotation_angle)*y;
			fprintf(TEST_FILE2,"%f\t%f\t%f\t%f\t%f\t%f\n",x_rot,y_rot,z,x_or,y_or,z_or);
		}
		close_file(TEST_FILE);
		close_file(TEST_FILE2);
	}

//==============================
// read in RADIANCE source files
//==============================

		for (i=0 ; i<(number_of_radiance_source_files) ; i++){

			//check if files exist
			if(!does_file_exist("file", radiance_source_files[i])){
				fprintf(stderr,"FATAL ERROR: radfiles2hea \n");
			}

			//if rotation_angle set, rotate scene
			if(rotation_angle !=0.0)
			{
				sprintf(befehl1,"%sxform -rz %f %s > %s.rotated.rad",bin_dir,rotation_angle,radiance_source_files[i],radiance_source_files[i]);
				printf("%s\n",befehl1);
				COMMAND=popen(befehl1,"r");
				pclose(COMMAND);
			}
		}

	return 0;
}

