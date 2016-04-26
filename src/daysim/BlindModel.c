/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#include <stdio.h>
#include <stdlib.h>
#include <rtmath.h>
#include <string.h>
#include <errno.h>

#include "sun.h"
#include "fropen.h"
#include "read_in_header.h"
#include "BlindModel.h"
#include "ds_el_lighting.h"
#include "ds_constants.h"


//====================================================================
// Function: getBlindSettings(int UserBehaviorBlinds)
// This function calculates setting of blinds during the course of the year
// It requires the existance of the arrays shading_profile[],
// direct_sun[][], and occ_profile[]. Results are written into
// shading_profile[].
// Input: 	UserBehaviorBlinds = 1 active or 2 passive
//====================================================================
int getBlindSettings(int UserBehaviorBlinds)
{

	int BlindGroupIndex = 0, i, j, k, arr;
	int month_1, day_1;
	float hour_1;
	float shading_fraction = 0.0;
	int JulianDay = 0;
	int SecondBlindGroupIndex = 0;
	float CurrentIlluminanceReading = 0, CurrentAllOpenIlluminanceReading = 0;
	double sd = 0;
	int GlareCondition = 0;
	double alt, azi, solar_time;
	int CoolingPeriodActivated = 0;
	FILE *ANNUAL_SHADING_PROFILE_FILE[11];

	// loop over blind gorups and check if a gorup has a preset annaul shading schedule
	for (BlindGroupIndex = 1; BlindGroupIndex <= NumberOfBlindGroups; BlindGroupIndex++)
	{
		if (strcmp(AnnualShadingProfile[BlindGroupIndex], ""))
		{
			//open shading profile and skip the first three header lines
			ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex] = open_input(AnnualShadingProfile[BlindGroupIndex]);
			fscanf(ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex], "%*[^\n]\n");
			fscanf(ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex], "%*[^\n]\n");
			fscanf(ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex], "%*[^\n]\n");
		}
	}

	/* annual loop */
	for (i = 0; i < time_steps_in_year; i += 24 * (int)(60 / time_step))
	
	{ 
		JulianDay++;
		GlareCondition=0;

		// find arrival time of the day
		arr=0;
		for (j=0 ; j<24*(int)(60/time_step) ; j++)
			if(occ_profile[i+j]==1 && arr==0)
				arr=i+j;

		//loop over day
		for (j=0 ; j<24*(int)(60/time_step) ; j++)
		{
			//loop over blind groups
			for (BlindGroupIndex=1 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
			{
				if(BlindSystemType[BlindGroupIndex]==1 && UserBehaviorBlinds==3 && DGP_filesDoNotExist) // active user (avoid 'disturbing discomfort glare [DGP>0.4])
				{
					printf("ds_el_lighting: SEVERE WARNING - Active user who avoids discomfort glare selected but no annual DGP profile available.\n");
					printf(" Behavior is changed to a user who avoids direct sunlight.\n");
					UserBehaviorBlinds=1;
				}

					
				//==============
				// static device (always open)
				//==============
				if(BlindSystemType[BlindGroupIndex] == 0)
				{
					shading_profile[BlindGroupIndex][i+j]=0;
				} //static device (always open)

				//===============
				// ManualControl
				//===============
				if(BlindSystemType[BlindGroupIndex]==1 || BlindSystemType[BlindGroupIndex]==2)
				{ //0=ManualControl, 1=IdealizedAutomatedControl

					if(UserBehaviorBlinds==1) // active user (avoid direct sunlight)
					{
						if((i+j)>0)
							shading_profile[BlindGroupIndex][i+j]=shading_profile[BlindGroupIndex][i+j-1];
						if((i+j) == arr )
							shading_profile[BlindGroupIndex][i+j]=0; /* open blinds upon arrival */
						if(BlindGroupIndex==1)
						{
							//Outside direct irradiance larger than 50W/m2 (direct sunlight); occupied
							//direct_sun[NumberOfBlindSettings_BG1][NumberOfBlindSettings_BG2][NumberOfBlindSettings_BG3][NumberOfBlindSettings_BG4][TimeStepInYear]
							if( direct_sun[shading_profile[BlindGroupIndex][i+j]][0][0][0][i+j]==1 && occ_profile[i+j]==1 )
							{
								k=shading_profile[BlindGroupIndex][i+j]; // current blind setting
								while(k<(NumberOfSettingsInBlindgroup[BlindGroupIndex]) && direct_sun[k][0][0][0][i+j]==1)
								{
									k++;
									shading_profile[BlindGroupIndex][i+j]=k;
								}
							} else if(BlindSystemType[BlindGroupIndex]==2 && direct_sun[shading_profile[BlindGroupIndex][i+j]][0][0][0][i+j]==0) // idealized automatic control
							{
								k=shading_profile[BlindGroupIndex][i+j];
								while(k>=0 && direct_sun[k][0][0][0][i+j]==0)
								{
									shading_profile[BlindGroupIndex][i+j]=k;
									k--;
								}
							}	
						} else if(BlindGroupIndex==2)
						{
							//Outside direct irradiance larger than 50W/m2; direct sunlight; occupied
							if( direct_sun[shading_profile[1][i+j]][shading_profile[BlindGroupIndex][i+j]][0][0][i+j]==1 && occ_profile[i+j]==1 )
							{
								k=shading_profile[BlindGroupIndex][i+j];
								while(k<(NumberOfSettingsInBlindgroup[BlindGroupIndex]) && direct_sun[shading_profile[1][i+j]][k][0][0][i+j]==1)
								{
									k++;
									shading_profile[BlindGroupIndex][i+j]=k;
								}
							}else if(BlindSystemType[BlindGroupIndex]==2 && direct_sun[shading_profile[1][i+j]][shading_profile[BlindGroupIndex][i+j]][0][0][i+j]==0) // idealized automatic control
							{
								k=shading_profile[BlindGroupIndex][i+j];
								while(k>=0 && direct_sun[shading_profile[1][i+j]][k][0][0][i+j]==0)
								{
									shading_profile[BlindGroupIndex][i+j]=k;
									k--;
								}
							}	
						}
					}
					if(BlindSystemType[BlindGroupIndex]==1 && UserBehaviorBlinds==2) // passive user
					{
						shading_profile[BlindGroupIndex][i+j]=NumberOfSettingsInBlindgroup[BlindGroupIndex];
					}
					if(BlindSystemType[BlindGroupIndex]==1 && UserBehaviorBlinds==3) // active user (avoid 'disturbing discomfort glare [DGP>0.4])
					{
						if(i+j==0)
							shading_profile[BlindGroupIndex][i+j]=0;
						else 
							shading_profile[BlindGroupIndex][i+j]=shading_profile[BlindGroupIndex][i+j-1];
						if(i+j == arr )
							shading_profile[BlindGroupIndex][i+j]=0; /* open blinds upon arrival */

						// see if shading have to be closed further
						if( occ_profile[i+j]==1)
						{
							//printf("time step %d eff dgp %f\n",i+j,effective_dgp[0][1][i+j]);
							if(effective_dgp[0][1][i+j]>=0.4) //glare for the base geometry
							{
								if(BlindGroupIndex==1)
								{
									shading_profile[BlindGroupIndex][i+j]=1;
									while((effective_dgp[1][shading_profile[BlindGroupIndex][i+j]][i+j]>=0.4) && shading_profile[BlindGroupIndex][i+j]<NumberOfSettingsInBlindgroup[BlindGroupIndex])
									{
										shading_profile[BlindGroupIndex][i+j]++; // close shading as much as possible
									}
								}
								if(BlindGroupIndex==2)
								{
									if(effective_dgp[1][shading_profile[1][i+j]][i+j]>=0.4) //there is still glare even thoughBlind Group 1 is activiated
									shading_profile[BlindGroupIndex][i+j]=1;
									while((effective_dgp[1][shading_profile[BlindGroupIndex][i+j]][i+j]>=0.4) && shading_profile[BlindGroupIndex][i+j]<NumberOfSettingsInBlindgroup[BlindGroupIndex])
									{
										shading_profile[BlindGroupIndex][i+j]++; // close shading as much as possible
									}								
								}								
							}
						}
					}
				}//end manual blind control

				//==========================
				// Automated Shading Control
				//==========================
				//BlindSystemType=3 Thermal Control: 
				//BlindSystemType=4 Thermal Control with Occupancy
				//BlindSystemType=5 Thermal and Glare Control
				//BlindSystemType=6 Thermal and Glare Control with Occupancy
				
				if(BlindSystemType[BlindGroupIndex]>=3 &&    BlindSystemType[BlindGroupIndex]<=6)
				{ 
					//by default assign blind setting form previous time step
					if((i+j)>0)
						shading_profile[BlindGroupIndex][i+j]=shading_profile[BlindGroupIndex][i+j-1];
					
					//If occupancy controlled (BlindSystemType=4 or 6) then determine if the time step is during the cooling period	
					// during the cooling period the shading group is fully closed during occupant absence, outside of the cooling period it is fully oepened.
					if( (BlindSystemType[BlindGroupIndex]==4 || BlindSystemType[BlindGroupIndex]==6))
					{
						CoolingPeriodActivated=0;
						//determine cooling periode
						//Northern Hemisphere
						if((BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][1]>=BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][0])&& (JulianDay>=BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][0] &&JulianDay<=BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][1]))
							CoolingPeriodActivated=1;
						//Southern Hemisphere
						if((BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][1]<BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][0])&& (JulianDay<=BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][0] ||JulianDay>=BlindGroupCoolingPeriodJulianDay[BlindGroupIndex][1]))
							CoolingPeriodActivated=1;
					} 
					if( (occ_profile[i+j]==0) &&(BlindSystemType[BlindGroupIndex]==4 || BlindSystemType[BlindGroupIndex]==6))
					{
						if(CoolingPeriodActivated ) //close as far as possible
							shading_profile[BlindGroupIndex][i+j]=NumberOfSettingsInBlindgroup[BlindGroupIndex];
						if((!CoolingPeriodActivated ) ) //open all the way
								shading_profile[BlindGroupIndex][i+j]=0;
					}else					
					//Shading device gets automatically set independant of occupancy
					{
						if(BlindSystemType[BlindGroupIndex]==4 || BlindSystemType[BlindGroupIndex]==6)
						{
							if((CoolingPeriodActivated && occ_profile[i+j]==1) ) //open all the way
								shading_profile[BlindGroupIndex][i+j]=0;
						}
						sd=sdec(JulianDay);
						solar_time=1.0*j*(time_step/60.0)+stadj(JulianDay);
						alt = degrees(salt(sd, solar_time));
						azi = degrees(sazi(sd, solar_time));
	
						GlareCondition=0;
						// Test for glare: The glare condition is that the sun is in a predetermined azimuth altitude 
						//range and that the external sensor above a user defined threshold level 
						if(BlindSystemType[BlindGroupIndex]==5 || BlindSystemType[BlindGroupIndex]==6)
						{
							//check if there is more than the upper threshold of illuminance on the external sensor:
							//if(MaximumBlindGroupIlluminance_External[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][i+j]>=BlindGroupThresholdForGlareSensor[BlindGroupIndex])
							if(MaximumBlindGroupIlluminance_External[0][1][i+j]>=BlindGroupThresholdForGlareSensor[BlindGroupIndex])
								if(azi>=BlindGroupAzimuthRange[BlindGroupIndex][0] && azi<=BlindGroupAzimuthRange[BlindGroupIndex][1])
									if(alt>=BlindGroupAltitudeRange[BlindGroupIndex][0] && alt<=BlindGroupAltitudeRange[BlindGroupIndex][1])
										GlareCondition=1;
						}//glare control activated
						//printf("MaximumBlindGroupIlluminance_External[%d][shading %d][i+j] = %.0f \n",BlindGroupIndex,shading_profile[BlindGroupIndex][i+j],MaximumBlindGroupIlluminance_External[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][i+j]);
						
						if(GlareCondition)
						{	//close as far as possible
							shading_profile[BlindGroupIndex][i+j]=NumberOfSettingsInBlindgroup[BlindGroupIndex];
						}else //use daylighting/energy control
						{
							//get current illuminance reading at the control sensor taking the current blind setting into account
							CurrentIlluminanceReading=MaximumBlindGroupIlluminance_Internal[0][1][i+j];
							CurrentAllOpenIlluminanceReading=CurrentIlluminanceReading;
							for (SecondBlindGroupIndex=1 ; SecondBlindGroupIndex<= NumberOfBlindGroups ; SecondBlindGroupIndex++)
								if(shading_profile[SecondBlindGroupIndex][i+j]>0)
									CurrentIlluminanceReading-=(CurrentAllOpenIlluminanceReading-MaximumBlindGroupIlluminance_Internal[SecondBlindGroupIndex][shading_profile[SecondBlindGroupIndex][i+j]][i+j]);
							if(CurrentIlluminanceReading<0)
								CurrentIlluminanceReading=0;

							//if CurrentIlluminanceReading is higher than the threshold level for the setting close the tinted level by one state
							if((CurrentIlluminanceReading>=BlindGroupSetpoints[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][1])&& (shading_profile[BlindGroupIndex][i+j]<NumberOfSettingsInBlindgroup[BlindGroupIndex]))
							{
								while((CurrentIlluminanceReading>=BlindGroupSetpoints[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][1])&& (shading_profile[BlindGroupIndex][i+j]<NumberOfSettingsInBlindgroup[BlindGroupIndex]))
								{
									shading_profile[BlindGroupIndex][i+j]++;
									CurrentIlluminanceReading=MaximumBlindGroupIlluminance_Internal[0][1][i+j];
									for (SecondBlindGroupIndex=1 ; SecondBlindGroupIndex<= NumberOfBlindGroups ; SecondBlindGroupIndex++)
									{
										if(shading_profile[SecondBlindGroupIndex][i+j]>0)
											CurrentIlluminanceReading-=(CurrentAllOpenIlluminanceReading-MaximumBlindGroupIlluminance_Internal[SecondBlindGroupIndex][shading_profile[SecondBlindGroupIndex][i+j]][i+j]);
										if(CurrentIlluminanceReading<0)
											CurrentIlluminanceReading=0;
									}
								}
							}else{
								//if CurrentIlluminanceReading is higher than the threshold level for the setting close the tinted level by one state
								while((CurrentIlluminanceReading<BlindGroupSetpoints[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][0])&& (shading_profile[BlindGroupIndex][i+j]>0))
								{
									shading_profile[BlindGroupIndex][i+j]--;
									CurrentIlluminanceReading=MaximumBlindGroupIlluminance_Internal[0][1][i+j];
									for (SecondBlindGroupIndex=1 ; SecondBlindGroupIndex<= NumberOfBlindGroups ; SecondBlindGroupIndex++)
									{
										if(shading_profile[SecondBlindGroupIndex][i+j]>0)
											CurrentIlluminanceReading-=(CurrentAllOpenIlluminanceReading-MaximumBlindGroupIlluminance_Internal[SecondBlindGroupIndex][shading_profile[SecondBlindGroupIndex][i+j]][i+j]);
										if(CurrentIlluminanceReading<0)
											CurrentIlluminanceReading=0;
									}
								}
							}
//printf("Blind: Time %d\tIll %.0f\tShading Status %d\tthreshold %.0f\treading all up %.f\n",i+j,CurrentIlluminanceReading,shading_profile[BlindGroupIndex][i+j],BlindGroupSetpoints[BlindGroupIndex][shading_profile[BlindGroupIndex][i+j]][1],MaximumBlindGroupIlluminance_Internal[0][1][i+j]);								
						
						}//end daylighting/energy control					
					}//space is occupied 
				} // end Automated Shading Control
				

				if (BlindSystemType[BlindGroupIndex] == 7)
				{	
					fscanf(ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex], "%f,%f,%f,%f", &month_1, &day_1, &hour_1, &shading_fraction);
					shading_profile[BlindGroupIndex][i + j] = (int)(rint(shading_fraction*NumberOfSettingsInBlindgroup[BlindGroupIndex]));
					if (shading_profile[BlindGroupIndex][i + j] < 0)
						shading_profile[BlindGroupIndex][i + j] = 0;
					if (shading_profile[BlindGroupIndex][i + j] > NumberOfSettingsInBlindgroup[BlindGroupIndex])
						shading_profile[BlindGroupIndex][i + j] = NumberOfSettingsInBlindgroup[BlindGroupIndex];
				}
				
			}
		} //loop over day

	

	}//loopover Blind Groups

	for (BlindGroupIndex = 1; BlindGroupIndex <= NumberOfBlindGroups; BlindGroupIndex++)
	{
		if (strcmp(AnnualShadingProfile[BlindGroupIndex], ""))
		{
			close_file(ANNUAL_SHADING_PROFILE_FILE[BlindGroupIndex]);
		}
	}


	return(UserBehaviorBlinds);
}

