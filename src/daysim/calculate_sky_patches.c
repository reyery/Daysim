/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 */



void assign_values(int month, int day)
{	int hour1;
	double Pi=3.14159;
	float st; /* solar time */
	float sd;
	float altitude, azimuth;
	double sunrise, sunset,alt2_rise,alt2_set;
	sd=sdec(jdate(month, day));
	/*sunrise=12+12-stadj(jdate(month, day))-solar_sunset(month,day);
	sunset=solar_sunset(month,day)-stadj(jdate(month, day));*/
	sunset=solar_sunset(month,day);
	sunrise=12+12-solar_sunset(month,day);
	alt2_rise=f_salt(  sd, 2.0*(Pi/180.0));
	alt2_set=24-f_salt(  sd, 2.0*(Pi/180.0));
	for(hour1=0; hour1< 24; hour1++){
		st=hour1;
		azimuth = (180/Pi)*sazi(sd, st);
		altitude = salt(sd,st)*(180/Pi);
			if( ( (hour1-0.5)<alt2_rise ) && (( hour1+0.5) > alt2_rise )){st=alt2_rise;}
			if( ( (hour1-0.5)<alt2_set ) && (( hour1+0.5) > alt2_set )){st=alt2_set;}

		if ( salt(sd,st)*(180/Pi) >1.999   ){
			direct_pts[number_direct_coefficients][0]=st;
			direct_pts[number_direct_coefficients][1]=salt(sd,st)*(180/Pi);
			if(direct_pts[number_direct_coefficients][1] <2.001){direct_pts[number_direct_coefficients][1]=2;}
			direct_pts[number_direct_coefficients][2]=(180/Pi)*sazi(sd, st);
			direct_calendar[month][hour1][0]=direct_pts[number_direct_coefficients][0];
			direct_calendar[month][hour1][1]=direct_pts[number_direct_coefficients][1];
			direct_calendar[month][hour1][2]=direct_pts[number_direct_coefficients][2];
			number_direct_coefficients++;
		}else{
			direct_calendar[month][hour1][0]=st;
			direct_calendar[month][hour1][1]=salt(sd,st)*(180/Pi);
			direct_calendar[month][hour1][2]=(180/Pi)*sazi(sd, st);
		}
	}
}



void calculate_sky_patches ()
{    // print radiance file for direct suns
	float alt, azi;
	float  patches_per_row[8][2];
	int i=0, j=0,k=0;
	double PI=3.14159;
	float angle_unit;
	FILE *DIRECT_POINTS_FILE;

	// calculate daylight coefficients depending on site

	//======================================
	// Tregenza distribution for diffuse DCs
	//======================================
		patches_per_row[0][0]=90;
		patches_per_row[0][1]=1;
		patches_per_row[1][0]=84;
		patches_per_row[1][1]=30;
		patches_per_row[2][0]=72;
		patches_per_row[2][1]=30; /* 24 */
		patches_per_row[3][0]=60;
		patches_per_row[3][1]=24;/* 24 */
		patches_per_row[4][0]=48;
		patches_per_row[4][1]=24;
		patches_per_row[5][0]=36;
		patches_per_row[5][1]=18;
		patches_per_row[6][0]=24;
		patches_per_row[6][1]=12;
		patches_per_row[7][0]=12;
		patches_per_row[7][1]=6;

		k=0;
		for(i=1; i<= 7; i++){
			angle_unit=360.0/patches_per_row[i][1];
			for( j=0; j<patches_per_row[i][1]; j++){
				diffuse_pts[k][0]=patches_per_row[i][0];
				diffuse_pts[k][1]=(0.5*angle_unit)+(angle_unit*j);
				diffuse_pts[k][2]=(2*PI/patches_per_row[i][1])*(cos((PI/180)*(patches_per_row[i+1][0]+6))-cos((PI/180)*(patches_per_row[i][0]+6)));
				k++;
			}
		}
		diffuse_pts[144][0]=0;
		diffuse_pts[144][1]=0;
		diffuse_pts[144][2]=(2*PI)*(1-cos((PI/180)*(6)));

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

		DIRECT_POINTS_FILE=open_output(direct_radiance_file);
		for(j=0; j< number_direct_coefficients; j++){
			fprintf(DIRECT_POINTS_FILE,"void light solar%d\n0 \n0 \n3 1000 1000 1000\n\n",j+1);
			fprintf(DIRECT_POINTS_FILE,"solar%d source sun\n0\n0\n",j+1);
			alt=0.017453*(90-direct_pts[j][1]);
			azi=0.017453*(-1)*(direct_pts[j][2]+90);
			fprintf(DIRECT_POINTS_FILE,"4 %f %f %f 0.0533\n\n",sin(alt)*cos(azi),sin(alt)*sin(azi),cos(alt));
		}
		i=close_file(DIRECT_POINTS_FILE);
}
