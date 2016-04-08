#ifndef CALCULATE_SKY_PATCHES_H
#define CALCULATE_SKY_PATCHES_H


void calculate_sky_patches (int *dc_direct_resolution, int *dir_rad,int *dif_pts,int *dir_pts,int *number_direct_coefficients);

/* calculates a set of daylight coefficients depending on site */
void get_dc(int *dc_direct_resolution, int *number_direct_coefficients);


void assign_values(int month, int day, int *number_direct_coefficients);

void get_horizon_factors();


#endif
