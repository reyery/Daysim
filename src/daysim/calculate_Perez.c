/* This file contains large pieces of the src file for the
 * RADIANCE program gendaylit, which has been written by J.J. Delaunay
 * at the Fraunhofer Institute for Solar Energy Systems
 * changes have been introduced by written by Christoph Reinhart to
 * implement the Perez sky model into the DAYSIM simulation environment.
 * last changed in February 2001
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <rtmath.h>
#include <string.h>

#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"
#include "sun.h"

#include "ds_illum.h"
#include "shadow_testing.h"


int write_segments_diffuse(double dir,double dif);
int write_segments_direct(double dir,double dif, int number_direct_coefficients,
						  int *shadow_testing_on, int count_var, int current_shadow_testing_value);
double  normsc(double altitude, int S_INTER);


#define  WHTEFFICACY            179.

/* constants */
const double	AU = 149597890E3;
const double 	solar_constant_e = 1367;    /* solar constant W/m^2 */
const double  	solar_constant_l = 127.5;   /* solar constant klux */
const double	half_sun_angle = 0.2665;
const double	skyclearinf = 1.000;	/* limitations for the variation of the Perez parameters */
const double	skyclearsup = 12.1;
const double	skybriginf = 0.01;
const double	skybrigsup = 0.6;
/* Perez sky parametrization : epsilon and delta calculations from the direct and diffuse irradiances */
double sky_brightness();
double sky_clearness();
/* calculation of the direct and diffuse components from the Perez parametrization */
double	diffus_irradiance_from_sky_brightness();
double 	direct_irradiance_from_sky_clearness();
/* misc */
double 	c_perez[5];
/* Perez global horizontal, diffuse horizontal and direct normal luminous efficacy models : input w(cm)=2cm, solar zenith angle(degrees); output efficacy(lm/W) */
double 	glob_h_effi_PEREZ();
double 	glob_h_diffuse_effi_PEREZ();
double 	direct_n_effi_PEREZ();
/*likelihood check of the epsilon, delta, direct and diffuse components*/
void 	check_parametrization();
void 	check_irradiances();
void 	check_illuminances();
void 	illu_to_irra_index();
/* Perez sky luminance model */
double  calc_rel_lum_perez(double dzeta, double gamma, double Z,
	double epsilon, double Delta, double *coeff_perez);
/* coefficients for the sky luminance perez model */
void 	coeff_lum_perez(double Z, double epsilon, double Delta, double *coeff_perez);
double	radians(double degres);
double	degres(double radians);
void	theta_phi_to_dzeta_gamma(double theta,double phi,double *dzeta,double *gamma, double Z);
double 	integ_lv(float *lv,float *theta);
/* astronomy and geometry*/
double 	get_eccentricity();
double 	air_mass();
double 	get_angle_sun_direction(double sun_zenith, double sun_azimut, double direction_zenith, double direction_azimut);
/* definition of the sky conditions through the Perez parametrization */
double	skyclearness, skybrightness;

//radiance of the sun disk and of the circumsolar area
double	solarradiance_visible_radiation;
double	solarradiance_solar_radiation;
double	solarradiance_luminance;

double	diffusilluminance, directilluminance, diffusirradiance, directirradiance;
double	sunzenith, daynumber=150, atm_preci_water=2;
double 	diffnormalization, dirnormalization;
double  diffnormalization_visible_radiation;
double  diffnormalization_solar_radiation;
double	diffnormalization_luminance;

/* default values */
int  cloudy = 0;				/* 1=standard, 2=uniform */
int  dosun = 1;
double  zenithbr_visible_radiation = -1.0;
double  zenithbr_solar_radiation = -1.0;
double  zenithbr_luminance = -1.0;

double  betaturbidity = 0.1;
/* computed values */
double  sundir[3];
double  groundbr_visible_radiation;
double 	groundbr_solar_radiation;
double 	groundbr_luminance;
double  F2;

/* defines current time */
double hour;
int day, month;


char temp_file[200];/* ="/tmp/temporary.oct";*/


void add_time_step(float time_step)
{
	div_t e;
	hour+=(time_step/60.0);
	e=div((int)hour,24);		/* hours*/
	day+=e.quot;		/* days */
	hour =hour-(e.quot)*(24.0);

	if(month==1 && day>31){month++;day-=31;}
	if(month==2 && day>28){month++;day-=28;}
	if(month==3 && day>31){month++;day-=31;}
	if(month==4 && day>30){month++;day-=30;}
	if(month==5 && day>31){month++;day-=31;}
	if(month==6 && day>30){month++;day-=30;}
	if(month==7 && day>31){month++;day-=31;}
	if(month==8 && day>31){month++;day-=31;}
	if(month==9 && day>30){month++;day-=30;}
	if(month==10 && day>31){month++;day-=31;}
	if(month==11 && day>30){month++;day-=30;}
	if(month==12 && day>31){month=1;day-=31;}
}

void calculate_perez(int *shadow_testing_on, int number_direct_coefficients)
{
    

	int  reset=0;
	double dir1 = 0.0, dif1 = 0.0, hour_bak = 0.0;
	double dir = 0.0, dif = 0.0;
	int header_implemented_in_wea=0;
	int shadow_testing_counter=0;
	int current_shadow_testing_value=0;
	int header_weather_data_short_file_units;
	double sunrise, sunset;
	int i=0,j=0,k=0,m=0,number_data_values=0;
	char keyword[200]="";
	double centrum_hour;

	if( *shadow_testing_on){
		shadow_testing(number_direct_coefficients);
		fprintf(stdout, "%s: initial shadow_testing done \n", progname);
		*shadow_testing_on=shadow_testing_new;
	}


	/* FILE I/O */
	INPUT_DATAFILE=open_input(input_datafile);
	for (k=0 ; k<TotalNumberOfDCFiles ; k++) {
		if( strcmp( shading_illuminance_file[k+1], "-" ) == 0 )
			SHADING_ILLUMINANCE_FILE[k]= stdout;
		else
		{
			//printf("shading file %d %s\n",k,shading_illuminance_file[k+1]);
			SHADING_ILLUMINANCE_FILE[k]=open_output(shading_illuminance_file[k+1]);
		}
	}

	/* get number of data values and test whether the climate input file has a header */
	fscanf(INPUT_DATAFILE,"%s", keyword);
	if( !strcmp(keyword,"place") ){
		double header_latitude;
		double header_longitude;
		double header_time_zone;
		double header_site_elevation;

		header_implemented_in_wea = 1;
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		//latitude
		fscanf(INPUT_DATAFILE,"%s %f", keyword, &header_latitude);
		header_latitude *= DTR;
		if ((header_latitude - s_latitude)>5 * DTR || (header_latitude - s_latitude)<-5 * DTR){
			error(WARNING, "latitude in climate file header (wea) and latitude on project file (hea) differ by more than 5 DEG");
		}
		//longitude
		fscanf(INPUT_DATAFILE,"%s %f", keyword, &header_longitude);
		header_longitude *= DTR;
		if ((header_longitude - s_longitude)>5 * DTR || (header_longitude - s_longitude)<-5 * DTR){
			error(WARNING, "longitude in climate file header (wea) and longitude on project file (hea) differ by more than 5 DEG");
		}
		//time_zone
		fscanf(INPUT_DATAFILE,"%s %f", keyword, &header_time_zone);
		header_time_zone *= DTR;
		if ((header_time_zone - s_meridian)>5 * DTR || (header_time_zone - s_meridian)<-5 * DTR){
			error(WARNING, "time zone in climate file header (wea) and time zone on project file (hea) differ by more than 5 DEG");
		}
		//site_elevation
		fscanf(INPUT_DATAFILE,"%s %f", keyword, &header_site_elevation);
		//weather_data_file_units
		fscanf(INPUT_DATAFILE,"%s %d", keyword, &header_weather_data_short_file_units);
		if(header_weather_data_short_file_units !=wea_data_short_file_units){
			error(WARNING, "climate file units differ in climate and project header files");
		}
		number_data_values--;
	}


	while( EOF != fscanf(INPUT_DATAFILE,"%*[^\n]")){number_data_values++;fscanf(INPUT_DATAFILE,"%*[\n\r]");}
	close_file(INPUT_DATAFILE);

	INPUT_DATAFILE=open_input(input_datafile);
	if(header_implemented_in_wea){
		/* skip header */
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
		fscanf(INPUT_DATAFILE,"%*[^\n]");fscanf(INPUT_DATAFILE,"%*[\n\r]");
	}

	for (m = 0; m<number_data_values; m++){

		fscanf(INPUT_DATAFILE, "%d %d %f %f %f", &month, &day, &hour, &dir1, &dif1);
		dir = dir1;
		dif = dif1;
		centrum_hour = hour;
		sunrise = 12 + 12 - stadj(jdate(month, day)) - solar_sunset(month, day);
		sunset = solar_sunset(month, day) - stadj(jdate(month, day));
		if ((hour - (0.5*time_step / 60.0) <= sunrise) && (hour + (0.5*time_step / 60.0)> sunrise)){
			hour = 0.5*(hour + (0.5*time_step / 60.0)) + 0.5*sunrise;
		}
		else if ((hour - (0.5*time_step / 60.0) <= sunset) && (hour + (0.5*time_step / 60.0)> sunset)){
			hour = 0.5*(hour - (0.5*time_step / 60.0)) + 0.5*sunset;
		}

		for (k = 0; k<TotalNumberOfDCFiles; k++)
			fprintf(SHADING_ILLUMINANCE_FILE[k], "%d %d %.3f ", month, day, centrum_hour);

		//assign value of current shadow test: "Is the sensor in the sun?"
		if (*shadow_testing_on &&dir >= dir_threshold && dif >= dif_threshold)
			current_shadow_testing_value = shadow_testing_results[shadow_testing_counter++];


		if ((dif<dif_threshold)
			|| (salt(sdec(jdate(month, day)), hour + stadj(jdate(month, day))) <0)) {
			for (k = 0; k < TotalNumberOfDCFiles; k++){
				for (j = 0; j<number_of_sensors; j++)
					fprintf(SHADING_ILLUMINANCE_FILE[k], " %.0f", 0.0);
			}
			if ((dif>dif_threshold)
				&& (salt(sdec(jdate(month, day)), hour + stadj(jdate(month, day))) <0)
				&& all_warnings) {
				sprintf(errmsg, "sun below horizon at %d %d %.3f (solar altitude: %.3f)", month, day, hour, 57.38*salt(sdec(jdate(month, day)), hour + stadj(jdate(month, day))));
				error(WARNING, errmsg);
			}
		}
		else{
			write_segments_diffuse(dir, dif);
			write_segments_direct(dir, dif, number_direct_coefficients, shadow_testing_on, 0, current_shadow_testing_value);
		}

		/* End of Line */
		for (k = 0; k<TotalNumberOfDCFiles; k++)
			fprintf(SHADING_ILLUMINANCE_FILE[k], "\n");

		if (reset){ hour = hour_bak; reset = 0; }
	}
	i = close_file(INPUT_DATAFILE);
	for (k = 0; k<TotalNumberOfDCFiles; k++)
		close_file(SHADING_ILLUMINANCE_FILE[k]);
}


