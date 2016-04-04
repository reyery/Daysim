#pragma once
#include <stdio.h>


// This file is a patch with all the extern declarations that didn't exist in the original codebase
// but are now needed for VS2013 to compile. (I don't know why it worked originally.)

extern int ActiveOccupant;
extern int AnnualHoursWithGlare;
extern float **AnnualLightExposure;
extern float AnnualNumberOfActivatedElectricLighting[11];
extern float AnnualNumberOfOccupiedHours;
extern float **continuous_daylight_autonomy;
extern float *da;
extern float **DA_max;
extern int *day_1;
extern float **daylight_autonomy;
extern float **daylight_illuminances;
extern float **DaylightSaturationPercentage;
extern FILE *EL_LIGHTING_FILE;
extern float **electric_lighting_profile;
extern float *ext_sensor_ill;
extern float *hour_1;
extern float HoursWithAView[11];
extern char input_file[200];
extern float *int_sensor_ill;
extern float LightingGroupEnergyUse[11][13];
extern float ***MaximumBlindGroupIlluminance_Internal;
extern float ***MaximumBlindGroupIlluminance_External;
extern float ***MaximumIlluminanancePerBlindGroup;
extern float **minimum_work_plane_ill;
extern int *month_1;
extern int PassiveOccupant;
extern float ****raw_illuminances;
extern float **UDI_100;
extern float **UDI_100_2000;
extern float **UDI_2000;
extern float ***work_plane_ill;