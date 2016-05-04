/*  Copyright (c) 2003
 *  National Research Council Canada
 *  written by Christoph Reinhart
 */

// radfiles2daysim:
// DAYSIM GUI subprogram that rotates RADIANCE sources files and the sensor point file
// around the z-axis

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "rterror.h"
#include "paths.h"
#include  "fropen.h"
#include  "read_in_header.h"
#include "ds_constants.h"


char header_file[200]="";
char  *progname;


int main( int argc, char  *argv[])
{
	int i;
	double x,y,z,x_or,y_or,z_or,x_rot,y_rot;
	double angle;
	char sensor_file_rotated[1024]="";
	char befehl1[1024]="";
	FILE *COMMAND_FILE;
	FILE *TEST_FILE;
	FILE  *TEST_FILE2;

	progname = fixargv0(argv[0]);

	if (argc < 2){
		fprintf(stderr, "%s: no input file specified\n", progname);
		fprintf(stderr,"start program with\n");
		fprintf(stderr, "%s <file>.hea \n\n", progname);
		exit(1);
	}

	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

	strcpy(header_file,argv[1]);

//===================================
// read in DAYSIM project header file
//===================================
	read_in_header(header_file);
	angle = radians(-rotation_angle);

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
			x_rot = cos(angle)*x + sin(angle)*y;
			y_rot = -sin(angle)*x + cos(angle)*y;
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
			if (!check_if_file_exists(radiance_source_files[i])){
				sprintf(errmsg, "cannot find file %s", radiance_source_files[i]);
				error(USER, errmsg);
			}

			//if rotation_angle set, rotate scene
			if(rotation_angle !=0.0)
			{
				sprintf(befehl1,"%sxform -rz %f %s > %s.rotated.rad",bin_dir,rotation_angle,radiance_source_files[i],radiance_source_files[i]);
				printf("%s\n",befehl1);
				COMMAND_FILE = popen(befehl1, "r");
				pclose(COMMAND_FILE);
			}
		}

	return 0;
}

