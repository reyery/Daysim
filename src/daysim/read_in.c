#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"

#include "ds_shortterm.h"
#include "read_in.h"
#include "ds_constants.h"


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
		error(USER, "input_units_genshortterm = 3 (illuminances) is currently not supported");
	}

	output_units_genshortterm=wea_data_short_file_units;
	read_in[11]=1;


	latitude = degrees(s_latitude);
	read_in[5]=1;

	longitude = degrees(s_longitude);
	read_in[6]=1;

	time_zone = degrees(s_meridian);
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
		error(USER, "the weather input file is missing");
	}

	if ( !read_in[1] )
	{
		strcpy(input_weather_data_shortterm,input_weather_data);
		strcat(input_weather_data_shortterm,".short");
		error(WARNING, "the variable input_weather_data_shortterm was not given and is set to *.short by default");
	}

	if ( !read_in[2] )
	{
		error(USER, "time step is not specified in read_in_genshortterm_header");
	}
	if (shortterm_timestep != 1 && shortterm_timestep != 2 && shortterm_timestep != 3 && shortterm_timestep != 4 && \
		shortterm_timestep != 5 && shortterm_timestep != 6 && shortterm_timestep != 10 && shortterm_timestep != 12 && \
		shortterm_timestep != 15 && shortterm_timestep != 20 && shortterm_timestep != 30)
	{
		error(USER, "shortterm_timestep is not allowed. Allowed values are (minutes): {1,2,3,4,5,6,10,12,15,20,30}");
	}

	if ( !read_in[3] )
	{
		error(USER, "the weather input units are not specified in read_in_genshortterm_header");
	}

	if ( !read_in[5] )
	{
		error(USER, "latitude is not specified in read_in_genshortterm_header");
	}

	if ( !read_in[6] )
	{
		error(USER, "longitude is not specified in read_in_genshortterm_header");
	}

	if ( !read_in[7] )
	{
		error(USER, "time_zone is not specified in read_in_genshortterm_header");
	}

	if ( !read_in[8] )
	{
		error(WARNING, "site_elevation is not specified, default value is sea level");
	}

	//if ( output_units_genshortterm == 2 && shortterm_timestep == 60 )
	//	{
	//		error(USER, "output_units_genshortterm=2 && shortterm_timestep=60 is not supported");
	//	}


}

int read_horizon_azimuth_data ( char *filename, float *horizon_azimuth )
{
	FILE *HORIZON_AZIMUTH_DATA;
	int i;
	float temp;

	if ( (HORIZON_AZIMUTH_DATA = open_input(filename)) == NULL )
	{
		sprintf(errmsg, "file %s cannot be opened", filename);
		error(WARNING, errmsg);
		return 1;
	}

	for ( i=1; i<=36; i++ )  {  fscanf(HORIZON_AZIMUTH_DATA,"%f",&temp);  horizon_azimuth[i-1] = temp;  }

	close_file(HORIZON_AZIMUTH_DATA);
	return 0;
}

