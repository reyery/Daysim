/*	This program has been written by Oliver Walkenhorst at the
 *	Fraunhofer Institute for Solar Energy Systems in Freiburg, Germany
 *	last changes were added in January 2001
 *
 *	Update by Nathaniel Jones at MIT, April 2016
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"

#include "paths.h"
#include "read_in.h"
#include "sun.h"
#include "numerical.h"
#include "clearsky_models.h"
#include "skartveit.h"
#include "ds_constants.h"


void  create60minTempFile();
char *header;
FILE *HEADER;               /*  header file  */
FILE *HOURLY_DATA;          /*  input weather data file  */
FILE *SHORT_TERM_DATA;      /*  input weather data shortterm file  */
FILE *SIXTYMIN_DATA;      		/*  temporary weather data shortterm file  */

/*  global variables for the header file key words and their default values */

char input_weather_data[200];
char input_weather_data_shortterm[200];   /*  default value: input_weather_data_shortterm = input_weather_data."shortterm_timestep"min  */
char temp_file[200]="";   /*  default value: input_weather_data_shortterm = input_weather_data."shortterm_timestep"min  */
int shortterm_timestep=60;                /*  in minutes  */
int input_units_genshortterm;
int output_units_genshortterm=1;
int input_timestep=60;                /*  in minutes  */
int test_input_time_step;
int solar_time=0;                     /*  0=LST ; 1=solar time  */
long random_seed=-10;                 /*  seed for the pseudo-random-number generator, random_seed has to be a negative integer  */
int new=1;

/*  global variables for the header file key words representing station specific data  */

double latitude;
double longitude;
double time_zone;
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

char keyword[200]="";			// strings for the header
char header_line_1[300]="";	//=======================
char header_line_2[300]="";
char header_line_3[300]="";
char header_line_4[300]="";
char header_line_5[300]="";
char header_line_6[300]="";

/*  main program  */

