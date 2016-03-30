/*	This program has been written by Oliver Walkenhorst at the
 *	Fraunhofer Institute for Solar Energy Systems in Freiburg, Germany
 *	last changes were added in January 2001
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

#include  <stdio.h>
#include  <string.h>
#include  <rtmath.h>
#include  <stdlib.h>
#include  <rterror.h>
#include  <paths.h>

#include "file.h"
#include "sun.h"
#include "nrutil.h"
#include "numerical.h"
#include "skartveit.h"


char *header;
FILE *HEADER;               /*  header file  */
FILE *HOURLY_DATA;          /*  input weather data file  */
FILE *SHORT_TERM_DATA;      /*  input weather data shortterm file  */

/*  global variables for the header file key words and their default values */

char input_weather_data[200]="";
char input_weather_data_shortterm[200]="";   /*  default value: input_weather_data_shortterm = input_weather_data."shortterm_timestep"min  */
int shortterm_timestep=60;                /*  in minutes  */
int input_units_genshortterm;
int output_units_genshortterm=1;
int input_timestep=60;                /*  in minutes  */
int solar_time=0;                     /*  0=LST ; 1=solar time  */
long random_seed=-10;                 /*  seed for the pseudo-random-number generator, random_seed has to be a negative integer  */
int new=1;

/*  global variables for the header file key words representing station specific data  */

float latitude=45.32;
float longitude=75.67;
float time_zone=75.0;
//float site_elevation=0.0;                   /*  in metres  */
float linke_turbidity_factor_am2[12];       /*  monthly means for jan-dec  */
char horizon_data_in[200];            /*  name of the horizon data file for the station where the input irradiance data were collected  */
                                      /*  (the file contains 36 horizon heights in degrees starting from N to E)                        */
char horizon_data_out[200];	      /*  name of the horizon data file for the location the output irradiance data are computed for    */
                                      /*  (the file contains 36 horizon heights in degrees starting from N to E)                        */

/*  other global variables  */

int sph=60;                               /*  sph=steps per hour: if shortterm_timestep < 60 1-min-data are generated  */
int horizon_in=0;                         /*  indicates if an input horizon data file is specified   */
int horizon_out=0;                        /*  indicates if an output horizon data file is specified  */
float horizon_azimuth_in[36];             /*  divide [-180°,180°] of the input horizon in 36 azimuth classes  */
                                          /*  (south=0°, horizon heights in degrees)                          */
float horizon_azimuth_out[36];            /*  divide [-180°,180°] of the output horizon in 36 azimuth classes */
                                          /*  (south=0°, horizon heights in degrees)                          */
int linke_estimation=1;                   /*  flag that indicates if estimation of the monthly linke factors is necessary  */

/*  constants used  */

const float solar_constant_e = 1367.0;
const int F = sizeof(float);
const int I = sizeof(int);

/*  versioning  */

extern char  VersionID[];	/* Radiance version ID string */


void solar_elev_azi_ecc(float latitude, float longitude, float time_zone, int jday, float time, int solar_time, float *solar_elevation, float *solar_azimuth, float *eccentricity_correction)

/*  angles in degrees, times in hours  */
{
	float sol_time;
	float solar_declination, jday_angle;

	/*  solar elevation and azimuth formulae from sun.c  */
	if (solar_time == 1)   sol_time = time;
	if (solar_time == 0)   sol_time = time + 0.170 * sin((4 * PI / 373) * (jday - 80)) - 0.129 * sin((2 * PI / 355) * (jday - 8)) + 12 / 180.0 * (time_zone - longitude);

	solar_declination = RTD * 0.4093 * sin((2 * PI / 368) * (jday - 81));
	jday_angle = 2 * PI*(jday - 1) / 365;

	*solar_elevation = RTD * asin(sin(latitude*DTR) * sin(solar_declination*DTR) - cos(latitude*DTR) * cos(solar_declination*DTR) * cos(sol_time*(PI / 12)));

	*solar_azimuth = RTD * (-atan2(cos(solar_declination*DTR) * sin(sol_time*(PI / 12)),
		-cos(latitude*DTR)*sin(solar_declination*DTR) -
		sin(latitude*DTR)*cos(solar_declination*DTR)*cos(sol_time*(PI / 12))));

	/*  eccentricity_correction formula used in genjdaylit.c */

	*eccentricity_correction = 1.00011 + 0.034221*cos(jday_angle) + 0.00128*sin(jday_angle) + 0.000719*cos(2 * jday_angle) + 0.000077*sin(2 * jday_angle);
}

