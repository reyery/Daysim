#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fropen.h"
#include "read_in_header.h"

#include "ds_shortterm.h"
#include "read_in.h"


void read_in_genshortterm_header()        /*  read in header file  */
{
	int i,read_in[30];
	char type_now[200];

	for ( i=0 ; i<30 ; i++ )   read_in[i]=0;

	read_in_header(header);

	strcpy(input_weather_data,wea_data_file);
	if(strcmp(input_weather_data,""))
	    read_in[0]=1;

	strcpy(input_weather_data_shortterm,wea_data_short_file);
	if(strcmp(input_weather_data_shortterm,""))
	    read_in[1]=1;

	shortterm_timestep=time_step;
	read_in[2]=1;

	input_units_genshortterm=wea_data_file_units;
	read_in[3]=1;
	if ( input_units_genshortterm == 3 )
	    {
			printf("read_in_genshortterm_header: fatal error - input_units_genshortterm = 3 (illuminances) is currently not supported\n");
			exit(1);
	    }

	output_units_genshortterm=wea_data_short_file_units;
	read_in[11]=1;


	latitude=s_latitude*(180.0/3.14159);
	read_in[5]=1;

    longitude=s_longitude*(180.0/3.14159);
	read_in[6]=1;

	time_zone=s_meridian*(180.0/3.14159);
	read_in[7]=1;

	//site elevation
	read_in[8]=1;

	HEADER=open_input(header);
	while ( EOF != fscanf(HEADER,"%s", type_now) )
		{
			if ( !strcmp(type_now,"linke_turbidity_factor_am2") )
				{
					for ( i=0 ; i<12 ; i++ )  fscanf(HEADER,"%f",&linke_turbidity_factor_am2[i]);
					read_in[9]=1;
					linke_estimation=0;
				}

			if ( !strcmp(type_now,"horizon_data_in") )
				{
					fscanf(HEADER,"%s", horizon_data_in);
					read_in[10]=1;
					horizon_in=1;
				}

			if ( !strcmp(type_now,"horizon_data_out") )
				{
					fscanf(HEADER,"%s", horizon_data_out);
					read_in[12]=1;
					horizon_out=1;
				}

			if ( !strcmp(type_now,"random_seed") )
				{
					fscanf(HEADER,"%ld", &random_seed);
					read_in[13]=1;
				}
		}

	close_file(HEADER);

	if ( !read_in[0] )
		{
			printf("read_in_genshortterm_header: fatal error - the weather input file is missing\n");
			exit(1);
		}

	if ( !read_in[1] )
		{
			strcpy(input_weather_data_shortterm,input_weather_data);
			strcat(input_weather_data_shortterm,".short");
			printf("read_in_genshortterm_header: warning - the variable input_weather_data_shortterm was not given\n");
			printf("read_in_genshortterm_header: it is set to *.short by default\n");
		}

	if ( !read_in[2] )
		{
			if ( shortterm_timestep != 1 && shortterm_timestep != 2 && shortterm_timestep != 3 && shortterm_timestep != 4 && \
				 shortterm_timestep != 5 && shortterm_timestep != 6 && shortterm_timestep != 10 && shortterm_timestep != 12 && \
				 shortterm_timestep != 15 && shortterm_timestep != 20 && shortterm_timestep != 30 )
				{
					printf("read_in_genshortterm_header: fatal error - shortterm_timestep is not allowed\n");
					printf("Allowed values are (minutes): {1,2,3,4,5,6,10,12,15,20,30}\n");
				}
			exit(1);
		}

	if ( !read_in[3] )
		{
			printf("read_in_genshortterm_header: fatal error - the weather input units are not specified\n");
			exit(1);
		}

	if ( !read_in[5] )
		{
			printf("read_in_genshortterm_header: fatal error - latitude is not specified\n");
			exit(1);
		}

	if ( !read_in[6] )
		{
			printf("read_in_genshortterm_header: fatal error - longitude is not specified\n");
			exit(1);
		}

	if ( !read_in[7] )
		{
			printf("read_in_genshortterm_header: fatal error - time_zone is not specified\n");
			exit(1);
		}

	if ( !read_in[8] )
		{
			printf("read_in_genshortterm_header: warning - site_elevation is not specified, default value is sea level\n");
		}

	//if ( output_units_genshortterm == 2 && shortterm_timestep == 60 )
	//	{
	//		printf("read_in_genshortterm_header: fatal error - output_units_genshortterm=2 && shortterm_timestep=60 is not supported\n");
	//		exit(1);
	//	}


}

int read_horizon_azimuth_data ( char *filename, float *horizon_azimuth )
{
	FILE *HORIZON_AZIMUTH_DATA;
	int i;
	float temp;

	if ( (HORIZON_AZIMUTH_DATA = open_input(filename)) == NULL )
		{
			fprintf(stderr,"file %s cannot be opened\n", filename);
			return 1;
		}

	for ( i=1; i<=36; i++ )  {  fscanf(HORIZON_AZIMUTH_DATA,"%f",&temp);  horizon_azimuth[i-1] = temp;  }

	close_file(HORIZON_AZIMUTH_DATA);
	return 0;
}

