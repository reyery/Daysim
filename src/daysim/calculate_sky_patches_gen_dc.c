/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 */

#include <stdio.h>
#include <math.h>

#include "fropen.h"
#include "read_in_header.h"
#include "sun.h"



extern float diffuse_pts[145][3]; /* position of sky patches of diffuse daylight coefficients */
extern float direct_pts[288][4];    /* position of sky patches of direct daylight coefficients if n.n.*/
extern float direct_calendar[13][25][4]; /* position of sky patches of direct daylight coefficients otherwise*/

extern int number_direct_coefficients;


void assign_values(int month, int day )
{
	int    hour1;
	float  st; /* solar time */
	float  sd;
	float  altitude, azimuth;
	double sunrise, sunset,alt2_rise,alt2_set;

	sd=sdec(jdate(month, day));
	/*sunrise=12+12-stadj(jdate(month, day))-solar_sunset(month,day);
	  sunset=solar_sunset(month,day)-stadj(jdate(month, day));*/
	sunset=solar_sunset(month,day);
	sunrise=12+12-solar_sunset(month,day);
	alt2_rise=f_salt(  sd, 2.0*(M_PI/180.0));
	alt2_set=24-f_salt(  sd, 2.0*(M_PI/180.0));
	for(hour1=0; hour1< 24; hour1++){
		st=hour1;
		azimuth = (180/M_PI)*sazi(sd, st);
		altitude = salt(sd,st)*(180/M_PI);
		if( ( (hour1-0.5)<alt2_rise ) && (( hour1+0.5) > alt2_rise )){st=alt2_rise;}
		if( ( (hour1-0.5)<alt2_set ) && (( hour1+0.5) > alt2_set )){st=alt2_set;}

		if ( salt(sd,st)*(180/M_PI) >1.999   ){
			direct_pts[number_direct_coefficients][0]=st;
			direct_pts[number_direct_coefficients][1]=salt(sd,st)*(180/M_PI);
			if(direct_pts[number_direct_coefficients][1] <2.001){direct_pts[number_direct_coefficients][1]=2;}
			direct_pts[number_direct_coefficients][2]=(180/M_PI)*sazi(sd, st);
			direct_calendar[month][hour1][0]=direct_pts[number_direct_coefficients][0];
			direct_calendar[month][hour1][1]=direct_pts[number_direct_coefficients][1];
			direct_calendar[month][hour1][2]=direct_pts[number_direct_coefficients][2];
			number_direct_coefficients++;
		}else{
			direct_calendar[month][hour1][0]=st;
			direct_calendar[month][hour1][1]=salt(sd,st)*(180/M_PI);
			direct_calendar[month][hour1][2]=(180/M_PI)*sazi(sd, st);
		}
	}
}

void calculate_sky_patches (char *sky_file_name)
{    // print radiance file for direct suns
	float alt, azi;
	int i=0, j=0;
	FILE *DIRECT_POINTS_FILE;

	// calculate daylight coefficients depending on site

	//============================
	// distribution for direct DCs
	//============================
	for(j=0; j< 24; j++){
		for(i=1; i< 13; i++){
			direct_calendar[i][j][0]=j;direct_calendar[i][j][1]=0;direct_calendar[i][j][2]=0;direct_calendar[i][j][3]=0;
		}
	}
	/*Julian date of December 21: */
	assign_values(12,21);
	/*Julian date of February 21: */
	assign_values(2,21);
	/*Julian date of March 21: */
	assign_values(3,21);
	/*Julian date of April 21: */
	assign_values(4,21);
	/*Julian date of June 21: */
	assign_values(6,21);


	//============================================
	// write out direct sunpositions RADIANCE file
	//============================================
	DIRECT_POINTS_FILE=open_output(sky_file_name);
	fprintf(DIRECT_POINTS_FILE,"# Radiance input file generate by DAYSIM subbprogram gen_dc for the \n");
	fprintf(DIRECT_POINTS_FILE,"# calculation of direct daylight coefficients \n\n");
	for(j=0; j< number_direct_coefficients; j++){
		fprintf(DIRECT_POINTS_FILE,"void light solar%d\n0 \n0 \n3 1000 1000 1000\n\n",j+1);
		fprintf(DIRECT_POINTS_FILE,"solar%d source sun\n0\n0\n",j+1);
		alt=0.017453*(90-direct_pts[j][1]);
		azi=0.017453*(-1)*(direct_pts[j][2]+90);
		fprintf(DIRECT_POINTS_FILE,"4 %f %f %f 0.533\n\n",sin(alt)*cos(azi),sin(alt)*sin(azi),cos(alt));
	}
	i=close_file(DIRECT_POINTS_FILE);
}


