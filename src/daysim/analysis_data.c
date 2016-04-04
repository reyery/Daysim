/* function file of the Lightswitch-2002 manual lighting control algorithm
 *  Copyright (c) 2002
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */


/* function calculates the electric lighting use per time step,
 * based on the status and type of the eletric lighting system
 *
    (1) UserLight 		1  user considers daylight (active use of lighting)
 						2  user does not consider daylight (passive use of lighting)
	(2)	UserBlind 		1  user uses blinds daily (active use of blinds)
*/

#include <stdlib.h>
#include "rterror.h"
#include "fropen.h"
#include "read_in_header.h"
#include "BlindModel.h"
#include "daylightfactor.h"
#include "ds_el_lighting.h"

void get_electric_lighting_energy_use(int UserLight,int UserBlind)
{
	int i,j=0;
	int SwitchForThermalOutput=0; //0=no thermal output, 1=active thermal file; 2=passive thermal file
	int LightingGroupIndex,BlindGroupIndex;
	int CounterThermalFile;
	float LightEnergyOutput=0;
	float MeanLightEnergyOutput[11], mean_occ=0, mean_shading=0,mean_dgp=0;
	FILE *THERMAL_FILE= NULL;
	FILE *EFFECTIVE_DGP_FILE = NULL;
	
	AnnualNumberOfOccupiedHours=0.0;
	for (j=0 ; j<=10 ; j++)
		HoursWithAView[j]=0.0;

	//calculate percentage of hours with a view
	//=========================================
	for (i=1 ; i<8760*(int)(60/time_step) ; i++)
	{
		if(occ_profile[i]==1)
			AnnualNumberOfOccupiedHours+=1.0*time_step/60;
		for (j=1 ; j<=NumberOfBlindGroups ; j++)
		{
			if(occ_profile[i]==1 && shading_profile[j][i]==0)
				HoursWithAView[j]+=1.0*time_step/60;
		}
	}
	for (j=0 ; j<=10 ; j++)
		HoursWithAView[j]=HoursWithAView[j]/AnnualNumberOfOccupiedHours*100.0;


	for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
	{
		AnnualNumberOfActivatedElectricLighting[LightingGroupIndex]=0.0;
		MeanLightEnergyOutput[LightingGroupIndex]=0.0;
	}
	
	for (i=1 ; i<8760*(int)(60/time_step) ; i++)
	{
		for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
		{
			if(electric_lighting_profile[LightingGroupIndex][i]==1)
				AnnualNumberOfActivatedElectricLighting[LightingGroupIndex]+=1.0*time_step/60;
		}
	}
	/*=====================*/
	/* Thermal Output File */
	/*=====================*/
	if(strcmp(thermal_sim_file_active,"") )
		SwitchForThermalOutput=1;
	if(strcmp(thermal_sim_file_passive,"") )
		SwitchForThermalOutput=2;

	if(!DGP_filesDoNotExist) //write out effective dgp profile file *dgp
	{
		EFFECTIVE_DGP_FILE=open_output(Effective_DGP_file);
		printf("effective dgp file: %s\n",Effective_DGP_file);
	}
		
	if(SwitchForThermalOutput>0)
	{
		if(SwitchForThermalOutput==1)
		{
			THERMAL_FILE=open_output(thermal_sim_file_active);
		}
		if(SwitchForThermalOutput==2)
		{
			THERMAL_FILE=open_output(thermal_sim_file_passive);
		}
		fprintf(THERMAL_FILE,"Daysim schedule file (to be used in combination with a thermal simulation program)\n");
		fprintf(THERMAL_FILE,"Daysim header file: %s; ",input_file);
		fprintf(THERMAL_FILE,"Occupant behavior: ");
		if(UserLight==1)
			fprintf(THERMAL_FILE,"The occupant(s) actively operate the electric lighting system and ");
		if(UserLight==2)
			fprintf(THERMAL_FILE,"The occupant(s) passively operate the electric lighting system and ");
		if(UserBlind==1)
			fprintf(THERMAL_FILE,"close the blinds to avoid direct sunlight on the work plane.,");
		if(UserBlind==2)
					fprintf(THERMAL_FILE,"keep the blinds lowered all year round.;");
		if(UserBlind==3)
					fprintf(THERMAL_FILE,"operates the blinds to avoid distrubing glare (DGP>0.4).,");
		fprintf(THERMAL_FILE,",,Occupied Hours: %.0f",AnnualNumberOfOccupiedHours);

		for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
			fprintf(THERMAL_FILE,",Lighting Group %d - %s",LightingGroupIndex,lighting_scenario_name[LightingGroupIndex]);
		for (BlindGroupIndex=1 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
			fprintf(THERMAL_FILE,",Shading Group %d - %s",BlindGroupIndex,BlindGroupName[BlindGroupIndex]);
		if(!DGP_filesDoNotExist) //write out effective dgp profile
		{
			//fprintf(THERMAL_FILE,",Daylight Glare Probability");
			for (i=1 ; i<8760*(int)(60/time_step) ; i++)
			{
				dgp_profile[i]=effective_dgp[0][1][i]; // set dgp originally to the all blinds up case
				if(NumberOfBlindGroups>0)
				{
					if(shading_profile[1][i]>0) //if blind group 1 closed
					{
							dgp_profile[i]=	effective_dgp[1][shading_profile[1][i]][i];
					}
					for (BlindGroupIndex=2 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++) // pick the smallest dgp resulting form the different blind groups
					{
						if(shading_profile[BlindGroupIndex][i]>0 && effective_dgp[BlindGroupIndex][shading_profile[BlindGroupIndex][i]][i]<dgp_profile[i]) //if blind group closed
						{
							dgp_profile[i]=	effective_dgp[BlindGroupIndex][shading_profile[BlindGroupIndex][i]][i];
						}	
					}
				}
			}
		}	
		fprintf(THERMAL_FILE,"\n");

		fprintf(THERMAL_FILE,",,,Installed Lighting Power [W]");
		for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
			fprintf(THERMAL_FILE,",%.1f",LightPower[LightingGroupIndex]+StandbyPower[LightingGroupIndex]);
		//if(NumberOfLightingGroups==0)
		//	fprintf(THERMAL_FILE,",No lighting groups specified.");
		for (BlindGroupIndex=1 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
			fprintf(THERMAL_FILE,",View %.0f",HoursWithAView[BlindGroupIndex]);
		if(NumberOfBlindGroups==0)
			fprintf(THERMAL_FILE,",No blind groups specified.");
		//if(!DGP_filesDoNotExist)
		//	fprintf(THERMAL_FILE,",");
		fprintf(THERMAL_FILE,"\n");

		
		fprintf(THERMAL_FILE,"month, day, hour, occupancy [0=absent...1=present]");
		for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
			fprintf(THERMAL_FILE,",lighting [0=off...1=full on]");
		for (BlindGroupIndex=1 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
			fprintf(THERMAL_FILE,",blinds [0=up; 1=down]");
		fprintf(THERMAL_FILE,"\n");	
		//fprintf(THERMAL_FILE,"effective DGP (if applicable; otherwise '-9999') \n");
	}

	
	/* loop over the year */
	CounterThermalFile=0;
	for (i=0 ; i<8760*(int)(60/time_step) ; i++){
		
		//write time stamp and occupancy
		if(SwitchForThermalOutput>0)
		{
			CounterThermalFile++;
			if(CounterThermalFile ==(int)(60/time_step))
			{
				mean_occ=0;
				for (j=0 ; j< (int)(60/time_step); j++)
					mean_occ+=occ_profile[i-j]*(time_step/60.0);
				fprintf(THERMAL_FILE,"%d,%d,%.3f,%.1f", month_1[i],day_1[i],hour_1[i],mean_occ);	
			}
		}
		
		for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
		{
			
			LightEnergyOutput=0;
			
			/*=============================*/
			/* ELECTRIC LIGHTING & STANDBY */
			/*=============================*/
			switch ( LightingSystemType[LightingGroupIndex]) {
				case 1:{// manual on/off switch by the door;
					if(electric_lighting_profile[LightingGroupIndex][i] >0 ){
		    			/* energy demand at full lighting output */
		    			LightEnergyOutput+=(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i];
					}
					break;
				}
				case 2:{// manual on/off switch by the door; energy effiicient occ. sensor
					// occupancy sensor is active when system turned on
					if(electric_lighting_profile[LightingGroupIndex][i] >0)
					{
				   		LightEnergyOutput+=(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i];
					}
				   	break;
				}
				case 3:{// manual on/off switch by the door; on/off occ. sensor
					// occupancy sensor which is alays active
					if(electric_lighting_profile[LightingGroupIndex][i] >0 ){
		    			/* energy demand at full lighting output */
		    			LightEnergyOutput+=(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
					}
				    LightEnergyOutput+=(time_step*1.0/60.0)*(StandbyPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
					break;
				}
				case 4: { //manual on/off switch by the door &ideally dimmed lighting system
					//sensor that is turned off with the system
					//the standby power = power demand of electronic ballast
					if(electric_lighting_profile[LightingGroupIndex][i] >0){
						if(minimum_work_plane_ill[LightingGroupIndex][i]<MinIllLevel[LightingGroupIndex])
						{
							LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01+((100.0-MinDimLevel[LightingGroupIndex])*0.01)*((MinIllLevel[LightingGroupIndex]-minimum_work_plane_ill[LightingGroupIndex][i])*1.0/(MinIllLevel[LightingGroupIndex])))*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						}else{
							/* lighting system only dims down to a minimum level */
							LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01)*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						}
						LightEnergyOutput+=(time_step*1.0/60.0)*(StandbyPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
					}
					break;
				}
				case 5: { //manual on/off switch by the door &ideally dimmed lighting system
					//sensor that is turned off with the system
					//the standby power = power demand of electronic ballast
					if(electric_lighting_profile[LightingGroupIndex][i] >0)
					{
						if(minimum_work_plane_ill[LightingGroupIndex][i] <MinIllLevel[LightingGroupIndex])
						{
							LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01+((100.0-MinDimLevel[LightingGroupIndex])*0.01)*((MinIllLevel[LightingGroupIndex]-minimum_work_plane_ill[LightingGroupIndex][i])*1.0/(MinIllLevel[LightingGroupIndex])))*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						}else{
							/* lighting system only dims down to a minimum level */
							LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01)*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						}
						LightEnergyOutput+=(time_step*1.0/60.0)*(StandbyPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
					}
					break;
				}
				case 6: { //on/off occ sensor &ideally dimmed lighting system
						//sensor that is turned off with the system
						//the standby power = power demand of electronic ballast
						if(electric_lighting_profile[LightingGroupIndex][i] >0)
						{
							if(minimum_work_plane_ill[LightingGroupIndex][i]<MinIllLevel[LightingGroupIndex])
							{
								LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01+((100.0-MinDimLevel[LightingGroupIndex])*0.01)*((MinIllLevel[LightingGroupIndex]-minimum_work_plane_ill[LightingGroupIndex][i])*1.0/(MinIllLevel[LightingGroupIndex])))*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
							}else{
								/* lighting system only dims down to a minimum level */
								LightEnergyOutput+=(MinDimLevel[LightingGroupIndex]*0.01)*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i]*(LightPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
							}
						}
						LightEnergyOutput+=(time_step*1.0/60.0)*(StandbyPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						// Lighting Output in W
						
						break;
				}
				case 10: { //scheduled lighting with an on/off photocell control
						//the standby power = power demand of electronic ballast
						if(electric_lighting_profile[LightingGroupIndex][i] >0)
						{
							if(minimum_work_plane_ill[LightingGroupIndex][i]<MinIllLevel[LightingGroupIndex])
							{
								LightEnergyOutput+=(LightPower[LightingGroupIndex]/1000.0)*(time_step*1.0/60.0)*electric_lighting_profile[LightingGroupIndex][i];
							}
							LightEnergyOutput+=(time_step*1.0/60.0)*(StandbyPower[LightingGroupIndex]/(StandbyPower[LightingGroupIndex]+LightPower[LightingGroupIndex]));
						}
						break;
				}
			}	

			/* yearly electric lighting energy demand */
			LightingGroupEnergyUse[LightingGroupIndex][0]+=LightEnergyOutput*(LightPower[LightingGroupIndex]+StandbyPower[LightingGroupIndex])/1000.0;	

			/* monthly electric lighting energy demand */
			LightingGroupEnergyUse[LightingGroupIndex][month_1[i]]+=LightEnergyOutput*(LightPower[LightingGroupIndex]+StandbyPower[LightingGroupIndex])/1000.0;

			/* thermal simulation profile */
			if(SwitchForThermalOutput>0){
				MeanLightEnergyOutput[LightingGroupIndex]+=LightEnergyOutput;
				if(CounterThermalFile ==(int)(60/time_step))
				{
					fprintf(THERMAL_FILE,",%.2f", MeanLightEnergyOutput[LightingGroupIndex]);
					MeanLightEnergyOutput[LightingGroupIndex]=0;
				}		
			}
		} //end loop over lighting group
		
		if(SwitchForThermalOutput>0 && (CounterThermalFile ==(int)(60/time_step)))
		{
			//if(NumberOfLightingGroups==0)
			//	fprintf(THERMAL_FILE,",0");

			if(simple_blinds_model==1)
				NumberOfSettingsInBlindgroup[1]=1;
				
			for (BlindGroupIndex=1 ; BlindGroupIndex<= NumberOfBlindGroups ; BlindGroupIndex++)
			{
				mean_shading=0;
				for (j=0 ; j< (int)(60/time_step); j++)
					mean_shading+=shading_profile[BlindGroupIndex][i-j]*(time_step/60.0);
				fprintf(THERMAL_FILE,",%.1f",1.0*mean_shading/NumberOfSettingsInBlindgroup[BlindGroupIndex]);
			} // end over blind groups
			if(NumberOfBlindGroups==0)
				fprintf(THERMAL_FILE,",0");
			CounterThermalFile=0;
			if(!DGP_filesDoNotExist) //write out effective dgp profile
			{
				mean_dgp=0;
				for (j=0 ; j< (int)(60/time_step); j++)
					mean_dgp+=dgp_profile[i-j]*(time_step/60.0);
					if(mean_occ==0)
						mean_dgp=-999;
				if(mean_dgp>0.4)
					AnnualHoursWithGlare++;
				//fprintf(THERMAL_FILE,",%.3f",1.0*mean_dgp);
				fprintf(EFFECTIVE_DGP_FILE,"%d %d %.3f %.3f\n",month_1[i], day_1[i], hour_1[i],1.0*mean_dgp);
				
			}
			fprintf(THERMAL_FILE,"\n");
		}
	} /* end loop over the year */

	if(SwitchForThermalOutput>0)
		close_file(THERMAL_FILE);
		
	if(!DGP_filesDoNotExist) //write out effective dgp profile file *dgp
		close_file(EFFECTIVE_DGP_FILE);
		
	
}

void genLightExposureAndDaylightAutonomy(int BlindBehavior)
{
	int Time_Index,k;
	float occupied_hours_per_year=0.0;
	
	if(BlindBehavior==3 ||BlindBehavior==1)
	{
		BlindBehavior=0;
	}else if(BlindBehavior==2)
		BlindBehavior=1;
	
	if(BlindBehavior==0)
		ActiveOccupant=1;
	if(BlindBehavior==1)
		PassiveOccupant=1;
		
	for (k=0 ; k< number_of_sensors ; k++)
	{
		AnnualLightExposure[k][BlindBehavior]=0;
		daylight_autonomy[k][BlindBehavior]=0;
		continuous_daylight_autonomy[k][BlindBehavior]=0;
		DA_max[k][BlindBehavior]=0;
		UDI_2000[k][BlindBehavior]=0;
		UDI_100[k][BlindBehavior]=0;
		UDI_100_2000[k][BlindBehavior]=0;
	}		
		
	for (Time_Index=0 ; Time_Index<8760*(int)(60/time_step) ; Time_Index++)
	{
		for (k=0 ; k< number_of_sensors ; k++)
		{
			if(occ_profile[Time_Index]==1) // occupied hour
			{
				if(k==0)
					occupied_hours_per_year++;
				
				// annual light exposure
				AnnualLightExposure[k][BlindBehavior]+=daylight_illuminances[Time_Index][k]*1.0*time_step/60.0;

				// daylight autonomy
				if(daylight_illuminances[Time_Index][k] >= ill_min)
					daylight_autonomy[k][BlindBehavior]+=1.0;
					
				// continuous daylight autonomy 
				if(daylight_illuminances[Time_Index][k] >= ill_min)
				{
					continuous_daylight_autonomy[k][BlindBehavior]+=1.0;
				}else{
					continuous_daylight_autonomy[k][BlindBehavior]+=1.0*(daylight_illuminances[Time_Index][k]/ill_min);
				}
				
				// DA_max (active and passive)
				if(daylight_illuminances[Time_Index][k] >= 10.0*ill_min)
					DA_max[k][BlindBehavior]+=1.0;
				
				// Useful Daylight Index
				if(daylight_illuminances[Time_Index][k] >= 2000.0)
				{
					UDI_2000[k][BlindBehavior]+=1.0;
				}else if(daylight_illuminances[Time_Index][k] <= 100.0)
				{
					UDI_100[k][BlindBehavior]+=1.0;
				}else{
					UDI_100_2000[k][BlindBehavior]+=1.0;
				}
			}
		}	
	}
	
	//nomalize daylighting metrics
	for (k=0 ; k< number_of_sensors ; k++)
	{
		daylight_autonomy[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		continuous_daylight_autonomy[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		DA_max[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		UDI_100[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		UDI_100_2000[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		UDI_2000[k][BlindBehavior]*=100.0/occupied_hours_per_year;
		
		//historic metric
		DaylightSaturationPercentage[k][BlindBehavior]=0;
		
			
	}
}

void writeRGB_DA_Files()
{
	FILE *FILE_RGB_ACTIVE_DA_AVAILABILITY= NULL;
	FILE *FILE_RGB_PASSIVE_DA_AVAILABILITY= NULL;
	FILE *FILE_RGB_ACTIVE_DA= NULL;
	FILE *FILE_RGB_PASSIVE_DA= NULL;
	FILE *FILE_RGB_DF= NULL;
	FILE *FILE_RGB_ACTIVE_con_DA= NULL;
	FILE *FILE_RGB_PASSIVE_con_DA= NULL;
	FILE *FILE_RGB_ACTIVE_DA_max= NULL;
	FILE *FILE_RGB_PASSIVE_DA_max= NULL;
	FILE *FILE_RGB_ACTIVE_UDI_100= NULL;
	FILE *FILE_RGB_PASSIVE_UDI_100= NULL;
	FILE *FILE_RGB_ACTIVE_UDI_100_2000= NULL;
	FILE *FILE_RGB_PASSIVE_UDI_100_2000= NULL;
	FILE *FILE_RGB_ACTIVE_UDI_2000= NULL;
	FILE *FILE_RGB_PASSIVE_UDI_2000= NULL;
	FILE *FILE_RGB_ACTIVE_DSP= NULL;
	FILE *FILE_RGB_PASSIVE_DSP= NULL;

	int k,i;
	FILE* PTS_FILE= NULL;
	float **sensor_points;

	//===========================
	//open files and write header
	//===========================
	if( strcmp(da_availability_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_DA_AVAILABILITY=open_output(da_availability_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_DA_AVAILABILITY,"# Daylight Availability (%.0f lux)\n# sensor file info\n",ill_min);
	}

	if( strcmp(da_availability_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_DA_AVAILABILITY=open_output(da_availability_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_DA_AVAILABILITY,"# Daylight Availability (%.0f lux) - Passive User\n# sensor file info\n",ill_min);
	}

	if( strcmp(da_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_DA=open_output(da_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_DA,"# Daylight Autonomy (%.0f lux) \n# sensor file info \n",ill_min);
	}

	if( strcmp(da_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_DA=open_output(da_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_DA,"# Daylight Autonomy (%.0f lux) - Passive User\n# sensor file info\n",ill_min);
	}

	if( strcmp(df_RGB_file,"") )
	{
		FILE_RGB_DF=open_output(df_RGB_file);
		fprintf(FILE_RGB_DF,"# Daysim Factor\n");
		fprintf(FILE_RGB_DF,"# sensor_file_info\n");
	}

	if( strcmp(con_da_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_con_DA=open_output(con_da_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_con_DA,"# Continuous Daylight Autonomy (%.0f lux)\n# sensor_file_info\n",ill_min);
	}

	if( strcmp(con_da_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_con_DA=open_output(con_da_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_con_DA,"# Continuous Daylight Autonomy (%.0f lux) - Passive User\n# sensor_file_info\n",ill_min);
	}

	if( strcmp(DA_max_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_DA_max=open_output(DA_max_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_DA_max,"# DA_max (%.0f lux)\n# sensor_file_info\n",ill_min);
	}

	if( strcmp(DA_max_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_DA_max=open_output(DA_max_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_DA_max,"# DA_max (%.0f lux) -Passive User\n# sensor_file_info\n",ill_min);
	}

	if( strcmp(UDI_100_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_UDI_100=open_output(UDI_100_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_UDI_100,"# UDI_<100\n# sensor_file_info\n");
	}

	if( strcmp(UDI_100_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_UDI_100=open_output(UDI_100_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_UDI_100,"# UDI_<100 - Passive User\n# sensor_file_info\n");
	}

	if( strcmp(UDI_100_2000_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_UDI_100_2000=open_output(UDI_100_2000_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_UDI_100_2000,"# UDI_100-2000\n# sensor_file_info\n");
	}

	if( strcmp(UDI_100_2000_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_UDI_100_2000=open_output(UDI_100_2000_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_UDI_100_2000,"# UDI_100-2000 - Passive User\n# sensor_file_info\n");
	}

	if( strcmp(UDI_2000_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_UDI_2000=open_output(UDI_2000_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_UDI_2000,"# UDI_>2000\n# sensor_file_info\n");
	}

	if( strcmp(UDI_2000_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_UDI_2000=open_output(UDI_2000_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_UDI_2000,"# Daysim output file: UDI_>2000 - Passive User\n# sensor_file_info\n");
	}


	if( strcmp(DSP_active_RGB_file,"") )
	{
		FILE_RGB_ACTIVE_DSP=open_output(DSP_active_RGB_file);
		fprintf(FILE_RGB_ACTIVE_DSP,"# Daylight Saturation Potential\n# sensor_file_info\n");
	}

	if( strcmp(DSP_passive_RGB_file,"") )
	{
		FILE_RGB_PASSIVE_DSP=open_output(DSP_passive_RGB_file);
		fprintf(FILE_RGB_PASSIVE_DSP,"# Daylight Saturation Potential - Passive User\n# sensor_file_info\n");
	}


	//read in sensor coordiantes
	sensor_points=(float**) malloc (sizeof(float*)*number_of_sensors);
	if (sensor_points == NULL) goto memerr;
	for (i=0 ; i<(number_of_sensors) ; i++){
		sensor_points[i]=(float*)malloc (sizeof(float)*6);
		if (sensor_points[i] == NULL) goto memerr;
		for (k=0 ; k<6 ; k++){
			sensor_points[i][k]=0.0;
		}
	}
	PTS_FILE=open_input(sensor_file);
   	for (k=0 ; k<(number_of_sensors) ; k++){
		fscanf(PTS_FILE,"%f %f %f %f %f %f",&sensor_points[k][0],&sensor_points[k][1],&sensor_points[k][2],&sensor_points[k][3],&sensor_points[k][4],&sensor_points[k][5]);
	}
	close_file(PTS_FILE);



	//write out x,y, z coordiante with the DA and DF values
	for (k=0 ; k< number_of_sensors ; k++)
	{
		if( strcmp(da_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_DA,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],daylight_autonomy[k][0]);

		if( strcmp(da_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_DA,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],daylight_autonomy[k][1]);

		if( strcmp(df_RGB_file,"") )
			fprintf(FILE_RGB_DF,"%f\t%f\t%f\t%f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],daylight_factor[k]);

		if( strcmp(con_da_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_con_DA,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],continuous_daylight_autonomy[k][0]);

	 	if( strcmp(con_da_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_con_DA,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],continuous_daylight_autonomy[k][1]);

		if( strcmp(DA_max_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_DA_max,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],DA_max[k][0]);

		if( strcmp(DA_max_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_DA_max,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],DA_max[k][1]);

		if( strcmp(UDI_100_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_UDI_100,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_100[k][0]);

		if( strcmp(UDI_100_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_UDI_100,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_100[k][1]);

		if( strcmp(UDI_100_2000_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_UDI_100_2000,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_100_2000[k][0]);

		if( strcmp(UDI_100_2000_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_UDI_100_2000,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_100_2000[k][1]);

		if( strcmp(UDI_2000_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_UDI_2000,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_2000[k][0]);

		if( strcmp(UDI_2000_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_UDI_2000,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],UDI_2000[k][1]);

		if( strcmp(DSP_active_RGB_file,"") )
			fprintf(FILE_RGB_ACTIVE_DSP,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],DaylightSaturationPercentage[k][0]);

		if( strcmp(DSP_passive_RGB_file,"") )
			fprintf(FILE_RGB_PASSIVE_DSP,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],DaylightSaturationPercentage[k][1]);

		if( strcmp(da_availability_active_RGB_file,"") )
		{
			if(DA_max[k][0]>=5.0)
			{
				fprintf(FILE_RGB_ACTIVE_DA_AVAILABILITY,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],-1.0*DA_max[k][0]);
			}else{
				fprintf(FILE_RGB_ACTIVE_DA_AVAILABILITY,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],daylight_autonomy[k][0]);
			}
		}
		if( strcmp(da_availability_passive_RGB_file,"") )
		{
			if(DA_max[k][1]>=5.0)
			{
				fprintf(FILE_RGB_PASSIVE_DA_AVAILABILITY,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],-1.0*DA_max[k][1]);
			}else{
				fprintf(FILE_RGB_PASSIVE_DA_AVAILABILITY,"%f\t%f\t%f\t%.0f\n",sensor_points[k][0],sensor_points[k][1],sensor_points[k][2],daylight_autonomy[k][1]);
			}
		}
	}
	for (i = 0; i<(number_of_sensors); i++)
		free(sensor_points[i]);
	free(sensor_points);

	if( strcmp(da_availability_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_DA_AVAILABILITY);

	if( strcmp(da_availability_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_DA_AVAILABILITY);

	if( strcmp(da_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_DA);

	if( strcmp(da_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_DA);

	if( strcmp(df_RGB_file,"") )
		close_file(FILE_RGB_DF);

	if( strcmp(con_da_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_con_DA);

	if( strcmp(con_da_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_con_DA);

	if( strcmp(DA_max_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_DA_max);

	if( strcmp(DA_max_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_DA_max);

	if( strcmp(UDI_100_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_UDI_100);

	if( strcmp(UDI_100_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_UDI_100);

	if( strcmp(UDI_100_2000_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_UDI_100_2000);

	if( strcmp(UDI_100_2000_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_UDI_100_2000);

	if( strcmp(UDI_2000_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_UDI_2000);

	if( strcmp(UDI_2000_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_UDI_2000);

	if( strcmp(DSP_active_RGB_file,"") )
		close_file(FILE_RGB_ACTIVE_DSP);

	if( strcmp(DSP_passive_RGB_file,"") )
		close_file(FILE_RGB_PASSIVE_DSP);

	return;
memerr:
	error(SYSTEM, "out of memory in writeRGB_DA_Files");
}
