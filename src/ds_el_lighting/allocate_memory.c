/* function file of the Lightswitch-2002 manual lighting control algorithm
 *  Copyright (c) 2002
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
*/

/* in this file the memory for the dynamic variables used
 * by the Lightswitch algorithm are dynamically allocated
*/

#include "globals.h"

void allocate_memory()
{
	int i,j,k,m,n;
/*==================*/
/* allocate memory */
/*==================*/
	i=8760*(int)(60/time_step);
	occ_profile=(int*) malloc (sizeof(int)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		occ_profile[i]=0;
	}
	
	i=8760*(int)(60/time_step);
	dgp_profile=(float*) malloc (sizeof(float)*i);
	for (i=0 ; i<8760*(float)(60/time_step) ; i++){
		dgp_profile[i]=0;
	}

	//resulting daylight_illuminances[TimeStepInYear]
	i=8760*(int)(60/time_step);
	daylight_illuminances=(float**) malloc (sizeof(float*)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)
		daylight_illuminances[i] =(float*) malloc (sizeof(float)*number_of_sensors);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)
	{
		for (j=0 ; j< number_of_sensors ; j++)
		{
			daylight_illuminances[i][j]=0;
		}
	}

	
	//2-dim array for the shading status of each blind group
	//shading_profile[NumberOfBlindGroups+1][TimeStepInYear]
	j=8760*(int)(60/time_step);
	shading_profile = (int**) malloc (sizeof(int*) * (NumberOfBlindGroups+1));
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
	{
		shading_profile[i] =(int*) malloc (sizeof(int)* j);
	}
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)
	{
		for (j=0 ; j< (NumberOfBlindGroups+1) ; j++)
		{
			shading_profile[j][i]=0;
		}
	}


	//2-dim array for the shading status of each lighting group
	//electric_lighting_profile[NumberOfLightingGroups+1][TimeStepInYear]
	j=8760*(int)(60/time_step);
	electric_lighting_profile = (float**) malloc (sizeof(float*) * (NumberOfLightingGroups+1));
	for (i=0 ; i<(NumberOfLightingGroups+1) ; i++)
	{
		electric_lighting_profile[i] =(float*) malloc (sizeof(float)* j);
	}
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)
	{
		for (j=0 ; j< (NumberOfLightingGroups+1) ; j++)
		{
			electric_lighting_profile[j][i]=0;
		}
	}
	
	//2-dim array for the max. ill. at the blind group contol points
	//MaximumBlindGroupIlluminance_Internal[NumberOfBlindGroups][NumberOfBlindSettings][TimeStepInYear]
	j=8760*(int)(60/time_step);
	MaximumBlindGroupIlluminance_Internal = (float***) malloc (sizeof(float*) * (NumberOfBlindGroups+1));
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		MaximumBlindGroupIlluminance_Internal[i] =(float**) malloc (sizeof(float*)* 11);
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			MaximumBlindGroupIlluminance_Internal[i][k] =(float*) malloc (sizeof(float)* 8760*(int)(60/time_step));
	
	for (i=0 ; i< (NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			for (j=0 ; j<8760*(int)(60/time_step) ; j++)
				MaximumBlindGroupIlluminance_Internal[i][k][j]=0;
	
	//2-dim array for the max. ill. at the blind group contol points
	//MaximumBlindGroupIlluminance_External[NumberOfBlindGroups][NumberOfBlindSettings][TimeStepInYear]
	j=8760*(int)(60/time_step);
	MaximumBlindGroupIlluminance_External = (float***) malloc (sizeof(float*) * (NumberOfBlindGroups+1));
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		MaximumBlindGroupIlluminance_External[i] =(float**) malloc (sizeof(float*)* 11);
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			MaximumBlindGroupIlluminance_External[i][k] =(float*) malloc (sizeof(float)* 8760*(int)(60/time_step));
	
	for (i=0 ; i< (NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			for (j=0 ; j<8760*(int)(60/time_step) ; j++)
				MaximumBlindGroupIlluminance_External[i][k][j]=0;
	
	
	
	//2-dim array for the min. ill. at the work place
	//minimum_work_plane_ill[NumberOfLightingGroups][TimeStepInYear]
	j=8760*(int)(60/time_step);
	minimum_work_plane_ill = (float**) malloc (sizeof(float*) * (NumberOfLightingGroups+1));
	for (i=0 ; i<(NumberOfLightingGroups+1) ; i++){
		minimum_work_plane_ill[i] =(float*) malloc (sizeof(float)* j);
	}
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		for (j=0 ; j<= NumberOfLightingGroups ; j++){
			minimum_work_plane_ill[j][i]=0;
		}
	}
	
	//3-dim array for the effective dgp at all view points
	//effective_dgp[NumberOfBlindGroups][MAxNumberOfSetting inalBlindGroups][TimeStepInYear]
	effective_dgp = (float***) malloc (sizeof(float**) * (NumberOfBlindGroups+1));
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		effective_dgp[i] =(float**) malloc (sizeof(float*)* 11);
		
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			effective_dgp[i][k] =(float*) malloc (sizeof(float)* 8760*(int)(60/time_step));
	for (i=0 ; i<(NumberOfBlindGroups+1) ; i++)
		for (k=0 ; k<11 ; k++)
			for (j=0 ; j<8760*(int)(60/time_step) ; j++)
				effective_dgp[i][k][j]=0;

				
	//++++++++++++++++
	// Dynamic Metrics
	//++++++++++++++++

	//daylight autonomy for active and passice user
	daylight_autonomy=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		daylight_autonomy[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			daylight_autonomy[i][j]=0;

	//continuous daylight autonomy for active and passice user
	continuous_daylight_autonomy=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		continuous_daylight_autonomy[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			continuous_daylight_autonomy[i][j]=0;

	//DA max for active and passice user
	DA_max=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		DA_max[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			DA_max[i][j]=0;

	//UDI_100 for active and passice user
	UDI_100=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		UDI_100[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			UDI_100[i][j]=0;

	//UDI_100_2000 for active and passice user
	UDI_100_2000=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		UDI_100_2000[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			UDI_100_2000[i][j]=0;

	//UDI_2000 for active and passice user
	UDI_2000=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		UDI_2000[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			UDI_2000[i][j]=0;


	//AnnualLightExposure[number_of_sensors][active/pasive]
	//array that stores the annual light exposire of each sensor point
	AnnualLightExposure=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		AnnualLightExposure[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			AnnualLightExposure[i][j]=0;
			
	//DaylightSaturationPercentage for active and passive user
	DaylightSaturationPercentage=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		DaylightSaturationPercentage[i]=(float*) malloc (sizeof(float)*2);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<2 ; j++)
			DaylightSaturationPercentage[i][j]=0;



	//3-dim array for the min. ill. at the work place
	//work_plane_ill[TotalNumberOfDCFiles+1][TimeStepInYear][number_of_sensors]
	j=8760*(int)(60/time_step);
	work_plane_ill = (float***) malloc (sizeof(float**) * (TotalNumberOfDCFiles+1));
	for (i=0 ; i<(TotalNumberOfDCFiles+1) ; i++){
		work_plane_ill[i] =(float**) malloc (sizeof(float*)* j);
	}
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		for (j=0 ; j< (TotalNumberOfDCFiles+1) ; j++){
			work_plane_ill[j][i]=(float*) malloc (sizeof(float)* number_of_sensors);
		}
	}
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		for (j=0 ; j< (TotalNumberOfDCFiles+1) ; j++){
			for (k=0 ; k< number_of_sensors ; k++){work_plane_ill[j][i][k]=0;}
		}
	}

	//4-dim array for the illuminances for the *.ill files
	//raw_illuminances[Blindgroup][BlindSetting][TimeStepInYear][number_of_sensors]
	raw_illuminances = (float****) malloc (sizeof(float***) * (NumberOfBlindGroups+1));
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
	for (i=0 ; i<(number_of_sensors+1) ; i++)
		da[i]=0;


	//five dimensional array into which the "direct sunlight" settings are read
	// direct_sun[NumberOfBlindSettings_BG1][NumberOfBlindSettings_BG2][NumberOfBlindSettings_BG3][NumberOfBlindSettings_BG4][TimeStepInYear]
		j=8760*(int)(60/time_step);
		direct_sun = (int*****) malloc (sizeof(int****) * (NumberOfSettingsInBlindgroup[1]+1));
		for (i=0 ; i<(NumberOfSettingsInBlindgroup[1]+1) ; i++)
			direct_sun[i] =(int****) malloc (sizeof(int***)* (NumberOfSettingsInBlindgroup[2]+1));
		for (i=0 ; i<(NumberOfSettingsInBlindgroup[1]+1) ; i++)
		{
			for (j=0 ; j< (NumberOfSettingsInBlindgroup[2]+1) ; j++)
			{
				direct_sun[i][j]=(int***) malloc (sizeof(int**)* (NumberOfSettingsInBlindgroup[3]+1));
			}
		}
		for (i=0 ; i<(NumberOfSettingsInBlindgroup[1]+1) ; i++)
		{
			for (j=0 ; j< (NumberOfSettingsInBlindgroup[2]+1) ; j++)
			{
				for (k=0 ; k< (NumberOfSettingsInBlindgroup[3]+1) ; k++)
				{
					direct_sun[i][j][k]=(int**) malloc (sizeof(int*)* (NumberOfSettingsInBlindgroup[4]+1));
				}
			}
		}
		for (i=0 ; i<(NumberOfSettingsInBlindgroup[1]+1) ; i++)
		{
			for (j=0 ; j< (NumberOfSettingsInBlindgroup[2]+1) ; j++)
			{
				for (k=0 ; k< (NumberOfSettingsInBlindgroup[3]+1) ; k++)
				{
					for (m=0 ; m< (NumberOfSettingsInBlindgroup[4]+1) ; m++)
					{
						direct_sun[i][j][k][m]=(int*) malloc (sizeof(int)* 8760*(int)(60/time_step));
					}
				}
			}
		}
		for (i=0 ; i<(NumberOfSettingsInBlindgroup[1]+1) ; i++)
		{
			for (j=0 ; j< (NumberOfSettingsInBlindgroup[2]+1) ; j++)
			{
				for (k=0 ; k< (NumberOfSettingsInBlindgroup[3]+1) ; k++)
				{
					for (m=0 ; m< (NumberOfSettingsInBlindgroup[4]+1) ; m++)
					{
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
		j=8760*(int)(60/time_step);
		MaximumIlluminanancePerBlindGroup = (float***) malloc (sizeof(float**) * (NumberOfBlindGroups+1));
		for (i=0 ; i<(NumberOfBlindGroups+1) ; i++){
			MaximumIlluminanancePerBlindGroup[i] =(float**) malloc (sizeof(float*)* 10);
		}
		for (i=0 ; i<(NumberOfBlindGroups+1) ; i++){
			for (j=0 ; j< 10 ; j++){
				MaximumIlluminanancePerBlindGroup[i][j]=(float*) malloc (sizeof(float)* 8760*(int)(60/time_step));
			}
		}
		for (i=0 ; i<(NumberOfBlindGroups+1) ; i++){
			for (j=0 ; j< 10 ; j++){
				for (k=0 ; k< 8760*(int)(60/time_step) ; k++)
				{
					MaximumIlluminanancePerBlindGroup[i][j][k]=0;
				}
			}
		}




	i=8760*(int)(60/time_step);
	dir=(float*) malloc (sizeof(float)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)dir[i]=0;

	i=8760*(int)(60/time_step);
	month_1=(int*) malloc (sizeof(int)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)month_1[i]=0;

	i=8760*(int)(60/time_step);
	day_1=(int*) malloc (sizeof(int)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)day_1[i]=0;

	i=8760*(int)(60/time_step);
	hour_1=(float*) malloc (sizeof(float)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)hour_1[i]=0;


	i=8760*(int)(60/time_step);
	int_sensor_ill=(float*) malloc (sizeof(float)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)int_sensor_ill[i]=0;

	i=8760*(int)(60/time_step);
	ext_sensor_ill=(float*) malloc (sizeof(float)*i);
	for (i=0 ; i<8760*(int)(60/time_step) ; i++)ext_sensor_ill[i]=0;



}

