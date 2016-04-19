/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"
#include  "sun.h"

#include "paths.h"

#include "ds_illum.h"
#include "shadow_testing.h"

int *shadow_testing_results;

// function write out sensor point file for initial shadow testing routine
void make_Direct_DC_point_file(char long_sensor_file[1024],int num_direct_coefficients,float x,float y, float z)
{
	FILE *POINT_FILE;
	int i;
	double alt = 0.0, azi = 0.0;

	POINT_FILE=open_output(long_sensor_file);
	for (i=0 ; i< num_direct_coefficients; i++){
		alt = DTR*(90 - direct_pts[i][1]);
		azi = -DTR*(direct_pts[i][2] + 90);
		fprintf(POINT_FILE,"%f %f %f %f %f %f\n",x,y,z,sin(alt)*cos(azi),sin(alt)*sin(azi),cos(alt));
	}
	close_file(POINT_FILE);
}


void  make_annual_point_file_old(float x,float y,float z,char long_sensor_file[1024])
{
	// function creates an rtrace input file that creates for all time steps
	// of the year when direct sunlight is above the threshold specified in the Daysim header file
	FILE *WEA;
	FILE *POINTS;
	FILE *COMMAND_FILE;
	int jd=0,i=0,j=0;
	double sd = 0, solar_time = 0;
	float red=0.0,green=0.0,blue=0.0;
	char befehl[1024]="";
	int month, day;
	float hour, dir, dif;
	double alt = 0.0, azi = 0.0;

	fprintf(stdout, "%s: final shadow testing \n", progname);

	WEA=open_input(wea_data_short_file);
	fscanf(WEA,"%s",befehl);
	  	if( !strcmp(befehl,"place") ) // check weather the climate file as a header
	  	{   // skip header
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		}else{
			rewind(WEA);
		}

	POINTS=open_output(long_sensor_file);
  	while(fscanf(WEA,"%d %d %f %f %f",&month,&day,&hour,&dir,&dif) != EOF)
  	{
		/* get sun position */
       	if(dir >= dir_threshold && dif >= dif_threshold)
		{
			jd= jdate(month, day);
			sd=sdec(jd);
			solar_time=hour+stadj(jdate(month, day));
			alt = salt( sd,solar_time);
			azi = sazi(sd,solar_time);
  			fprintf(POINTS,"%f %f %f %f %f %f\n",x,y,z,-cos(alt)*sin(azi),-cos(alt)*cos(azi),sin(alt) );
  			i++;
		}
  	}
	close_file(POINTS);
	close_file(WEA);

	shadow_testing_results=(int*) malloc (sizeof(int)* i);
	if (shadow_testing_results == NULL)
		error(SYSTEM, "out of memory in make_annual_point_file_old");
	for (j=0 ; j<i ; j++)
		shadow_testing_results[j]=0;


	sprintf(befehl,"rtrace_dc -ab 0 -h  %s < %s  \n",temp_octree,long_sensor_file);
	//printf("%s ",befehl);
	COMMAND_FILE = popen(befehl, "r");

	// run one rtrace to determine whether direct DC is hit by sunlight
	//==================================================================

	sprintf(befehl,"rtrace_dc -ab 0 -h  %s < %s  \n",temp_octree,long_sensor_file);
	//printf("%s ",befehl);
	j=0;
	COMMAND_FILE = popen(befehl, "r");
	while (fscanf(COMMAND_FILE, " %f %f %f ", &red, &green, &blue) != EOF)
		if(red>0.00)
		{
			shadow_testing_results[j]=1;
			j++;
		}
	close_file(COMMAND_FILE);

}


