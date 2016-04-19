/*  Copyright (c) 2003
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */


#include <stdio.h>
#include <stdlib.h>
#include <rtmath.h>
#include <string.h>
#include <errno.h>

#include "fropen.h"
#include "read_in_header.h"
#include "daylightfactor.h"
#include "ds_el_lighting.h"
#include "ds_constants.h"


/* this function write out the DAYSIM simulation results including simulation assumptions in
   html format.
*/
void simulation_assumptions()
{
	FILE* PTS_FILE;
	FILE* TEMPLATE_FILE=NULL;
	float **sensor_points;
	double DF_ratio = 0.0;
	int i,j,k;
	int sensors_need_to_be_specified = 0;
	char WarningString[100000]="";
	char TipsString[100000]="";
	double DA_mean_active = 0.0;
	double DA_mean_passive = 0.0;
	double DF_mean = 0.0;
	double DA_50_active = 0.0;
	double DA_50_passive = 0.0;
	double DA_con_mean_active = 0.0;
	double DA_con_mean_passive = 0.0;
	double DA_MAX_active = 0.0;
	double DA_MAX_passive = 0.0;
	double UDI_100_2000_larger50_active = 0.0;
	double UDI_100_2000_larger50_passive = 0.0;
	char DynamicShadingSystem[3000]="";

	char keyword[3000]="";
	char MainTextString[300000]="";
	int sensor_passed_LEED_criteria=0;
	int sensor_failed_DA_max_criteria_active=0;
	int sensor_failed_DA_max_criteria_passive=0;
	int number_of_illuminance_sensors=0;

	//open html template file
	if(strcmp(TemplateFileName,""))
	{
		TEMPLATE_FILE=open_input(TemplateFileName);
	}


	//=============
	// Preparation
	//=============
	//read in sensor coordiantes
	sensor_points=(float**) malloc (sizeof(float*)*number_of_sensors);
	for (i=0 ; i<(number_of_sensors) ; i++){
		sensor_points[i]=(float*)malloc (sizeof(float)*6);
	}
   	for (k=0 ; k<(number_of_sensors) ; k++){
		for (i=0 ; i<6 ; i++){
			sensor_points[k][i]=0.0;
		}
	}
	PTS_FILE=open_input(sensor_file);
   	for (k=0 ; k<(number_of_sensors) ; k++){
		fscanf(PTS_FILE,"%f %f %f %f %f %f",&sensor_points[k][0],&sensor_points[k][1],&sensor_points[k][2],&sensor_points[k][3],&sensor_points[k][4],&sensor_points[k][5]);
	}
	close_file(PTS_FILE);

	//calculate daylight factors
	getDaylightFactor( );


	for (i=0 ; i<( number_of_sensors) ; i++)
	{
		if(sensor_unit[i]==0) //illuminance sensor
		{	
			number_of_illuminance_sensors++;
			
			//LEED criteria
			if(daylight_factor[i]>=2.0)
				sensor_passed_LEED_criteria++;
			
			//DF mean
			DF_mean+=daylight_factor[i];
			
			//DA mean
			DA_mean_active+=daylight_autonomy[i][0];
			DA_mean_passive+=daylight_autonomy[i][1];
			if(daylight_autonomy[i][0]>=50.0)
				DA_50_active+=1.0;
			if(daylight_autonomy[i][1]>=50.0)
				DA_50_passive+=1.0;
				
			//DA con mean
			DA_con_mean_active+=continuous_daylight_autonomy[i][0];
			DA_con_mean_passive+=continuous_daylight_autonomy[i][1];
			
			//DA_con and DA_max
			if(DA_max[i][0]>=5.0)
				sensor_failed_DA_max_criteria_active++;
			if(DA_max[i][1]>=5.0)
				sensor_failed_DA_max_criteria_passive++;
				
			//UDI >50%
			if(UDI_100_2000[i][0]>=50.0)
				UDI_100_2000_larger50_active+=1.0;
			if(UDI_100_2000[i][1]>=50.0)
				DA_50_passive+=1.0;
			
		}
	}

	if((number_of_illuminance_sensors)>0)
	{
		DF_ratio = 1.0*sensor_passed_LEED_criteria / number_of_illuminance_sensors;
		DF_mean = DF_mean / number_of_illuminance_sensors;
		DA_MAX_active = 100.0*sensor_failed_DA_max_criteria_active / number_of_illuminance_sensors;
		DA_MAX_passive = 100.0*sensor_failed_DA_max_criteria_passive / number_of_illuminance_sensors;
		DA_mean_active = DA_mean_active / number_of_illuminance_sensors;
		DA_mean_passive = DA_mean_passive / number_of_illuminance_sensors;
		DA_50_active = 100.0*DA_50_active / number_of_illuminance_sensors;
		DA_50_passive = 100.0*DA_50_passive / number_of_illuminance_sensors;
		DA_con_mean_active = DA_con_mean_active / number_of_illuminance_sensors;
		DA_con_mean_passive = DA_con_mean_passive / number_of_illuminance_sensors;
		UDI_100_2000_larger50_active = 100.0*UDI_100_2000_larger50_active / number_of_illuminance_sensors;
		UDI_100_2000_larger50_passive = 100.0*UDI_100_2000_larger50_passive / number_of_illuminance_sensors;
	}

//++++++++++++++++++++++
//+++++ HTML File ++++++
//++++++++++++++++++++++

	//================
	//Warning Messages
	//================
	for (j=1 ; j<= NumberOfBlindGroups ; j++)
	{
		if(BlindGroupSensorSpecified[j]==0 && !simple_blinds_model)
		{
			sprintf(WarningString,"%s ShadingGroup %d has no reference sensors.",WarningString,j);
		}
		// tests if internal or external sensors have been specified
		if((BlindSystemType[j]==5 ||BlindSystemType[j]==6 )&& ExternalBlindGroupSensorSpecified[j]==0)
		{
			sprintf(WarningString,"%s ShadingGroup %d has no external reference sensors.",WarningString,j);			
		}



	}
	for (j=1 ; j<= NumberOfLightingGroups ; j++)
	{
		if(LightingGroupSensorSpecified[j]==0)
			sprintf(WarningString,"%s Lighting Group %d has no reference sensors.",WarningString,j);
	}
	
		
	//===================
	//Daylighting Analyis
	//===================
	sprintf(MainTextString,"<h1>Daysim Simulation Report</h1><p>\n");

	sprintf(MainTextString,"%s<p>\n",MainTextString);
	sprintf(MainTextString,"%s\n",MainTextString);
	sprintf(MainTextString,"%s<table width=\"600\" border=\"0\"><tr>\n",MainTextString);
	sprintf(MainTextString,"%s <td width=\"81\"></td><td width=\"294\"><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Daylit Area (DA<sub>%.0flux</sub>[50%%])</font></div></td>\n",MainTextString,ill_min);
	sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.0f%% of floor area</h1></font></div></td></tr><tr>\n",MainTextString,DA_50_active);
	sprintf(MainTextString,"%s <td></td><td><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Mean Daylight Factor </font></div></td>\n",MainTextString);
	sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.1f%%</font></div></td> </tr>  <tr>\n",MainTextString,DF_mean);
	sprintf(MainTextString,"%s <td></td><td><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Occupancy </font></div></td>\n",MainTextString);
	sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.0f hours per year</font></div></td> </tr>  <tr>\n",MainTextString,AnnualNumberOfOccupiedHours);

	if(!DGP_filesDoNotExist)
	{
		sprintf(MainTextString,"%s <td></td><td><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Glare </font></div></td>\n",MainTextString);
		sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.1f%% of occupied hours</font></div></td> </tr>  <tr>\n",MainTextString,100.0*AnnualHoursWithGlare/AnnualNumberOfOccupiedHours);
	}
	for (j=1 ; j<=NumberOfBlindGroups ; j++)
	{
		sprintf(MainTextString,"%s <td width=\"81\"></td><td width=\"294\"><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Shading Group %d open</font></div></td>\n",MainTextString,j);
		sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.0f%% of occupied hours</font></div></td> </tr>  <tr>\n",MainTextString,HoursWithAView[j]);
	}
		
	
	sprintf(MainTextString,"%s\n",MainTextString);
	sprintf(MainTextString,"%s</tr></table>\n",MainTextString);
	sprintf(MainTextString,"%s<p></p>\n",MainTextString);
	sprintf(MainTextString,"%s\n",MainTextString);
	
	sprintf(MainTextString,"%s<font color=\"#000000\">\n",MainTextString);
	

	//sprintf(MainTextString,"%s<td></td><td><div align=\"left\"><font size=\"+1\" color=\"#000000\" >Mean Daylight Factor</font></div></td>\n",MainTextString);	
	//sprintf(MainTextString,"%s<td><div align=\"left\"><font size=\"+1\" color=\"#990000\" >%.1f%%</font></div></td>\n",MainTextString,DF_mean);
	sprintf(MainTextString,"%s\n",MainTextString);
	sprintf(MainTextString,"%s</tr></table>\n",MainTextString);
	sprintf(MainTextString,"%s<p></p>\n",MainTextString);
	sprintf(MainTextString,"%s\n",MainTextString);
	
	sprintf(MainTextString,"%s<font color=\"#000000\">\n",MainTextString);
	
  
sprintf(MainTextString,"%s<p>\n",MainTextString);
	
	// daylight factor
	sprintf(MainTextString,"%s<u>Daylight Factor (DF) Analysis:</u> %.0f%% of all illuminance sensors have a daylight factor of 2%% or higher.\n",MainTextString,100.0*DF_ratio);
	if(DF_ratio>=0.75)
	{
		sprintf(MainTextString,"%sAssuming that the sensors are evenly distributed across \'all spaces occupied for critical visual tasks\', the investigated lighting zone should qualify for the LEED-NC 2.1 daylighting credit 8.1 (see <a href=\"http://www.usgbc.org/LEED/\">www.usgbc.org/LEED/</a>).</br>\n",MainTextString);
	}else{
		sprintf(MainTextString,"%sAssuming that the sensors are evenly distributed across \'all spaces occupied for critical visual tasks\', the investigated lighting zone does <b>not</b> qualify for LEED-NC 2.1 daylighting credit 8.1.</br>\n",MainTextString);
	}
	sprintf(MainTextString,"%s<p>\n",MainTextString);

	// daylight autonomy
	sprintf(MainTextString,"%s<u>Daylight Autonomy (DA) Analysis:</u>\n",MainTextString);
	sprintf(MainTextString,"%sThe mean daylight autonomy is ",MainTextString);
	if(ActiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for active ",MainTextString,DA_mean_active);
	if(ActiveOccupant && PassiveOccupant)
		sprintf(MainTextString,"%s and ",MainTextString);
	if(PassiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for passive \n",MainTextString,DA_mean_passive);
	sprintf(MainTextString,"%soccupant behavior.\n",MainTextString);
	sprintf(MainTextString,"%sThe percentage of the space with a daylight autonomy larger than 50%% is ",MainTextString);
	if(ActiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for active ",MainTextString,DA_50_active);
	if(ActiveOccupant && PassiveOccupant)
		sprintf(MainTextString,"%s and ",MainTextString);
	if(PassiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for passive \n",MainTextString,DA_50_passive);
	sprintf(MainTextString,"%soccupant behavior.<p>\n",MainTextString);

	
	// continuous daylight autonomy
	sprintf(MainTextString,"%s<u>Continuous Daylight Autonomy (DA) Analysis:</u>\n",MainTextString);
	sprintf(MainTextString,"%sThe mean continuous daylight autonomy is ",MainTextString);
	if(ActiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for active ",MainTextString,DA_con_mean_active);
	if(ActiveOccupant && PassiveOccupant)
		sprintf(MainTextString,"%s and ",MainTextString);
	if(PassiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for passive \n",MainTextString,DA_con_mean_passive);
	sprintf(MainTextString,"%soccupant behavior.\n",MainTextString);

	sprintf(MainTextString,"%sThe percentage of sensors with a DA_MAX > 5%% is ",MainTextString);
	if(ActiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for active ",MainTextString,DA_MAX_active);
	if(ActiveOccupant && PassiveOccupant)
		sprintf(MainTextString,"%s and ",MainTextString);
	if(PassiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for passive \n",MainTextString,DA_MAX_passive);
	sprintf(MainTextString,"%soccupant behavior<p>\n",MainTextString);

	
	// Useful Daylight Index
	sprintf(MainTextString,"%s<u>Useful Daylight Illuminance (UDI):</u>\n",MainTextString);
	sprintf(MainTextString,"%sThe percentage of the space with a UDI<sub><100-2000lux</sub> larger than 50%% is ",MainTextString);
	if(ActiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for active ",MainTextString,UDI_100_2000_larger50_active);
	if(ActiveOccupant && PassiveOccupant)
		sprintf(MainTextString,"%s and ",MainTextString);
	if(PassiveOccupant)
		sprintf(MainTextString,"%s%.0f%% for passive \n",MainTextString,UDI_100_2000_larger50_passive);
	sprintf(MainTextString,"%soccupant behavior.<p>\n",MainTextString);


	//=================
	//Electric Lighting
	//=================
	sprintf(MainTextString,"%s<u>Electric Lighting Use:</u> \n",MainTextString);
	sprintf(MainTextString,"%sThe predicted annual electric lighting energy use is: <ul><font color=\"#000000\">\n",MainTextString);
	for (j=1 ; j<= NumberOfLightingGroups ; j++)
	{
		sprintf(MainTextString,"%s<li>Lighting Group %d (%s): %.1f kWh</li>\n",MainTextString,j,lighting_scenario_name[j],LightingGroupEnergyUse[j][0]);
	sprintf(MainTextString,"%s\n",MainTextString);
	}
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s<p>\n", MainTextString);
	

	sprintf(MainTextString,"%s<h2>Simulation Assumptions</h2><p>\n",MainTextString);
	//============
	// html site
	//============
	sprintf(MainTextString,"%s<u>Site Description:</u>\n",MainTextString);
	sprintf(MainTextString,"%s<ul><font color=\"#000000\">\n",MainTextString);
	sprintf(MainTextString,"%s The investigated building is located in %s ",MainTextString,place);
	if(s_latitude>0)
		sprintf(MainTextString, "%s(%2.2f N/", MainTextString, s_latitude * RTD);
	else
		sprintf(MainTextString, "%s(%2.2f S/", MainTextString, s_latitude * -RTD);

	if(s_longitude>0)
		sprintf(MainTextString, "%s %2.2f E). ", MainTextString, s_longitude * RTD);
	else
		sprintf(MainTextString, "%s %2.2f W). ", MainTextString, s_longitude * -RTD);
	
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s\n<p>\n",MainTextString);


	//=================
	// user description
	//=================
	sprintf(MainTextString,"%s\n\n<u>User Description:</u>",MainTextString);
	sprintf(MainTextString,"%s<ul><font color=\"#000000\">\n",MainTextString);
	sprintf(MainTextString,"%s The total annual hours of occupancy at the work place are %.0f.",MainTextString, AnnualNumberOfOccupiedHours);
	//sprintf(MainTextString,"%sSensors related to any shadingor light group sensors are marked in <font color=\"#0000FF\"><b>blue </b><font color=\"#000000\"> in the table below.<p>",MainTextString);

	if(PNGScheduleExists){
		if(NumberOfBlindGroups==0)
			sprintf(DynamicShadingSystem,"%s","no dynamic shading");
		else if(simple_blinds_model==1)
			sprintf(DynamicShadingSystem,"%s","conceptual dynamic shading");
		else 
			sprintf(DynamicShadingSystem,"%s","detailed dynamic shading");			
		sprintf(MainTextString,"%s<img src=\"%s.%.0flx %sOccupancy.png\" width=720 height=\"240\" alt=\"Occupancy Schedule\">\n",MainTextString,project_name,ill_min,DynamicShadingSystem);
	}
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s<p>",MainTextString);
		
		
		
		

	//=================
	// Lighting Control
	//=================
	sprintf(MainTextString,"%s\n\n<u>Lighting Control:</u>",MainTextString);
	sprintf(MainTextString,"%s<ul><font color=\"#000000\">\n",MainTextString);
	if(NumberOfLightingGroups==0)
		sprintf(MainTextString,"%s There is no electric lighting system specified for the scene.\n",MainTextString);
	
	for (j=1 ; j<= NumberOfLightingGroups ; j++)
	{
		sprintf(MainTextString,"%s<li>Lighting Group %d (%s): \n",MainTextString,j,lighting_scenario_name[j]);
		sprintf(MainTextString,"%sThe system has an installed electric lighting power of %.1fW. It is \n",MainTextString,LightPower[j]);
		if(LightingSystemType[j]==1)
			sprintf(MainTextString,"%smanually controlled with an on/off switch. " ,MainTextString);
		if(LightingSystemType[j]==2)
			sprintf(MainTextString,"%smanually controlled with an on/off switch combined with a switch off occupancy sensor with a delay time of %d minutes.\n The role of the occupancy sensor is to switch the lighting off (not on!).\n When the lighting is activated, the sensor has a standby power of %.2fW.",MainTextString,OccSenDelayTime[j],StandbyPower[j]);
		if(LightingSystemType[j]==3)
			sprintf(MainTextString,"%sautomatically controlled via an on/off occupancy sensor with a delay time of %d minutes. \nThe sensor has a permanent standby power of %.2fW.",MainTextString,OccSenDelayTime[j],StandbyPower[j]) ;
		if(LightingSystemType[j]==4)
			sprintf(MainTextString,"%smanually controlled with an on/off switch. The dimming system has an ideally commissioned photosensor-control with a ballast loss factor of %.0f percent. The photocell has a standby power of %.2fW.",MainTextString,(1.0*(MinDimLevel[j])),StandbyPower[j]) ;
		if(LightingSystemType[j]==5)
			sprintf(MainTextString,"%smanually controlled with an on/off switch combined with a switch off occupancy sensor with a delay time of %d minutes. The occupancy sensor only switches the lighting off (not on!).\n The dimming system has an ideally commissioned photocell controll with a ballast loss factor of %.2f percent. The lighting system has a total standby power of %.2fW.",MainTextString,OccSenDelayTime[j],1.0*MinDimLevel[j],StandbyPower[j]) ;
		if(LightingSystemType[j]==6)
			sprintf(MainTextString,"%s automatically controlled via an on/off occupancy sensor with a delay time of %d minutes. The dimming system has an ideally commissioned photocell control with a ballast loss factor of %.2f percent. The lighting system has a total standby power of %.2fW.",MainTextString,OccSenDelayTime[j],1.0*MinDimLevel[j],StandbyPower[j]) ;
		if(LightingSystemType[j]==10)
			sprintf(MainTextString,"%s is always activated with a timer during office hours. An ideally commissioned photocell control switches the lightng off if the minimm illumiance at the work place is larger than the minimum illumianance threshold.",MainTextString) ;
		
		sprintf(MainTextString,"%s</li>\n",MainTextString);
		
		if(PNGScheduleExists){
			if(NumberOfBlindGroups==0)
				sprintf(DynamicShadingSystem,"%s","no dynamic shading");
			else if(simple_blinds_model==1)
				sprintf(DynamicShadingSystem,"%s","conceptual dynamic shading");
			else 
				sprintf(DynamicShadingSystem,"%s","detailed dynamic shading");
				
			sprintf(MainTextString,"%s<img src=\"%s.%.0flx %sLighting Group %d - %s.png\" width=720 height=\"240\" alt=\"LG%d Annual Schedule\">\n",MainTextString,project_name,ill_min,DynamicShadingSystem,j,lighting_scenario_name[j],j);
		}

		
	}
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s<p>\n", MainTextString);


	//=================
	// ShadingControl
	//=================
	sprintf(MainTextString,"%s\n\n<u>ShadingControl:</u>",MainTextString);
	sprintf(MainTextString,"%s<ul><font color=\"#000000\">\n",MainTextString);
	if(NumberOfBlindGroups==0)
		sprintf(MainTextString,"%s There is no dynamic shading system in the scene.\n",MainTextString);
	
	
	for (j=1 ; j<= NumberOfBlindGroups ; j++)
	{
		sprintf(MainTextString,"%s<li>ShadingGroup %d: \n",MainTextString,j);
		
		
		
		sprintf(MainTextString,"%sThe system is \n",MainTextString);
		if(BlindSystemType[j]==0)
			sprintf(MainTextString,"%s always openend. " ,MainTextString);
		if(BlindSystemType[j]==1)
			sprintf(MainTextString,"%s manually controlled according to the <a href=\"http://daysim.ning.com/page/concept-lightswitch-model\">Lightswitch model</a>. " ,MainTextString);
		if(BlindSystemType[j]==2)
			sprintf(MainTextString,"%s ideally automatically controlled, i.e. the blinds are closed when direct sunlight above 50Wm</sup>-2</sup> hits a work plane sensor and open otherwise. " ,MainTextString);
		if(BlindSystemType[j]==3)
		{
			sprintf(MainTextString,"%s automatically controlled in a way that excessive interior daylighting levels are avoided. When the illuminance at any of the the control sensors rises beyond %.0f lux" ,MainTextString,BlindGroupSetpoints[j][0][1]);
			sprintf(MainTextString,"%s, the system automatically readjusts to the next lower setting. If the system is not fully opened and the illuminance at a control sensor falls below %.0f lux the system is opened to the next higher state.\n",MainTextString,BlindGroupSetpoints[j][1][0]);
		}
		if(BlindSystemType[j]==4)
		{
			sprintf(MainTextString,"%s automatically controlled in a way that excessive interior daylighting levels are avoided. When the illuminance at any of the the control sensors rises beyond %.0f lux" ,MainTextString,BlindGroupSetpoints[j][0][1]);
			sprintf(MainTextString,"%s, the system automatically readjusts to the next lower setting. If the system is not fully opened and the illuminance at a control sensor falls below %.0f lux the system is opened to the next higher state.\n",MainTextString,BlindGroupSetpoints[j][1][0]);
			sprintf(MainTextString,"%s The control systems is also equipped with an occupancy sensor and once the occupant leaves the shading system is either completely opened during the heating season or competely closed during the cooling season which goes from %2d/%2d [mm/dd] to %2d/%2d.\n",MainTextString,BlindGroupCoolingPeriodJulianDay[j][2],BlindGroupCoolingPeriodJulianDay[j][3],BlindGroupCoolingPeriodJulianDay[j][4],BlindGroupCoolingPeriodJulianDay[j][5]);
		}
if(BlindSystemType[j]==5)
		{
			sprintf(MainTextString,"%s automatically controlled in a way that excessive interior daylighting levels are avoided. When the illuminance at any of the the control sensors rises beyond %.0f lux" ,MainTextString,BlindGroupSetpoints[j][0][1]);
			sprintf(MainTextString,"%s, the system automatically readjusts to the next lower setting. If the system is not fully opened and the illuminance at a control sensor falls below %.0f lux the system is opened to the next higher state.\n",MainTextString,BlindGroupSetpoints[j][1][0]);
			sprintf(MainTextString,"%s To mitigate glare the control system also closes the shading system completely when more than %.0f lux fall on a control sensor  while the sun is at an azimuth angle between %.0f Deg and %.0f Degand an altitude angle between %.0f Deg and %.0f Deg.\n",MainTextString,BlindGroupThresholdForGlareSensor[j],BlindGroupAzimuthRange[j][0],BlindGroupAzimuthRange[j][1],BlindGroupAltitudeRange[j][0],BlindGroupAltitudeRange[j][1]);
		}
if(BlindSystemType[j]==6)
		{
			sprintf(MainTextString,"%s automatically controlled in a way that excessive interior daylighting levels are avoided. When the illuminance at any of the the control sensors rises beyond %.0f lux" ,MainTextString,BlindGroupSetpoints[j][0][1]);
			sprintf(MainTextString,"%s, the system automatically readjusts to the next lower setting. If the system is not fully opened and the illuminance at a control sensor falls below %.0f lux the system is opened to the next higher state.\n",MainTextString,BlindGroupSetpoints[j][1][0]);
			sprintf(MainTextString,"%s To mitigate glare the control system also closes the shading system completely when more than %.0f lux fall on a control sensor  while the sun is at an azimuth angle between %.0f Deg and %.0f Degand an altitude angle between %.0f Deg and %.0f Deg.\n",MainTextString,BlindGroupThresholdForGlareSensor[j],BlindGroupAzimuthRange[j][0],BlindGroupAzimuthRange[j][1],BlindGroupAltitudeRange[j][0],BlindGroupAltitudeRange[j][1]);
			sprintf(MainTextString,"%s The control systems is also equipped with an occupancy sensor and once the occupant leaves the shading system is either completely opened during the heating season or competely closed during the cooling season which goes from %2d/%2d [mm/dd] to %2d/%2d.\n",MainTextString,BlindGroupCoolingPeriodJulianDay[j][2],BlindGroupCoolingPeriodJulianDay[j][3],BlindGroupCoolingPeriodJulianDay[j][4],BlindGroupCoolingPeriodJulianDay[j][5]);
		}

//test whether there is a dynnamic shading system that requires a control sensor but none is specified
for (k = 0; k < NumUserProfiles; k++){
	if (UserBlindControl[k] == 1)
		sensors_need_to_be_specified = 1;
}

if (BlindSystemType[j] == 1 && !sensors_need_to_be_specified)
	sprintf(MainTextString, "%s The system is being closed once the daylight glare proability is above 40%% (disturbing glare). ", MainTextString);

	//if(BlindGroupSensorSpecified[j]==0 && !simple_blinds_model) UserBlindControl[behavior]
if (BlindGroupSensorSpecified[j] == 0 && !simple_blinds_model && sensors_need_to_be_specified)
		{
			sprintf(MainTextString,"%s <b> WARNING: Since Shading Group %d has no reference sensors specified, all sensors are considered to be reference sensors.</b>",MainTextString,j);
		}
	if((BlindSystemType[j]==5 ||BlindSystemType[j]==6 )&& ExternalBlindGroupSensorSpecified[j]==0)
		{
			sprintf(MainTextString,"%s<b> ERROR: ShadingGroup %d has no external reference sensors specified</b>.",MainTextString,j);			
		}

	
		sprintf(MainTextString,"%s</li>\n",MainTextString);

		if(PNGScheduleExists)
		{
			if(simple_blinds_model==1)
				sprintf(DynamicShadingSystem,"%s","conceptual dynamic shading");
			else 
				sprintf(DynamicShadingSystem,"%s","detailed dynamic shading");	
			sprintf(MainTextString,"%s<img src=\"%s.%.0flx %sShading Group %d - %s.png\" width=720 height=\"240\" alt=\"BG%d Annual Schedule\">\n",MainTextString,project_name,ill_min,DynamicShadingSystem,j,BlindGroupName[j],j);
		}

		if(PNGScheduleExists && (!DGP_filesDoNotExist))
		{
			sprintf(MainTextString,"%s<p>\n<img src=\"%s.png\" width=720 height=\"240\" alt=\"Effective DGP profile not found.\">\n",MainTextString,Effective_DGP_file_html);
		}


	}
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s</ul>\n",MainTextString);
	sprintf(MainTextString,"%s\n", MainTextString);
	
	//=================
	// Detailed Results
	//=================

//	sprintf(MainTextString,"%s<h2>Simulation Results</h2>\n",MainTextString);

//	sprintf(MainTextString,"%sThe table below shows the daylight factor and various climate-based daylighting metrics ",MainTextString);
//	sprintf(MainTextString,"%s for all sensor points individually. Definitions of these metrics can be found <a href=\"http://www.diva-for-rhino.com/documents/2006.LEUKOS_PaperOnClimateBasedMetrics.pdf\">here</a>. ",MainTextString);
//	sprintf(MainTextString,"%sTo guide the reader's eye, the following color code is used:<ul>\n",MainTextString);
//	sprintf(MainTextString,"%s<li>Coordinates of core workplane sensors are shown in <font color=\"#0000FF\"><b>blue </b><font color=\"#000000\">.</li>",MainTextString);
//	sprintf(MainTextString,"%s<li>Daylight factor levels over 2%% are shown in <font color=\"#40FF00\"><b>green</b><font color=\"#000000\">.</li>",MainTextString);
//	sprintf(MainTextString,"%s<li>Annual light exposure levels of medium and high sensitivity (CIE Categories III and IV) are shown in <font color=\"#408000\"><b>dark green </b><font color=\"#000000\"> and <font color=\"#40FF00\"><b>light green </b><font color=\"#000000\">.</li>",MainTextString);
//	sprintf(MainTextString,"%s</ul>\n",MainTextString);
//	sprintf(MainTextString,"%s<font color=\"#000000\">\n",MainTextString);
	
//	sprintf(MainTextString,"%s<div onClick=\"openClose('SimulationResultsText')\" style=\"cursor:hand; cursor:pointer\"><font color=\"#990000\">Click here to show table.<font color=\"#000000\"></div>\n",MainTextString);
//	sprintf(MainTextString,"%s<div id=\"SimulationResultsText\" class=\"texter\">\n",MainTextString);
//	sprintf(MainTextString,"%s</div>\n",MainTextString);

	//Generate Tips String
	sprintf(TipsString,"Daysim generates a schedule file, that can be linked to a thermal simulation program. To open file click the link below<a href=\"file://%s\">%s</a>",thermal_sim_file_active,thermal_sim_file_active);

		
	//==================================
	//write out Daysim Simulation Report
	//==================================
	while( fscanf( TEMPLATE_FILE, "%s[^\n]", keyword ) != EOF)
	{
		if( !strcmp(keyword,"%Warnings_Text%"))
		{
			if( !strcmp(WarningString,""))
				sprintf(WarningString,"No warnings.");
			fprintf(EL_LIGHTING_FILE,"%s\n",WarningString);
		}else if( !strcmp(keyword,"%Simulation_Tips%"))
		{
			fprintf(EL_LIGHTING_FILE,"%s\n",TipsString);
		}else if( !strcmp(keyword,"%Header_FileName%"))
		{
			fprintf(EL_LIGHTING_FILE,"<a href=\"file://%s\">%s</a>\n",input_file,input_file);
		}else if( !strcmp(keyword,"'<style"))
		{
			fprintf(EL_LIGHTING_FILE,"'<style "); 
		}else if( !strcmp(keyword,"%Main_Window_Text%"))
		{
			fprintf(EL_LIGHTING_FILE,"%s\n",MainTextString);
		}else{
			fprintf(EL_LIGHTING_FILE,"%s\n",keyword);
		}
	}
	
	//close html template file
	if(strcmp(TemplateFileName,""))
		close_file(TEMPLATE_FILE);

	
}