int get_sky_patch_number( float Dx,float  Dy,float Dz)
{
	int i=0,j=0,k=0,l=0;
	double a=0;
	int  patches[8] = { 30, 30, 24, 24, 18, 12, 6, 1 };

	a = RTD * asin(Dz);
	if( (a>=0.0)&&(a<12.0)){j=0;k=0;}
	if( (a>=12.0)&&(a<24.0)){j=1;k=30;}
	if( (a>=24.0)&&(a<36.0)){j=2;k=60;}
	if( (a>=36.0)&&(a<48.0)){j=3;k=84;}
	if( (a>=48.0)&&(a<60.0)){j=4;k=108;}
	if( (a>=60.0)&&(a<72.0)){j=5;k=126;}
	if( (a>=72.0)&&(a<84.0)){j=6;k=138;}
	if( (a>=84.0)&&(a<90.0)){j=7;k=144;i=144;}
	/*printf(" %f %f\n",a,Dz);*/
	a= RTD * atan2(Dy, Dx);
	for (l=0 ; l<patches[j]; l++){
		if(((l+1)*(360.0/patches[j])>=a )&& ((l)*(360.0/patches[j])<a )){i=k+l;l=patches[j];}
	}

	return(i);
}


double skybright(double Dx, double Dy, double Dz, double A1, double A2, double A3, double A4,
	double A5, double A6, double A7, double A8, double A9, double A10, double dir, int number)
{
	/* Perez Model */
	double illuminance, intersky,  gamma;

	if(( Dx*A8 + Dy*A9 + Dz*A10)<1 ) {
		gamma = acos(Dx*A8 + Dy*A9 + Dz*A10);
	} else {
		gamma=0.0093026;
	}
	if ( (gamma < 0.0093026) ){ gamma=0.0093026;}

	if(Dz >0.01) {
		intersky= A1* (1 + A3*exp(A4/Dz  ) ) * ( 1 + A5*exp(A6*gamma) + A7*cos(gamma)*cos(gamma) );
	} else {
		intersky= A1* (1 + A3*exp(A4/0.01) ) * ( 1 + A5*exp(A6*gamma) + A7*cos(gamma)*cos(gamma) ) ;
	}

	illuminance=(pow(Dz+1.01,10)*intersky+pow(Dz+1.01,-10)*A2)/(pow(Dz+1.01,10)+pow(Dz+1.01,-10));

	if (  illuminance <0 ) illuminance =0;
	return(illuminance);
}

double CIE_SKY(double Dz,float A2,float A3)
{	/* CIE overcast sky */
	double illuminance, intersky;

	intersky= A2 * (1 + 2*Dz)/3;

	illuminance=(pow(Dz+1.01,10)*intersky+pow(Dz+1.01,-10)*A3)/(pow(Dz+1.01,10)+pow(Dz+1.01,-10));
	return(illuminance);
}

