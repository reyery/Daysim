/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */


#include <math.h>

#include "sun.h"


int jdate( int month, int day)		/* Julian date (days into year) */
{
	static short  mo_da[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	
	return(mo_da[month-1] + day);
}


double stadj( int jd)		/* float f_E(int day)-12 * (s_meridian - s_longitude) / M_PI ) solar time adjustment from Julian date */
{	
	extern float s_meridian;
	extern float s_longitude;
	return( 0.170 * sin( (4*M_PI/373) * (jd - 80) ) -
		0.129 * sin( (2*M_PI/355) * (jd - 8) ) +
		12 * (s_meridian - s_longitude) / M_PI );
}



double sdec( int jd)		/*  solar declination angle from Julian date */
{
	return( 0.4093 * sin( (2*M_PI/368) * (jd - 81) ) );
}



float f_altitude(float lat,float sd,float st)
{
	return(salt(  sd,  st)) ;
}


float f_azimuth(float latitude_rad, float sd, float st,float altitude)
{
	float azi=0;
	azi = (180/M_PI)*sazi(  sd,   st);
	return(azi);
}

float solar_sunset(int month,int day)
{	float W;
	extern float s_latitude;
	W=-1*(tan(s_latitude)*tan(sdec(jdate(month, day))));
	return((M_PI/2 - atan2(W,sqrt(1-W*W)))*180/(M_PI*15)+12);
}
double sazi( double sd,  double st)	/* solar azimuth from solar declination and solar time */
{	extern float s_latitude;
	return( -atan2( cos(sd)*sin(st*(M_PI/12)),
 			-cos(s_latitude)*sin(sd) -
 			sin(s_latitude)*cos(sd)*cos(st*(M_PI/12)) ) );
}
double salt( double sd, double st)	/* solar altitude from solar declination and solar time */
{
	extern float s_latitude;
	
	return( asin( sin(s_latitude) * sin(sd) -
			cos(s_latitude) * cos(sd) * cos(st*(M_PI/12)) ) );
}

double f_salt( double sd, double alt)	/* solar altitude from solar declination and solar time */
{
	extern float s_latitude;
	double E,F;
	E= sin(s_latitude) * sin(sd);
	F=cos(s_latitude) * cos(sd);
	/*return(((E-sin(alt))/F));*/
	return((12.0/M_PI)*acos((E-sin(alt))/F));
}	


