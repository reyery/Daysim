#ifndef READ_IN_HEADER_H
#define READ_IN_HEADER_H

#include "parse.h"

#include "fvect.h"

#include <stdio.h>


/* number and luminance of sky segments */
extern int    numberOfSkySegments;
extern double luminanceOfSkySegments;

/* rotation axis, number of rotations, order and angle(in degrees) */
enum RotationAxis { XAxis= 1, YAxis, ZAxis };
extern int  rotationNumber;
extern enum RotationAxis rotationAxis[3];
extern double rotationAngle[3];

/* sensor point analysis */
extern int LightingGroupSensorSpecified[11]; /* switch that checks whether the user specified at least one sensor for each lighting group */
extern int BlindGroupSensorSpecified[11]; /* switch that checks whether the user specified at least one sensor for each blind group */
extern int ExternalBlindGroupSensorSpecified[11];
extern int IllumianceSensorExists; /* switch that checks whether the user specified any illuminance sensor in the sensor point file */


/*===============
 *project details
 *===============*/
extern char project_directory[1024]; /* directory in which the project header file is located */
/* this variable is automatically updated by the daysim GUI */
extern char project_name[1024];		/* name of project header file */
extern char tmp_directory[1024];	/* directory in which temporary files are stored */
extern char MaterialDatabaseDirectory[1024];	/* directory in which Radiance material database entries are stored */
extern char TemplateFileName[1024]; 	// name of template file

/*================
 *site information
 *================*/
extern int  month, day;
extern float hour;
extern float site_elevation;       /*  in metres */
extern int time_step;   		   /* time_step in minutes */
extern int first_weekday; 			/* 1= Monday... 7= Sunday */
extern int daylight_savings_time;    /* switched daylight savings time on/off (0/1) */
extern float  s_latitude;  		/* in RADIANS North is positive */
extern float  s_longitude; 		/* in RADIANS West is positive */
extern float  s_meridian;			/* meridian of pertaining time zone */
extern float  gprefl; 				/* ground reflectance */
extern float dir_threshold;			/* threshold for direct irradiances */
extern float dif_threshold; 			/* threshold for diffuse irradiances */
extern char place[200];				/* single string of building location */
extern float rotation_angle; 		/* scene rotation angle */
extern int OutputUnits;				//output units of annual illumiance file

//==============
// building info
//==============
extern char geometry_file[1024];  	// name of RADIANCE geometry building file
extern char material_file[1024];  	// name of RADIANCE material file
extern char sensor_file[1024];		// name of RADIANCE sensor file
extern char DaylightGlareProbability_ViewPoints_file[1024];		// name of RADIANCE View Point FIle for Annuasl DGP calculation
extern char DDS_sensor_file[1024];	// name of DDS sensor file ( contains additional info such as sensor tpe, zone etc.)
extern char DDS_file[1024];			// name of DDS file
extern int number_of_sensors;
extern int NumberOfBlindGroups;
extern int number_of_view_points;
extern int ShadingSpecified;			// switch that tests whether "shading" is in header file
extern int number_of_radiance_source_files; 	// number of radiance source files
extern int input_units;				//	Switch for the units in the geometry file
extern int output_units;			//	Switch for the output units for length
extern int display_units;			//	Switch for the display units of illuminance
extern float grid_array[9];			//	Array holding the arguments for creating a calculation grid  (number of layers, x-space, y-space, x-offset, y-offset, z-offset, xd, yd, zd)
extern char grid_layers[10][1024];		//	Array holding the names of the layers that should be used for the calculation grid.
/*extern const int max_size;*/
extern char (*radiance_source_files)[1024]; // names of radiance source files
extern int dds_file_format ; //switch to change calculation for old for generalized dds file format
extern int TotalNumberOfDCFiles; // intenger that calcualtes the total number of DC files that have ot be calculated for the advanced blind model.
extern char file_name_extension[1024];


//==================
// DAYSIM simulation
//==================
// calculation mode for gen_directsunlight
extern int calculation_mode_dir; /*  standard=0 and fast=1*/
extern float dir_azi_low;
extern float dir_azi_high;
extern float dir_alt_low;
extern float dir_alt_high;
extern float ***dc_ab0;
extern int SkyConditionCounter;
extern int PNGScheduleExists;
extern int MaxNumberOfSettingsInBlindgroup;

// ds_illum: direct coefficients coupling mode
extern int dc_coupling_mode;
// 0= interpolated, 1=nearest neighbor, 2=shadow testing, 3=no direct contribution
// Reinhart C F, Walkenhorst O, Dynamic RADIANCE-based Daylight Simulations for a
//full-scale Test Office with outer Venetian Blinds, Energy & Buildings, Vol. 33 pp. 683-697, 2001
extern long idum; // stochastic realisation parameter
extern int *shadow_testing_results;
extern int AdvancedBlindModel;
extern float ***effective_dgp;				//array that stores the effective DGP for multiple view points in DGP file