int shadow_testing(int number_direct_coefficients)
{
	int i=0,j=0,k=0;
	char befehl[1024];
	char befehl1[1024];
	char sun_rad[1024];

	float red=0.0,green=0.0,blue=0.0;
	FILE *POINTS;
	FILE *COMMAND_FILE;
	char long_sensor_file[1024];

	// pick reference_file
	// the first 'work plane sensor found is the reference point
	POINTS=open_input(sensor_file);
	i=0;
	while ( i<number_of_sensors  ){
		for (j=0 ; j<6 ; j++){fscanf(POINTS,"%f",&point_coefficients[j]);}
		if( sensor_typ[i] ==1){i=number_of_sensors+10;}
		i++;
	}
	if(i != number_of_sensors+11)
	{
		error(USER, "shadow testing no \'work plane sensor\' specified in sensor file");
	}else{
		fprintf(stdout, "%s: initial shadow_testing invoked for sensor \n\t", progname);
	 	fprintf(stdout,"x:%.3f\ty:%.3f\tz:%.3f\n\tdx:%.3f\tdy:%.3f\tdz:%.3f\n",point_coefficients[0],point_coefficients[1],point_coefficients[2],point_coefficients[3],point_coefficients[4],point_coefficients[5]);
	 }
	i=close_file(POINTS);

	/* make outside_rad, i.e. a glowing celestial hemisphere */
	sprintf(sun_rad,"%s.sky_initial_shadow_testing.tmp.rad",tmp_directory);
	POINTS=open_output(sun_rad);
	fprintf(POINTS,"void glow outside_sphere\n 0\n 0\n 4 1000	1000	1000 0\n\n ");
	fprintf(POINTS,"outside_sphere source himmel\n 0\n 0\n 4 0	0	1 360\n ");
	close_file(POINTS);

	// generate octree of building scene with the direct daylight coefficient direct suns
	sprintf(befehl1,"oconv -f %s %s %s %s > %s\n",sun_rad,material_file,geometry_file,shading_rad_file[0],temp_octree);
	//sprintf(befehl1,"oconv -f %s %s %s %s > %s\n",direct_radiance_file,material_file,geometry_file,shading_rad_file[0],temp_octree);
	//printf("%s\n",befehl1);
	COMMAND_FILE = popen(befehl1, "r");
	pclose(COMMAND_FILE);

	//test whether the sensor point "sees" the position of the direct daylight
	// coefficients
	if( strcmp(sensor_file,""))
		sprintf(long_sensor_file,"%s.shadow_testing_temp_sensors.pts",sensor_file);

	// write directions from reference point to position of the sun
	// for the direct daylight coefficient
	make_Direct_DC_point_file(long_sensor_file,number_direct_coefficients,point_coefficients[0],point_coefficients[1],point_coefficients[2]);

	// run one rtrace to determine whether direct DC is hit by sunlight
	//==================================================================
	sprintf(befehl,"rtrace_dc -ab 0 -h  %s < %s  \n",temp_octree,long_sensor_file);
	//printf("%s ",befehl);
	COMMAND_FILE = popen(befehl, "r");
	shadow_testing_new=0;//switch that determines whether any sensor is hit by direct sunlight
	for(i=0; i<number_direct_coefficients;i++  )
	{
		fscanf(COMMAND_FILE, " %f %f %f ", &red, &green, &blue);
		if(red>0.00)
		{
			direct_pts[i][3]=1;
			shadow_testing_new=1;
			for(j=0; j< 24; j++)
			{
				for(k=1; k< 13; k++)
				{
					if((direct_calendar[k][j][1]==direct_pts[i][1]) &&(direct_calendar[k][j][2]== direct_pts[i][2]))
						direct_calendar[k][j][3]=1;
				}
			}
		}else
			direct_pts[i][3]=0;
	}
	close_file(COMMAND_FILE);

	printf("Results from initial shadow testing:\n");
		for(k=1; k< 13; k++)
			{
				switch ( k)
				{
	 				case 2 :{
						printf("Feb/Oct ");
	 					break;
					}
	 				case 3 :{
						printf("Mar/Sep ");
	 					break;
					}
	 				case 4 :{
						printf("Apr/Aug ");
	 					break;
					}
	 				case 6 :{
						printf("Jun     ");
	 					break;
					}
	 				case 12 :{
						printf("Dec     ");
	 					break;
					}
				}
				switch ( k)
				{
	 				case 2 : case 3 : case 4: case 6: case 12: {
						for(j=0; j< 24; j++)
						{
							printf("%.0f ",direct_calendar[k][j][3]);
						}
						printf("\n");
	 					break;
					}
				}
			}

	if(shadow_testing_new)
	{
		// write out all all elegible sun positions of the year into a file
		// an elegible sun position is one in which
		// direct sunlight is above the theshold value set in the Daysim
		// header file
		make_annual_point_file_old(point_coefficients[0],point_coefficients[1],point_coefficients[2],long_sensor_file);
	}else{
		error(WARNING, "shadow testing is turned off !");
		dc_coupling_mode=0;
	}

	return 0;
}

int shadow_test_single(double sun_x, double sun_y, double sun_z)
{
	int value=0;
	char befehl[1024];
	char befehl1[1024];
	FILE *FILE1;

	FILE1=open_output(temp_rad);
	fprintf(FILE1,"void light solar\n 0\n 0\n 3 100	100	100\n\n ");
	fprintf(FILE1,"solar source sun \n 0\n 0\n");
	fprintf(FILE1,"4 %f %f %f  0.533\n \n",sun_x,sun_y,sun_z);
	close_file(FILE1);
	sprintf(befehl,"oconv -f %s %s %s %s > %s\n", temp_rad,material_file,geometry_file,shading_rad_file[0],temp_octree);
	/*printf("%s",befehl);*/
	FILE1 = popen(befehl, "r");
	pclose(FILE1);
	sprintf(befehl1,"echo %f %f %f ",point_coefficients[0],point_coefficients[1],point_coefficients[2]);
	sprintf(befehl," %f %f %f ",sun_x,sun_y,sun_z);
	strcat(befehl1,befehl);
	sprintf(befehl,"%s |rtrace -ab 0 -h  %s  |awk '{if( $1 <= 5){printf(\"0\\n\");}else{printf(\"1\\n\");}}'  \n",befehl1, temp_octree);
	/*printf("%s",befehl);*/
	FILE1 = popen(befehl, "r");
	fscanf(FILE1,"%d ",&value);
	pclose(FILE1);
	return(value);
}
