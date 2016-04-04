/* function file of the Lightswitch-2002 manual lighting control algorithm
 *  Copyright (c) 2003
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

/* routine reads in illuminances for all work plane sensors and assigns	*/
/* dgp values [if available]	   	to	DaylightGlareProbability[]		*/

#include <stdlib.h>
#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"
#include "BlindModel.h"
#include "ds_el_lighting.h"

void get_illuminances( )
{
	int i,j,k,l, counter=1;
	int Setting_Index;
	FILE *ILL_INPUT;
	float x, current_raw_illuminance;
	//=====================
	// read in illuminances
	//=====================
	for (i=0 ; i<= NumberOfBlindGroups ; i++)
	{
		for (Setting_Index=1 ; Setting_Index<=NumberOfSettingsInBlindgroup[i] ; Setting_Index++)
		{
			//printf("open shading file: %s\n",shading_illuminance_file[counter]);
			ILL_INPUT =open_input(shading_illuminance_file[counter]);
			counter++;
			for (j = 0; j < time_steps_in_year; j++)
			{
				fscanf(ILL_INPUT, "%d %d %f ",&k,&l,&x); //read in month day hour
				MaximumBlindGroupIlluminance_Internal[i][Setting_Index][j]=-999;
				MaximumBlindGroupIlluminance_External[i][Setting_Index][j]=-999;
				for (k=0 ; k< number_of_sensors ; k++)
				{
					fscanf(ILL_INPUT, "%f ",&current_raw_illuminance);
					//set maximum illuminances for each internal and external blind group sensor
					if((BlindGroup[i][k]==1) &&   MaximumBlindGroupIlluminance_Internal[i][Setting_Index][j]<current_raw_illuminance)
					{
						MaximumBlindGroupIlluminance_Internal[i][Setting_Index][j]=current_raw_illuminance;
					}
					if((BlindGroup[i][k]==2) &&   MaximumBlindGroupIlluminance_External[i][Setting_Index][j]<current_raw_illuminance)
					{					
						MaximumBlindGroupIlluminance_External[i][Setting_Index][j]=current_raw_illuminance;
					}	
					//raw_illuminances[i][Setting_Index][j][k]=current_raw_illuminance;
					
				}
				//printf("Time %d MaximumBlindGroupIlluminance_External[%d][%d][j]=%.0f\n",j,i,Setting_Index,MaximumBlindGroupIlluminance_External[i][Setting_Index][j]);
			}
			close_file(ILL_INPUT);
		
		}
	}
	
}



