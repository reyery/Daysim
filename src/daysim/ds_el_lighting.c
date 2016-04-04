/*  Copyright (c) 2003
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

/* ds_el_lighting is a DAYSIM subprogram that predicts the status of the electric
   lighting and blinds in an office throughout the year based on user
   occupancy and indoor illuminances.
*/


#include  <stdio.h>
#include  <string.h>
#include  <math.h>
#include  <stdlib.h>
#include  <errno.h>
#include  <paths.h>

#include  "read_in_header.h"
#include  "sun.h"
#include  "fropen.h"
#include  "occ_func.h"
#include  "daylightfactor.h"
#include  "BlindModel.h"
#include  "allocate_memory.h"
#include  "get_illuminances.h"
#include  "lightswitch.h"
#include  "analysis_data.h"

/* In simulation_assumptions.c */
extern void simulation_assumptions();

/*=======================*/
/* initialize variables */
/*=======================*/
FILE *HISTO;
FILE *EL_LIGHTING_FILE;

float LightingGroupEnergyUse[11][13];			//annual and monthly mean energy demands for one user for each group
float user_energy_stddev[13];

float **electric_lighting_profile;
int AnnualHoursWithGlare=0;
int **shading_profile;
int BlindGroupIndex=0;
int BlindGroup2Index=0;
int BlindGroup3Index=0;
int BlindGroup4Index=0;
int *month_1, *day_1;
int overview=1; 	/*switch to */
int da_overview=0;  /*switch for comparative analysis of daylight autonomies */
int *****direct_sun;
int ActiveOccupant=0;
int PassiveOccupant=0;
float *hour_1;
float *da;
float *dir;
float **daylight_illuminances;
float ***work_plane_ill;
float ****raw_illuminances;
float ***DaylightGlareProbability;
float ***MaximumIlluminanancePerBlindGroup;
float *ext_sensor_ill;
float *int_sensor_ill;
float **AnnualLightExposure;
char input_file[200]="";
float **minimum_work_plane_ill;
float ***MaximumBlindGroupIlluminance_Internal;
float ***MaximumBlindGroupIlluminance_External;
float **daylight_autonomy;
float **continuous_daylight_autonomy;
float **DA_max;
float **UDI_100;
float **UDI_100_2000;
float **UDI_2000;
float **DaylightSaturationPercentage;
float UDI_100_zone_active=0;
float UDI_100_2000_zone_active=0;
float UDI_2000_zone_active=0;
float UDI_100_zone_passive=0;
float UDI_100_2000_zone_passive=0;
float UDI_2000_zone_passive=0;

/*===================================*/
/* temporary variables: for analysis */
/*===================================*/
int histo[20][1001];
float histo_step=0.1;
int number_of_weekdays=0;
float AnnualNumberOfOccupiedHours=0;
float HoursWithAView[11];
float AnnualNumberOfActivatedElectricLighting[11];
char histo_file[200]="";// plus line below

float MeanLightingSystemEnergy=0;
float MeanLightingSystemStDev=0;

/*  versioning  */

extern char  VersionID[];	/* Radiance version ID string */

int main(int argc, char** argv )
{	int  i,j,k,l;
	int behavior=0;
	float x,y,last_time_step=0;
	FILE *SUN_DIR;
	
	if (argc == 1 || argv[1] == NULL) {
		char *progname = fixargv0(argv[0]);
		fprintf(stderr, "\n%s:\n", progname);
		fprintf(stderr, "Program that predicts the status of the electric lighting and blinds in an office throughout the year based on user occupancy and indoor illuminances.\n\n");
		fprintf(stderr, "Example:\n");
		fprintf(stderr, "\t%s header_file\n", progname);
		exit(1);
	}
	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}
	strcpy(input_file, argv[1]);

	for (i=0 ; i<501 ; i++)
		histo[0][i]=0;


/*=====================*/
/* read in header file */
/*=====================*/
	read_in_header(input_file);




/*==================================================*/
/* allocate memory for dynamic arrays               */
/* and assign values for month_1, day_1, and hour_1 */
/*==================================================*/
	allocate_memory();


/*=======================================*/
/* assign dates for month, day, and hour */
/*=======================================*/
	month=1;
	day=1;
	hour=time_step*1.0/120;
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		month_1[i]=month;
		day_1[i]=day;
		hour_1[i]=hour;
		hour+=time_step*1.0/60;
		if(hour>=24.0 ){
			hour= time_step*1.0/120;
			day++;
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
	}

	