float diffuse_fraction(float irrad_glo, float solar_elevation, float eccentricity_correction)
{
	/*  estimation of the diffuse fraction according to Reindl et al., Solar Energy, Vol.45, pp.1-7, 1990  = [Rei90]  */
	/*                        (reduced form without temperatures and humidities)                                                                          */

	float irrad_ex;
	float index_glo_ex;
	float dif_frac;

	if (solar_elevation > 0)  irrad_ex = solar_constant_e * eccentricity_correction * sin(DTR*solar_elevation);
	else irrad_ex = 0;

	if (irrad_ex > 0)   index_glo_ex = irrad_glo / irrad_ex;
	else  return 0;

	if (index_glo_ex < 0)  { error(INTERNAL, "negative irrad_glo in diffuse_fraction_th"); }
	if (index_glo_ex > 1)  { index_glo_ex = 1; }

	if (index_glo_ex <= 0.3)
	{
		dif_frac = 1.02 - 0.254*index_glo_ex + 0.0123*sin(DTR*solar_elevation);
		if (dif_frac > 1)  { dif_frac = 1; }
	}

	if (index_glo_ex > 0.3 && index_glo_ex < 0.78)
	{
		dif_frac = 1.4 - 1.749*index_glo_ex + 0.177*sin(DTR*solar_elevation);
		if (dif_frac > 0.97)  { dif_frac = 0.97; }
		if (dif_frac < 0.1)   { dif_frac = 0.1; }
	}

	if (index_glo_ex >= 0.78)
	{
		dif_frac = 0.486*index_glo_ex - 0.182*sin(DTR*solar_elevation);
		if (dif_frac < 0.1)  { dif_frac = 0.1; }
	}

	return dif_frac;
}