// =========================================================
// This function calculates the Sky lumiance distribution
// and lumiance efficacy model based on direct and diffuse
// irradiances/illuminances (depending on input climate file
// =========================================================
int write_segments_diffuse(double dir,double dif)
{
    
	int j, jd;
	double 	dzeta, gamma;
	double	normfactor=0.0;
	double altitude,azimuth;
	int S_INTER=0;
	double  A1,A2,A3,A4,A5,A6,A7;
	double reduction=1.0;
	double  sd, st;

	float 	lv_mod[145];  /* 145 luminance values*/

	static float theta_o[145] = {
		84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
		72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
		60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
		48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
		36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		12, 12, 12, 12, 12, 12,
		0
	};

	static float phi_o[145] = {
		0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240, 252, 264, 276, 288, 300, 312, 324, 336, 348,
		0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240, 252, 264, 276, 288, 300, 312, 324, 336, 348,
		0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345,
		0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345,
		0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300, 320, 340,
		0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330,
		0, 60, 120, 180, 240, 300,
		0
	};

	static double coeff_perez[160] = {
		1.352500, -0.257600, -0.269000, -1.436600, -0.767000, 0.000700, 1.273400, -0.123300,
		2.800000, 0.600400, 1.237500, 1.000000, 1.873400, 0.629700, 0.973800, 0.280900,
		0.035600, -0.124600, -0.571800, 0.993800, -1.221900, -0.773000, 1.414800, 1.101600,
		-0.205400, 0.036700, -3.912800, 0.915600, 6.975000, 0.177400, 6.447700, -0.123900,
		-1.579800, -0.508100, -1.781200, 0.108000, 0.262400, 0.067200, -0.219000, -0.428500,
		-1.100000, -0.251500, 0.895200, 0.015600, 0.278200, -0.181200, -4.500000, 1.176600,
		24.721901, -13.081200, -37.700001, 34.843800, -5.000000, 1.521800, 3.922900, -2.620400,
		-0.015600, 0.159700, 0.419900, -0.556200, -0.548400, -0.665400, -0.267200, 0.711700,
		0.723400, -0.621900, -5.681200, 2.629700, 33.338902, -18.299999, -62.250000, 52.078098,
		-3.500000, 0.001600, 1.147700, 0.106200, 0.465900, -0.329600, -0.087600, -0.032900,
		-0.600000, -0.356600, -2.500000, 2.325000, 0.293700, 0.049600, -5.681200, 1.841500,
		21.000000, -4.765600, -21.590599, 7.249200, -3.500000, -0.155400, 1.406200, 0.398800,
		0.003200, 0.076600, -0.065600, -0.129400, -1.015600, -0.367000, 1.007800, 1.405100,
		0.287500, -0.532800, -3.850000, 3.375000, 14.000000, -0.999900, -7.140600, 7.546900,
		-3.400000, -0.107800, -1.075000, 1.570200, -0.067200, 0.401600, 0.301700, -0.484400,
		-1.000000, 0.021100, 0.502500, -0.511900, -0.300000, 0.192200, 0.702300, -1.631700,
		19.000000, -5.000000, 1.243800, -1.909400, -4.000000, 0.025000, 0.384400, 0.265600,
		1.046800, -0.378800, -2.451700, 1.465600, -1.050000, 0.028900, 0.426000, 0.359000,
		-0.325000, 0.115600, 0.778100, 0.002500, 31.062500, -14.500000, -46.114799, 55.375000,
		-7.231200, 0.405000, 13.350000, 0.623400, 1.500000, -0.642600, 1.856400, 0.563600
	};

	jd = jdate(month, day);		/* Julian date */
	sd = sdec(jd);			/* solar declination */
	st = hour + stadj(jd);
	altitude = salt(sd, st);
	azimuth = sazi(sd, st);
	daynumber = (double)jdate(month, day);
	sundir[0] = -sin(azimuth)*cos(altitude);
	sundir[1] = -cos(azimuth)*cos(altitude);
	sundir[2] = sin(altitude);
	sunzenith = 90 - altitude * RTD;

	/* compute the inputs for the calculation of the light distribution over the sky*/
	/*input = 1; input_unities 1	 dir normal Irr [W/m^2] dif hor Irr [W/m^2] */
	if (input_unities==1){	directirradiance = dir;
		diffusirradiance = dif;}
	/*input = 2; input_unities 3 dir hor Irr [W/m^2] dif hor Irr [W/m^2] */
	if (input_unities==2){	directirradiance = dir;
		diffusirradiance = dif;}

	/*input = 3; input_unities 2 dir nor Ill [Lux] dif hor Ill [Lux] */
	if (input_unities==3){	directilluminance = dir;
		diffusilluminance = dif;}

	//output 	OutputUnits
	//  	0	W visible
	//  	1	W solar radiation
	//  	2	lumen

	// compute the inputs for the calculation of the light distribution over the sky

	if (input_unities==1 || input_unities==2) {
		if (input_unities==2){
			if (altitude*RTD<1.0)
				{directirradiance=0;diffusirradiance=0;}
			else {
				if (directirradiance>0.0)
					directirradiance=directirradiance/sin(altitude);
			}
		}
		check_irradiances();
		skybrightness = sky_brightness();
		skyclearness =  sky_clearness();
		check_parametrization();
		
		diffusilluminance = diffusirradiance*glob_h_diffuse_effi_PEREZ();/*diffuse horizontal illuminance*/
		directilluminance = directirradiance*direct_n_effi_PEREZ();
		check_illuminances();
		
	}
	if (input_unities==3){
		check_illuminances();
		illu_to_irra_index();
		check_parametrization();
	}


	/* parameters for the perez model */
	coeff_lum_perez(radians(sunzenith), skyclearness, skybrightness, coeff_perez);

	/*calculation of the modelled luminance */
	for (j=0;j<145;j++)	{
		theta_phi_to_dzeta_gamma(radians(theta_o[j]),radians(phi_o[j]),&dzeta,&gamma,radians(sunzenith));
		lv_mod[j] = (float)calc_rel_lum_perez(dzeta, gamma, radians(sunzenith), skyclearness, skybrightness, coeff_perez);
	}

	/* integration of luminance for the normalization factor, diffuse part of the sky*/
	diffnormalization = integ_lv(lv_mod, theta_o);


	/*normalization coefficient in lumen or in watt*/
	//OutputUnits==0 visible W
	diffnormalization_visible_radiation = diffusilluminance/diffnormalization/WHTEFFICACY;

	//OutputUnits==1 solar radiation
	diffnormalization_solar_radiation = diffusirradiance/diffnormalization;

	// OutputUnits==3 illuminance/luminance
	diffnormalization_luminance = diffusilluminance/diffnormalization;


	/* calculation for the solar source */
	solarradiance_visible_radiation = directilluminance / (2 * PI*(1 - cos(half_sun_angle*DTR))) / WHTEFFICACY;
	solarradiance_solar_radiation = directirradiance / (2 * PI*(1 - cos(half_sun_angle*DTR)));
	solarradiance_luminance = directilluminance / (2 * PI*(1 - cos(half_sun_angle*DTR)));


	/* Compute the ground radiance */
	zenithbr_visible_radiation=calc_rel_lum_perez(0.0,radians(sunzenith),radians(sunzenith),skyclearness,skybrightness,coeff_perez);
	zenithbr_solar_radiation=zenithbr_visible_radiation;
	zenithbr_luminance=zenithbr_visible_radiation;

	zenithbr_visible_radiation*=diffnormalization_visible_radiation;
	zenithbr_solar_radiation*=diffnormalization_solar_radiation;
	zenithbr_luminance*=diffnormalization_luminance;
	if (skyclearness==1)
		normfactor = 0.777778;

	if (skyclearness>=6)
		{
			F2 = 0.274*(0.91 + 10.0*exp(-3.0*(PI/2.0-altitude)) + 0.45*sundir[2]*sundir[2]);
			normfactor = normsc(altitude,S_INTER)/F2/PI;
		}

	if ( (skyclearness>1) && (skyclearness<6) )
		{
			S_INTER=1;
			F2 = (2.739 + .9891*sin(.3119+2.6*altitude)) * exp(-(PI/2.0-altitude)*(.4441+1.48*altitude));
			normfactor = normsc(altitude,S_INTER)/F2/PI;
		}

	groundbr_visible_radiation = zenithbr_visible_radiation*normfactor;
	groundbr_solar_radiation = zenithbr_solar_radiation*normfactor;
	groundbr_luminance = zenithbr_luminance*normfactor;

	if (dosun&&(skyclearness>1))
		{
			groundbr_visible_radiation += 6.8e-5/PI*solarradiance_visible_radiation*sundir[2];
			groundbr_solar_radiation += 6.8e-5/PI*solarradiance_solar_radiation*sundir[2];
			groundbr_luminance += 6.8e-5/PI*solarradiance_luminance*sundir[2];
		}
	groundbr_visible_radiation *= gprefl;
	groundbr_solar_radiation *= gprefl;
	groundbr_luminance *= gprefl;


	/*=============================*/
	/*=== diffuse contribution ====*/
	/*=============================*/


	if (dosun&&(skyclearness==1))
		{
			solarradiance_visible_radiation=0;
			solarradiance_solar_radiation=0;
			solarradiance_luminance=0;
		}


	/*change input irradiances according to the quantities specified in the header*/
	reduction=1.0;
	if(solar_pen){
		reduction=0.75;
		if(altitude> lower_lat && altitude < upper_lat){
			if(azimuth> lower_azi && azimuth<upper_azi){
				if(dir>=50){
					reduction=reduction_diffuse;
					solarradiance_visible_radiation=0;
					solarradiance_solar_radiation=0;
					solarradiance_luminance=0;
				}
			}
		}
	}

			//assign Perez Coefficients
			A1=diffnormalization_luminance;	/*A1			- diffus normalization*/
			A2=groundbr_luminance;			/*A2			- ground brightness*/
			A3= c_perez[0];				/*A3            - coefficients for the Perez model*/
			A4= c_perez[1];
			A5= c_perez[2];
			A6= c_perez[3];
			A7= c_perez[4];


			for (j=0 ; j<145 ; j++){
				SkyPatchLuminance[j] = (float)(horizon_factor[j] * skybright(Dx_dif_patch[j], Dy_dif_patch[j], Dz_dif_patch[j], A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, j));
				if( horizon_factor[j]<1)
					SkyPatchLuminance[j] += (float)((1 - horizon_factor[j]) * skybright(0.9961946, 0, -0.087156, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0));
				if(reduction<1.0)
					SkyPatchLuminance[j] *= (float)reduction;
			}

			// assign ground brightness
			if(dds_file_format) // new dds file format chosen
				{
					/* 0 to 10 */  SkyPatchLuminance[145] = (float)skybright(0.9397, 0, -0.342, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
					//		/* 0 to 10 */  SkyPatchLuminance[145]=skybright(0.9961946,0,-0.087156,A1,A2,A3,A4,A5,A6,A7,sundir[0],sundir[1],sundir[2],dir,0);
				}else{
				/* 0 to 10 */  SkyPatchLuminance[145] = (float)skybright(0.9961946, 0, -0.087156, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
				/* 10 to 30*/  SkyPatchLuminance[146] = (float)skybright(0.9397, 0, -0.342, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
				/* 30 to 90 */ SkyPatchLuminance[147] = (float)skybright(0, 0, -1.0, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
			}

			for (j=0 ; j<148 ; j++) {
				if(SkyPatchLuminance[j]>180000)
					SkyPatchLuminance[j]=0;
			}


			//assign Perez Coefficients
			A1=diffnormalization_solar_radiation;	/*A1			- diffus normalization*/
			A2=groundbr_solar_radiation;			/*A2			- ground brightness*/
			A3= c_perez[0];				/*A3            - coefficients for the Perez model*/
			A4= c_perez[1];
			A5= c_perez[2];
			A6= c_perez[3];
			A7= c_perez[4];
			
			for (j=0 ; j<145 ; j++){
				SkyPatchSolarRadiation[j] = (float)(horizon_factor[j] * skybright(Dx_dif_patch[j], Dy_dif_patch[j], Dz_dif_patch[j], A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, j));
				if( horizon_factor[j]<1)
					SkyPatchSolarRadiation[j] += (float)((1 - horizon_factor[j])*skybright(0.9961946, 0, -0.087156, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0));
				if(reduction<1.0)
					SkyPatchSolarRadiation[j] *= (float)reduction;
			}
			// assign ground brightness
			if(dds_file_format) // new dds file format chosen
			{
				/* 0 to 10 */  SkyPatchSolarRadiation[145] = (float)skybright(0.9397, 0, -0.342, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
			}else{
				/* 0 to 10 */  SkyPatchSolarRadiation[145] = (float)skybright(0.9961946, 0, -0.087156, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
				/* 10 to 30*/  SkyPatchSolarRadiation[146] = (float)skybright(0.9397, 0, -0.342, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
				/* 30 to 90 */ SkyPatchSolarRadiation[147] = (float)skybright(0, 0, -1.0, A1, A2, A3, A4, A5, A6, A7, sundir[0], sundir[1], sundir[2], dir, 0);
			}

			for (j=0 ; j<148 ; j++) {
				if(SkyPatchSolarRadiation[j]>1700)
					SkyPatchSolarRadiation[j]=0;
			}
			
			
			
	return 0;
}





double
normsc(double altitude,int       S_INTER)			/* compute normalization factor (E0*F2/L0) */
{
	static double  nfc[2][5] = {
		/* clear sky approx. */
		{2.766521, 0.547665, -0.369832, 0.009237, 0.059229},
		/* intermediate sky approx. */
		{3.5556, -2.7152, -1.3081, 1.0660, 0.60227},
	};
	register double  *nf;
	double  x, nsc;
	register int  i;
	/* polynomial approximation */
	nf = nfc[S_INTER];
	x = (altitude - PI/4.0)/(PI/4.0);
	nsc = nf[i=4];
	while (i--)
		nsc = nsc*x + nf[i];

	return(nsc);
}





/*=============================*/
/*=== direct contribution =====*/
/*=============================*/

int write_segments_direct(double dir,double dif, int number_direct_coefficients, int *shadow_testing_on, int count_var,int current_shadow_testing_value)
{
	static int number145[8]= { 0, 30, 60, 84, 108, 126, 138, 144 };
	static double ring_division145[8]= { 30.0, 30.0, 24.0, 24.0, 18.0, 12.0, 6.0, 0.0 };
	static int number2305[29]= { 0,120,240,360,480,600,720,840,960,1056,1152,1248,1344,1440,1536,1632,1728,1800,1872,1944,2016,2064,2112,2160,2208,2232,2256,2280,2304 };
	static double ring_division2305[29]= { 120.0,120.0,120.0,120.0,120.0,120.0,120.0,120.0,96.0,96.0,96.0,96.0,96.0,96.0,96.0,96.0,72.0,72.0,72.0,72.0,48.0,48.0,48.0,48.0,24.0,24.0,24.0,24.0,0.0 };

	static float DirectDC_theta[145] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
		30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
		42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
		54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
		66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
		78, 78, 78, 78, 78, 78,
		90
	};

	static float DirectDC_phi[145] = {
		-96, -108, -120, -132, -144, -156, -168, 180, 168, 156, 144, 132, 120, 108, 96, 84, 72, 60, 48, 36, 24, 12, 0, -12, -24, -36, -48, -60, -72, -84,
		-96, -108, -120, -132, -144, -156, -168, 180, 168, 156, 144, 132, 120, 108, 96, 84, 72, 60, 48, 36, 24, 12, 0, -12, -24, -36, -48, -60, -72, -84,
		-97.5, -112.5, -127.5, -142.5, -157.5, -172.5, 172.5, 157.5, 142.5, 127.5, 112.5, 97.5, 82.5, 67.5, 52.5, 37.5, 22.5, 7.5, -7.5, -22.5, -37.5, -52.5, -67.5, -82.5,
		-97.5, -112.5, -127.5, -142.5, -157.5, -172.5, 172.5, 157.5, 142.5, 127.5, 112.5, 97.5, 82.5, 67.5, 52.5, 37.5, 22.5, 7.5, -7.5, -22.5, -37.5, -52.5, -67.5, -82.5,
		-100, -120, -140, -160, 180, 160, 140, 120, 100, 80, 60, 40, 20, 0, -20, -40, -60, -80,
		-105, -135, -165, 165, 135, 105, 75, 45, 15, -15, -45, -75,
		-120, 180, 120, 60, 0, -60,
		0
	};

	int ringnumber;

	int i=0,j=0,h00=0,h01=0,k=0,i_last=0;
	double angle1 = 0.0, max_angle = 0.0, Nx = 0.0, Ny = 0.0, Nz = 0.0;
	int mon1=0, mon0=0,jd0=0,jd1=0, jd=0;
	double solar_time=0.0, sd=0.0;
	int chosen_value1=0, chosen_value2=0, chosen_value3=0, chosen_value4=0;
	float min_diff1=1, min_diff2=1, min_diff3=1, min_diff4=1;
	double azimuth_tmp = 0;
	double chosen_time=0.0 ,time_difference=0.0,max_time_difference=0.0, min_alt_difference=0.0;
	double weight1=0.0,weight2=0.0, weight3=0.0,weight4=0.0,sum_weight=0.0;
	double altitude=0.0, azimuth=0.0;
	double altitude1=0.0, azimuth1=0.0,altitude0=0.0, azimuth0=0.0;
	int chosen_value=0, shadow_counter=0;
	double adapted_time0=0, adapted_time1=0;
	int number_of_diffuse_and_ground_dc=148;
	double summe1 = 0.0;
	double Dx, Dy, Dz;
	int j1=0;
	float *pointer_dc;
	float *pointer_sky= NULL;
	double DirectDirectSkyPatchSolarRadiation = 0;
	double DirectDirectSkyPatchLuminance = 0;
	double DirectDirectContribution = 0;
	int base_value=0;

	if(dds_file_format) { //DDS
		number_of_diffuse_and_ground_dc=146;
		dc_coupling_mode=4;
	}

	if( dir<=dir_threshold ) {	//discard direct contribution
		for (k=0 ; k < TotalNumberOfDCFiles; k++) {
			dc_shading_coeff_rewind( k );

			for (j=0 ; j<number_of_sensors ; j++) {
//#ifndef PROCESS_ROW
//#else
//			struct dc_shading_coeff_data_s* dcd= dc_shading_coeff_read( k );
//#endif


				if(sensor_unit[j]==0 || sensor_unit[j]==1)
					pointer_sky = &SkyPatchLuminance[0];
				if(sensor_unit[j]==2 || sensor_unit[j]==3 )
					pointer_sky = &SkyPatchSolarRadiation[0];

				summe1=0;
				i=0;
//#ifndef PROCESS_ROW
				pointer_dc = dc_shading[k][j];
//#else
//				pointer_dc= dcd->dc;
//#endif
				while(i<number_of_diffuse_and_ground_dc){
					summe1+= (*pointer_dc) * (*pointer_sky);
//#ifndef PROCESS_ROW
					pointer_dc+=dc_shading_next[k][j][i]-i;
					pointer_sky+=dc_shading_next[k][j][i]-i;
					i= dc_shading_next[k][j][i];
//#else
//					pointer_dc+= dcd->next[i] - i;
//					pointer_sky+= dcd->next[i] - i;
//					i= dcd->next[i];
//#endif
				}
				// simplified blinds model
				if(simple_blinds_model==1 && k ==1 )
					summe1*=0.25;
				if(sensor_unit[j]==0 || sensor_unit[j]==1)	
					fprintf(SHADING_ILLUMINANCE_FILE[k]," %.0f",summe1);
				if(sensor_unit[j]==2 || sensor_unit[j]==3 )	
					fprintf(SHADING_ILLUMINANCE_FILE[k]," %.2f",summe1);
			}
		}

		/*reset SkyPatchLuminance & SkyPatchSolarRadiation */
		for (i=0 ; i< number_of_diffuse_and_ground_dc; i++)
			{
				SkyPatchLuminance[i]=0;
				SkyPatchSolarRadiation[i]=0;
			}


	}else{
		jd= jdate(month, day);
		sd=sdec(jd);
		solar_time=hour+stadj(jdate(month, day));
		altitude = RTD * salt( sd,solar_time);
		azimuth = RTD * sazi(sd, solar_time);


		switch ( dc_coupling_mode) {
			/* interpolated among 4 nearest direct coefficients =0*/
			/* interpolation with shadow testing =2 */
		case 0 : case 2  : {
			/* regular or advanced interpolation*/

			//===========================================
			// calculate weights of four nearest patches
			//===========================================

			// Step 1: find the four nearest patches
			//======================================
			if (jd >= 355 || jd < 52) {			/*Dec Feb*/
				mon0=12; mon1=2;jd0=355;jd1=52;}
			if (jd >= 52 && jd < 80) { 			/* Feb Mar*/
				mon0=2; mon1=3;jd0=52;jd1=80;}
			if (jd >= 80 && jd < 111) { 			/*Mar Apr*/
				mon0=3; mon1=4;jd0=80;jd1=111;}
			if (jd >= 111 && jd < 172) { 			/*Apr Jun*/
				mon0=4; mon1=6;jd0=111;jd1=172;}
			if (jd >= 172 && jd < 233) {			 /*Jun Aug*/
				mon0=4; mon1=6;jd1=172;jd0=233;}
			if (jd >= 233 && jd < 264) { 			/*Aug Sep*/
				mon0=3; mon1=4;jd1=233;jd0=264;}
			if (jd >= 264 && jd < 294) { 			/*Sep Oct*/
				mon0=2; mon1=3;jd1=264;jd0=294;}
			if (jd >= 294 && jd < 355) { 			 /*Oct Dec*/
				mon0=12; mon1=2;jd1=294;jd0=355;}

			altitude0 = RTD * salt(sdec(jd0), solar_time);
			azimuth0 = RTD * sazi(sdec(jd0), solar_time);
			altitude1 = RTD * salt(sdec(jd1), solar_time);
			azimuth1 = RTD * sazi(sdec(jd1), solar_time);

			/* find corresponding time */
			adapted_time0=hour+stadj(jdate(mon0, 21));
			adapted_time1=hour+stadj(jdate(mon1, 21));
			for (j=1 ; j<25 ; j++){
				if ( adapted_time0 > direct_calendar[mon0][j][0] && adapted_time0 < direct_calendar[mon0][j+1][0] ){h00=j;j=25;}
				/*printf("adapted %f %f\n",adapted_time,direct_calendar[mon0][j+1][0]);*/
				if (j == 24) error(INTERNAL, "loop in gendaylit_algorithm failed");
			}
			for (j=1 ; j<25 ; j++){
				if ( adapted_time1 > direct_calendar[mon1][j][0] && adapted_time1 < direct_calendar[mon1][j+1][0] ){h01=j;j=25;}
				if (j == 24) error(INTERNAL, "loop in gendaylit_algorithm failed");
			}

			Dx = cos(DTR*azimuth)*cos(DTR*altitude);
			Dy = sin(DTR*azimuth)*cos(DTR*altitude);
			Dz = sin(DTR*altitude);

			// Step 2: assign weights
			//======================================
			weight1=0.0;
			weight2=0.0;
			weight3=0.0;
			weight4=0.0;

			// consider only patches with a positive altitude
			shadow_counter=0;
			if(direct_calendar[mon0][h00][1]>=0.0)
				weight1=1.0;
			if(direct_calendar[mon0][h00+1][1]>=0.0)
				weight2=1.0;
			if(direct_calendar[mon1][h01][1]>=0.0)
				weight3=1.0;
			if(direct_calendar[mon1][h01+1][1]>=0.0)
				weight4=1.0;

			// in shadow testing activated take out those patches that
			// a have a different status than the current sun position
			if( *shadow_testing_on){
				if(current_shadow_testing_value != direct_calendar[mon0][h00][3])
					{
						fprintf(stdout,"weight 1 set to 0 (%d date: %d %d %.3f)\n",current_shadow_testing_value, month,day,hour);
						weight1=0.0;
					}
				if(current_shadow_testing_value != direct_calendar[mon0][h00+1][3])
					{
						fprintf(stdout,"weight 2 set to 0 (%d date: %d %d %.3f)\n",current_shadow_testing_value, month,day,hour);
						weight2=0.0;
					}
				if(current_shadow_testing_value != direct_calendar[mon1][h01][3])
					{
						fprintf(stdout,"weight 3 set to 0 (%d date: %d %d %.3f)\n",current_shadow_testing_value, month,day,hour);
						weight3=0.0;
					}
				if(current_shadow_testing_value != direct_calendar[mon1][h01+1][3])
					{
						fprintf(stdout,"weight 4 set to 0 (%d date: %d %d %.3f)\n",current_shadow_testing_value, month,day,hour);
						weight4=0.0;
					}
			}

			//assign the weight according to the distance of the current patch from the
			// individual four patches
			weight1*=(altitude1-altitude)/(altitude1-altitude0);
			weight2*=(altitude1-altitude)/(altitude1-altitude0);
			weight3*=(altitude-altitude0)/(altitude1-altitude0);
			weight4*=(altitude-altitude0)/(altitude1-altitude0);
			weight1*=((direct_calendar[mon0][h00+1][0]-adapted_time0)/(direct_calendar[mon0][h00+1][0]-direct_calendar[mon0][h00][0]));
			weight2*=((adapted_time0-direct_calendar[mon0][h00][0])/(direct_calendar[mon0][h00+1][0]-direct_calendar[mon0][h00][0]));
			weight3*=((direct_calendar[mon1][h01+1][0]-adapted_time1)/(direct_calendar[mon1][h01+1][0]-direct_calendar[mon1][h01][0]));
			weight4*=((adapted_time1-direct_calendar[mon1][h01][0])/(direct_calendar[mon1][h01+1][0]-direct_calendar[mon1][h01][0]));

			// normalize weights
			sum_weight=weight1+weight2+weight3+weight4;
			if(sum_weight>0){
				weight1=weight1/sum_weight;
				weight2=weight2/sum_weight;
				weight3=weight3/sum_weight;
				weight4=weight4/sum_weight;
			}else{
				if( *shadow_testing_on)
				{
					sprintf(errmsg, "shadow testing - all 4 nearest patches have a different status than the current sun position (%d date: %d %d %.3f), direct sun contribution turned off",current_shadow_testing_value, month,day,hour);
					error(WARNING, errmsg);
				}
				weight1=0.0;
				weight2=0.0;
				weight3=0.0;
				weight4=0.0;
			}

			//====================================================
			//combine direct irradiance with daylight coefficients
			//====================================================
			j1=0;
			i=12;

			if ( ( i==mon0 ) || ( i==mon1 )){
				if ( i==mon0 ) {
					for (j=1 ; j<h00 ; j++){
						if(direct_calendar[i][j][1]>0)
							j1++;
					}
					if (direct_calendar[i][h00][1]>0){
						j1++;
						SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight1);
						SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight1);
					}
					if (direct_calendar[i][h00+1][1]>0){
						j1++;
						SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight2);
						SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight2);
					}
					for (j=h00+2 ; j<25 ; j++){
						if(direct_calendar[i][j][1]>0)
							j1++;
					}
				}
				if ( i==mon1 ) {
					for (j=1 ; j<h01 ; j++){
						if(direct_calendar[i][j][1]>0){
							j1++;
						}
					}
					if (direct_calendar[i][h01][1]>0){
						j1++;
						SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight3);
						SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight3);
					}
					if (direct_calendar[i][h01+1][1]>0){
						j1++;
						SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight4);
						SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight4);
					}
					for (j=h01+2 ; j<25 ; j++){
						if(direct_calendar[i][j][1]>0){
							j1++;
						}
					}
				}
			}else{ // not ( i==mon0 ) || ( i==mon1 )
				for (j=1 ; j<25 ; j++){
					if(direct_calendar[i][j][1]>0)
						j1++;
				}
			}

			for (i=1 ; i<12 ; i++){
				if ( ( i==mon0 ) || ( i==mon1 )){
					if ( i==mon0 ) {
						for (j=1 ; j<h00 ; j++){
							if(direct_calendar[i][j][1]>0)
								j1++;
						}
						if (direct_calendar[i][h00][1]>0){
							j1++;
							SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight1);
							SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight1);
						}
						if (direct_calendar[i][h00+1][1]>0){
							j1++;
							SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight2);
							SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight2);
						}
						for (j=h00+2 ; j<25 ; j++){
							if(direct_calendar[i][j][1]>0){
								j1++;
							}
						}
					}
					if ( i==mon1 ) {
						for (j=1 ; j<h01 ; j++){
							if(direct_calendar[i][j][1]>0){
								j1++;
							}
						}
						if (direct_calendar[i][h01][1]>0){
							j1++;
							SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight3);
							SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight3);
						}
						if (direct_calendar[i][h01+1][1]>0){
							j1++;
							SkyPatchLuminance[147 + j1] = (float)(solarradiance_luminance * weight4);
							SkyPatchSolarRadiation[147 + j1] = (float)(solarradiance_solar_radiation * weight4);
						}
						for (j=h01+2 ; j<25 ; j++){
							if(direct_calendar[i][j][1]>0)
								j1++;
						}
					}
				}else{
					for (j=1 ; j<25 ; j++){
						if(direct_calendar[i][j][1]>0)
							j1++;
					}
				}

			}
			break;}
		case 1 :{ /* nearest neighbor*/
			chosen_time=0; time_difference=0;
			max_time_difference=12; min_alt_difference=90;
			max_angle=2*PI;
			Dx = cos(DTR*azimuth)*cos(DTR*altitude);
			Dy = sin(DTR*azimuth)*cos(DTR*altitude);
			Dz = sin(DTR*altitude);
			for (j=0 ; j< number_direct_coefficients; j++){
				Nx = cos(DTR*direct_pts[j][2])*cos(DTR*direct_pts[j][1]);
				Ny = sin(DTR*direct_pts[j][2])*cos(DTR*direct_pts[j][1]);
				Nz = sin(DTR*direct_pts[j][1]);
				angle1=acos(Nx*Dx+Ny*Dy+Nz*Dz);
				if (angle1 < max_angle ){
					if(!(*shadow_testing_on) || ((direct_view[count_var] - direct_pts[j][3]) == 0)){
						max_angle = angle1;
						chosen_value = j;
					}
				}
			}
			if(max_angle==2*PI) {
				sprintf(errmsg, "shadow test of point at %d %d %f does not correspond to any direct daylight coefficient", month, day, hour);
				error(WARNING, errmsg);
			}
			SkyPatchLuminance[148 + chosen_value] = (float)solarradiance_luminance;
			SkyPatchSolarRadiation[148 + chosen_value] = (float)solarradiance_solar_radiation;
			break;
		}
		case 3 : { /* no direct contribution*/
			break;
		}
		case 4 : { /* new dds file format*/

			//find four nearest direct-indirect sun postions
			//++++++++++++++++++++++++++++++++++++++++++++++
			chosen_value1=0;
			chosen_value2=0;
			chosen_value3=0;
			chosen_value4=0;
			min_diff1=0;
			min_diff2=0;
			min_diff3=0;
			min_diff4=0;

			if(azimuth<=-90)
				azimuth_tmp=(-1)*(azimuth+90.0);
			else
				azimuth_tmp=(270.0-azimuth);

			azimuth_tmp *= DTR;

			Dx = cos(azimuth_tmp)*cos(DTR*altitude);
			Dy = sin(azimuth_tmp)*cos(DTR*altitude);
			Dz = sin(DTR*altitude);

			//determine four surrounding direct-indirect DC coordinates:

			if(altitude<=6.0) //interpolate between two DC
			{
				for (j=0 ; j<30;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j])<12.0) && ((fabs(azimuth - DirectDC_phi[j + 1]) <= 12.0) || (fabs(azimuth - DirectDC_phi[j] - 360.0) <= 12.0)))
					{
						chosen_value1=j;
						chosen_value2=j+1;
						if(chosen_value2==30)
							chosen_value2=0;
						j=30;
					}
				}

				chosen_value3=40;//assign bogis value and set weight to zero
				chosen_value4=41;//assign bogis value and set weight to zero

				weight1=1.0;
				weight2=1.0;
				weight3=0.0;
				weight4=0.0;


			}

			//second band 6 Deg to 18 Deg
			//===========================
			if(altitude>6.0 && altitude <=18.0)
			{
				//get two direct-indirect DC in first band
				base_value=0;
				for (j=0 ; j<30;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<12.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 12.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 12.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+30)
							chosen_value2=base_value;
						j=30;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+29;
					chosen_value2=base_value;
				}

				//get two direct-indirect DC in second band
				base_value=30;
				for (j=0 ; j<30;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<12.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 12.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 12.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+30)
							chosen_value4=base_value;
						j=30;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+29;
					chosen_value4=base_value;
				}

				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}

			//third band 18 Deg to 30 Deg
			//===========================
			if(altitude>18.0 && altitude <=30.0)
			{
				//get two direct-indirect DC in first band
				base_value=30;
				for (j=0 ; j<30;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<12.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 12.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 12.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+30)
							chosen_value2=base_value;
						j=30;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+29;
					chosen_value2=base_value;
				}

				//get two direct-indirect DC in second band
				base_value=60;
				for (j=0 ; j<24;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<15.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 15.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 15.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+24)
							chosen_value4=base_value;
						j=24;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+23;
					chosen_value4=base_value;
				}

				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}


			//forth band 30 Deg to 42 Deg
			//===========================
			if(altitude>30.0 && altitude <=42.0)
			{
				//get two direct-indirect DC in first band
				base_value=60;
				for (j=0 ; j<24;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<15.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 15.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 15.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+24)
							chosen_value2=base_value;
						j=24;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+23;
					chosen_value2=base_value;
				}


				//get two direct-indirect DC in second band
				base_value=84;
				for (j=0 ; j<24;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<15.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 15.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 15.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+24)
							chosen_value4=base_value;
						j=24;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+23;
					chosen_value4=base_value;
				}

				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}

			//fifth band 42 Deg to 54 Deg
			//===========================
			if(altitude>42.0 && altitude <=54.0)
			{
				//get two direct-indirect DC in first band
				base_value=84;
				for (j=0 ; j<24;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<15.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 15.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 15.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+24)
							chosen_value2=base_value;
						j=24;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+23;
					chosen_value2=base_value;
				}

				//get two direct-indirect DC in second band
				base_value=108;
				for (j=0 ; j<18;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<20.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 20.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 20.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+18)
							chosen_value4=base_value;
						j=18;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+17;
					chosen_value4=base_value;
				}

				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}


			//sixth band 54 Deg to 66 Deg
			//===========================
			if(altitude>54.0 && altitude <=66.0)
			{
				//get two direct-indirect DC in first band
				base_value=108;
				for (j=0 ; j<18;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<20.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 20.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 20.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+18)
							chosen_value2=base_value;

						j=18;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+17;
					chosen_value2=base_value;
				}

				//get two direct-indirect DC in second band
				base_value=126;
				for (j=0 ; j<12;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<30.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 30.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 30.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+12)
							chosen_value4=base_value;
						j=12;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+11;
					chosen_value4=base_value;
				}
				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}


			//seventh band 66 Deg to 78 Deg
			//===========================
			if(altitude>66.0 && altitude <=78.0)
			{
				//get two direct-indirect DC in first band
				base_value=126;
				for (j=0 ; j<12;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<30.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 30.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 30.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+12)
							chosen_value2=base_value;
						j=12;
					}
				}
				if(chosen_value1==0)
				{
					chosen_value1=base_value+11;
					chosen_value2=base_value;
				}

				//get two direct-indirect DC in second band
				base_value=138;
				for (j=0 ; j<6;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<60.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 60.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 60.0)))
					{
						chosen_value3=base_value+j;
						chosen_value4=base_value+j+1;
						if(chosen_value4==base_value+6)
							chosen_value4=base_value;
						j=6;
					}
				}
				if(chosen_value3==0)
				{
					chosen_value3=base_value+5;
					chosen_value4=base_value;
				}

				weight1=1.0;
				weight2=1.0;
				weight3=1.0;
				weight4=1.0;
			}

			//eighth band 78 Deg to 90 Deg
			//===========================
			if(altitude>78.0 && altitude <=90.0)
			{
				//get two direct-indirect DC in first band
				base_value=138;
				for (j=0 ; j<6;j++)
				{
					if ((fabs(azimuth - DirectDC_phi[j + base_value])<60.0) && ((fabs(azimuth - DirectDC_phi[j + 1 + base_value]) <= 60.0) || (fabs(azimuth - DirectDC_phi[j + base_value] - 360.0) <= 60.0)))
					{
						chosen_value1=base_value+j;
						chosen_value2=base_value+j+1;
						if(chosen_value2==base_value+6)
							chosen_value2=0;
						j=6;
					}
				}

				//get two direct-indirect DC in second band
				chosen_value3=144;
				chosen_value4=1;

				if(altitude ==90.0)
				{
					weight3=1.0;
					weight1=0.0;
					weight2=0.0;
					weight4=0.0;
				} else{
					weight1=1.0;
					weight2=1.0;
					weight3=1.0;
					weight4=0.0;
				}
			}


			//++++++++++++++++++++++++++
			//assign and normize weights
			//++++++++++++++++++++++++++

			weight1 *= fabs((azimuth - DirectDC_phi[chosen_value2]) / (DirectDC_phi[chosen_value1] - DirectDC_phi[chosen_value2]));
			weight2 *= fabs((azimuth - DirectDC_phi[chosen_value1]) / (DirectDC_phi[chosen_value1] - DirectDC_phi[chosen_value2]));
			weight3 *= fabs((azimuth - DirectDC_phi[chosen_value4]) / (DirectDC_phi[chosen_value3] - DirectDC_phi[chosen_value4]));
			weight4 *= fabs((azimuth - DirectDC_phi[chosen_value3]) / (DirectDC_phi[chosen_value3] - DirectDC_phi[chosen_value4]));

			weight1 *= (DirectDC_theta[chosen_value3] - altitude) / (DirectDC_theta[chosen_value3] - DirectDC_theta[chosen_value1]);
			weight2 *= (DirectDC_theta[chosen_value3] - altitude) / (DirectDC_theta[chosen_value3] - DirectDC_theta[chosen_value1]);
			weight3 *= (altitude - DirectDC_theta[chosen_value1]) / (DirectDC_theta[chosen_value3] - DirectDC_theta[chosen_value1]);
			weight4 *= (altitude - DirectDC_theta[chosen_value1]) / (DirectDC_theta[chosen_value3] - DirectDC_theta[chosen_value1]);


			sum_weight=weight1+weight2+weight3+weight4;
			weight1=weight1/sum_weight;
			weight2=weight2/sum_weight;
			weight3=weight3/sum_weight;
			weight4=weight4/sum_weight;


			chosen_value1++;
			chosen_value2++;
			chosen_value3++;
			chosen_value4++;



			SkyPatchLuminance[145 + chosen_value1] = (float)(solarradiance_luminance * weight1);
			SkyPatchLuminance[145 + chosen_value2] = (float)(solarradiance_luminance * weight2);
			SkyPatchLuminance[145 + chosen_value3] = (float)(solarradiance_luminance * weight3);
			SkyPatchLuminance[145 + chosen_value4] = (float)(solarradiance_luminance * weight4);

			SkyPatchSolarRadiation[145 + chosen_value1] = (float)(solarradiance_solar_radiation * weight1);
			SkyPatchSolarRadiation[145 + chosen_value2] = (float)(solarradiance_solar_radiation * weight2);
			SkyPatchSolarRadiation[145 + chosen_value3] = (float)(solarradiance_solar_radiation * weight3);
			SkyPatchSolarRadiation[145 + chosen_value4] = (float)(solarradiance_solar_radiation * weight4);
			

			// Step 2: Identify in which patch the sun currently is for the direct-direct DC
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			ringnumber=(int)(1.0*altitude/3.157895);
			if((azimuth>-90) && (azimuth < 180)){
				chosen_value = number2305[ringnumber] + (int)((270.0 - azimuth) / (360.0 / ring_division2305[ringnumber]));
			}
			else{
				chosen_value = number2305[ringnumber] + (int)((-90.0 - azimuth) / (360.0 / ring_division2305[ringnumber]));
			}

			//printf(" ringnumber %d  chosen_value 2305: %d\n",ringnumber, chosen_value+290);

			SkyPatchLuminance[290 + chosen_value] = (float)solarradiance_luminance;
			DirectDirectSkyPatchLuminance=solarradiance_luminance;
			
			SkyPatchSolarRadiation[290 + chosen_value] = (float)solarradiance_solar_radiation;
			DirectDirectSkyPatchSolarRadiation=solarradiance_solar_radiation;
			
			break;
		}
		case 5 : { /* new dds file format with nearest neighbor for direct-indirect and direct-direct*/

			// Step 1: Identify in which patch the sun currently is for the direct-indirect DC
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			ringnumber=(int)(1.0*altitude/12.0);
			// according to Tregenza, the celestial hemisphere is divided into 7 bands and
			// the zenith patch. The bands range from:
			//												altitude center
			// Band 1		0 to 12 Deg			30 patches	6
			// Band 2		12 to 24 Deg		30 patches	18
			// Band 3		24 to 36 Deg		24 patches	30
			// Band 4		36 to 48 Deg		24 patches	42
			// Band 5		48 to 60 Deg		18 patches	54
			// Band 6		60 to 72 Deg		12 patches	66
			// Band 7		72 to 84 Deg		 6 patches	78
			// Band 8		84 to 90 Deg		 1 patche	90
			// since the zenith patch is only takes 6Deg instead of 12, the arc length
			// between 0 and 90 Deg (equlas 0 and Pi/2) is divided into 7.5 units:
			// Therefore, 7.5 units = (int) asin(z=1)/(Pi/2)
			//				1 unit = asin(z)*(2*7.5)/Pi)
			//				1 unit = asin(z)*(15)/Pi)
			// Note that (int) always rounds to the next lowest integer
			if((azimuth>-90) && (azimuth < 180)){
				chosen_value = number145[ringnumber] + (int)((270.0 - azimuth) / (360.0 / ring_division145[ringnumber]));
			}
			else{
				chosen_value = number145[ringnumber] + (int)((-90.0 - azimuth) / (360.0 / ring_division145[ringnumber]));
			}

			SkyPatchLuminance[145 + chosen_value] = (float)solarradiance_luminance;
			SkyPatchSolarRadiation[145 + chosen_value] = (float)solarradiance_solar_radiation;


			//printf("dds %d dc.coup %d, date %d %d %.3f altitude %f azimuth %f  chosen_value %d ",dds_file_format,dc_coupling_mode,month, day,hour,altitude, azimuth,chosen_value);

			// Step 2: Identify in which patch the sun currently is for the direct-direct DC
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			ringnumber=(int)(1.0*altitude/3.157895);
			if((azimuth>-90) && (azimuth < 180)){
				chosen_value = number2305[ringnumber] + (int)((270.0 - azimuth) / (360.0 / ring_division2305[ringnumber]));
			}
			else{
				chosen_value = number2305[ringnumber] + (int)((-90.0 - azimuth) / (360.0 / ring_division2305[ringnumber]));
			}

			//printf(" ringnumber %d  chosen_value 2305: %d\n",ringnumber, chosen_value+290);

			SkyPatchLuminance[290 + chosen_value] = (float)solarradiance_luminance;
			DirectDirectSkyPatchLuminance=solarradiance_luminance;
			
			SkyPatchSolarRadiation[290 + chosen_value] = (float)solarradiance_solar_radiation;
			DirectDirectSkyPatchSolarRadiation=solarradiance_solar_radiation;

			break;
		}
		} /* end switch */


		/* write out files */
		i_last=number_of_diffuse_and_ground_dc;
			NextNonEmptySkyPatch[number_of_diffuse_and_ground_dc]=(number_of_diffuse_and_ground_dc+ number_direct_coefficients);
			for (i=number_of_diffuse_and_ground_dc ; i<(number_of_diffuse_and_ground_dc+ number_direct_coefficients) ; i++) {
				if(i>number_of_diffuse_and_ground_dc && (SkyPatchLuminance[i]>0.0 )){  //|| SkyPatchSolarRadiation[i]>0.0
					NextNonEmptySkyPatch[i_last]=i;
					NextNonEmptySkyPatch[i]=(number_of_diffuse_and_ground_dc+ number_direct_coefficients);
					i_last=i;
				}
			}

		for (k=0 ; k < TotalNumberOfDCFiles ; k++) {
//#ifndef PROCESS_ROW
//#else
			dc_shading_coeff_rewind( k );
//#endif
			for (j=0 ; j<number_of_sensors ; j++) {


//#ifndef PROCESS_ROW
//#else
//			struct dc_shading_coeff_data_s* dcd= dc_shading_coeff_read( k );
//#endif

				summe1=0;
				i=0;
//#ifndef PROCESS_ROW
				pointer_dc = dc_shading[k][j];
//#else
//				pointer_dc= dcd->dc;
//#endif
				if(sensor_unit[j]==0 || sensor_unit[j]==1)
					{
						pointer_sky = &SkyPatchLuminance[0];
						DirectDirectContribution = DirectDirectSkyPatchLuminance;
					}
				if(sensor_unit[j]==2 || sensor_unit[j]==3 )
					{
						pointer_sky = &SkyPatchSolarRadiation[0];
						DirectDirectContribution = DirectDirectSkyPatchSolarRadiation;
					}

				// simplified blind model:
				// sum over all diffuse and ground dc and mutilpy the sum by 0.25
				if(simple_blinds_model==1 && k ==1 ){
					while(i<number_of_diffuse_and_ground_dc){
						summe1+= (*pointer_dc) * (*pointer_sky);
//#ifndef PROCESS_ROW
						pointer_dc+=dc_shading_next[k][j][i]-i;
						pointer_sky+=dc_shading_next[k][j][i]-i;
						i=dc_shading_next[k][j][i];
//#else
//						pointer_dc+= dcd->next[i] - i;
//						pointer_sky+= dcd->next[i] - i;
//						i= dcd->next[i];
//#endif
					}
					summe1*=.25;
				}else if(dds_file_format==2) { //DDS and SHADOWTESTING
					//*number_direct_coefficients= 145; //count only direct indirect
					while(i< (number_of_diffuse_and_ground_dc+ 145)){
						summe1+= (*pointer_dc) * (*pointer_sky);
						if(i<number_of_diffuse_and_ground_dc){
//#ifndef PROCESS_ROW
							pointer_dc+=dc_shading_next[k][j][i]-i;
							pointer_sky+=dc_shading_next[k][j][i]-i;
							i=dc_shading_next[k][j][i];
//#else
//							pointer_dc+= dcd->next[i] - i;
//							pointer_sky+= dcd->next[i] - i;
//							i= dcd->next[i];
//#endif
							if(i>number_of_diffuse_and_ground_dc){
								pointer_dc+=number_of_diffuse_and_ground_dc-i;
								pointer_sky+=number_of_diffuse_and_ground_dc-i;
								i=number_of_diffuse_and_ground_dc;
							}
						}else{
							pointer_dc+=NextNonEmptySkyPatch[i]-i;
							pointer_sky+=NextNonEmptySkyPatch[i]-i;
							i=NextNonEmptySkyPatch[i];
//#ifndef PROCESS_ROW
							while( i < (number_of_diffuse_and_ground_dc+ number_direct_coefficients) && dc_shading[k][j][i]==0.0 ) {
//#else
//							while( i < (number_of_diffuse_and_ground_dc+ number_direct_coefficients) && dcd->dc[i]==0.0 ) {
//#endif
								pointer_dc+=NextNonEmptySkyPatch[i]-i;
								pointer_sky+=NextNonEmptySkyPatch[i]-i;
								i=NextNonEmptySkyPatch[i];
							}
						}
					}
					//now read in the direct direct contribution from the pre-simulation run
					summe1+=1.0*DirectDirectContribution*dc_ab0[j][SkyConditionCounter][k];
				} else { // regular blind model or no blinds
					while(i< (number_of_diffuse_and_ground_dc+ number_direct_coefficients)){
						summe1+= (*pointer_dc) * (*pointer_sky);
						if(i<number_of_diffuse_and_ground_dc){
//#ifndef PROCESS_ROW
							pointer_dc+=dc_shading_next[k][j][i]-i;
							pointer_sky+=dc_shading_next[k][j][i]-i;
							i=dc_shading_next[k][j][i];
//#else
//							pointer_dc+= dcd->next[i] - i;
//							pointer_sky+= dcd->next[i] - i;
//							i= dcd->next[i];
//#endif
							if(i>number_of_diffuse_and_ground_dc){
								pointer_dc+=number_of_diffuse_and_ground_dc-i;
								pointer_sky+=number_of_diffuse_and_ground_dc-i;
								i=number_of_diffuse_and_ground_dc;
							}
						}else{
							pointer_dc+=NextNonEmptySkyPatch[i]-i;
							pointer_sky+=NextNonEmptySkyPatch[i]-i;
							i=NextNonEmptySkyPatch[i];
//#ifndef PROCESS_ROW
							while( i<(number_of_diffuse_and_ground_dc+ number_direct_coefficients) && dc_shading[k][j][i]==0.0){
//#else
//							while( i<(number_of_diffuse_and_ground_dc+ number_direct_coefficients) && dcd->dc[i]==0.0) {
//#endif

								pointer_dc+=NextNonEmptySkyPatch[i]-i;
								pointer_sky+=NextNonEmptySkyPatch[i]-i;
								i=NextNonEmptySkyPatch[i];
							}
						}
					}
				}
				if(sensor_unit[j]==0 || sensor_unit[j]==1)	
					fprintf(SHADING_ILLUMINANCE_FILE[k]," %.0f",summe1);
				if(sensor_unit[j]==2 || sensor_unit[j]==3 )	
					fprintf(SHADING_ILLUMINANCE_FILE[k]," %.2f",summe1);
			}

		}
		SkyConditionCounter++;

		//reset SkyPatchLuminance and SkyPatchSolarRadiation
		for (i=0 ; i< number_of_diffuse_and_ground_dc+ number_direct_coefficients; i++) 
		{
			SkyPatchLuminance[i]=0;
			SkyPatchSolarRadiation[i]=0;
		}
	} /* end else */

	return 0;
}


