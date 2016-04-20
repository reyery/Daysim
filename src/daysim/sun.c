
/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#include <rtmath.h>
#include <stdio.h>
#include <rterror.h>

#include "sun.h"
#include "ds_constants.h"


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
	return degrees(sazi(sd, st));
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

void solar_elev_azi_ecc(double latitude, double longitude, double time_zone, int jday, double time, int solar_time, double *solar_elevation, double *solar_azimuth, double *eccentricity_correction)

/*  angles in degrees, times in hours  */
{
	double sol_time;
	double solar_declination, jday_angle;

	/*  solar elevation and azimuth formulae from sun.c  */
	if (solar_time)   sol_time = time;
	else   sol_time = time + 0.170 * sin((4 * PI / 373) * (jday - 80)) - 0.129 * sin((2 * PI / 355) * (jday - 8)) + 12 / 180.0 * (time_zone - longitude);

	solar_declination = degrees(0.4093 * sin((2 * PI / 368) * (jday - 81)));
	jday_angle = 2 * PI*(jday - 1) / 365;

	*solar_elevation = degrees(asin(sin(radians(latitude)) * sin(radians(solar_declination)) - cos(radians(latitude)) * cos(radians(solar_declination)) * cos(sol_time*(PI / 12))));

	*solar_azimuth = degrees(-atan2(cos(radians(solar_declination)) * sin(sol_time*(PI / 12)),
		-cos(radians(latitude))*sin(radians(solar_declination)) -
		sin(radians(latitude))*cos(radians(solar_declination))*cos(sol_time*(PI / 12))));

	/*  eccentricity_correction formula used in genjdaylit.c */

	*eccentricity_correction = 1.00011 + 0.034221*cos(jday_angle) + 0.00128*sin(jday_angle) + 0.000719*cos(2 * jday_angle) + 0.000077*sin(2 * jday_angle);
}

double diffuse_fraction(double irrad_glo, double solar_elevation, double eccentricity_correction)
{
	/*  estimation of the diffuse fraction according to Reindl et al., Solar Energy, Vol.45, pp.1-7, 1990  = [Rei90]  */
	/*                        (reduced form without temperatures and humidities)                                      */

	double irrad_ex;
	double index_glo_ex;
	double dif_frac;

	if (solar_elevation > 0)  irrad_ex = SOLAR_CONSTANT_E * eccentricity_correction * sin(radians(solar_elevation));
	else irrad_ex = 0;

	if (irrad_ex <= 0) return 0;
	index_glo_ex = irrad_glo / irrad_ex;

	if (index_glo_ex < 0)  { error(INTERNAL, "negative irrad_glo in diffuse_fraction"); }
	if (index_glo_ex > 1)  { index_glo_ex = 1; }

	if (index_glo_ex <= 0.3)
	{
		dif_frac = 1.02 - 0.254*index_glo_ex + 0.0123*sin(radians(solar_elevation));
		if (dif_frac > 1)  { dif_frac = 1; }
	}
	else if (index_glo_ex < 0.78)
	{
		dif_frac = 1.4 - 1.749*index_glo_ex + 0.177*sin(radians(solar_elevation));
		if (dif_frac > 0.97)  { dif_frac = 0.97; }
		else if (dif_frac < 0.1)   { dif_frac = 0.1; }
	}
	else
	{
		dif_frac = 0.486*index_glo_ex - 0.182*sin(radians(solar_elevation));
		if (dif_frac < 0.1)  { dif_frac = 0.1; }
	}

	return dif_frac;
}

void sunrise_sunset_localtime(double latitude, double longitude, double time_zone, int jday, double *sunrise_localtime, double *sunset_localtime)
{ double solar_declination;
  double sunrise_solartime, sunset_solartime, time_dif;

  solar_declination = degrees(0.4093 * sin((2 * PI / 368) * (jday - 81)));
  sunrise_solartime = 12 / PI * acos(tan(radians(latitude)) * tan(radians(solar_declination)));
  sunset_solartime = 12 / PI * (2 * PI - acos(tan(radians(latitude)) * tan(radians(solar_declination))));

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