/*  main program  */
int main(int argc, char *argv[])
{
	int i;
	int month, day, jday;
	int *daylight_status;     /*  0=night hour, 1=sunrise/sunset hour, 2=innerday hour  */
	int command_line = 0;
	int file_input_output = 0;

	float time, *times;
	float irrad_glo, irrad_beam_nor, irrad_dif;     /* in W/m² */
	float *irrads_glo, *irrads_beam_nor, *irrads_dif, *indices_glo, *indices_beam, *sr_ss_indices_glo;
	float *irrads_glo_st, *irrads_glo_clear_st, *irrads_beam_nor_st, *irrads_dif_st, *indices_glo_st;
	float solar_elevation, solar_azimuth, eccentricity_correction;



	/* get the arguments */
	if (argc > 1)
	{

		for (i = 1; i < argc; i++) {
			if (argv[i] == NULL || argv[i][0] != '-')
				break;			/* break from options */
			if (!strcmp(argv[i], "-version")) {
				puts(VersionID);
				exit(0);
			}
			switch (argv[i][1])
			{
				case 'a':
					latitude = atof(argv[++i]);
					break;
				case 'm':
					time_zone = atof(argv[++i]);
					break;
				case 'l':
					longitude = atof(argv[++i]);
					break;

				case 'i':
					strcpy(input_weather_data, argv[++i]);
					file_input_output++;
					break;
				case 'o':
					strcpy(input_weather_data_shortterm, argv[++i]);
					file_input_output++;
					break;
				case 'g':
					month = atoi(argv[++i]);
					day = atoi(argv[++i]);
					time = atof(argv[++i]);
					irrad_glo = atof(argv[++i]);
					command_line = 1;
					break;

			}
		}
	}

	if ((!command_line && (file_input_output<2)) || argc == 1)
	{
		char *progname = fixargv0(argv[0]);
		fprintf(stdout, "\n%s: \n", progname);
		printf("Program that splits input global irradiances into direct normal and diffuse horizontal irradiances using the Reindl model. ");
		printf("The program has a command line and an input file option depending on wheter a single or mutliple irradiances are to be converted.\n");
		printf("\n");
		printf("Supported options are: \n");
		printf("-m\t time zone [DEG, West is positive] (required input)\n");
		printf("-l\t longitude [DEG, West is positive] (required input)\n");
		printf("-a\t latitude [DEG, North is positive] (required input)\n\n");
		printf("-g\t month day hour global_irradiation [W/m2] (command line version)\n");
		printf("-i\t input file [format: month day hour global_irradiation] \n");
		printf("-o\t output file [format: month day hour dir_norm_irrad dif_hor_irrad] \n");
		printf("Example: \n");
		printf("\t%s -a 42.3 -o 71 -m 75 -g 6 21 12.0 800 \n", progname);
		printf("\t%s -a 42.3 -o 71 -m 75 -i input_file.txt -o output_file.txt\n", progname);
		exit(0);
	}
	if ((times = malloc(24 * F)) == NULL) goto memerr;
	if ((irrads_glo = malloc(24 * F)) == NULL) goto memerr;
	if ((irrads_beam_nor = malloc(24 * F)) == NULL) goto memerr;
	if ((irrads_dif = malloc(24 * F)) == NULL) goto memerr;
	if ((indices_glo = malloc(24 * F)) == NULL) goto memerr;
	if ((indices_beam = malloc(24 * F)) == NULL) goto memerr;
	if ((sr_ss_indices_glo = malloc(3 * F)) == NULL) goto memerr;
	if ((daylight_status = malloc(24 * I)) == NULL) goto memerr;

	if ((irrads_glo_st = malloc(sph*F)) == NULL) goto memerr;
	if ((irrads_glo_clear_st = malloc(sph*F)) == NULL) goto memerr;
	if ((irrads_beam_nor_st = malloc(sph*F)) == NULL) goto memerr;
	if ((irrads_dif_st = malloc(sph*F)) == NULL) goto memerr;
	if ((indices_glo_st = malloc(sph*F)) == NULL) goto memerr;



	for (i = 0; i < 36; i++)
		horizon_azimuth_in[i] = 0;


	if (command_line)
	{
		jday = month_and_day_to_julian_day(month, day);
		if (irrad_glo < 0 || irrad_glo > solar_constant_e)          /*  check irradiances and exit if necessary  */
			irrad_glo = solar_constant_e;

		solar_elev_azi_ecc(latitude, longitude, time_zone, jday, time, 0, &solar_elevation, &solar_azimuth, &eccentricity_correction);
		irrad_dif = diffuse_fraction(irrad_glo, solar_elevation, eccentricity_correction)*irrad_glo;
		if (solar_elevation > 5.0)
		{
			irrad_beam_nor = (irrad_glo - irrad_dif)*1.0 / sin(DTR*solar_elevation);
		}
		else{
			irrad_beam_nor = 0;
			irrad_dif = irrad_glo;
		}
		if (irrad_beam_nor > solar_constant_e)
		{
			irrad_beam_nor = solar_constant_e;
			irrad_dif = irrad_glo - irrad_beam_nor*sin(DTR*solar_elevation);
		}
		fprintf(stdout, "%.0f %.0f\n", irrad_beam_nor, irrad_dif);

	}else{	
		HOURLY_DATA = open_input(input_weather_data);
		SHORT_TERM_DATA = open_output(input_weather_data_shortterm);

		while (EOF != fscanf(HOURLY_DATA, "%d %d %f %f", &month, &day, &time, &irrad_glo))
		{
			jday = month_and_day_to_julian_day(month, day);
			if (irrad_glo < 0 || irrad_glo > solar_constant_e)          /*  check irradiances and exit if necessary  */
				irrad_glo = solar_constant_e;

			solar_elev_azi_ecc(latitude, longitude, time_zone, jday, time, 0, &solar_elevation, &solar_azimuth, &eccentricity_correction);

			irrad_dif = diffuse_fraction(irrad_glo, solar_elevation, eccentricity_correction)*irrad_glo;
			if (solar_elevation > 5.0)
			{
				irrad_beam_nor = (irrad_glo - irrad_dif)*1.0 / sin(DTR*solar_elevation);
			}else{
				irrad_beam_nor = 0;
				irrad_dif = irrad_glo;
			}
			if (irrad_beam_nor > solar_constant_e)
			{
				irrad_beam_nor = solar_constant_e;
				irrad_dif = irrad_glo - irrad_beam_nor*sin(DTR*solar_elevation);
			}
			fprintf(SHORT_TERM_DATA, "%d %d %.3f %.0f %.0f\n", month, day, time, irrad_beam_nor, irrad_dif);
		}
		close_file(HOURLY_DATA);
		close_file(SHORT_TERM_DATA);
	}

	exit(0);

memerr:
	error(SYSTEM, "out of memory in function main");
}
