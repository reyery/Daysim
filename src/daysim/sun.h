#pragma once

void solar_elev_azi_ecc ( float latitude, float longitude, float time_zone, int jday, float time, int solar_time, \
                          float *solar_elevation, float *solar_azimuth, float *eccentricity_correction);

void sunrise_sunset_localtime ( float latitude, float longitude, float time_zone, int jday,\
                                float *sunrise_localtime, float *sunset_localtime );	

int day_to_month (int day);

int julian_day_to_day_of_month (int day);

int month_and_day_to_julian_day (int month, int day);

int jday_first_of_month (int month);
