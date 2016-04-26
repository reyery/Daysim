/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 */
#ifndef CALCULATE_SKY_PACHES_H
#define CALCULATE_SKY_PACHES_H


void assign_values(int month, int day);
void calculate_sky_patches (char *sky_file_name);
void calculate_sky_patchesDDS (int NumOfDirectDC,char *sky_file_name);



#endif