int main(int argc, char *argv[])
{
	int i, j, k;
	int nh=0;              /*  counter for the total number of hours  */
	int nhd=0;             /*  counter for the number of hours of the actual day  */
	int month=0, day=0, jday=0, jday_hoy=0, last_day=0, last_month=0, last_jday=0;
	int status=6;             /*  indicates EOF of the input weather data file  */
	int fatal=0;              /*  indicates soon exit due to fatal error        */
	int azimuth_class=0;
	int *daylight_status;     /*  0=night hour, 1=sunrise/sunset hour, 2=innerday hour  */

	double time, centrum_time, *times;
	double irrad_glo = 0.0, irrad_beam_nor, irrad_beam_hor, irrad_dif;     /* in W/m² */
	double *irrads_glo, *irrads_beam_nor, *irrads_dif, *indices_glo, *indices_beam, sr_ss_indices_glo[3];
	double *irrads_glo_st, *irrads_glo_clear_st, *irrads_beam_nor_st, *irrads_dif_st, *indices_glo_st;
	double time_t, time_k, mean_glo_st, mean_beam_st, mean_dif_st, sum_beam_nor, sum_beam_hor, sum_dif;
	double sunrise_localtime, sunset_localtime;
	double solar_elevation, solar_azimuth, eccentricity_correction;
	double punk;              /*  indicates nice sound  */
	double previous_ligoh = 0, actual_ligoh;
	/*  ligoh = last index_glo of an hour: introduced to minimize discontinuities between subsequent hours  */

	
	
	if (argc == 1) {
		char *progname = fixargv0(argv[0]);
		fprintf(stdout, "%s: fatal error -  header file missing\n", progname);
		fprintf(stdout, "start program with:  %s  <header file>\n ", progname);
		exit(1);
	}

	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

	header=argv[1];
	read_in_genshortterm_header();

	



	/*printf("input_weather_data=%s\n",input_weather_data);
	  printf("input_weather_data_shortterm=%s\n",input_weather_data_shortterm);
	  printf("shortterm_timestep=%d\n",shortterm_timestep);
	  printf("input_units_genshortterm=%d\n",input_units_genshortterm);
	  printf("output_units_genshortterm=%d\n",output_units_genshortterm);
	  printf("solar_time=%d\n",solar_time);
	  printf("latitude=%f \nlongitude=%f \ntime_zone=%f\n",latitude,longitude,time_zone);
	  printf("site_elevation=%f\n",site_elevation);
	  printf("horizon_in=%d horizon_data_in=%s\n",horizon_in,horizon_data_in);
	  printf("horizon_out=%d horizon_data_out=%s\n",horizon_out,horizon_data_out);
	  printf("linke_estimation=%d\n",linke_estimation);*/

	HOURLY_DATA = open_input(input_weather_data);
	/* added by C. Reinhart                   */
	/* test whether input file has a header   */
	/* in case the is a header, it is skipped */
	fscanf(HOURLY_DATA,"%s", keyword);
  	if( !strcmp(keyword,"place") ){
		rewind(HOURLY_DATA);
		fgets(header_line_1,300,HOURLY_DATA);
		fgets(header_line_2,300,HOURLY_DATA);
		fgets(header_line_3,300,HOURLY_DATA);
		fgets(header_line_4,300,HOURLY_DATA);
		fgets(header_line_5,300,HOURLY_DATA);
		fgets(header_line_6,300,HOURLY_DATA);
		// get time step of input file
		fscanf(HOURLY_DATA,"%d %d %f", &month, &day, &centrum_time);fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
		fscanf(HOURLY_DATA,"%d %d %f", &month, &day, &time);fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
		test_input_time_step=(int)(60.0*fabs(time-centrum_time));
		//printf("The time step of the input file (%s) is %d minutes\n",input_weather_data, test_input_time_step);
		if(test_input_time_step != shortterm_timestep && test_input_time_step != 60 ){
		 	printf("wea_data_file does not have a 60 minute time step interval! %d\n",test_input_time_step);
		}
		rewind(HOURLY_DATA);
		fgets(header_line_1,300,HOURLY_DATA);
		fgets(header_line_2,300,HOURLY_DATA);
		fgets(header_line_3,300,HOURLY_DATA);
		fgets(header_line_4,300,HOURLY_DATA);
		fgets(header_line_5,300,HOURLY_DATA);
		fgets(header_line_6,300,HOURLY_DATA);
	}else{	// input file has no header
		rewind(HOURLY_DATA);
		// get time step of input file
		fscanf(HOURLY_DATA,"%d %d %f", &month, &day, &centrum_time);fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
		fscanf(HOURLY_DATA,"%d %d %f", &month, &day, &time);fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
		test_input_time_step=(int)(60.0*fabs(time-centrum_time));
		fprintf(stderr,"The time step of the inpt file (%s) is %d minutes\n",input_weather_data, test_input_time_step);
		if(test_input_time_step != shortterm_timestep && test_input_time_step != 60 ){
		 	fprintf(stderr,"wea_data_file does not have a 60 minute time step interval! %d\n",test_input_time_step);
		}
		rewind(HOURLY_DATA);
		sprintf(header_line_1,"place %s\n",input_weather_data);
		sprintf(header_line_2,"latitude %f\n",latitude);
		sprintf(header_line_3,"longitude %f\n",longitude);
		sprintf(header_line_4,"time_zone %f\n",time_zone);
		sprintf(header_line_5,"site_elevation %f\n",site_elevation);
		sprintf(header_line_6,"weather_data_file_units %d\n",output_units_genshortterm);

	}


	SHORT_TERM_DATA = open_output(input_weather_data_shortterm);
    //print file header
  	fprintf(SHORT_TERM_DATA,"%s", header_line_1);
  	fprintf(SHORT_TERM_DATA,"%s", header_line_2);
  	fprintf(SHORT_TERM_DATA,"%s", header_line_3);
  	fprintf(SHORT_TERM_DATA,"%s", header_line_4);
  	fprintf(SHORT_TERM_DATA,"%s", header_line_5);
  	fprintf(SHORT_TERM_DATA,"%s", header_line_6);


	if ((times = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((irrads_glo = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((irrads_beam_nor = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((irrads_dif = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((indices_glo = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((indices_beam = malloc(24 * sizeof(double))) == NULL) goto memerr;
	if ((daylight_status = malloc(24 * sizeof(int))) == NULL) goto memerr;

	if ((irrads_glo_st = malloc(sph*sizeof(double))) == NULL) goto memerr;
	if ((irrads_glo_clear_st = malloc(sph*sizeof(double))) == NULL) goto memerr;
	if ((irrads_beam_nor_st = malloc(sph*sizeof(double))) == NULL) goto memerr;
	if ((irrads_dif_st = malloc(sph*sizeof(double))) == NULL) goto memerr;
	if ((indices_glo_st = malloc(sph*sizeof(double))) == NULL) goto memerr;


	if ( shortterm_timestep == test_input_time_step )      /*  no generation of shortterm data, but conversion of direct-hor to direct-norm irradiance  */
		{
			//fprintf(stderr,"ds_shortterm: message: input time step equals output time step.\n");
			while ( status > 0 )                       /*  as long as EOF is not reached  */
				{
					if ( input_units_genshortterm == 1 )
					{
						status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_nor, &irrad_dif);
						if(irrad_beam_nor<0|| irrad_dif<0)
							{
								status=-1;
								printf("FATAL ERROR: Negative direct or diffuse irradiance at month: %d, day: %d, time %.1f\n",month,day,time);
								printf("Generating cliamte file stopped.\n");
							}
					}
					if ( input_units_genshortterm == 2 )
					{
						status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_hor, &irrad_dif);
						if(irrad_beam_hor<0|| irrad_dif<0)
							{
								status=-1;
								printf("FATAL ERROR: Negative direct or diffuse irradiance at month: %d, day: %d, time %.1f\n",month,day,time);
								printf("Generating cliamte file stopped.\n");
							}
					}
					if ( status <= 0 )  goto end;

					if ( input_units_genshortterm == 2 )                             /*  calculate irrad_beam_nor  */
						{
							if ( irrad_beam_hor > 0 )
								{
									jday = jdate(month, day);
									sunrise_sunset_localtime ( latitude, longitude, time_zone, jday, &sunrise_localtime, &sunset_localtime );
									centrum_time=time;
									if ( fabs(time-sunrise_localtime) <= 0.5 )  centrum_time=sunrise_localtime+(time+0.5-sunrise_localtime)/2.0;
									if ( fabs(time-sunset_localtime) <= 0.5 )  centrum_time=time-0.5+(sunset_localtime-(time-0.5))/2.0;
									solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, centrum_time, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);
									irrad_beam_nor = irrad_beam_hor / sin(radians(solar_elevation));
									if ( irrad_beam_nor < 0 )  irrad_beam_nor=0;
								}
							else irrad_beam_nor=0;
						}
					//}

					fprintf(SHORT_TERM_DATA,"%d %d %.3f %.0f %.0f\n", month, day, time, irrad_beam_nor, irrad_dif);
				}
		}


	else                                 /*  generation of 1-min short-term data according to modified Skartveit & Olseth  */
		{                                    /*  by Oliver Walkenhorst, November 2000                                          */

			if ( test_input_time_step < 60 )create60minTempFile();
			if ( horizon_in )
				{
					printf("reads in input horizon data ... \n");
					read_horizon_azimuth_data ( horizon_data_in, &horizon_azimuth_in[0] );
				}
			else  for ( i=0 ; i<36 ; i++ )  horizon_azimuth_in[i]=0;

			if ( horizon_out )
				{
					printf("reads in output horizon data ... \n");
					read_horizon_azimuth_data ( horizon_data_out, &horizon_azimuth_out[0] );
				}
			else  for ( i=0 ; i<36 ; i++ )  horizon_azimuth_out[i]=0;

			if ( linke_estimation )
				{
					//printf("\nEstimating monthly Linke Turbidities from hourly direct irradiances ... ");
					estimate_linke_factor_from_hourly_direct_irradiances();
					//for (i=0;i<12;i++)  printf("%.1f ",linke_turbidity_factor_am2[i]);
					//printf(" ");
				}

			rewind(HOURLY_DATA);
			/* added by C. Reinhart                   */
			/* test whether input file has a header   */
			/* in case the is a header, it is skipped */
			fscanf(HOURLY_DATA,"%s", keyword);
			if( !strcmp(keyword,"place") )
				{
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
					fscanf(HOURLY_DATA,"%*[^\n]");fscanf(HOURLY_DATA,"%*[\n\r]");
				}else{
					rewind(HOURLY_DATA);
				}



			while ( status > 0 )               /*  read data from the input weather file as long as EOF is not reached  */
				{
					if ( input_units_genshortterm == 1 )
						status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_nor, &irrad_dif);
					if ( input_units_genshortterm == 2 )
						status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_hor, &irrad_dif);
					if ( status <= 0 )  goto process_last_day;

					nh++;

					jday = jdate(month, day);

					if ( input_units_genshortterm == 1 )         /*  calculation of the global irradiance for the actual hour  */
						{
							if ( irrad_beam_nor > 0 )
								{
									sunrise_sunset_localtime ( latitude, longitude, time_zone, jday, &sunrise_localtime, &sunset_localtime );
									centrum_time=time;
									if ( fabs(time-sunrise_localtime) <= 0.5 )  centrum_time=sunrise_localtime+(time+0.5-sunrise_localtime)/2.0;
									if ( fabs(time-sunset_localtime) <= 0.5 )  centrum_time=time-0.5+(sunset_localtime-(time-0.5))/2.0;
									solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, centrum_time, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);
									irrad_beam_hor = irrad_beam_nor * sin(radians(solar_elevation));
									if ( irrad_beam_hor < 0 )  irrad_beam_hor=0;
								}
							else irrad_beam_hor=0;

							irrad_glo=irrad_beam_hor+irrad_dif;
						}

					if ( input_units_genshortterm == 2 )         /*  calculation of the global irradiance for the actual hour  */
						{
							if ( irrad_beam_hor > 0 )
								{
									sunrise_sunset_localtime ( latitude, longitude, time_zone, jday, &sunrise_localtime, &sunset_localtime );
									centrum_time=time;
									if ( fabs(time-sunrise_localtime) <= 0.5 )  centrum_time=sunrise_localtime+(time+0.5-sunrise_localtime)/2.0;
									if ( fabs(time-sunset_localtime) <= 0.5 )  centrum_time=time-0.5+(sunset_localtime-(time-0.5))/2.0;
									solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, centrum_time, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);
									irrad_beam_nor = irrad_beam_hor / sin(radians(solar_elevation));
									if ( irrad_beam_nor < 0 )  irrad_beam_nor=0;
								}
							else irrad_beam_nor=0;

							irrad_glo=irrad_beam_hor+irrad_dif;
						}


					/*  check irradiances and correct numbers if necessary  */
					if ( irrad_glo < 0 )
						{
							sprintf(errmsg, "irrad_glo=%f at month: %d day: %d time: %.3f has been replaced with 0", irrad_glo, month, day, time);
							error(WARNING, errmsg);
							irrad_glo=0.0;
						}
					if (irrad_glo > SOLAR_CONSTANT_E)
						{
							sprintf(errmsg, "irrad_glo=%f at month: %d day: %d time: %.3f has been replaced with %f\n", irrad_glo, month, day, time, SOLAR_CONSTANT_E);
							error(WARNING, errmsg);
							irrad_glo = SOLAR_CONSTANT_E;
						}

					if ( irrad_beam_nor < 0 )
						{
							sprintf(errmsg, "irrad_beam_nor=%e at month: %d day: %d time: %.3f has been replaced with %f\n", irrad_beam_nor, month, day, time, 0.0);
							error(WARNING, errmsg);
							irrad_beam_nor = 0.0;
						}
					if (irrad_beam_nor > SOLAR_CONSTANT_E)
						{
							sprintf(errmsg, "irrad_beam_nor=%f at month: %d day: %d time: %.3f has been replaced with %f\n", irrad_beam_nor, month, day, time, SOLAR_CONSTANT_E);
							error(WARNING, errmsg);
							irrad_beam_nor = SOLAR_CONSTANT_E;
						}

					if ( irrad_dif < 0 )
						{
							sprintf(errmsg, "irrad_dif=%f at month: %d day: %d time: %.3f has been replaced with %f\n", irrad_beam_nor, month, day, time, 0.0);
							error(WARNING, errmsg);
							irrad_dif = 0.0;
						}
					if (irrad_dif > SOLAR_CONSTANT_E)
						{
							sprintf(errmsg, "irrad_dif=%f at month: %d day: %d time: %.3f has been replaced with %f\n", irrad_beam_nor, month, day, time, SOLAR_CONSTANT_E);
							error(WARNING, errmsg);
							irrad_dif = SOLAR_CONSTANT_E;
						}

					if ( fatal == 1 )  exit(1);


					if ( last_jday == jday || nh == 1 )      /*  store the hourly irradiances of the actual day  */
						{
							times[nhd]=time;
							irrads_glo[nhd] = irrad_glo;
							irrads_beam_nor[nhd] = irrad_beam_nor;
							irrads_dif[nhd] = irrad_dif;
							nhd++;
						}

					else
						{
						process_last_day:                                     /*  process the last day  */
							{
								for ( i=0 ; i<nhd ; i++ )              /*  determine the daylight status of each hour  */
									{
										if ( i == 0 || i == nhd-1 )
											{
												if ( irrads_glo[i] < 0.001 )  daylight_status[i]=0;
												if ( irrads_glo[i] >= 0.001 )  daylight_status[i]=1;
											}
										if ( i > 0 && i<nhd-1 )
											{
												if ( irrads_glo[i-1] < 0.001 && irrads_glo[i] < 0.001 )  daylight_status[i]=0;
												if ( irrads_glo[i] < 0.001 && irrads_glo[i+1] < 0.001 )  daylight_status[i]=0;
												if ( irrads_glo[i-1] >= 0.001 && irrads_glo[i] < 0.001 && irrads_glo[i+1] >= 0.001 )
												{
													irrads_glo[i]=0.5*(irrads_glo[i-1]+irrads_glo[i+1]);
													sprintf(errmsg, "at %d %d %.3f global irradiance = 0 in between two hours with\n non-vanishing global irradiance: check your data and try again", last_month, last_day, times[i]);
													error(WARNING, errmsg);
												}

												if ( irrads_glo[i-1] < 0.001 && irrads_glo[i] >= 0.001 && irrads_glo[i+1] < 0.001 )
												{
													irrads_glo[i]=0.5*(irrads_glo[i-1]+irrads_glo[i+1]);
													sprintf(errmsg, "month=%d day=%d contains only one hour with non-vanishing global irradiance: remove this day from your input data file and try again", last_month, last_day);
													error(WARNING, errmsg);
												}
												if ( irrads_glo[i-1] < 0.001 && irrads_glo[i] >= 0.001 && irrads_glo[i+1] >= 0.001 )  daylight_status[i]=1;
												if ( irrads_glo[i-1] >= 0.001 && irrads_glo[i] >= 0.001 && irrads_glo[i+1] < 0.001 )  daylight_status[i]=1;
												if ( irrads_glo[i-1] >= 0.001 && irrads_glo[i] >= 0.001 && irrads_glo[i+1] >= 0.001 )  daylight_status[i]=2;
											}

										if ( daylight_status[i] > 0 )        /*  calculate the clearness indices  */
											{
												glo_and_beam_indices_hour ( latitude, longitude, time_zone, last_jday, times[i], solar_time, irrads_glo[i], irrads_beam_nor[i], &indices_glo[i], &indices_beam[i] );
												if ( i < nhd-1 && times[i+1]-times[i] > 1.5 )
												{
													sprintf(errmsg, "at %d %d %.3f the time difference to the subsequent hour is greater than 1.5: check your data and try again (innerday time differences should equal 1)", last_month, last_day, times[i]);
													error(USER, errmsg);
												}
											}
									}

								for ( i=0 ; i<nhd ; i++ )                /*  process each hour  */
									{
										if ( daylight_status[i] == 0 )         /*  print zeros for hours without global irradiance  */
											{
												for ( j=1 ; j<=(60/shortterm_timestep) ; j++ )
													{
														time_t = times[i] - 0.5 + ( j - 0.5 ) / (60/shortterm_timestep);
														if ( output_units_genshortterm == 1 )
															fprintf ( SHORT_TERM_DATA,"%d %d %.3f %.0f %.0f\n", last_month, last_day, time_t, 0.0, 0.0 );
														if ( output_units_genshortterm == 2 )
															{
																jday_hoy = jdate(last_month, last_day);
																solar_elev_azi_ecc (latitude, longitude, time_zone, jday_hoy, time_t, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);

																fprintf ( SHORT_TERM_DATA,"%d %d %.3f %.0f %.0f\n", last_month, last_day, time_t, 0.0, 0.0 );
															}
													}
											}

										else                                  /*  generate short-term irradiances for daylight hours  */
											{
												irrads_clear_st ( latitude, longitude, time_zone, last_jday, times[i], solar_time, sph, irrads_glo_clear_st);

												if ( daylight_status[i] == 1 && times[i] < 12 )             /*  sunrise hour  */
													{
														sr_ss_indices_glo[0] = indices_glo[i+1];
														sr_ss_indices_glo[1] = indices_glo[i];
														sr_ss_indices_glo[2] = indices_glo[i+1];
														skartveit ( sr_ss_indices_glo, indices_beam[i], sph, previous_ligoh, indices_glo_st, &actual_ligoh );
													}

												if ( daylight_status[i] == 1 && times[i] >= 12 )            /*  sunset hour  */
													{
														sr_ss_indices_glo[0] = indices_glo[i-1];
														sr_ss_indices_glo[1] = indices_glo[i];
														sr_ss_indices_glo[2] = indices_glo[i-1];
														skartveit ( sr_ss_indices_glo, indices_beam[i], sph, previous_ligoh, indices_glo_st, &actual_ligoh );
													}

												if ( daylight_status[i] == 2 )                             /*  innerday hours  */
													{
														if ( irrads_glo[i] <= 0.001 )
															{
																irrads_glo[i]=0.5*(irrads_glo[i-1]+irrads_glo[i+1]);
																sprintf(errmsg, "at month=%d day=%d time=%.3f should be non-vanishing global irradiance: check your input file and try again", last_month, last_day, times[i]);
																error(USER, errmsg);
															}
														else  skartveit ( &indices_glo[i-1], indices_beam[i], sph, previous_ligoh, indices_glo_st, &actual_ligoh );
													}

												previous_ligoh = actual_ligoh;

												for ( j=1 ; j<=sph ; j++ )  irrads_glo_st[j-1] = indices_glo_st[j-1] * irrads_glo_clear_st[j-1];

												mean_glo_st = mean ( sph, &irrads_glo_st[0] );

												if ( mean_glo_st > 0 )                /*  global renormalization to the given hourly mean value  */
													for ( j=1 ; j<=sph ; j++ )
														irrads_glo_st[j-1] = irrads_glo[i] / mean_glo_st * irrads_glo_st[j-1];

												for (j = 1; j <= sph; j++)  if (irrads_glo_st[j - 1] > SOLAR_CONSTANT_E)  irrads_glo_st[j - 1] = SOLAR_CONSTANT_E;

												for ( j=1 ; j<=sph ; j++ )        /*  Reindl diffuse fraction estimation  */
													{
														solar_elev_azi_ecc (latitude, longitude, time_zone, last_jday, times[i]-0.5+(j-0.5)/sph, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);
														irrads_dif_st[j-1]= diffuse_fraction(irrads_glo_st[j-1],solar_elevation,eccentricity_correction)*irrads_glo_st[j-1];

														if ( solar_azimuth < 0 )  azimuth_class = ((int)solar_azimuth)/10 + 17;
														else   azimuth_class = ((int)solar_azimuth)/10 + 18;

														if ( solar_elevation > horizon_azimuth_out[azimuth_class] )
															irrads_beam_nor_st[j - 1] = (irrads_glo_st[j - 1] - irrads_dif_st[j - 1]) / sin(radians(solar_elevation));
														else
															{
																irrads_beam_nor_st[j-1]=0;
																irrads_dif_st[j-1]=irrads_glo_st[j-1];
															}

														if (irrads_beam_nor_st[j - 1] > SOLAR_CONSTANT_E)  irrads_beam_nor_st[j - 1] = SOLAR_CONSTANT_E;
													}

												mean_beam_st = mean ( sph, &irrads_beam_nor_st[0] );
												mean_dif_st = mean ( sph, &irrads_dif_st[0] );

												if ( mean_beam_st > 0 )        /*  beam renormalization to the given hourly mean value  */
													for ( j=1 ; j<=sph ; j++ )
														irrads_beam_nor_st[j-1] = irrads_beam_nor[i] / mean_beam_st * irrads_beam_nor_st[j-1];


												if ( daylight_status[i] == 1 ) { /*  Tito  */
													k=0;
													for ( j=1 ; j<=sph ; j++ ){
														if( irrads_dif_st[j-1]>0.01){ k++;}
													}
													if( (k+30) < 60 )
														irrads_dif[i]*=(k*1.0/(k+30.0));
												}

												if ( mean_dif_st > 0 )         /*  diffuse renormalization to the given hourly mean value  */
													for ( j=1 ; j<=sph ; j++ )
														irrads_dif_st[j-1] = irrads_dif[i] / mean_dif_st * irrads_dif_st[j-1];

												for ( j=1 ; j<=sph ; j++ )
													if (irrads_beam_nor_st[j - 1] > SOLAR_CONSTANT_E)  irrads_beam_nor_st[j - 1] = SOLAR_CONSTANT_E;

												for ( j=1 ; j<=(60/shortterm_timestep) ; j++ )
													{
														time_t = times[i] - 0.5 + ( j - 0.5 ) / (60/shortterm_timestep);
														if ( shortterm_timestep == 1 )
															{
																if ( output_units_genshortterm == 1 )
																	fprintf ( SHORT_TERM_DATA,"%d %d %.3f %.0f %.0f\n", last_month, last_day, time_t, irrads_beam_nor_st[j-1], irrads_dif_st[j-1] );
																if ( output_units_genshortterm == 2 )
																	{
																		solar_elev_azi_ecc (latitude, longitude, time_zone, last_jday, time_t, solar_time,&solar_elevation, &solar_azimuth, &eccentricity_correction);
																		/*if ( solar_elevation < 0 )  solar_elevation=0;*/
																		punk = solar_elevation;
																		if ( solar_elevation < 0 )  punk=0;
																		fprintf(SHORT_TERM_DATA, "%.3f %.0f %.0f %.3f %.3f\n", (last_jday - 1) * 24 + time_t, irrads_beam_nor_st[j - 1] * sin(radians(punk)), irrads_dif_st[j - 1], solar_elevation, solar_azimuth);
																	}
															}
														else
															{
																if ( output_units_genshortterm == 1 )
																	{
																		sum_beam_nor=0;
																		sum_dif=0;
																		for ( k=(j-1)*shortterm_timestep ; k<j*shortterm_timestep ; k++ )
																			{
																				sum_beam_nor+=irrads_beam_nor_st[k];
																				sum_dif+=irrads_dif_st[k];
																			}
																		fprintf ( SHORT_TERM_DATA,"%d %d %.3f %.0f %.0f\n",last_month, last_day, time_t, sum_beam_nor/shortterm_timestep, sum_dif/shortterm_timestep );
																	}

																if ( output_units_genshortterm == 2 )
																	{
																		sum_beam_hor=0;
																		sum_dif=0;
																		for ( k=(j-1)*shortterm_timestep ; k<j*shortterm_timestep ; k++ )
																			{
																				time_k = times[i] - 0.5 + ( k + 0.5 ) / 60;
																				solar_elev_azi_ecc (latitude, longitude, time_zone, last_jday, time_k, solar_time,&solar_elevation, &solar_azimuth, &eccentricity_correction);
																				if ( solar_elevation < 0 )  solar_elevation=0;
																				sum_beam_hor += irrads_beam_nor_st[k] * sin(radians(solar_elevation));
																				sum_dif+=irrads_dif_st[k];
																			}
																		fprintf ( SHORT_TERM_DATA,"%.3f %.0f %.0f\n", (last_jday-1)*24+time_t, sum_beam_hor/shortterm_timestep, sum_dif/shortterm_timestep );
																	}
															}
													}
											}
									}

								if ( status <= 0 )  goto end;

								times[0]=time;
								irrads_glo[0] = irrad_glo;
								irrads_beam_nor[0] = irrad_beam_nor;
								irrads_dif[0] = irrad_dif;
								nhd=1;
							}
						}

					last_day=day;
					last_month=month;
					last_jday=jday;
				}
		}

 end:
	{
		close_file(HOURLY_DATA);
		close_file(SHORT_TERM_DATA);
	}
	if (day!=31 || month !=12)
	{
		printf("WARNING - Incomplete input climate file (%s)! The file ends on month %d and day %d.\n",input_weather_data,month,day);
		printf("Please review the output file before proceedingto Step3 using SITE>>OPEN CLIMATE FILE IN TEXT EDITOR.\n");
	} else
	{
			printf("DS_SHORTTERM - A climate file with a time step of %d minutes has been generated under %s.\n\n",time_step,input_weather_data_shortterm);

	}
	return 0;
memerr:
	error(SYSTEM, "out of memory in function main");
}