//================
// lighting system
//================
extern int OccSenDelayTime[10];  				// inertia time of occupancy sensor in minutes
extern int NumberOfLightingGroups;
extern char lighting_scenario_name[10][1024]; 	// lighting scenario name
extern int LightingSystemType[10];		  		// lighting system type
extern float LightPower[10];
extern float StandbyPower[10];
extern float MinIllLevel[10];
extern float MinDimLevel[10];
extern float SubZoneArea[10];					// percentage of sub-zone area to total lighting zone area
extern int NumberPhotoCellSensor[10];			// number of sensor in sensor file that corresponds to the
     											// photocell positon for the subzone
extern float RatioPhotoCell2WorkPlane[10];		// ratio of illuminance at the photocell to the work plane
												// illuminance

//======
// zones
//======
extern float ZoneSize[10];              // size of zone
extern int NumberLightingZones;				// number of lighting zones controlled by a single switch

//================
//	Luminaire data
//================
extern int NumSched;							// Number of luminaires in the luminaire schedule
extern char LumLabel[20][1024];					// Labels for the luminaires in the luminaire schedule
extern char IESFile[20][1024];					// File names for the IES files to associate to luminaires
extern char LumBallastDriverLabel[20][1024];	// Labels for the ballasts to associate to the luminaire
extern float LLF[20];							// Light loss factors (not including ballast factor) to associate to the luminaire
extern float LampLumens[20];					// Lamp Lumens to associate to the luminaire

extern int NumLayout;							// Number of individual luminaires in the simulation
extern char LayoutLumLabel[200][1024];			// Luminaire Label for each of the luminaires in the simulation
extern char LayoutZoneLabel[200][1024];			// Zone Label for each of the luminaires in the simulation
extern float LayoutLocOrient[200][6];			// X Y Z rot tilt spin for each of the luminaires in the simulation

extern int NumBD;								// Number of Ballasts/Drivers in the ballast-driver schedule
extern char BallastDriverLabel[20][1024];		// Label for a particulart ballast in the ballast-driver schedule
extern int BDtype[20];							// Type of ballast for determining what information comes next
extern float BDTypeArguments[20][10];			// Arguments for each of the ballasts.  Arguments change depending on type.


//=============
// Control data
//=============
extern int NumSensors;							// Number of photosensors defined in the simulation
extern char PSLabel[20][1024];					// Labels for the defined photosensors
extern int PhotosensorType[20];					// Type of photosensor for determining what information is in the file
extern char PhotosensorFile[20][1024];			// File names for the Photosensor label

extern int NumControlZones;						// Number of control zones
extern char zone_label[20][1024];				// Labels for the control zones
extern char ZonePSLabel[20][1024];				// Label of the photosensor associated with the zone
extern float PSLocDir[20][7];					// X Y Z Xd Yd Zd Spin for the photosensor
extern char PSAlgorithm[20][1024];				// Selected algorithm for the photosensor associated to a zone
extern char CriticalPointMethod[20][1024];		// Method for controlling the critical point
extern char CPArguments[20][10][1024];			// Arguments for the critical point depending on the method chosen
extern int CriticalPoints[20][10];			//	Array to hold the critical points from the sensor_file_info line
extern int NumCPsensor[20];				//	Vector holding the number of critical points for each control zone from the sensor_file_info line
extern int numCP[20];							//	Number of CP points to allow for
extern float TPval[20];						//	Target percentage for the CP analysis
extern float PSAlgorithmArguments[20][10];		// Arguments for the Photosensor Algorithm depending on which was selected
extern char ControlMethod[20][1024];			// on, off, dimmed_to_min, dimmed_to_off, switched  (this is used for optimal control)

//=============
// blind system
//=============
extern int number_of_blind_controls;		// # of investigated blind control systems
extern int BlindSystemType[10];			// blind system type
extern float BlindGroupSetpoints[11][11][2];	//Setpoint at which blinds are automatically lowered.
extern int BlindGroupCoolingPeriodJulianDay[11][6];	//Start and end julian day of the cooling periode for the blind group
extern float BlindGroupAzimuthRange[11][2];	//Azimuth range for glare
extern float BlindGroupAltitudeRange[11][2];	//Altitude range for glare
extern float BlindGroupThresholdForGlareSensor[11];	

extern char blind_control_name[10][1024];	// blind system description
extern char BlindGroupSetpointsFilename[10][1024];
extern int static_shading_scenario[10];	// if mode==2: this position corredponds to the static blind position in "shading"
extern int simple_blinds_model;			// switch ds_illum: simple blinds model on(1) or off (0)
extern int NumBlindSettings; 			// # of blind settings considered (see keyword "shading")

