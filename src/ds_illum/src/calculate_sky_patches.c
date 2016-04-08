/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "common/fropen.h"
#include "common/read_in_header.h"
#include "common/sun.h"

#include "common/ds_illum.h"


void calculate_sky_patches (int *dc_direct_resolution, int *dir_rad,int *dif_pts,int *dir_pts,int *number_direct_coefficients)
{
	int 	i,j;
	float alt,azi;
	FILE *DIFFUSE_POINTS_FILE;
	FILE *DIRECT_POINTS_FILE;
	get_dc(&dc_direct_resolution,number_direct_coefficients);	/* calculate daylight coefficients depending on site */

	if (*dir_pts) {/* print direct daylight coefficients */
		DIRECT_POINTS_FILE=open_output(direct_points_file);
		for(i=0; i< *number_direct_coefficients ; i++){fprintf(DIRECT_POINTS_FILE," %f %f %f \n",direct_pts[i][0],direct_pts[i][1],direct_pts[i][2]);}
		i=close_file(DIRECT_POINTS_FILE);
	}
	if (*dif_pts) {/* print diffuse daylight coefficients */
		DIFFUSE_POINTS_FILE=open_output(diffuse_points_file);
		for(i=0; i<= 144; i++){fprintf(DIFFUSE_POINTS_FILE," %f %f 0.0 \n",diffuse_pts[i][0],diffuse_pts[i][1]);}
		i=close_file(DIFFUSE_POINTS_FILE);

	}

	/* print radiance file for direct suns */
	if(dc_coupling_mode ==2){
		if(!strcmp(direct_radiance_file,"")){
			fprintf(stderr,"WARNING: ds_illum: shadow testing turned on but no direct radiance file specified in header!");exit(0);
		}else{
			DIFFUSE_POINTS_FILE=open_output(direct_radiance_file);
			for(j=0; j< *number_direct_coefficients; j++){
				fprintf(DIFFUSE_POINTS_FILE,"void light solar%d\n0 \n0 \n3 1000 1000 1000\n\n",j+1);
				fprintf(DIFFUSE_POINTS_FILE,"solar%d source sun\n0\n0\n",j+1);
				alt=0.017453*(90-direct_pts[j][1]);
				azi=0.017453*(-1)*(direct_pts[j][2]+90);
				fprintf(DIFFUSE_POINTS_FILE,"4 %f %f %f 0.533\n\n",sin(alt)*cos(azi),sin(alt)*sin(azi),cos(alt));
			}
			i=close_file(DIFFUSE_POINTS_FILE);
		}
	}


}


void get_dc(int *dc_direct_resolution, int *number_direct_coefficients) /* calculates a set of daylight coefficients depending on site */
{
	float  patches_per_row[8][2];
	int i=0, j=0,k=0;
	float angle_unit;

	/* Tregenza distribution */
	patches_per_row[0][0]=90;
	patches_per_row[0][1]=1;
	patches_per_row[1][0]=84;
	patches_per_row[1][1]=30;
	patches_per_row[2][0]=72;
	patches_per_row[2][1]=30; /* 24 */
	patches_per_row[3][0]=60;
	patches_per_row[3][1]=24;/* 28 */
	patches_per_row[4][0]=48;
	patches_per_row[4][1]=24;
	patches_per_row[5][0]=36;
	patches_per_row[5][1]=18;
	patches_per_row[6][0]=24;
	patches_per_row[6][1]=12;
	patches_per_row[7][0]=12;
	patches_per_row[7][1]=6;

	/* calculate diffuse dcs */
	k=0;
	for(i=1; i<= 7; i++){
		angle_unit=360.0/patches_per_row[i][1];
		for( j=0; j<patches_per_row[i][1]; j++){
			diffuse_pts[k][0]=patches_per_row[i][0];
			diffuse_pts[k][1]=(0.5*angle_unit)+(angle_unit*j);
			k++;
		}
	}
	diffuse_pts[144][0]=0;
	diffuse_pts[144][1]=0;

	/* get limits for the altitude angles of the sun */
	if (*dc_direct_resolution==1){
		assign_values(12,21, number_direct_coefficients);
	}else{
		for(j=0; j< 24; j++){
			for(i=1; i< 13; i++){
				direct_calendar[i][j][0]=j;
				direct_calendar[i][j][1]=0;
				direct_calendar[i][j][2]=0;
				direct_calendar[i][j][3]=0;
			}
		}
		/*Julian date of December 21: */
		assign_values(12,21,number_direct_coefficients);
		/*Julian date of February 21: */
		assign_values(2,21,number_direct_coefficients);
		/*Julian date of March 21: */
		assign_values(3,21,number_direct_coefficients);
		/*Julian date of April 21: */
		assign_values(4,21,number_direct_coefficients);
		/*Julian date of June 21: */
		assign_values(6,21,number_direct_coefficients);
	}
}