void get_daylight_illuminances( )
{
	//This function calculates resulting illuminances and minimum illumiances at each time step based on the status of the blind groups
	FILE *ILL_INPUT;
	int BlindGroupIndex, Time_Index;
	int LG_Index,Sensor_Index,Setting_Index;
	int i,j,k,l, counter=1;
	float x;
	float current_raw_illuminance=0;
	float** Daylight_Illuminance_No_Blinds;
	
	//allocate memory for Daylight_Illuminance_No_Blinds[TimeState][NumberOfSensors]
	Daylight_Illuminance_No_Blinds = (float**)malloc(sizeof(float*)*time_steps_in_year);
	if (Daylight_Illuminance_No_Blinds == NULL) goto memerr;
	for (i = 0; i < time_steps_in_year; i++) {
		Daylight_Illuminance_No_Blinds[i] = (float*)malloc(sizeof(float)*number_of_sensors);
		if (Daylight_Illuminance_No_Blinds[i] == NULL) goto memerr;
		for (j = 0; j < number_of_sensors; j++)
			Daylight_Illuminance_No_Blinds[i][j] = 0;
	}

	
	
	for (BlindGroupIndex=0 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
	{
		for (Setting_Index=1 ; Setting_Index<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; Setting_Index++)
		{
			ILL_INPUT =open_input(shading_illuminance_file[counter]);
			counter++;
			for (Time_Index = 0; Time_Index < time_steps_in_year; Time_Index++)
			{
				fscanf(ILL_INPUT, "%d %d %f ",&k,&l,&x); //read in month day hour
				
				for (Sensor_Index=0 ; Sensor_Index< number_of_sensors ; Sensor_Index++)
				{
					fscanf(ILL_INPUT, "%f ",&current_raw_illuminance);
					if(BlindGroupIndex==0)
					{
						daylight_illuminances[Time_Index][Sensor_Index]=current_raw_illuminance;
						Daylight_Illuminance_No_Blinds[Time_Index][Sensor_Index]=current_raw_illuminance;
					}else{
						if(shading_profile[BlindGroupIndex][Time_Index]>0 && shading_profile[BlindGroupIndex][Time_Index]==Setting_Index) // blind group is closed
						{ 
							daylight_illuminances[Time_Index][Sensor_Index]-=(Daylight_Illuminance_No_Blinds[Time_Index][Sensor_Index]-current_raw_illuminance);
						}
					}
					if(daylight_illuminances[Time_Index][Sensor_Index]<0)
						daylight_illuminances[Time_Index][Sensor_Index]=0;
				}
			}
			close_file(ILL_INPUT);		
		}
	}
	
	for (i = 0; i < time_steps_in_year; i++)
		free(Daylight_Illuminance_No_Blinds[i]);
	free(Daylight_Illuminance_No_Blinds);
	
	//get minimum work plane illuminance
	for (Time_Index = 0; Time_Index < time_steps_in_year; Time_Index++)
	{
		for(LG_Index=1 ; LG_Index<= NumberOfLightingGroups ; LG_Index++)
			minimum_work_plane_ill[LG_Index][Time_Index]=100000.0;
		for (Sensor_Index=0 ; Sensor_Index< number_of_sensors ; Sensor_Index++)
		{
			for(LG_Index=1 ; LG_Index<= NumberOfLightingGroups ; LG_Index++)
			{
				if(LightingGroup[LG_Index][Sensor_Index]==1  && daylight_illuminances[Time_Index][Sensor_Index]<minimum_work_plane_ill[LG_Index][Time_Index])
					minimum_work_plane_ill[LG_Index][Time_Index]=daylight_illuminances[Time_Index][Sensor_Index];
					
				//if no work plane sensor is provided, the illuminance level is set to zero (no daylight)
				if(IllumianceSensorExists ==0)
					minimum_work_plane_ill[LG_Index][Time_Index]=0;
			}			
		}	
	}
memerr:
	error(SYSTEM, "out of memory in get_daylight_illuminances");
}
				

//====================
//read in DGP profiles
//====================
void get_DGP_profiles( )
{
	int j,k,l,i,counter=1;
	int number_of_dgp_columns=0;
	int last_element_blank=0;
	int StringLength=0;
	int BlindGroupIndex,Setting_Index;
	float x;
	float current_DGP=0;
	FILE *ILL_INPUT;
	char  line_string[1000001]="";
	
	for (BlindGroupIndex=0 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
	{
		for (Setting_Index=1 ; Setting_Index<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; Setting_Index++)
		{
			//Step (1): Determine file names and check whether the files exist.
			StringLength=strlen(shading_illuminance_file[counter]);
			for (j = 0; j < StringLength-4; j++)
		    {
		        DGP_Profiles_file[counter][j] = shading_illuminance_file[counter][j];
			}
			sprintf(DGP_Profiles_file[counter],"%s.dgp",DGP_Profiles_file[counter]);
            //printf("Searching for Blind Group %d Setting file: %d \"%s\"\n",BlindGroupIndex,Setting_Index,DGP_Profiles_file[counter]);
			//Step (2) Check if file exisits
			if(check_if_file_exists(DGP_Profiles_file[counter]))
			{
				ILL_INPUT=open_input(DGP_Profiles_file[counter]);
				//check the number of DGP columns in the file
				fgets(line_string,1000000,ILL_INPUT);
				number_of_dgp_columns=0;
				for (i=0;i<1000000;i++)
				{
					//make sure you don't exceed the bounds of the array
					if (line_string[i] == '\n')
					{
						if(last_element_blank==0)
							number_of_dgp_columns++;
						break; //exits the loop without completing the current iteration
					}
					if(line_string[i] == ' ' || line_string[i] == '\t')
					{
						if(last_element_blank==0)
							number_of_dgp_columns++;
						last_element_blank=1;
						continue; //goes to the next iteration of the loop without finishing the current iteration
					}else {
						last_element_blank=0;
						continue;
					}
				}
				number_of_dgp_columns=number_of_dgp_columns-3;
				if(number_of_dgp_columns<1)
				{
					DGP_filesDoNotExist=1;
					printf("WARNING: DGP profile: %s has the wrong data format.\n",DGP_Profiles_file[counter]);
				}
				if(number_of_dgp_columns!=number_of_view_points)
				{
					printf("WARNING: DGP profile: %s The number of dgp columns in the file (%d) differs form the number of lines in the view file (%d). Going forward %d is used.\n",DGP_Profiles_file[counter],number_of_dgp_columns,number_of_view_points,number_of_dgp_columns);
					number_of_view_points=number_of_dgp_columns;
				}
				
				//printf("Number of dgp columns in %s is %d.\n",DGP_Profiles_file[counter], number_of_view_points);
				rewind(ILL_INPUT);	
				for (j = 0; j < time_steps_in_year; j++)
					{
							fscanf(ILL_INPUT,"%d %d %f ",&k,&l,&x);// read in month day time
							if(AdaptiveZoneApplies == 1 )
									effective_dgp[BlindGroupIndex][Setting_Index][j]=1.0;
							if(AdaptiveZoneApplies == 0 )
								effective_dgp[BlindGroupIndex][Setting_Index][j]=-9999;
							for (k=0 ; k< number_of_view_points ; k++)
							{
								fscanf(ILL_INPUT, "%f ",&current_DGP);
								//======================
								// assign effective DGP
								//======================
								if(AdaptiveZoneApplies == 1 ) //lowest DGP in view point file counts
								{
									if(current_DGP<effective_dgp[BlindGroupIndex][Setting_Index][j])
										effective_dgp[BlindGroupIndex][Setting_Index][j]=current_DGP;
								}
								if(AdaptiveZoneApplies == 0 ) //highest DGP in view point file counts
								{
									if(current_DGP>effective_dgp[BlindGroupIndex][Setting_Index][j])
										effective_dgp[BlindGroupIndex][Setting_Index][j]=current_DGP;
								}
							}
					}
				close_file(ILL_INPUT);
				if(BlindGroupIndex==0)
					printf("DGP for Base Geometry found and processed: %s\n\n",DGP_Profiles_file[counter]);
				else
					printf("DGP for Blind Group %d setting %d found and processed: %s\n\n",BlindGroupIndex, Setting_Index, DGP_Profiles_file[counter]);
			}else{ //DGP file does not exist.
				DGP_filesDoNotExist=1;
				printf("WARNING: DGP profile: %s not found.\n",DGP_Profiles_file[counter]);
			}
			counter++;
		}	
	}

}



