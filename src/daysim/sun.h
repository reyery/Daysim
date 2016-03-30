
/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#pragma once

#ifndef PI
#include <rtmath.h>
#endif

#define DTR (PI/180.0)
#define RTD (180.0/PI)

int    jdate( int month, int day);     /* Julian date (days into year) */
double stadj( int jd);                 /* solar time adjustment from Julian date */
double sdec( int jd);                  /* solar declination angle from Julian date */
float  f_altitude(float lat,float sd,float st);
float  f_azimuth(float latitude_rad, float sd, float st,float altitude);
float  solar_sunset(int month,int day);
double sazi( double sd,  double st);	 /* solar azimuth from solar declination and solar time */
double salt( double sd, double st);	   /* solar altitude from solar declination and solar time */
double f_salt( double sd, double alt); /* solar altitude from solar declination and solar time */

void solar_elev_azi_ecc ( float latitude, float longitude, float time_zone, int jday, float time, int solar_time, \
                          float *solar_elevation, float *solar_azimuth, float *eccentricity_correction);

void sunrise_sunset_localtime ( float latitude, float longitude, float time_zone, int jday,\
                                float *sunrise_localtime, float *sunset_localtime );	

int day_to_month (int day);

int julian_day_to_day_of_month (int day);

int month_and_day_to_julian_day (int month, int day);

int jday_first_of_month (int month);