/*============================================*/
/* Perez models */
/*============================================*/

/* Perez global horizontal luminous efficacy model */
double glob_h_effi_PEREZ()
{

	double 	value;
	double 	category_bounds[10], a[10], b[10], c[10], d[10];
	int   	category_total_number, category_number=0, i;

	//if (skyclearness<skyclearinf || skyclearness>skyclearsup || skybrightness<=skybriginf || skybrightness>skybrigsup){;}

	/* initialize category bounds (clearness index bounds) */

	category_total_number = 8 ; /*changed from 8 , Tito */
	category_bounds[1] = 1;
	category_bounds[2] = 1.065;
	category_bounds[3] = 1.230;
	category_bounds[4] = 1.500;
	category_bounds[5] = 1.950;
	category_bounds[6] = 2.800;
	category_bounds[7] = 4.500;
	category_bounds[8] = 6.200;
	category_bounds[9] = 12.1; /*changed from 12.01 , Tito */

	/* initialize model coefficients */
	a[1] = 96.63;
	a[2] = 107.54;
	a[3] = 98.73;
	a[4] = 92.72;
	a[5] = 86.73;
	a[6] = 88.34;
	a[7] = 78.63;
	a[8] = 99.65;

	b[1] = -0.47;
	b[2] = 0.79;
	b[3] = 0.70;
	b[4] = 0.56;
	b[5] = 0.98;
	b[6] = 1.39;
	b[7] = 1.47;
	b[8] = 1.86;

	c[1] = 11.50;
	c[2] = 1.79;
	c[3] = 4.40;
	c[4] = 8.36;
	c[5] = 7.10;
	c[6] = 6.06;
	c[7] = 4.93;
	c[8] = -4.46;

	d[1] = -9.16;
	d[2] = -1.19;
	d[3] = -6.95;
	d[4] = -8.31;
	d[5] = -10.94;
	d[6] = -7.60;
	d[7] = -11.37;
	d[8] = -3.15;

	for (i=1; i<=category_total_number; i++)
		{
			if ( (skyclearness >= category_bounds[i]) && (skyclearness < category_bounds[i+1]) )
				category_number = i;
		}

	value = a[category_number] + b[category_number]*atm_preci_water  +
	    c[category_number]*cos(sunzenith*DTR) +  d[category_number]*log(skybrightness);

	return(value);
}