/*==========================================*/
/* assign direct sunlight values from *.dir */
/*==========================================*/
if(NumberOfBlindGroups!=0){ // load direct sunlight file only if a shading group exists
	SUN_DIR =open_input(direct_sunlight_file);
	fscanf(SUN_DIR,"%d %d %f ",&i,&k,&x);
	fscanf( SUN_DIR, "%*[^\n]\n" );
	last_time_step=x;
	fscanf(SUN_DIR,"%d %d %f ",&i,&k,&x);

	if((x-last_time_step)*60.0 > 1.1*time_step || (x-last_time_step)*60.0 < 0.9*time_step)
	{
			fprintf(stderr,"ds_el_lighting FATAL ERROR:\nThe time step of the direct sunlight file(%s)\n does not correspond to %d minutes.\n\n",direct_sunlight_file,time_step);
			fprintf(stderr,"Please rerun gen_directsunlight %s to update the file.\n",input_file);
			fprintf(stderr,"In case you are using the DAYSIM GUI, the GUI should automatically do this for you.\n");
			exit(1);
	}
	rewind(SUN_DIR);

	i=0;
	while(fscanf(SUN_DIR,"%d",&l) !=EOF)  //loop over year
	{
		fscanf(SUN_DIR,"%d %f %f %f",&k,&x,&dir[i],&y);
	    for (BlindGroupIndex=1 ; BlindGroupIndex<=NumberOfBlindGroups ; BlindGroupIndex++)
	    {
			if(BlindGroupIndex==1)
			{
				for (k=0 ; k<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; k++)
					fscanf(SUN_DIR,"%d ",&direct_sun[k][0][0][0][i]);
			}
			if(BlindGroupIndex==2)
			{
				for (k=0 ; k<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; k++)
					fscanf(SUN_DIR,"%d ",&direct_sun[0][k][0][0][i]);
				
				for (BlindGroup2Index=1 ; BlindGroup2Index<=NumberOfSettingsInBlindgroup[1] ; BlindGroup2Index++)
					for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
						fscanf(SUN_DIR,"%d ",&direct_sun[BlindGroup2Index][j][0][0][i]);
			}
			if(BlindGroupIndex==3)
			{
				for (k=0 ; k<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; k++)
					fscanf(SUN_DIR,"%d ",&direct_sun[0][0][k][0][i]);
				for (BlindGroup2Index=1 ; BlindGroup2Index<=NumberOfSettingsInBlindgroup[1] ; BlindGroup2Index++)
					for (BlindGroup3Index=1 ; BlindGroup3Index<=NumberOfSettingsInBlindgroup[2] ; BlindGroup3Index++)
						for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
							fscanf(SUN_DIR,"%d ",&direct_sun[BlindGroup2Index][BlindGroup3Index][j][0][i]);
			}
			if(BlindGroupIndex==4)
			{
				for (k=0 ; k<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; k++)
					fscanf(SUN_DIR,"%d ",&direct_sun[0][0][0][k][i]);
				for (BlindGroup2Index=1 ; BlindGroup2Index<=NumberOfSettingsInBlindgroup[1] ; BlindGroup2Index++)
					for (BlindGroup3Index=1 ; BlindGroup3Index<=NumberOfSettingsInBlindgroup[2] ; BlindGroup3Index++)
						for (BlindGroup4Index=1 ; BlindGroup4Index<=NumberOfSettingsInBlindgroup[3] ; BlindGroup4Index++)
							for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
								fscanf(SUN_DIR,"%d ",&direct_sun[BlindGroup2Index][BlindGroup3Index][BlindGroup4Index][j][i]);
			}
		}
		
		i++;
	}
	close_file(SUN_DIR);
}


/*=========================================================================*/
/*read in DGP values from different *.gdp files and for different view points */
/*=========================================================================*/
get_DGP_profiles();



/*==============================*/
/* get annual occupancy profile */
/*==============================*/
occupancy_profile(occupancy_mode, daylight_savings_time, start_work, end_work, time_step, first_weekday);

/*=========================================================================*/
/*read in illuminances from different *.ill files and for different points */
/*=========================================================================*/
get_illuminances();


//======================
//loop over user profile
//======================
for (behavior=0 ; behavior< NumUserProfiles; behavior++){


	// Determine Status of Blinds
	UserBlindControl[behavior]= getBlindSettings(UserBlindControl[behavior]);
	
	// Determin daylight illuminances
	get_daylight_illuminances( );
	
	// determines status of electric lighting systems
	//================================================
	lightswitch_function(UserLightControl[behavior]);
	
	//determine electric lighting energy use
	get_electric_lighting_energy_use(UserLightControl[behavior], UserBlindControl[behavior]);

	genLightExposureAndDaylightAutonomy(UserBlindControl[behavior]);
	
	
}//end loop over user profile


writeRGB_DA_Files(); //write out DA files for DIVA and Ecotect


	
	// if output file specifed the prgram writes out an *.html  file with simulation assumptions
	if(strcmp(el_file,""))
	{
		EL_LIGHTING_FILE=open_output(el_file);
		simulation_assumptions();
		close_file(EL_LIGHTING_FILE);
	}

return 0;
}
