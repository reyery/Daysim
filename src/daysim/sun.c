
/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#include <rtmath.h>
#include <stdio.h>
#include <rterror.h>

#include "sun.h"


int jdate( int month, int day)		/* Julian date (days into year) */
{
    static short  mo_da[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    
    return(mo_da[month-1] + day);
}


double stadj( int jd)		/* float f_E(int day)-12 * (s_meridian - s_longitude) / PI ) solar time adjustment from Julian date */
{
    extern float s_meridian;
    extern float s_longitude;
    return( 0.170 * sin( (4*PI/373) * (jd - 80) ) -
           0.129 * sin( (2*PI/355) * (jd - 8) ) +
           12 * (s_meridian - s_longitude) / PI );
}



double sdec( int jd)		/*  solar declination angle from Julian date */
{
    return( 0.4093 * sin( (2*PI/368) * (jd - 81) ) );
}



float f_altitude(float lat,float sd,float st)
{
    return(salt(  sd,  st)) ;
}


float f_azimuth(float latitude_rad, float sd, float st,float altitude)
{
	return RTD*sazi(sd, st);
}

float solar_sunset(int month,int day)
{	float W;
    extern float s_latitude;
    W=-1*(tan(s_latitude)*tan(sdec(jdate(month, day))));
    return((PI/2 - atan2(W,sqrt(1-W*W)))*180/(PI*15)+12);
}
double sazi( double sd,  double st)	/* solar azimuth from solar declination and solar time */
{	extern float s_latitude;
    return( -atan2( cos(sd)*sin(st*(PI/12)),
                   -cos(s_latitude)*sin(sd) -
                   sin(s_latitude)*cos(sd)*cos(st*(PI/12)) ) );
}
double salt( double sd, double st)	/* solar altitude from solar declination and solar time */
{
    extern float s_latitude;
    
    return( asin( sin(s_latitude) * sin(sd) -
                 cos(s_latitude) * cos(sd) * cos(st*(PI/12)) ) );
}

double f_salt( double sd, double alt)	/* solar altitude from solar declination and solar time */
{
    extern float s_latitude;
    double E,F;
    E= sin(s_latitude) * sin(sd);
    F=cos(s_latitude) * cos(sd);
    /*return(((E-sin(alt))/F));*/
    return((12.0/PI)*acos((E-sin(alt))/F));
}	

void sunrise_sunset_localtime ( float latitude, float longitude, float time_zone, int jday,\
                                float *sunrise_localtime, float *sunset_localtime )
{ float solar_declination;
  float sunrise_solartime, sunset_solartime, time_dif;

  solar_declination = RTD * 0.4093 * sin( (2*PI/368) * (jday - 81) );
  sunrise_solartime = 12/PI * acos ( tan ( latitude*DTR ) * tan ( solar_declination*DTR ) );
  sunset_solartime = 12/PI * ( 2*PI - acos ( tan ( latitude*DTR ) * tan ( solar_declination*DTR ) ));

  time_dif = + 0.170 * sin( (4*PI/373) * (jday - 80) ) - 0.129 * sin( (2*PI/355) * (jday - 8) ) + 12/180.0 * (time_zone - longitude);

  *sunrise_localtime = sunrise_solartime - time_dif;
  *sunset_localtime = sunset_solartime - time_dif;
}

int day_to_month (int day)                    /*  gives the month to which the day belongs ( year with 365 days )  */
{
	while (day < 1) day += 365;
	day = (day - 1) % 365 + 1;
  if ( day <= 31 )  return 1;
  if ( day <= 59 )  return 2;
  if ( day <= 90 )  return 3;
  if ( day <= 120 )  return 4;
  if ( day <= 151 )  return 5;
  if ( day <= 181 )  return 6;
  if ( day <= 212 )  return 7;
  if ( day <= 243 )  return 8;
  if ( day <= 273 )  return 9;
  if ( day <= 304 )  return 10;
  if ( day <= 334 )  return 11;
  return 12;
}

int julian_day_to_day_of_month (int day)
{
	while (day < 1) day += 365;
	day = (day - 1) % 365 + 1;
  if ( day <= 31 )  return day;
  if ( day <= 59 )  return day-31;
  if ( day <= 90 )  return day-59;
  if ( day <= 120 )  return day-90;
  if ( day <= 151 )  return day-120;
  if ( day <= 181 )  return day-151;
  if ( day <= 212 )  return day-181;
  if ( day <= 243 )  return day-212;
  if ( day <= 273 )  return day-243;
  if ( day <= 304 )  return day-273;
  if ( day <= 334 )  return day-304;
  return day-334;
}

int month_and_day_to_julian_day (int month, int day)
{
  if ( month == 1 )  return day;
  if ( month == 2 )  return day+31;
  if ( month == 3 )  return day+59;
  if ( month == 4 )  return day+90;
  if ( month == 5 )  return day+120;
  if ( month == 6 )  return day+151;
  if ( month == 7 )  return day+181;
  if ( month == 8 )  return day+212;
  if ( month == 9 )  return day+243;
  if ( month == 10 )  return day+273;
  if ( month == 11 )  return day+304;
  if ( month == 12 )  return day+334;
  error(WARNING, "bad month");
  return 0;
}

int jday_first_of_month (int month)
{
  if ( month == 1 )  return 1;
  if ( month == 2 )  return 32;
  if ( month == 3 )  return 60;
  if ( month == 4 )  return 91;
  if ( month == 5 )  return 121;
  if ( month == 6 )  return 152;
  if ( month == 7 )  return 182;
  if ( month == 8 )  return 213;
  if ( month == 9 )  return 244;
  if ( month == 10 )  return 274;
  if ( month == 11 )  return 305;
  if ( month == 12 )  return 335;
  error(WARNING, "bad month");
  return 0;
}