/* global horizontal diffuse efficacy model, according to PEREZ */
double glob_h_diffuse_effi_PEREZ()
{
	double 	value;
	double 	category_bounds[10], a[10], b[10], c[10], d[10];
	int   	category_total_number, category_number=0, i;

	if (skyclearness<skyclearinf || skyclearness>skyclearsup || skybrightness<=skybriginf || skybrightness>skybrigsup)
		{/*fprintf(stdout, "Warning : skyclearness or skybrightness out of range ; \n Check your input parameters\n");*/};

	/* initialize category bounds (clearness index bounds) */

	category_total_number = 8;
	category_bounds[1] = 1;
	category_bounds[2] = 1.065;
	category_bounds[3] = 1.230;
	category_bounds[4] = 1.500;
	category_bounds[5] = 1.950;
	category_bounds[6] = 2.800;
	category_bounds[7] = 4.500;
	category_bounds[8] = 6.200;
	category_bounds[9] = 12.1; /*changed from 12.01 , Tito */

	/* initialize model coefficients */
	a[1] = 97.24;
	a[2] = 107.22;
	a[3] = 104.97;
	a[4] = 102.39;
	a[5] = 100.71;
	a[6] = 106.42;
	a[7] = 141.88;
	a[8] = 152.23;

	b[1] = -0.46;
	b[2] = 1.15;
	b[3] = 2.96;
	b[4] = 5.59;
	b[5] = 5.94;
	b[6] = 3.83;
	b[7] = 1.90;
	b[8] = 0.35;

	c[1] = 12.00;
	c[2] = 0.59;
	c[3] = -5.53;
	c[4] = -13.95;
	c[5] = -22.75;
	c[6] = -36.15;
	c[7] = -53.24;
	c[8] = -45.27;

	d[1] = -8.91;
	d[2] = -3.95;
	d[3] = -8.77;
	d[4] = -13.90;
	d[5] = -23.74;
	d[6] = -28.83;
	d[7] = -14.03;
	d[8] = -7.98;

	for (i=1; i<=category_total_number; i++)
		{
			if ( (skyclearness >= category_bounds[i]) && (skyclearness <= category_bounds[i+1]) )
				category_number = i;
		}
	value = a[category_number] + b[category_number]*atm_preci_water  + c[category_number]*cos(sunzenith*DTR) +
	    d[category_number]*log(skybrightness);

	return(value);
}


