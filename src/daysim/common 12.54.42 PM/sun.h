/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#ifndef SUN_H
#define SUN_H


int    jdate( int month, int day);     /* Julian date (days into year) */
double stadj( int jd);                 /* solar time adjustment from Julian date */
double sdec( int jd);                  /* solar declination angle from Julian date */
float  f_altitude(float lat,float sd,float st);
float  f_azimuth(float latitude_rad, float sd, float st,float altitude);
float  solar_sunset(int month,int day);
double sazi( double sd,  double st);	 /* solar azimuth from solar declination and solar time */
double salt( double sd, double st);	   /* solar altitude from solar declination and solar time */
double f_salt( double sd, double alt); /* solar altitude from solar declination and solar time */





#endif