//=============
// user profile
//=============
extern int AdaptiveZoneApplies;
extern int NumUserProfiles;			// # of user profiles considered by ds_el_lighting
extern int DGP_filesDoNotExist;
extern int UserLightControl[10];		// type of user behavior: considers daylight or not
extern int UserBlindControl[10];		// user changes blinds daily or not
extern float user_type_frequency[10];	// percentage of user type compared to all user types
extern char user_type_name[10][1024];	// short description of user typ: type1,...

extern float ill_min;								// minimum illuminance for daylight autonomy file
extern float DirectIrradianceGlareThreshold;		// threshold level above which direct glare is assumed
													// the default vlaue is 50Wm-2 accoirdng to
													// Reinhart C.F., Voss K.,Monitoring Manual Control of Electric
	       											// Lighting and Blinds, Lighting Research & Technology, 35:2,
	       											// pp. 16, in press 2003.

//=================
// occupancy module
//=================
extern int occupancy_mode;           /* determines how user occupancy is simulated */
extern int weekday_occ_mea;          /* 1st weekday in measured occupancy input file (occupancy_mode=3) 1= Monday */
extern char occ_output_file[1024];  /* name of occupancy output file */
extern char routine_file[1024];     /* name of occupancy routine file (occupancy_mode=2 or 3)*/
extern char measured_occ[1024];     /* name of measured occupancies file (occupancy_mode=3) */
extern char measured_occ_hea[1024]; /* name of measured occupancies header file (occupancy_mode=3) */
extern char static_occupancy_profile[1024]; //static occupancy pattern
extern float occ_routine[288][9];      /* data from occupancy routine file (occupancy_mode=2 or 3)*/
extern float occ_cummulated[288][9];
extern float  start_work;						// variables for stochastic occupancy model
extern float  end_work;
extern float lunch_time,lunch_time_length;
extern float morning_start, morning_length;
extern float lunch_start, lunch_length;
extern float afternoon_start, afternoon_length;
extern float arrival,departure;

//=====================
// file names and paths
//=====================
extern char wea_data_file[1024];			// file name for imported wea data input file
extern int wea_data_file_units;
extern char wea_data_short_file[1024];	// file name for wea data project file
extern int wea_data_short_file_units;
extern char DGP_Profiles_file[100][1024];
extern char direct_radiance_file[1024];
extern char direct_radiance_file_2305[1024];
extern char direct_sunlight_file[1024];  // tmp file for direct daylight coeficients
extern char percentage_of_visible_sky_file[1024];
extern char direct_glare_file[1024];
extern char bin_dir[1024];				// file name for DAYSIm nnary directory
extern char da_file[1024];				// daylight autonomy output file
extern char da_active_RGB_file[1024];	// daylight autonomy (active) output fileoutput file for Ecotect
extern char da_passive_RGB_file[1024];	// daylight autonomy (passive) output fileoutput file for Ecotect
extern char da_availability_active_RGB_file[1024];	// daylight availability (active) output fileoutput file for Ecotect
extern char da_availability_passive_RGB_file[1024];	// daylight availability (passive) output fileoutput file for Ecotect


extern char df_RGB_file[1024];			// daylight factor output fileoutput file for Ecotect

extern char con_da_active_RGB_file[1024];	// con daylight autonomy (active) output fileoutput file for Ecotect
extern char con_da_passive_RGB_file[1024];	// con daylight autonomy (passive) output fileoutput file for Ecotect
extern char DA_max_active_RGB_file[1024];			// DA max output fileoutput file for Ecotect
extern char DA_max_passive_RGB_file[1024];			// DA max output fileoutput file for Ecotect

extern char UDI_100_active_RGB_file[1024];					// UDI (active) output fileoutput file for Ecotect
extern char UDI_100_2000_active_RGB_file[1024];				// daylight autonomy (passive) output fileoutput file for Ecotect
extern char UDI_2000_active_RGB_file[1024];					// daylight factor output fileoutput file for Ecotect
extern char UDI_100_passive_RGB_file[1024];					// UDI (active) output fileoutput file for Ecotect
extern char UDI_100_2000_passive_RGB_file[1024];				// daylight autonomy (passive) output fileoutput file for Ecotect
extern char UDI_2000_passive_RGB_file[1024];					// daylight factor output fileoutput file for Ecotect
extern char DSP_active_RGB_file[1024];						// daylight saturation percentage (active) output fileoutput file for Ecotect
extern char DSP_passive_RGB_file[1024];						// daylight saturation percentage (passive) output file for Ecotect
extern char rshow_da_file[1024];				// daylight autonomy output file


extern char df_file[1024];				// daylight factor output file
extern char rshow_df_file[1024];
extern char el_file[1024];				// electric lighting output file long
extern char thermal_sim_file_active[1024];			// output file for thermal simulation program
extern char thermal_sim_file_passive[1024];
extern char letter[1];