/* direct normal efficacy model, according to PEREZ */

double direct_n_effi_PEREZ()

{
	double 	value;
	double 	category_bounds[10], a[10], b[10], c[10], d[10];
	int   	category_total_number, category_number=0, i;


	if (skyclearness<skyclearinf || skyclearness>skyclearsup || skybrightness<=skybriginf || skybrightness>skybrigsup)
		{/*fprintf(stdout, "Warning : skyclearness or skybrightness out of range ; \n Check your input parameters\n")*/;}


	/* initialize category bounds (clearness index bounds) */

	category_total_number = 8;

	category_bounds[1] = 1;
	category_bounds[2] = 1.065;
	category_bounds[3] = 1.230;
	category_bounds[4] = 1.500;
	category_bounds[5] = 1.950;
	category_bounds[6] = 2.800;
	category_bounds[7] = 4.500;
	category_bounds[8] = 6.200;
	category_bounds[9] = 12.1;


	/* initialize model coefficients */
	a[1] = 57.20;
	a[2] = 98.99;
	a[3] = 109.83;
	a[4] = 110.34;
	a[5] = 106.36;
	a[6] = 107.19;
	a[7] = 105.75;
	a[8] = 101.18;

	b[1] = -4.55;
	b[2] = -3.46;
	b[3] = -4.90;
	b[4] = -5.84;
	b[5] = -3.97;
	b[6] = -1.25;
	b[7] = 0.77;
	b[8] = 1.58;

	c[1] = -2.98;
	c[2] = -1.21;
	c[3] = -1.71;
	c[4] = -1.99;
	c[5] = -1.75;
	c[6] = -1.51;
	c[7] = -1.26;
	c[8] = -1.10;

	d[1] = 117.12;
	d[2] = 12.38;
	d[3] = -8.81;
	d[4] = -4.56;
	d[5] = -6.16;
	d[6] = -26.73;
	d[7] = -34.44;
	d[8] = -8.29;



	for (i=1; i<=category_total_number; i++)
		{
			if ( (skyclearness >= category_bounds[i]) && (skyclearness <= category_bounds[i+1]) )
				category_number = i;
		}

	value = a[category_number] + b[category_number]*atm_preci_water  + c[category_number]*exp(5.73*sunzenith*DTR - 5) +  d[category_number]*skybrightness;

	if (value < 0) value = 0;

	return(value);
}


