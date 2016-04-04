/* function file of the Lightswitch-2002 manual lighting control algorithm
 *  Copyright (c) 2002
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

/* in this file the memory for the dynamic variables used
 * by the Lightswitch algorithm are dynamically allocated
*/

#include <stdlib.h>
#include "rterror.h"
#include "read_in_header.h"
#include "ds_el_lighting.h"

#define NUM_BLINDS_SETTINGS	11
#define NUM_USERS	2

void allocate_memory()
{
	int i,j,k,m,n;
	int time_steps_in_year = 8760 * (int)(60 / time_step);
/*==================*/
/* allocate memory */
/*==================*/
	occ_profile = (int*)malloc(sizeof(int)*time_steps_in_year);
	if (occ_profile == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) {
		occ_profile[i]=0;
	}
	
	dgp_profile = (float*)malloc(sizeof(float)*time_steps_in_year);
	if (dgp_profile == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) {
		dgp_profile[i]=0;
	}

	//resulting daylight_illuminances[TimeStepInYear]
	daylight_illuminances = (float**)malloc(sizeof(float*)*time_steps_in_year);
	if (daylight_illuminances == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++)
	{
		daylight_illuminances[i] = (float*)malloc(sizeof(float)*number_of_sensors);
		if (daylight_illuminances[i] == NULL) goto memerr;
		for (j = 0; j < number_of_sensors; j++)
		{
			daylight_illuminances[i][j]=0;
		}
	}

	
	//2-dim array for the shading status of each blind group
	//shading_profile[NumberOfBlindGroups+1][TimeStepInYear]
	shading_profile = (int**) malloc (sizeof(int*) * (NumberOfBlindGroups+1));
	if (shading_profile == NULL) goto memerr;
	for (i = 0; i<(NumberOfBlindGroups + 1); i++)
	{
		shading_profile[i] = (int*)malloc(sizeof(int)* time_steps_in_year);
		if (shading_profile[i] == NULL) goto memerr;
		for (j = 0; j < time_steps_in_year; j++)
		{
			shading_profile[i][j]=0;
		}
	}


	//2-dim array for the shading status of each lighting group
	//electric_lighting_profile[NumberOfLightingGroups+1][TimeStepInYear]
	electric_lighting_profile = (float**) malloc (sizeof(float*) * (NumberOfLightingGroups+1));
	if (electric_lighting_profile == NULL) goto memerr;
	for (i = 0; i<(NumberOfLightingGroups + 1); i++)
	{
		electric_lighting_profile[i] = (float*)malloc(sizeof(float)* time_steps_in_year);
		if (electric_lighting_profile[i] == NULL) goto memerr;
		for (j = 0; j < time_steps_in_year; j++)
		{
			electric_lighting_profile[i][j]=0;
		}
	}
	
	//2-dim array for the max. ill. at the blind group contol points
	//MaximumBlindGroupIlluminance_Internal[NumberOfBlindGroups][NumberOfBlindSettings][TimeStepInYear]
	MaximumBlindGroupIlluminance_Internal = (float***) malloc (sizeof(float*) * (NumberOfBlindGroups+1));
	if (MaximumBlindGroupIlluminance_Internal == NULL) goto memerr;
	for (i = 0; i < (NumberOfBlindGroups + 1); i++) {
		MaximumBlindGroupIlluminance_Internal[i] = (float**)malloc(sizeof(float*) * NUM_BLINDS_SETTINGS);
		if (MaximumBlindGroupIlluminance_Internal[i] == NULL) goto memerr;
		for (k = 0; k < NUM_BLINDS_SETTINGS; k++) {
			MaximumBlindGroupIlluminance_Internal[i][k] = (float*)malloc(sizeof(float) * time_steps_in_year);
			if (MaximumBlindGroupIlluminance_Internal[i][k] == NULL) goto memerr;
			for (j = 0; j < time_steps_in_year; j++)
				MaximumBlindGroupIlluminance_Internal[i][k][j]=0;
		}
	}

	//2-dim array for the max. ill. at the blind group contol points
	//MaximumBlindGroupIlluminance_External[NumberOfBlindGroups][NumberOfBlindSettings][TimeStepInYear]
	MaximumBlindGroupIlluminance_External = (float***) malloc (sizeof(float*) * (NumberOfBlindGroups+1));
	if (MaximumBlindGroupIlluminance_External == NULL) goto memerr;
	for (i = 0; i < (NumberOfBlindGroups + 1); i++) {
		MaximumBlindGroupIlluminance_External[i] = (float**)malloc(sizeof(float*)* NUM_BLINDS_SETTINGS);
		if (MaximumBlindGroupIlluminance_External[i] == NULL) goto memerr;
		for (k = 0; k < NUM_BLINDS_SETTINGS; k++) {
			MaximumBlindGroupIlluminance_External[i][k] = (float*)malloc(sizeof(float) * time_steps_in_year);
			if (MaximumBlindGroupIlluminance_External[i][k] == NULL) goto memerr;
			for (j = 0; j < time_steps_in_year; j++)
				MaximumBlindGroupIlluminance_External[i][k][j] = 0;
		}
	}
	
	//2-dim array for the min. ill. at the work place
	//minimum_work_plane_ill[NumberOfLightingGroups][TimeStepInYear]
	minimum_work_plane_ill = (float**) malloc (sizeof(float*) * (NumberOfLightingGroups+1));
	if (minimum_work_plane_ill == NULL) goto memerr;
	for (i = 0; i<(NumberOfLightingGroups + 1); i++){
		minimum_work_plane_ill[i] = (float*)malloc(sizeof(float)* time_steps_in_year);
		if (minimum_work_plane_ill[i] == NULL) goto memerr;
		for (j = 0; j <= time_steps_in_year; j++)
		{
			minimum_work_plane_ill[i][j]=0;
		}
	}
	
	//3-dim array for the effective dgp at all view points
	//effective_dgp[NumberOfBlindGroups][MAxNumberOfSetting inalBlindGroups][TimeStepInYear]
	effective_dgp = (float***) malloc (sizeof(float**) * (NumberOfBlindGroups+1));
	if (effective_dgp == NULL) goto memerr;
	for (i = 0; i < (NumberOfBlindGroups + 1); i++) {
		effective_dgp[i] = (float**)malloc(sizeof(float*) * NUM_BLINDS_SETTINGS);
		if (effective_dgp[i] == NULL) goto memerr;
		for (k = 0; k < NUM_BLINDS_SETTINGS; k++) {
			effective_dgp[i][k] = (float*)malloc(sizeof(float) * time_steps_in_year);
			if (effective_dgp[i][k] == NULL) goto memerr;
			for (j = 0; j < time_steps_in_year; j++)
				effective_dgp[i][k][j] = 0;
		}
	}

				
	//++++++++++++++++
	// Dynamic Metrics
	//++++++++++++++++

	//daylight autonomy for active and passive user
	daylight_autonomy=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (daylight_autonomy == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		daylight_autonomy[i] = (float*)malloc(sizeof(float) * NUM_USERS);
		if (daylight_autonomy[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			daylight_autonomy[i][j] = 0;
	}

	//continuous daylight autonomy for active and passive user
	continuous_daylight_autonomy=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (continuous_daylight_autonomy == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		continuous_daylight_autonomy[i] = (float*)malloc(sizeof(float)*NUM_USERS);
		for (j = 0; j < NUM_USERS; j++)
			continuous_daylight_autonomy[i][j] = 0;
	}

	//DA max for active and passive user
	DA_max=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (DA_max == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		DA_max[i] = (float*)malloc(sizeof(float) * NUM_USERS);
		if (DA_max[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			DA_max[i][j] = 0;
	}

	//UDI_100 for active and passive user
	UDI_100=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (UDI_100 == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		UDI_100[i] = (float*)malloc(sizeof(float) * NUM_USERS);
		if (UDI_100[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			UDI_100[i][j] = 0;
	}

	//UDI_100_2000 for active and passive user
	UDI_100_2000=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (UDI_100_2000 == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		UDI_100_2000[i] = (float*)malloc(sizeof(float)*NUM_USERS);
		if (UDI_100_2000[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			UDI_100_2000[i][j] = 0;
	}

	//UDI_2000 for active and passive user
	UDI_2000=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (UDI_2000 == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		UDI_2000[i] = (float*)malloc(sizeof(float)*NUM_USERS);
		if (UDI_2000[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			UDI_2000[i][j] = 0;
	}

	//AnnualLightExposure[number_of_sensors][active/pasive]
	//array that stores the annual light exposire of each sensor point
	AnnualLightExposure=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (AnnualLightExposure == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		AnnualLightExposure[i] = (float*)malloc(sizeof(float)*NUM_USERS);
		if (AnnualLightExposure[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			AnnualLightExposure[i][j] = 0;
	}
			
	//DaylightSaturationPercentage for active and passive user
	DaylightSaturationPercentage=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (DaylightSaturationPercentage == NULL) goto memerr;
	for (i = 0; i < number_of_sensors; i++) {
		DaylightSaturationPercentage[i] = (float*)malloc(sizeof(float)*NUM_USERS);
		if (DaylightSaturationPercentage[i] == NULL) goto memerr;
		for (j = 0; j < NUM_USERS; j++)
			DaylightSaturationPercentage[i][j] = 0;
	}

	//3-dim array for the min. ill. at the work place
	//work_plane_ill[TotalNumberOfDCFiles+1][TimeStepInYear][number_of_sensors]
	work_plane_ill = (float***) malloc (sizeof(float**) * (TotalNumberOfDCFiles+1));
	if (work_plane_ill == NULL) goto memerr;
	for (i = 0; i<(TotalNumberOfDCFiles + 1); i++) {
		work_plane_ill[i] =(float**) malloc (sizeof(float*)* time_steps_in_year);
		if (work_plane_ill[i] == NULL) goto memerr;
		for (j = 0; j < time_steps_in_year; j++) {
			work_plane_ill[i][j] = (float*)malloc(sizeof(float)* number_of_sensors);
			if (work_plane_ill[i][j] == NULL) goto memerr;
			for (k = 0; k< number_of_sensors; k++)
				work_plane_ill[i][j][k] = 0;
		}
	}

	//4-dim array for the illuminances for the *.ill files
	//raw_illuminances[Blindgroup][BlindSetting][TimeStepInYear][number_of_sensors]
	raw_illuminances = (float****) malloc (sizeof(float***) * (NumberOfBlindGroups+1));
	if (raw_illuminances == NULL) goto memerr;
	//	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
//	{
//		raw_illuminances[i] =(float***) malloc (sizeof(float**)* (MaxNumberOfSettingsInBlindgroup+1));
//	}

//	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
//	{
//		for (k=0 ; k<(MaxNumberOfSettingsInBlindgroup+1) ; k++)
//		{
//			raw_illuminances[i][k] =(float**) malloc (sizeof(float*)* 8760*(int)(60/time_step));
//		}
//	}

//	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
//	{
//		for (k=0 ; k<(MaxNumberOfSettingsInBlindgroup+1) ; k++)
//		{
//			for (j=0 ; j<8760*(int)(60/time_step) ; j++)
//			{
//				raw_illuminances[i][k][j]=(float*) malloc (sizeof(float)* number_of_sensors);
//				if (raw_illuminances[i][k][j] == NULL) 
//				{
//					printf(" ds_electric Lighting: merror calling maloc for \'raw_illuminances\'\n");
//					exit(1);					
//				} 
//			}
//		}
//	}

//	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
//	{
//		for (k=0 ; k<(MaxNumberOfSettingsInBlindgroup+1) ; k++)
//		{
//			for (j=0 ; j<8760*(int)(60/time_step) ; j++)
//			{
//				for (m=0 ; m< number_of_sensors ; m++)
//				{
//					raw_illuminances[i][k][j][m]=0;
//				}
//			}
//		}
//	}
	

	da = (float*) malloc (sizeof(float) * (number_of_sensors+1));
	if (da == NULL) goto memerr;
	for (i = 0; i<(number_of_sensors + 1); i++)
		da[i]=0;


	//five dimensional array into which the "direct sunlight" settings are read
	// direct_sun[NumberOfBlindSettings_BG1][NumberOfBlindSettings_BG2][NumberOfBlindSettings_BG3][NumberOfBlindSettings_BG4][TimeStepInYear]
		direct_sun = (int*****) malloc (sizeof(int****) * (NumberOfSettingsInBlindgroup[1]+1));
		if (direct_sun == NULL) goto memerr;
		for (i = 0; i<(NumberOfSettingsInBlindgroup[1] + 1); i++)
		{
			direct_sun[i] =(int****) malloc (sizeof(int***)* (NumberOfSettingsInBlindgroup[2]+1));
			if (direct_sun[i] == NULL) goto memerr;
			for (j=0 ; j< (NumberOfSettingsInBlindgroup[2]+1) ; j++)
			{
				direct_sun[i][j]=(int***) malloc (sizeof(int**)* (NumberOfSettingsInBlindgroup[3]+1));
				if (direct_sun[i][j] == NULL) goto memerr;
				for (k=0 ; k< (NumberOfSettingsInBlindgroup[3]+1) ; k++)
				{
					direct_sun[i][j][k]=(int**) malloc (sizeof(int*)* (NumberOfSettingsInBlindgroup[4]+1));
					if (direct_sun[i][j][k] == NULL) goto memerr;
					for (m=0 ; m< (NumberOfSettingsInBlindgroup[4]+1) ; m++)
					{
						direct_sun[i][j][k][m]=(int*) malloc (sizeof(int)* 8760*(int)(60/time_step));
						if (direct_sun[i][j][k][m] == NULL) goto memerr;
						for (n=0 ; n< 8760*(int)(60/time_step) ; n++)
						{
							direct_sun[i][j][k][m][n]=0;
						}	
					}
				}
			}
		}
		

	//three dimensional array into which the "maximum blind group illuminance" values are read
	// MaximumIlluminanancePerBlindGroup[BlindgroupNumber][NumberOfSettingsInBlindgroup[TimeStepInYear]
		MaximumIlluminanancePerBlindGroup = (float***) malloc (sizeof(float**) * (NumberOfBlindGroups+1));
		if (MaximumIlluminanancePerBlindGroup == NULL) goto memerr;
		for (i = 0; i<(NumberOfBlindGroups + 1); i++){
			MaximumIlluminanancePerBlindGroup[i] = (float**)malloc(sizeof(float*)* (NUM_BLINDS_SETTINGS - 1));
			if (MaximumIlluminanancePerBlindGroup[i] == NULL) goto memerr;
			for (j = 0; j < NUM_BLINDS_SETTINGS - 1; j++) { // WHY minus 1?
				MaximumIlluminanancePerBlindGroup[i][j] = (float*)malloc(sizeof(float)* time_steps_in_year);
				if (MaximumIlluminanancePerBlindGroup[i][j] == NULL) goto memerr;
				for (k = 0; k < time_steps_in_year; k++)
				{
					MaximumIlluminanancePerBlindGroup[i][j][k]=0;
				}
			}
		}




	dir=(float*) malloc (sizeof(float)*time_steps_in_year);
	if (dir == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) dir[i] = 0;

	month_1 = (int*)malloc(sizeof(int)*time_steps_in_year);
	if (month_1 == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) month_1[i] = 0;

	day_1 = (int*)malloc(sizeof(int)*time_steps_in_year);
	if (day_1 == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) day_1[i] = 0;

	hour_1 = (float*)malloc(sizeof(float)*time_steps_in_year);
	if (hour_1 == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) hour_1[i] = 0;

	int_sensor_ill = (float*)malloc(sizeof(float)*time_steps_in_year);
	if (int_sensor_ill == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) int_sensor_ill[i] = 0;

	ext_sensor_ill = (float*)malloc(sizeof(float)*time_steps_in_year);
	if (ext_sensor_ill == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) ext_sensor_ill[i] = 0;

	return;
memerr:
	error(SYSTEM, "out of memory in allocate_memory");
}