//============================================
// write out direct sunpositions RADIANCE file
// DDS file format
//============================================

void calculate_sky_patchesDDS (int NumOfDirectDC,char *sky_file_name)
{    // print radiance file for 145 direct suns
	float alt, azi;
	double RingAltitude=0, RingAzimuth=0;
	int j=0,i=0,k=0,NumOfRingDivisions=0;
	FILE *DIRECT_POINTS_FILE;
	float DirectDC[2305][2];

	for(j=0; j< 2305; j++)
	{
		DirectDC[j][0]=0.0;
		DirectDC[j][1]=0.0;
	}
	if(NumOfDirectDC ==145)
	{
		DirectDC[0][0]=-96.0; DirectDC[0][1]=2.0;
		DirectDC[1][0]=-108.0; DirectDC[1][1]=2.0;
		DirectDC[2][0]=-120.0; DirectDC[2][1]=2.0;
		DirectDC[3][0]=-132.0; DirectDC[3][1]=2.0;
		DirectDC[4][0]=-144.0; DirectDC[4][1]=2.0;
		DirectDC[5][0]=-156.0; DirectDC[5][1]=2.0;
		DirectDC[6][0]=-168.0; DirectDC[6][1]=2.0;
		DirectDC[7][0]=180.0; DirectDC[7][1]=2.0;
		DirectDC[8][0]=168.0; DirectDC[8][1]=2.0;
		DirectDC[9][0]=156.0; DirectDC[9][1]=2.0;
		DirectDC[10][0]=144.0; DirectDC[10][1]=2.0;
		DirectDC[11][0]=132.0; DirectDC[11][1]=2.0;
		DirectDC[12][0]=120.0; DirectDC[12][1]=2.0;
		DirectDC[13][0]=108.0; DirectDC[13][1]=2.0;
		DirectDC[14][0]=96.0; DirectDC[14][1]=2.0;
		DirectDC[15][0]=84.0; DirectDC[15][1]=2.0;
		DirectDC[16][0]=72.0; DirectDC[16][1]=2.0;
		DirectDC[17][0]=60.0; DirectDC[17][1]=2.0;
		DirectDC[18][0]=48.0; DirectDC[18][1]=2.0;
		DirectDC[19][0]=36.0; DirectDC[19][1]=2.0;
		DirectDC[20][0]=24.0; DirectDC[20][1]=2.0;
		DirectDC[21][0]=12.0; DirectDC[21][1]=2.0;
		DirectDC[22][0]=0.0; DirectDC[22][1]=2.0;
		DirectDC[23][0]=-12.0; DirectDC[23][1]=2.0;
		DirectDC[24][0]=-24.0; DirectDC[24][1]=2.0;
		DirectDC[25][0]=-36.0; DirectDC[25][1]=2.0;
		DirectDC[26][0]=-48.0; DirectDC[26][1]=2.0;
		DirectDC[27][0]=-60.0; DirectDC[27][1]=2.0;
		DirectDC[28][0]=-72.0; DirectDC[28][1]=2.0;
		DirectDC[29][0]=-84.0; DirectDC[29][1]=2.0;
		DirectDC[30][0]=-96.0; DirectDC[30][1]=18.0;
		DirectDC[31][0]=-108.0; DirectDC[31][1]=18.0;
		DirectDC[32][0]=-120.0; DirectDC[32][1]=18.0;
		DirectDC[33][0]=-132.0; DirectDC[33][1]=18.0;
		DirectDC[34][0]=-144.0; DirectDC[34][1]=18.0;
		DirectDC[35][0]=-156.0; DirectDC[35][1]=18.0;
		DirectDC[36][0]=-168.0; DirectDC[36][1]=18.0;
		DirectDC[37][0]=180.0; DirectDC[37][1]=18.0;
		DirectDC[38][0]=168.0; DirectDC[38][1]=18.0;
		DirectDC[39][0]=156.0; DirectDC[39][1]=18.0;
		DirectDC[40][0]=144.0; DirectDC[40][1]=18.0;
		DirectDC[41][0]=132.0; DirectDC[41][1]=18.0;
		DirectDC[42][0]=120.0; DirectDC[42][1]=18.0;
		DirectDC[43][0]=108.0; DirectDC[43][1]=18.0;
		DirectDC[44][0]=96.0; DirectDC[44][1]=18.0;
		DirectDC[45][0]=84.0; DirectDC[45][1]=18.0;
		DirectDC[46][0]=72.0; DirectDC[46][1]=18.0;
		DirectDC[47][0]=60.0; DirectDC[47][1]=18.0;
		DirectDC[48][0]=48.0; DirectDC[48][1]=18.0;
		DirectDC[49][0]=36.0; DirectDC[49][1]=18.0;
		DirectDC[50][0]=24.0; DirectDC[50][1]=18.0;
		DirectDC[51][0]=12.0; DirectDC[51][1]=18.0;
		DirectDC[52][0]=0.0; DirectDC[52][1]=18.0;
		DirectDC[53][0]=-12.0; DirectDC[53][1]=18.0;
		DirectDC[54][0]=-24.0; DirectDC[54][1]=18.0;
		DirectDC[55][0]=-36.0; DirectDC[55][1]=18.0;
		DirectDC[56][0]=-48.0; DirectDC[56][1]=18.0;
		DirectDC[57][0]=-60.0; DirectDC[57][1]=18.0;
		DirectDC[58][0]=-72.0; DirectDC[58][1]=18.0;
		DirectDC[59][0]=-84.0; DirectDC[59][1]=18.0;
		DirectDC[60][0]=-97.5; DirectDC[60][1]=30.0;
		DirectDC[61][0]=-112.5; DirectDC[61][1]=30.0;
		DirectDC[62][0]=-127.5; DirectDC[62][1]=30.0;
		DirectDC[63][0]=-142.5; DirectDC[63][1]=30.0;
		DirectDC[64][0]=-157.5; DirectDC[64][1]=30.0;
		DirectDC[65][0]=-172.5; DirectDC[65][1]=30.0;
		DirectDC[66][0]=172.5; DirectDC[66][1]=30.0;
		DirectDC[67][0]=157.5; DirectDC[67][1]=30.0;
		DirectDC[68][0]=142.5; DirectDC[68][1]=30.0;
		DirectDC[69][0]=127.5; DirectDC[69][1]=30.0;
		DirectDC[70][0]=112.5; DirectDC[70][1]=30.0;
		DirectDC[71][0]=97.5; DirectDC[71][1]=30.0;
		DirectDC[72][0]=82.5; DirectDC[72][1]=30.0;
		DirectDC[73][0]=67.5; DirectDC[73][1]=30.0;
		DirectDC[74][0]=52.5; DirectDC[74][1]=30.0;
		DirectDC[75][0]=37.5; DirectDC[75][1]=30.0;
		DirectDC[76][0]=22.5; DirectDC[76][1]=30.0;
		DirectDC[77][0]=7.5; DirectDC[77][1]=30.0;
		DirectDC[78][0]=-7.5; DirectDC[78][1]=30.0;
		DirectDC[79][0]=-22.5; DirectDC[79][1]=30.0;
		DirectDC[80][0]=-37.5; DirectDC[80][1]=30.0;
		DirectDC[81][0]=-52.5; DirectDC[81][1]=30.0;
		DirectDC[82][0]=-67.5; DirectDC[82][1]=30.0;
		DirectDC[83][0]=-82.5; DirectDC[83][1]=30.0;
		DirectDC[84][0]=-97.5; DirectDC[84][1]=42.0;
		DirectDC[85][0]=-112.5; DirectDC[85][1]=42.0;
		DirectDC[86][0]=-127.5; DirectDC[86][1]=42.0;
		DirectDC[87][0]=-142.5; DirectDC[87][1]=42.0;
		DirectDC[88][0]=-157.5; DirectDC[88][1]=42.0;
		DirectDC[89][0]=-172.5; DirectDC[89][1]=42.0;
		DirectDC[90][0]=172.5; DirectDC[90][1]=42.0;
		DirectDC[91][0]=157.5; DirectDC[91][1]=42.0;
		DirectDC[92][0]=142.5; DirectDC[92][1]=42.0;
		DirectDC[93][0]=127.5; DirectDC[93][1]=42.0;
		DirectDC[94][0]=112.5; DirectDC[94][1]=42.0;
		DirectDC[95][0]=97.5; DirectDC[95][1]=42.0;
		DirectDC[96][0]=82.5; DirectDC[96][1]=42.0;
		DirectDC[97][0]=67.5; DirectDC[97][1]=42.0;
		DirectDC[98][0]=52.5; DirectDC[98][1]=42.0;
		DirectDC[99][0]=37.5; DirectDC[99][1]=42.0;
		DirectDC[100][0]=22.5; DirectDC[100][1]=42.0;
		DirectDC[101][0]=7.5; DirectDC[101][1]=42.0;
		DirectDC[102][0]=-7.5; DirectDC[102][1]=42.0;
		DirectDC[103][0]=-22.5; DirectDC[103][1]=42.0;
		DirectDC[104][0]=-37.5; DirectDC[104][1]=42.0;
		DirectDC[105][0]=-52.5; DirectDC[105][1]=42.0;
		DirectDC[106][0]=-67.5; DirectDC[106][1]=42.0;
		DirectDC[107][0]=-82.5; DirectDC[107][1]=42.0;
		DirectDC[108][0]=-100.0; DirectDC[108][1]=54.0;
		DirectDC[109][0]=-120.0; DirectDC[109][1]=54.0;
		DirectDC[110][0]=-140.0; DirectDC[110][1]=54.0;
		DirectDC[111][0]=-160.0; DirectDC[111][1]=54.0;
		DirectDC[112][0]=180.0; DirectDC[112][1]=54.0;
		DirectDC[113][0]=160.0; DirectDC[113][1]=54.0;
		DirectDC[114][0]=140.0; DirectDC[114][1]=54.0;
		DirectDC[115][0]=120.0; DirectDC[115][1]=54.0;
		DirectDC[116][0]=100.0; DirectDC[116][1]=54.0;
		DirectDC[117][0]=80.0; DirectDC[117][1]=54.0;
		DirectDC[118][0]=60.0; DirectDC[118][1]=54.0;
		DirectDC[119][0]=40.0; DirectDC[119][1]=54.0;
		DirectDC[120][0]=20.0; DirectDC[120][1]=54.0;
		DirectDC[121][0]=0.0; DirectDC[121][1]=54.0;
		DirectDC[122][0]=-20.0; DirectDC[122][1]=54.0;
		DirectDC[123][0]=-40.0; DirectDC[123][1]=54.0;
		DirectDC[124][0]=-60.0; DirectDC[124][1]=54.0;
		DirectDC[125][0]=-80.0; DirectDC[125][1]=54.0;
		DirectDC[126][0]=-105.0; DirectDC[126][1]=66.0;
		DirectDC[127][0]=-135.0; DirectDC[127][1]=66.0;
		DirectDC[128][0]=-165.0; DirectDC[128][1]=66.0;
		DirectDC[129][0]=165.0; DirectDC[129][1]=66.0;
		DirectDC[130][0]=135.0; DirectDC[130][1]=66.0;
		DirectDC[131][0]=105.0; DirectDC[131][1]=66.0;
		DirectDC[132][0]=75.0; DirectDC[132][1]=66.0;
		DirectDC[133][0]=45.0; DirectDC[133][1]=66.0;
		DirectDC[134][0]=15.0; DirectDC[134][1]=66.0;
		DirectDC[135][0]=-15.0; DirectDC[135][1]=66.0;
		DirectDC[136][0]=-45.0; DirectDC[136][1]=66.0;
		DirectDC[137][0]=-75.0; DirectDC[137][1]=66.0;
		DirectDC[138][0]=-120.0; DirectDC[138][1]=78.0;
		DirectDC[139][0]=180.0; DirectDC[139][1]=78.0;
		DirectDC[140][0]=120.0; DirectDC[140][1]=78.0;
		DirectDC[141][0]=60.0; DirectDC[141][1]=78.0;
		DirectDC[142][0]=0.0; DirectDC[142][1]=78.0;
		DirectDC[143][0]=-60.0; DirectDC[143][1]=78.0;
		DirectDC[144][0]=0.0; DirectDC[144][1]=90.0;
	}


if(NumOfDirectDC ==2305)
	{
		k=0;
		for(j=0; j< 29; j++){ //ring index
			//assign ring latitude
			if(j<28)
				RingAltitude=1.0*(j+0.5)*(90.0/28.5);
			else
				RingAltitude=90.0;

			if(RingAltitude<2.0)
				RingAltitude=2.0;

			if(j<8)
				NumOfRingDivisions=30*4;
			if(j>=8 && j<16)
				NumOfRingDivisions=24*4;
			if(j>=16 && j<20)
				NumOfRingDivisions=18*4;
			if(j>=20 && j<24)
				NumOfRingDivisions=12*4;
			if(j>=24 && j<28)
				NumOfRingDivisions=6*4;
			if(j==28)
				NumOfRingDivisions=1;

			//assign azimuth
			for(i=0; i< NumOfRingDivisions; i++){ //azimuth
				RingAzimuth=1.0*(i+0.5)*(360.0/NumOfRingDivisions);
				if(RingAzimuth<=90)
					RingAzimuth=-90-RingAzimuth;
				if(RingAzimuth>90 && RingAzimuth<=360)
					RingAzimuth=270-RingAzimuth;

				DirectDC[k][0]=RingAzimuth;
				DirectDC[k][1]=RingAltitude;
				k++;
				//printf("%d azi %f alt %f \n",k,RingAzimuth,RingAltitude);
			}
		}
	}




	//============================================
	// write out direct sunpositions RADIANCE file
	//============================================
	//printf("num\tazimuth\taltitude\n");
		DIRECT_POINTS_FILE=open_output(sky_file_name);

	fprintf(DIRECT_POINTS_FILE,"# ********************** DDS file format ************************** \n");
	fprintf(DIRECT_POINTS_FILE,"# Radiance input file generate by DAYSIM subbprogram gen_dc for the \n");
	fprintf(DIRECT_POINTS_FILE,"# calculation of %d direct daylight coefficients \n\n",NumOfDirectDC);
		for(j=0; j< NumOfDirectDC; j++){
		fprintf(DIRECT_POINTS_FILE,"void light solar%d\n0 \n0 \n3 1000 1000 1000\n\n",j+1);
		fprintf(DIRECT_POINTS_FILE,"solar%d source sun\n0\n0\n",j+1);
		alt=0.017453292*(90.0-DirectDC[j][1]);
		azi=0.017453292*(-1)*(DirectDC[j][0]+90.0);
		fprintf(DIRECT_POINTS_FILE,"4 %f %f %f 0.533\n\n",sin(alt)*cos(azi),sin(alt)*sin(azi),cos(alt));
		//printf("%d\t%f\t%f\n", (j+1),DirectDC[j][0],DirectDC[j][1]);
	}
	close_file(DIRECT_POINTS_FILE);

}