/*check the range of epsilon and delta indexes of the perez parametrization*/
void check_parametrization()
{

	if (skyclearness<skyclearinf || skyclearness>skyclearsup || skybrightness<skybriginf || skybrightness>skybrigsup)
		{
			if (skyclearness<skyclearinf){
				if(all_warnings &&((fabs(hour- 12+12-stadj(jdate(month, day))-solar_sunset(month,day))<0.25)||(fabs(hour-solar_sunset(month,day)-stadj(jdate(month, day)) )<0.25) )){
					sprintf(errmsg, "sky clearness (%.1f) below range (%d %d %.3f)", skyclearness, month, day, hour);
					error(WARNING, errmsg);
				}
				skyclearness=skyclearinf;
			}
			if (skyclearness>skyclearsup){
				if(all_warnings &&((fabs(hour- 12+12-stadj(jdate(month, day))-solar_sunset(month,day))<0.25)||(fabs(hour-solar_sunset(month,day)-stadj(jdate(month, day)) )<0.25) )){
					sprintf(errmsg, "sky clearness (%.1f) above range (%d %d %.3f)", skyclearness, month, day, hour);
					error(WARNING, errmsg);
				}
				skyclearness=skyclearsup;
			}
			if (skybrightness<skybriginf){
				if(all_warnings &&((fabs(hour- 12+12-stadj(jdate(month, day))-solar_sunset(month,day))<0.25)||(fabs(hour-solar_sunset(month,day)-stadj(jdate(month, day)) )<0.25) )){
					sprintf(errmsg, "sky brightness (%.1f) below range (%d %d %.3f)", skybrightness, month, day, hour);
					error(WARNING, errmsg);
				}
				skybrightness=skybriginf;
			}
			if (skybrightness>skybrigsup){
				if(all_warnings &&((fabs(hour- 12+12-stadj(jdate(month, day))-solar_sunset(month,day))<0.25)||(fabs(hour-solar_sunset(month,day)-stadj(jdate(month, day)) )<0.25) )){
					sprintf(errmsg, "sky brightness (%.1f) above range (%d %d %.3f)", skybrightness, month, day, hour);
					error(WARNING, errmsg);
				}
				skybrightness=skybrigsup;
			}

		}
}


/* likelihood of the direct and diffuse components */
void 	check_illuminances()
{
	if (!( (directilluminance>=0) && (directilluminance<=solar_constant_l*1000) && (diffusilluminance>0) ))
		{
			if(directilluminance > solar_constant_l*1000){
				directilluminance=solar_constant_l*1000;
				sprintf(errmsg, "direct illuminance set to max at %d %d %.3f", month, day, hour);
				error(WARNING, errmsg);
			}
			else{ directilluminance = 0; diffusilluminance = 0; }
			/*fprintf(stdout,"direct or diffuse illuminances out of range at %d %d %f\n", month,day,hour);*/
		}
}


void 	check_irradiances()
{
	if (!( (directirradiance>=0) && (directirradiance<=solar_constant_e) && (diffusirradiance>0) ))
	{
		if(diffusirradiance!=0 && directirradiance!=0){
			sprintf(errmsg, "direct or diffuse irradiances out of range (date %d %d %f\n dir %f dif %f)", month, day, hour, directirradiance, diffusirradiance);
			error(WARNING, errmsg);
		}
	}
}