//========
// Shading
//========
extern char shading_illuminance_file[100][1024];	//blind setting ill file
extern char shading_variant_name[100][1024];		//blind setting description
extern char shading_rad_file[100][1024];			//blind setting rad file
extern char shading_dc_file[100][1024];			//blind setting dc file

// New two-dimentional arrays. This data structure for shading allows to model multiple blind groups (up to 10) and
// multiple blind settings (up to ten) using the concept of differential daylight coefficients.
extern char BlindGroupName[11][1024];					// names of blind groups
extern char BlindGroupGeometryInRadiance[11][11][1024];	// Radiance files for different blind settings
extern char BlindGroupDiffDC_File[11][11][1024];		// differential daylight coefficients for individual blind settings and groups
extern char BlindGroupIlluminanceProfiles[11][11][1024];	// annual illuminance profiles
extern int  NumberOfSettingsInBlindgroup[11];
extern char DaylightGlareProbabilityProfiles[11][11][200];	// annual DGP profiles
extern char AnnualShadingProfile[11][1024]; //present annual shading profile

extern int NumGlazingMaterials[10];				//	Number of glazing materials in each window group
extern char GlazingMaterials[10][10][1024];		//	Name of the glazing materials for each window group (group, setting)
extern int NumBSDFMaterials[10][11];				//	Number of BSDF materials (group, setting)  Setting postion 0 is for the base case
extern char BSDFMaterials[10][11][10][1024];		//	Name of the BSDF file (group, setting, Position of BSDF in list) Setting position 0 is for the base case
extern char BSDFExchangeMaterials[10][11][10][1024];	//	Name of the Material that gets exchanged for the BSDF file (group, setting, Position of BSDF in list) Setting position 0 is for the base case
extern int sDA_setting[10];						//	Setting number indicating which of the blind settings is the fully closed case for use with sDA
extern int MaterialCase[10];						//	Number indicating whether the shades are of the simplified version or the advanced version (1 simple, 2 advanced)
extern char ShadeControlMethod[10][1024];			//	Shade control method determines whether the method is signal, angle, or both
extern char ShadeControlPhotosensorLabel[10][1024];	//	Name of the photosensor to associate to the zone
extern float ShadeControlAzimuth[10];				//	Azimuth angle for use with the angle shade control
extern float ShadeControlSignalArguments[10][10];	//	Signal thresholds to control the blinds (group, setting)
extern float ShadeControlAngleArguments[10][10];	//	Angle thresholds to control the blinds (group, setting)
extern float ShadePSLocDir[10][7];					//	Photosensor placement array for the shades
//========
// DGP
//========
extern char viewpoint_file[1024];		// name of viewpoint file for the DGP-calculation
extern char Effective_DGP_file[1024];
extern char Effective_DGP_file_html[1024];

extern int number_of_views;
extern char dgp_out_file[1024];


extern FILE *RTRACE_RESULTS[100];
extern FILE *ACOS_ALPHA;

//shared variables
extern int *occ_profile;
extern float *dgp_profile;

extern int *sensor_typ;
extern int	  **BlindGroup;
extern int	  **LightingGroup;
extern int *sensor_unit;
extern float **sensor_coordinates;
extern float schedule_start[10];
extern float schedule_end[10];
extern char  **sensor_file_info;

//========
// DGP
//========
extern char viewpoint_file[1024];	// name of viewpoint file for the DGP-calculation
extern int number_of_views;
extern char dgp_out_file[1024];		// name of output file for the DGP-calculation
extern char dgp_check_file[1024];		// name of output check-image of the DGP-calculation
extern int checkfile;
extern int dgp_image_x_size;
extern int dgp_image_y_size;
extern int dgp_profile_cut;

extern float abPARAM;								//	Variable holding the ab parameter for rtrace
extern float adPARAM;								//	Variable holding the ad parameter for rtrace
extern float asPARAM;								//	Variable holding the as parameter for rtrace
extern float arPARAM;								//	Variable holding the ar parameter for rtrace
extern float aaPARAM;								//	Variable holding the aa parameter for rtrace
extern float lrPARAM;								//	Variable holding the lr parameter for rtrace
extern float stPARAM;								//	Variable holding the st parameter for rtrace
extern float sjPARAM;								//	Variable holding the sj parameter for rtrace
extern float lwPARAM;								//	Variable holding the lw parameter for rtrace
extern float djPARAM;								//	Variable holding the dj parameter for rtrace
extern float dsPARAM;								//	Variable holding the ds parameter for rtrace
extern float drPARAM;								//	Variable holding the dr parameter for rtrace
extern float dpPARAM;								//	Variable holding the dp parameter for rtrace

/* function declarations */
void read_in_header( char* header_file );



#endif
