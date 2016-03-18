#include <math.h>
#include <stdio.h>

#include "globals.h"

void sunrise_sunset_localtime ( float latitude, float longitude, float time_zone, int jday,\
                                float *sunrise_localtime, float *sunset_localtime )
{ float solar_declination;
  float sunrise_solartime, sunset_solartime, time_dif;

  solar_declination = RTD * 0.4093 * sin( (2*Pi/368) * (jday - 81) );
  sunrise_solartime = 12/Pi * acos ( tan ( latitude*DTR ) * tan ( solar_declination*DTR ) );
  sunset_solartime = 12/Pi * ( 2*Pi - acos ( tan ( latitude*DTR ) * tan ( solar_declination*DTR ) ));

  time_dif = + 0.170 * sin( (4*Pi/373) * (jday - 80) ) - 0.129 * sin( (2*Pi/355) * (jday - 8) ) + 12/180.0 * (time_zone - longitude);

  *sunrise_localtime = sunrise_solartime - time_dif;
  *sunset_localtime = sunset_solartime - time_dif;
}

int day_to_month (int day)                    /*  gives the month to which the day belongs ( year with 365 days )  */
{
  if ( day >= 1 && day <= 31 )  return 1;
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
  if ( day <= 365 )  return 12;
  if ( day <= 396 )  return 1;
  if ( day <= 424 )  return 2;
  if ( day <= 455 )  return 3;
  if ( day <= 485 )  return 4;
  if ( day <= 516 )  return 5;
  if ( day <= 546 )  return 6;
  if ( day <= 577 )  return 7;
  if ( day <= 608 )  return 8;
  if ( day <= 638 )  return 9;
  if ( day <= 669 )  return 10;
  if ( day <= 699 )  return 11;
  if ( day <= 730 )  return 12;
  else	{  fprintf(stderr, "bad day");  return 0;  }
}

int julian_day_to_day_of_month (int day)
{
  if ( day >= 1 && day <= 31 )  return day;
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
  if ( day <= 365 )  return day-334;
  if ( day <= 396 )  return day-365;
  if ( day <= 424 )  return day-396;
  if ( day <= 455 )  return day-424;
  if ( day <= 485 )  return day-455;
  if ( day <= 516 )  return day-485;
  if ( day <= 546 )  return day-516;
  if ( day <= 577 )  return day-546;
  if ( day <= 608 )  return day-577;
  if ( day <= 638 )  return day-608;
  if ( day <= 669 )  return day-638;
  if ( day <= 699 )  return day-669;
  if ( day <= 730 )  return day-699;
  else	{  fprintf(stderr, "bad day");  return 0;  }
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
  else  {  fprintf(stderr, "bad month");  return 0;  }
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
  else  {  fprintf(stderr, "bad month");  return 0;  }
}