/* Perez sky's brightness */
double sky_brightness()
{
	double value;

	value = diffusirradiance * air_mass() / ( solar_constant_e*get_eccentricity());

	return(value);
}


/* Perez sky's clearness */
double sky_clearness()
{
	double value;
	if(diffusirradiance > 0){
		value = ((diffusirradiance + directirradiance) / (diffusirradiance)+1.041*sunzenith*DTR*sunzenith*DTR*sunzenith*DTR) / (1 + 1.041*sunzenith*DTR * sunzenith*DTR * sunzenith*DTR);
	}else{value=0;}
	return(value);
}



/* diffus horizontal irradiance from Perez sky's brightness */
double diffus_irradiance_from_sky_brightness()
{
	double value;

	value = skybrightness / air_mass() * ( solar_constant_e*get_eccentricity());

	return(value);
}


/* direct normal irradiance from Perez sky's clearness */
double direct_irradiance_from_sky_clearness()
{
	double value;

	value = diffus_irradiance_from_sky_brightness();
	value = value * ((skyclearness - 1) * (1 + 1.041*sunzenith*DTR*sunzenith*DTR * sunzenith*DTR));
	return(value);
}


void illu_to_irra_index(void)
{
	double	test1=0.1, test2=0.1;
	int	counter=0;

	diffusirradiance = diffusilluminance*solar_constant_e/(solar_constant_l*1000);
	directirradiance = directilluminance*solar_constant_e/(solar_constant_l*1000);
	skyclearness =  sky_clearness();
	skybrightness = sky_brightness();
	if (skyclearness>12) skyclearness=12;
	if (skybrightness<0.05) skybrightness=0.01;


	while ( ((fabs(diffusirradiance-test1)>10) || (fabs(directirradiance-test2)>10)
			 || skyclearness>skyclearinf || skyclearness<skyclearsup
			 || skybrightness>skybriginf || skybrightness<skybrigsup )
			&& !(counter==5) )
		{
			/*fprintf(stdout, "conversion illuminance into irradiance %lf\t %lf\n", diffusirradiance, directirradiance);*/

			test1=diffusirradiance;
			test2=directirradiance;
			counter++;

			diffusirradiance = diffusilluminance/glob_h_diffuse_effi_PEREZ();
			directirradiance = directilluminance/direct_n_effi_PEREZ();

			skybrightness = sky_brightness();
			skyclearness =  sky_clearness();
			if (skyclearness>12) skyclearness=12;
			if (skybrightness<0.05) skybrightness=0.01;
		}
}


/* sky luminance perez model */
double calc_rel_lum_perez(double dzeta, double gamma, double Z,
	double epsilon, double Delta, double *coeff_perez)
{
	double x[5][4];
	int i,j,num_lin=0;
	double c_perez[5];

	if ( (epsilon <  skyclearinf) || (epsilon > skyclearsup) )
	{
		sprintf(errmsg, "epsilon out of range in function calc_rel_lum_perez ! (%f)", epsilon);
		error(USER, errmsg);
	}

	/* correction de modele de Perez solar energy ...*/
	if ( (epsilon > 1.065) && (epsilon < 2.8) )
		{
			if ( Delta < 0.2 ) Delta = 0.2;
		}

	if ( (epsilon >= 1.000) && (epsilon < 1.065) ) num_lin = 0;
	if ( (epsilon >= 1.065) && (epsilon < 1.230) ) num_lin = 1;
	if ( (epsilon >= 1.230) && (epsilon < 1.500) ) num_lin = 2;
	if ( (epsilon >= 1.500) && (epsilon < 1.950) ) num_lin = 3;
	if ( (epsilon >= 1.950) && (epsilon < 2.800) ) num_lin = 4;
	if ( (epsilon >= 2.800) && (epsilon < 4.500) ) num_lin = 5;
	if ( (epsilon >= 4.500) && (epsilon < 6.200) ) num_lin = 6;
	if ( (epsilon >= 6.200) && (epsilon < 14.00) ) num_lin = 7;

	for (i=0;i<5;i++)
		for (j=0;j<4;j++)
			{
				x[i][j] = coeff_perez[20*num_lin + 4*i +j];
				/* printf("x %d %d vaut %f\n",i,j,x[i][j]); */
			}


	if (num_lin)
		{
			for (i=0;i<5;i++)
				c_perez[i] = x[i][0] + x[i][1]*Z + Delta * (x[i][2] + x[i][3]*Z);
		}
	else
		{
			c_perez[0] = x[0][0] + x[0][1]*Z + Delta * (x[0][2] + x[0][3]*Z);
			c_perez[1] = x[1][0] + x[1][1]*Z + Delta * (x[1][2] + x[1][3]*Z);
			c_perez[4] = x[4][0] + x[4][1]*Z + Delta * (x[4][2] + x[4][3]*Z);
			c_perez[2] = exp( pow(Delta*(x[2][0]+x[2][1]*Z),x[2][2])) - x[2][3];
			c_perez[3] = -exp( Delta*(x[3][0]+x[3][1]*Z) )+x[3][2]+Delta*x[3][3];
		}


	return (1 + c_perez[0]*exp(c_perez[1]/cos(dzeta)) ) *
	    (1 + c_perez[2]*exp(c_perez[3]*gamma) +
		 c_perez[4]*cos(gamma)*cos(gamma) );
}



/* coefficients for the sky luminance perez model */
void coeff_lum_perez(double Z, double epsilon, double Delta, double *coeff_perez)
{
	double x[5][4];
	int i,j,num_lin=0;

	if ( (epsilon <  skyclearinf) || (epsilon > skyclearsup) )
	{
		sprintf(errmsg, "epsilon out of range in function coeff_lum_perez ! (%f)", epsilon);
		error(USER, errmsg);
	}

	/* correction du modele de Perez solar energy ...*/
	if ( (epsilon > 1.065) && (epsilon < 2.8) )
		{
			if ( Delta < 0.2 ) Delta = 0.2;
		}

	if ( (epsilon >= 1.000) && (epsilon < 1.065) ) num_lin = 0;
	if ( (epsilon >= 1.065) && (epsilon < 1.230) ) num_lin = 1;
	if ( (epsilon >= 1.230) && (epsilon < 1.500) ) num_lin = 2;
	if ( (epsilon >= 1.500) && (epsilon < 1.950) ) num_lin = 3;
	if ( (epsilon >= 1.950) && (epsilon < 2.800) ) num_lin = 4;
	if ( (epsilon >= 2.800) && (epsilon < 4.500) ) num_lin = 5;
	if ( (epsilon >= 4.500) && (epsilon < 6.200) ) num_lin = 6;
	if ( (epsilon >= 6.200) && (epsilon < 14.00) ) num_lin = 7;

	for (i=0;i<5;i++)
		for (j=0;j<4;j++)
			{
				x[i][j] = coeff_perez[20*num_lin + 4*i +j];
				/* printf("x %d %d vaut %f\n",i,j,x[i][j]); */
			}


	if (num_lin)
		{
			for (i=0;i<5;i++)
				c_perez[i] = x[i][0] + x[i][1]*Z + Delta * (x[i][2] + x[i][3]*Z);

		}
	else
		{
			c_perez[0] = x[0][0] + x[0][1]*Z + Delta * (x[0][2] + x[0][3]*Z);
			c_perez[1] = x[1][0] + x[1][1]*Z + Delta * (x[1][2] + x[1][3]*Z);
			c_perez[4] = x[4][0] + x[4][1]*Z + Delta * (x[4][2] + x[4][3]*Z);
			c_perez[2] = exp( pow(Delta*(x[2][0]+x[2][1]*Z),x[2][2])) - x[2][3];
			c_perez[3] = -exp( Delta*(x[3][0]+x[3][1]*Z) )+x[3][2]+Delta*x[3][3];
		}

	return;
}


/* degrees into radians */
double radians(double degres)
{
	return degres * DTR;
}

/* radian into degrees */
double degres(double radians)
{
	return radians * RTD;
}

/* calculation of the angles dzeta and gamma */
void theta_phi_to_dzeta_gamma(double theta,double phi,double *dzeta,double *gamma, double Z)
{
	*dzeta = theta; /* dzeta = phi */
	if ( (cos(Z)*cos(theta)+sin(Z)*sin(theta)*cos(phi)) > 1 && (cos(Z)*cos(theta)+sin(Z)*sin(theta)*cos(phi) < 1.1 ) )
		*gamma = 0;
	else if ( (cos(Z)*cos(theta)+sin(Z)*sin(theta)*cos(phi)) > 1.1 )
	{
		error(INTERNAL, "error in calculation of gamma (angle between point and sun");
	}
	else
		*gamma = acos(cos(Z)*cos(theta)+sin(Z)*sin(theta)*cos(phi));
}


/********************************************************************************/
/*	Fonction: integ_lv							*/
/*										*/
/*	In: float *lv,*theta							*/
/*	    int sun_pos								*/
/*										*/
/*	Out: double								*/
/*										*/
/*	Update: 29/08/93							*/
/*										*/
/*	But: calcul l'integrale de luminance relative sans la dir. du soleil	*/
/*										*/
/********************************************************************************/
double integ_lv(float *lv,float *theta)
{
	int i;
	double buffer=0.0;

	for (i=0;i<145;i++)
		buffer += lv[i] * cos(radians(theta[i]));

	return buffer*2*PI/144;
}


/* enter day number(double), return E0 = square(R0/R):  eccentricity correction factor  */
double get_eccentricity()
{
	double day_angle;
	double E0;

	day_angle  = 2*PI*(daynumber - 1)/365;
	E0         = 1.00011+0.034221*cos(day_angle)+0.00128*sin(day_angle)+
	    0.000719*cos(2*day_angle)+0.000077*sin(2*day_angle);

	return (E0);
}

/* enter sunzenith angle (degrees) return relative air mass (double) */
double 	air_mass()
{
	double	m;
	m = 1/( cos(sunzenith*DTR)+0.15*exp( log(93.885-sunzenith)*(-1.253) ) );
	return(m);
}


double get_angle_sun_direction(double sun_zenith, double sun_azimut, double direction_zenith, double direction_azimut)
{
	double angle;
	if (sun_zenith == 0)
        error(WARNING, "zenith_angle = 0 in function get_angle_sun_direction");

	angle = acos(cos(sun_zenith*DTR)*cos(direction_zenith*DTR) + sin(sun_zenith*DTR)*sin(direction_zenith*DTR)*cos((sun_azimut - direction_azimut)*DTR));
	angle = angle*RTD;
	return(angle);
}