void assign_values(int month, int day, int *number_direct_coefficients)
{	int hour1;
 float st; /* solar time */
 float sd;
 double alt2_rise,alt2_set;
 sd=sdec(jdate(month, day));
 alt2_rise=f_salt(  sd, 2.0*(M_PI/180.0));
 alt2_set=24-f_salt(  sd, 2.0*(M_PI/180.0));
 for(hour1=0; hour1< 24; hour1++){
	 st=hour1;
	 if( ( (hour1-0.5)<alt2_rise ) && (( hour1+0.5) > alt2_rise )){st=alt2_rise;}
	 if( ( (hour1-0.5)<alt2_set ) && (( hour1+0.5) > alt2_set )){st=alt2_set;}

	 if ( salt(sd,st)*(180/M_PI) >1.999   ){
		 direct_pts[*number_direct_coefficients][0]=st;
		 direct_pts[*number_direct_coefficients][1]=salt(sd,st)*(180/M_PI);
		 if(direct_pts[*number_direct_coefficients][1] <2.001){direct_pts[*number_direct_coefficients][1]=2;}
		 direct_pts[*number_direct_coefficients][2]=(180/M_PI)*sazi(sd, st);
		 direct_calendar[month][hour1][0]=direct_pts[*number_direct_coefficients][0];
		 direct_calendar[month][hour1][1]=direct_pts[*number_direct_coefficients][1];
		 direct_calendar[month][hour1][2]=direct_pts[*number_direct_coefficients][2];
		 *number_direct_coefficients= *number_direct_coefficients +1;
	 }else{
		 direct_calendar[month][hour1][0]=st;
		 direct_calendar[month][hour1][1]=salt(sd,st)*(180/M_PI);
		 direct_calendar[month][hour1][2]=(180/M_PI)*sazi(sd, st);
	 }
 }
}

void get_horizon_factors()
{
	float step_genweather,step_hor,altitude_u1, altitude_u2,altitude_d1, altitude_d2,ratio;
	int i,j;

	step_genweather=360.0/30.0;
	step_hor=360.0/36.0;

	for (i=0;i<30;i++){
		/* get two horizon heights */
		for (j=0;j<36;j++){
			if((j+1)*step_hor>=i*step_genweather && j*step_hor<=(i+1)*step_genweather ){
				if(horizon[j]>=12){
					altitude_u1=(horizon[j]-12.0)/12.0;
					altitude_d1=1;
				}else{
					altitude_u1=0;
					altitude_d1=(horizon[j])/12.0;
				}
				if(horizon[j+1]>=12){
					altitude_u2=(horizon[j+1]-12.0)/12.0;
					altitude_d2=1;
				}else{
					altitude_u2=0;
					altitude_d2=(horizon[j+1])/12.0;
				}
				ratio=((j+1)*step_hor-i*step_genweather)/step_genweather;
				/* get horizon factors */
				horizon_factor[i]=1-(ratio*altitude_d1+(1-ratio)*altitude_d2);
				horizon_factor[i+30]=1-(ratio*altitude_u1+(1-ratio)*altitude_u2);
				j=36;
			}
		}
	}
}

