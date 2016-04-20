#ifndef DS_SHORTTERM_H
#define DS_SHORTTERM_H

#include <stdio.h>

#include "read_in_header.h"

#include "sun.h"



extern char *header;
extern FILE *HEADER;               /*  header file  */
extern FILE *HOURLY_DATA;          /*  input weather data file  */
extern FILE *SHORT_TERM_DATA;      /*  input weather data shortterm file  */
extern FILE *SIXTYMIN_DATA;      		/*  temporary weather data shortterm file  */
   
/*  global variables for the header file key words and their default values */

extern char input_weather_data[200];
extern char input_weather_data_shortterm[200];   /*  default value: input_weather_data_shortterm = input_weather_data."shortterm_timestep"min  */
extern char temp_file[200];   /*  default value: input_weather_data_shortterm = input_weather_data."shortterm_timestep"min  */
extern int shortterm_timestep;                /*  in minutes  */
extern int input_units_genshortterm;
extern int output_units_genshortterm;
extern int input_timestep;                /*  in minutes  */
extern int test_input_time_step;
extern int solar_time;                     /*  0=LST ; 1=solar time  */
extern long random_seed;                 /*  seed for the pseudo-random-number generator, random_seed has to be a negative integer  */
extern int new;

/*  global variables for the header file key words representing station specific data  */

extern float latitude;
extern float longitude;
extern float time_zone;
extern float linke_turbidity_factor_am2[12];       /*  monthly means for jan-dec  */
extern char horizon_data_in[200];            /*  name of the horizon data file for the station where the input irradiance data were collected  */
/*  (the file contains 36 horizon heights in degrees starting from N to E)                        */
extern char horizon_data_out[200];	      /*  name of the horizon data file for the location the output irradiance data are computed for    */
/*  (the file contains 36 horizon heights in degrees starting from N to E)                        */			      

/*  other global variables  */

extern int sph;                               /*  sph=steps per hour: if shortterm_timestep < 60 1-min-data are generated  */
extern int horizon_in;                         /*  indicates if an input horizon data file is specified   */
extern int horizon_out;                        /*  indicates if an output horizon data file is specified  */
extern float horizon_azimuth_in[36];             /*  divide [-180°,180°] of the input horizon in 36 azimuth classes  */
/*  (south=0°, horizon heights in degrees)                          */
extern float horizon_azimuth_out[36];            /*  divide [-180°,180°] of the output horizon in 36 azimuth classes */
/*  (south=0°, horizon heights in degrees)                          */
extern int linke_estimation;                   /*  flag that indicates if estimation of the monthly linke factors is necessary  */
 
/*  constants used  */

extern char keyword[200];			// strings for the header

extern char header_line_1[300];	//=======================
extern char header_line_2[300];
extern char header_line_3[300];
extern char header_line_4[300];
extern char header_line_5[300];
extern char header_line_6[300];




#endif
