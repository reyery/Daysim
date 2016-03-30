/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

/* The global variables defined in this file are used in all DAYSIM subprograms.
 * The varialbes are assigned in the read_in_header().
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rtmath.h>
#include <string.h>
#include <errno.h>

#include "fropen.h"
#include "read_in_header.h"

#define DTR (PI/180.0)

//===============
//project details
//===============
char	project_directory[1024]="";	// directory in which the project header file is located
// this variable is automatically updated by the daysim GUI
char	project_name[1024]="";		// name of project header file
char	tmp_directory[1024]=".";	// directory in which temporary files are stored
char 	zone_description[1024]="";	// string that describes the invesitgated building zone
char MaterialDatabaseDirectory[1024];	// directory in which Radiance material database entries are stored
char TemplateFileName[1024]=""; 		// name of template file

//================
//site information
//================
int		month, day;
float	hour;
float	site_elevation=0.0;		//  in metres
int		time_step=5;   			// time_step in minutes
int		first_weekday=1; 		// 1= Monday... 7= Sunday
int		daylight_savings_time=1;// switched daylight savings time on/off (0/1)
float	s_latitude=-999;  		// in RADIANS North is positive
float	s_longitude=-999; 		// in RADIANS West is positive
float	s_meridian=-999;		// meridian of pertaining time zone
float	gprefl=0.0; 			// ground reflectance

float	dir_threshold=0;		// lower threshold in Wm-2 for direct irradiances for the
								// wea_data_file . For values below this threshold the sky
								// luminance distribution is set to zero.
float	dif_threshold=0; 		// lower threshold in Wm-2 for diffuse irradiances for the
								// wea_data_file. For values below this threshold the sky
								// luminance distribution is set to zero.
char	place[200]="";			// single string of building location
float rotation_angle = 0.0; 	// scene rotation angle
int OutputUnits=2;				//output units of annual illumiance file
//==============
// building info
//==============
char geometry_file[1024] ="";  	// name of RADIANCE geometry building file
char material_file[1024] ="";  	// name of RADIANCE material file
char sensor_file[1024]="";		// name of RADIANCE sensor file
char sensor_file_rotated[1024]="";		// name of RADIANCE sensor file
char DDS_sensor_file[1024]="";	// name of DDS sensor file ( contains additional info such as sensor tpe, zone etc.)
char DDS_file[1024]="";			// name of DDS file

int number_of_sensors=0;
int number_of_view_points=0;
int ShadingSpecified=0;			// switch that tests whether "shading" is in header file
int NumberOfBlindGroups=0;
int number_of_radiance_source_files=0; 	// number of radiance source files
char (*radiance_source_files)[1024]; // names of radiance source files
float ZoneSize[10];              // size of zone
int input_units=0;				//	Switch for the geometric units in the geometry file		Added by Craig Casey [Penn State University]
int output_units;				//	Switch for the output units for length			Added by Craig Casey [PSU]
int display_units;				//	Switch for the display units of illuminance		Added by Craig Casey [PSU]
float grid_array[9];			//	Array holding the arguments for creating a calculation grid  (number of layers, x-space, y-space, x-offset, y-offset, z-offset, xd, yd, zd)    Added by Craig Casey [PSU]
char grid_layers[10][1024];		//	Array holding the names of the layers that should be used for the calculation grid.		Added by Craig Casey [PSU]
//sensor point analysis
int WorkPlaceSpecified=0; //switch that checks whether the user specified any sensor on the work plane
int MaxNumberOfSettingsInBlindgroup=0;
int PNGScheduleExists=0;

int LightingGroupSensorSpecified[11]; /* switch that checks whether the user specified at least one sensor for each lighting group */
int BlindGroupSensorSpecified[11]; /* switch that checks whether the user specified at least one sensor for each blind group */
int ExternalBlindGroupSensorSpecified[11]; /* switch that checks whether the user specified at least one sensor for each blind group */

int IllumianceSensorExists=0; //switch that checks whether the user specified any illuminance sensor in the sensor point file
int dds_file_format=0 ; //switch to change calculation from previous old for generalized dds file format
int TotalNumberOfDCFiles=0; // integer that calcualtes the total number of DC files that have to be calculated for the advanced blind model.
char file_name_extension[1024]="";

//==================
// DAYSIM simulation
//==================

// calculation mode for gen_directsunlight
float dir_azi_low=-60;
float dir_azi_high=-50;
float dir_alt_low=0;
float dir_alt_high=90;
float ***dc_ab0;
int SkyConditionCounter=0;

// ds_illum: direct coefficients coupling mode
int dc_coupling_mode=0;
// 0= interpolated, 1=nearest neighbor, 2=shadow testing, 3=no direct contribution
// Reinhart C F, Walkenhorst O, Dynamic RADIANCE-based Daylight Simulations for a
//full-scale Test Office with outer Venetian Blinds, Energy & Buildings, Vol. 33 pp. 683-697, 2001
long idum = -10; // stochastic realisation parameter
int *shadow_testing_results;


//================
// lighting system
//================
int OccSenDelayTime[10];  				// inertia time of occupancy sensor in minutes
int NumberOfLightingGroups=0;
char lighting_scenario_name[10][1024]; 	// lighting scenario name
int LightingSystemType[10];		  		// lighting system type
int NumberLightingZones;				// number of lighting zones controlled by a single switch
float SubZoneArea[10];					// percentage of sub-zone area to total lighting zone area
int NumberPhotoCellSensor[10];			// number of sensor in sensor file that corresponds to the
										// photocell positon for the subzone
float RatioPhotoCell2WorkPlane[10];		// ratio of illuminance at the photocell to the work plane
										// illuminance
float LightPower[10];
float StandbyPower[10];
float MinIllLevel[10];
float MinDimLevel[10];


//=============
// blind system
//=============
int number_of_blind_controls=0;		// # of investigated blind control systems
int BlindSystemType[10];			// blind system type
float BlindGroupSetpoints[11][11][2];	//Setpoint at which blinds are automatically lowered and retracted
int BlindGroupCoolingPeriodJulianDay[11][6];	//Start and end julian day of the cooling periode for the blind group
float BlindGroupAzimuthRange[11][2];	//Azimuth range for glare
float BlindGroupAltitudeRange[11][2];	//Altitude range for glare
float BlindGroupThresholdForGlareSensor[11];	

char blind_control_name[10][1024];	// blind system description
char BlindGroupSetpointsFilename[10][1024];	// blind system description
int static_shading_scenario[10];	// if mode==2: this position corredponds to the static blind position in "shading"
int simple_blinds_model=0;			// switch ds_illum: simple blinds model on(1) or off (0)
int NumBlindSettings=1; 			// # of blind settings considered (see keyword "shading")
int AdvancedBlindModel=0; 			//Switch to turn the new advanced blind model on or off
float ***effective_dgp;				//array that stores the effective DGP for multiple view points in DGP file

//=============
// user profile
//=============
int AdaptiveZoneApplies=1;
int NumUserProfiles=0;			// # of user profiles considered by ds_el_lighting
int DGP_filesDoNotExist=0;
int UserLightControl[10];		// type of user behavior: considers daylight or not
int UserBlindControl[10];		// user changes blinds daily or not
float user_type_frequency[10];	// percentage of user type compared to all user types
char user_type_name[10][1024];	// short description of user typ: type1,...
float DirectIrradianceGlareThreshold=50.0; 	// threshold level above which direct glare is assumed
								// the default vlaue is 50Wm-2 according to
								// Reinhart C.F., Voss K.,Monitoring Manual Control of Electric
	       						// Lighting and Blinds, Lighting Research & Technology, 35:2,
	       						// pp. 16, in press 2003.


//=================
// occupancy module
//=================
int occupancy_mode=1;           /* determines how user occupancy is simulated */
int weekday_occ_mea=0;          /* 1st weekday in measured occupancy input file (occupancy_mode=3) 1= Monday */
char occ_output_file[1024] ="";  /* name of occupancy output file */
char routine_file[1024] ="";     /* name of occupancy routine file (occupancy_mode=2 or 3)*/
char measured_occ[1024] ="";     /* name of measured occupancies file (occupancy_mode=3) */
char measured_occ_hea[1024] =""; /* name of measured occupancies header file (occupancy_mode=3) */
char static_occupancy_profile[1024]=""; //static occupancy pattern
float occ_routine[288][9];      /* data from occupancy routine file (occupancy_mode=2 or 3)*/
float occ_cummulated[288][9];
float  start_work=8;						// variables for stochastic occupancy model
float  end_work=18;
float lunch_time=12,lunch_time_length=2;
float morning_start=0, morning_length=0;
float lunch_start=0, lunch_length=0;
float afternoon_start=0, afternoon_length=0;
float arrival=0,departure=0;

//=====================
// file names and paths
//=====================
char wea_data_file[1024]="";			// file name for imported wea data input file
int wea_data_file_units=1;
char wea_data_short_file[1024]="";	// file name for wea data project file
int wea_data_short_file_units=1;
char direct_radiance_file[1024]="";
char direct_radiance_file_2305[1024]="";

char direct_sunlight_file[1024]="";  // tmp file for direct daylight coeficients
char percentage_of_visible_sky_file[1024]="";
char direct_glare_file[1024]="";

char bin_dir[1024]="";				// file name for DAYSIm nnary directory
char da_file[1024]="";				// daylight autonomy output file
char da_active_RGB_file[1024];	// daylight autonomy (active) output fileoutput file for Ecotect
char da_passive_RGB_file[1024];	// daylight autonomy (passive) output fileoutput file for Ecotect
char da_availability_active_RGB_file[1024];	// daylight availability (active) output fileoutput file for Ecotect
char da_availability_passive_RGB_file[1024];	// daylight availability (passive) output fileoutput file for Ecotect
char df_RGB_file[1024];			// daylight factor output fileoutput file for Ecotect
char con_da_active_RGB_file[1024];	// con daylight autonomy (active) output fileoutput file for Ecotect
char con_da_passive_RGB_file[1024];	// con daylight autonomy (passive) output fileoutput file for Ecotect
char DA_max_active_RGB_file[1024];			// DA max output fileoutput file for Ecotect
char DA_max_passive_RGB_file[1024];			// DA max output fileoutput file for Ecotect
char UDI_100_active_RGB_file[1024];					// UDI (active) output fileoutput file for Ecotect
char UDI_100_2000_active_RGB_file[1024];				// daylight autonomy (passive) output fileoutput file for Ecotect
char UDI_2000_active_RGB_file[1024];					// daylight factor output fileoutput file for Ecotect
char UDI_100_passive_RGB_file[1024];					// UDI (active) output fileoutput file for Ecotect
char UDI_100_2000_passive_RGB_file[1024];				// daylight autonomy (passive) output fileoutput file for Ecotect
char UDI_2000_passive_RGB_file[1024];					// daylight factor output fileoutput file for Ecotect
char DSP_active_RGB_file[1024];						// daylight saturation percentage (active) output fileoutput file for Ecotect
char DSP_passive_RGB_file[1024];

float ill_min=500;					// minimum illuminance for daylight autonomy file
char df_file[1024]="";				// daylight factor output file
char rshow_da_file[1024]="";			// rshow output file for daylight autonomy
char rshow_df_file[1024]="";			// rshow output file for daylight factor
char el_file[1024]="";				// electric lighting output file long
char el_file_short[1024]="";			// electric lighting output file short										Inherited from PennState Daysim
float sDA_settings[4];					//	Settings to use for the sDA calculation (ill, %, Start Time, End Time)	Added by Craig Casey [PSU]
float target_ill;						//	Target illuminance


//================
//	Luminaire data					Added by Craig Casey [PSU]
//================
int NumSched;							// Number of luminaires in the luminaire schedule
char LumLabel[20][1024];				// Labels for the luminaires in the luminaire schedule
char IESFile[20][1024];					// File names for the IES files to associate to luminaires
char LumBallastDriverLabel[20][1024];	// Labels for the ballasts to associate to the luminaire
float LLF[20];							// Light loss factors (not including ballast factor) to associate to the luminaire
float LampLumens[20];					// Lamp Lumens to associate to the luminaire

int NumLayout;							// Number of individual luminaires in the simulation
char LayoutLumLabel[200][1024];			// Luminaire Label for each of the luminaires in the simulation
char LayoutZoneLabel[200][1024];		// Zone Lable for each of the luminaires in the simulation
float LayoutLocOrient[200][6];			// X Y Z rot tilt spin for each of the luminaires in the simulation

int NumBD;								// Number of Ballasts/Drivers in the ballast-driver schedule
char BallastDriverLabel[20][1024];		// Label for a particulart ballast in the ballast-driver schedule
int BDtype[20];							// Type of ballast for determining what information comes next
float BDTypeArguments[20][10];			// Arguments for each of the ballasts.  Arguments change depending on type.


//=============
// Control data						Added by Craig Casey [PSU]
//=============
int NumSensors;							// Number of photosensors defined in the simulation
char PSLabel[20][1024];				// Labels for the defined photosensors
int PhotosensorType[20];				// Type of photosensor for determining what information is in the file
char PhotosensorFile[20][1024];			// File names for the Photosensor label

int NumControlZones;					// Number of control zones
char zone_label[20][1024];				// Labels for the control zones
char ZonePSLabel[20][1024];			// Label of the photosensor associated with the zone
float PSLocDir[20][7];					// X Y Z Xd Yd Zd Spin for the photosensor
char PSAlgorithm[20][1024];			// Selected algorithm for the photosensor associated to a zone
char CriticalPointMethod[20][1024];		// Method for controlling the critical point
char CPArguments[20][10][1024];		// Arguments for the critical point depending on the method chosen
int CriticalPoints[20][10];			//	Array to hold the critical points from the sensor_file_info line
int NumCPsensor[20];				//	Vector holding the number of critical points for each control zone from the sensor_file_info line
int numCP[20];						//	Number of CP points to allow for
float TPval[20];					//	Target percentage for the CP analysis
float PSAlgorithmArguments[20][10];	// Arguments for the Photosensor Algorithm depending on which was selected
char ControlMethod[20][1024];		// on, off, dimmed_to_min, dimmed_to_off, switched  (this is used for optimal control)



//========
// Shading
//========
char shading_illuminance_file[100][1024];	//blind setting ill file
char shading_variant_name[100][1024];		//blind setting description
char shading_rad_file[100][1024];			//blind setting rad file
char shading_dc_file[100][1024];			//blind setting dc file
char DGP_Profiles_file[100][1024];

// New two-dimentional arrays. This data structure for shading allows to model multiple blind groups (up to 10) and
// multiple blind settings (up to ten) using the concept of differential daylight coefficients.
char BlindGroupName[11][1024];					// names of blind groups
char BlindGroupGeometryInRadiance[11][11][1024];	// Radiance files for different blind settings
char BlindGroupDiffDC_File[11][11][1024];		// differential daylight coefficients for individual blind settings and groups
char BlindGroupIlluminanceProfiles[11][11][1024];	// annual illuminance profiles
int  NumberOfSettingsInBlindgroup[11];
char DaylightGlareProbabilityProfiles[11][11][200];	// annual DGP profiles
char AnnualShadingProfile[11][1024]; //present annual shading profile

//New advanced shading variables for sDA implemented in shading -11					Added by Craig Casey [PSU]
int NumGlazingMaterials[10];				//	Number of glazing materials in each window group
char GlazingMaterials[10][10][1024];		//	Name of the glazing materials for each window group (group, setting)
int NumBSDFMaterials[10][11];				//	Number of BSDF materials (group, setting)  Setting postion 0 is for the base case
char BSDFMaterials[10][11][10][1024];		//	Name of the BSDF file (group, setting, Position of BSDF in list) Setting position 0 is for the base case
char BSDFExchangeMaterials[10][11][10][1024];	//	Name of the Material that gets exchanged for the BSDF file (group, setting, Position of BSDF in list) Setting position 0 is for the base case
int sDA_setting[10];						//	Setting number indicating which of the blind settings is the fully closed case for use with sDA
int MaterialCase[10];						//	Number indicating whether the shades are of the simplified version or the advanced version (1 simple, 2 advanced)
char ShadeControlMethod[10][1024];			//	Shade control method determines whether the method is signal, angle, or both
char ShadeControlPhotosensorLabel[10][1024];	//	Name of the photosensor to associate to the zone
float ShadeControlAzimuth[10];				//	Azimuth angle for use with the angle shade control
float ShadeControlSignalArguments[10][10];	//	Signal thresholds to control the blinds (group, setting)
float ShadeControlAngleArguments[10][10];	//	Angle thresholds to control the blinds (group, setting)
float ShadePSLocDir[10][7];					//	Photosensor placement array for the shades

//========
// DGP
//========
char viewpoint_file[1024];		// name of viewpoint file for the DGP-calculation
char Effective_DGP_file[1024];
char Effective_DGP_file_html[1024];

int number_of_views=0;
char dgp_out_file[1024];
char dgp_check_file[1024]= "";
int checkfile=0;
int dgp_image_x_size=800;
int dgp_image_y_size=800;
int dgp_profile_cut=2;

FILE *RTRACE_RESULTS[100];
FILE *ACOS_ALPHA;


char thermal_sim_file_active[1024]="";			// output file for thermal simulation program
char thermal_sim_file_passive[1024]="";
char letter[1]="";
//shared variables
int	  *occ_profile;
float	  *dgp_profile;
int	  *sensor_typ;
int	  **BlindGroup;
int	  **LightingGroup;
char  **sensor_file_info;

int	  *sensor_unit;
float **sensor_coordinates;
float schedule_start[10];
float schedule_end[10];

// functions cut of blanks at the beginning and end of a string
const int comma = ',';


/* number and luminance of sky segments */
int    numberOfSkySegments=     148;
double luminanceOfSkySegments= 1000.0;

/* rtrace simulation options for direct and diffuse illumination calculation */
RtraceOptions	directOptions;
RtraceOptions	diffuseOptions;
//Radiance simulation parameter variables				Added by Craig Casey [PSU]
float abPARAM;								//	Variable holding the ab parameter for rtrace
float adPARAM;								//	Variable holding the ad parameter for rtrace
float asPARAM;								//	Variable holding the as parameter for rtrace
float arPARAM;								//	Variable holding the ar parameter for rtrace
float aaPARAM;								//	Variable holding the aa parameter for rtrace
float lrPARAM;								//	Variable holding the lr parameter for rtrace
float stPARAM;								//	Variable holding the st parameter for rtrace
float sjPARAM;								//	Variable holding the sj parameter for rtrace
float lwPARAM;								//	Variable holding the lw parameter for rtrace
float djPARAM;								//	Variable holding the dj parameter for rtrace
float dsPARAM;								//	Variable holding the ds parameter for rtrace
float drPARAM;								//	Variable holding the dr parameter for rtrace
float dpPARAM;								//	Variable holding the dp parameter for rtrace



/* number of rotations, order of rotation and angle(in degree) */
int rotationNumber= 0;
enum RotationAxis rotationAxis[3];
double rotationAngle[3]= { 0.0, 0.0, 0.0 };

int julian_date( int month, int day)		/* Julian date (days into year) */
{
	static short  mo_da[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	
	return(mo_da[month-1] + day);
}

/**
 * Initialise default radiance and photon map options.
 */
void initRadianceOptions( const enum Illumination illum, const enum CalculationMode mode,
						  RtraceOptions* options )
{
	options->illumination= illum;
	options->calculationMode= mode;

	/* compute irradiance */
	strcpy( options->rad.irradianceSwitch, "I+" );

	options->rad.aa= 0.2;
	options->rad.ab= 0;
	options->rad.ad= 512;
	options->rad.ar= 128;
	options->rad.as= 0;

	options->rad.dj= 0.0;
	options->rad.dp= 512;
	options->rad.dr= 2;
	options->rad.ds= 0.2;

	options->rad.lr= 6;
	options->rad.lw= 0.004;

	options->rad.sj= 1.0;
	options->rad.st= 0.15;

	if( mode == RtracePhotonMap ) {
		options->pmap.report= 0;

		options->pmap.predistribution= 0.25;
		options->pmap.photonPort[0]= '\0';

		options->pmap.globalPmap[0]= '\0';
		options->pmap.globalPhotons= 0;
		options->pmap.globalCollect= 0;

		options->pmap.causticPmap[0]= '\0';
		options->pmap.causticPhotons= 0;
		options->pmap.causticCollect= 0;
	}
}


/*
 *
 */
static int rtraceParameterGroup( char* key, FILE* fp )
{
	int ret= 1;
	RtraceOptions* opts= NULL;

	/* direct illumination options */
	if( !strcmp( key, "<direct_rtrace>" ) || !strcmp( key, "<direct-rtrace>" ) )
		opts= &directOptions;
	else if( !strcmp( key, "<diffuse_rtrace>") || !strcmp( key, "<diffuse-rtrace>" ) )
		opts= &diffuseOptions;

	if( opts ) {
		while( fscanf( fp, "%s", key ) > 0 ) {
			if( key[0] == '#' ) {
				fscanf( fp, "%*[^\n]\n" );
			} else {
				if( parse( key, fp, rtrace_parse_params ) == 0 ) {
					memcpy( opts, &rtrace_options, sizeof(RtraceOptions) );
					break;
				}
			}
		}

		if( opts == &directOptions )
			ret= !strcmp( key, "</direct_rtrace>") || !strcmp( key, "</direct-rtrace>" );
		else
			ret= !strcmp( key, "</diffuse_rtrace>") || !strcmp( key, "</diffuse-rtrace>" );
	}
	return ret;
}


/*
 *
 */
void read_in_header( char *header_file )
{
	int   i,j,k, month,day;
	float frequency_sum=0;
	int Blind_Group_index;
	int Lighting_Group_index;
	char  keyword[1000]="";
	char keyword_sensor[100]="";
	int   my_letter = 0;
	FILE  *HEADER_FILE;
	FILE  *TEST_FILE;
	int	  copy_rtrace_options= 0;
	//char* c;
	//float M_PI=3.1415926535897932;      //Added this section to make this part work with the existing read_in_header

	initRadianceOptions( DirectIllumination, RtraceClassic, &directOptions );
	initRadianceOptions( DiffuseIllumination, RtraceClassic, &diffuseOptions );
	initRadianceOptions( DiffuseIllumination, RtraceClassic, &rtrace_options );

	for (i=0 ; i<10 ; i++){
		OccSenDelayTime[i]=0;
		LightPower[i]=0;
		StandbyPower[i]=0;
		static_shading_scenario[i]=0;
		UserLightControl[i]=0;
		UserBlindControl[i]=0;
		MinIllLevel[i]=0;
		MinDimLevel[i]=0;
		NumberOfSettingsInBlindgroup[i]=0;
	}
	for (i=0;i<20;i++){
		NumCPsensor[i]=0;
	}
	
	/*======================*/
	/* first header reading */
	/*======================*/
	HEADER_FILE= open_input(header_file);

	while( fscanf( HEADER_FILE, "%s", keyword ) != EOF ) {
        //===============
		//project details
		if( !strcmp(keyword,"project_directory")){
			getWord( project_directory, HEADER_FILE, PATH_SIZE, '\n' );
			//fscanf(HEADER_FILE,"%s", project_directory);
		}
		else if( !strcmp(keyword,"project_name")){
			fscanf(HEADER_FILE,"%s", project_name);
		}
		else if( !strcmp(keyword,"tmp_directory")){
			fscanf(HEADER_FILE,"%s", tmp_directory);
		}
		else if( !strcmp(keyword,"material_directory")){
			fscanf(HEADER_FILE,"%s", MaterialDatabaseDirectory);
		}
		else if( !strcmp(keyword,"bin_directory")){
			fscanf(HEADER_FILE,"%s", bin_dir);
		}
		else if( !strcmp(keyword,"Template_File")){
			fscanf(HEADER_FILE,"%s", TemplateFileName);
		}
		
		
		//================
		//site information
		//================
		else if( !strcmp(keyword,"place")){
			fscanf(HEADER_FILE,"%s", place);
		}

		else if( !strcmp(keyword,"time_zone") ){
			fscanf(HEADER_FILE,"%f",&s_meridian);
			if( (s_meridian > 180.0 )|| (s_meridian < -180.0)) fprintf(stderr," WARNING: time_zone lies out of range (%.2f)!\n",s_meridian);
			s_meridian *= DTR;
		}
		else if( !strcmp(keyword,"time_step")){
			fscanf(HEADER_FILE,"%d",&time_step);
			if(time_step!=60 && time_step!=30 && time_step!=20 && time_step!=15 && time_step!=12 && time_step!=10 && time_step!=6 && time_step!=5 && time_step!=4 && time_step!=3 && time_step!=2 && time_step!=1){
				fprintf(stderr,"read_in_header: variable \'time_step\' out of bound (%d)\n",time_step);
				exit(1);
			}
		}
		else if( !strcmp(keyword,"longitude") ){
			fscanf(HEADER_FILE,"%f",&s_longitude);
			if( s_longitude >180 || s_longitude < -180)
				fprintf(stderr," WARNING: longitude lies out of range (%.2f)!\n",s_longitude);
			s_longitude *= DTR;
		}
		else if( !strcmp(keyword,"site_elevation") ){
			fscanf(HEADER_FILE,"%f",&site_elevation);
		}
		else if( !strcmp(keyword,"ground_reflectance") ){
			fscanf(HEADER_FILE,"%f",&gprefl);/*ground reflectance */
		}
		else if( !strcmp(keyword,"latitude")  ){
			fscanf(HEADER_FILE,"%f",&s_latitude);
			if( s_latitude >90 || s_latitude <-90)
				fprintf(stderr," WARNING: longitude lies out of range (%.2f)!\n",s_latitude);
			s_latitude *= DTR;
		}
		else if( !strcmp(keyword,"lower_direct_threshold") ){
			fscanf(HEADER_FILE,"%f",&dir_threshold);
		}
		else if( !strcmp(keyword,"lower_diffuse_threshold") ){
			fscanf(HEADER_FILE,"%f",&dif_threshold);
		}
		else if( !strcmp(keyword,"output_units") ){
			fscanf(HEADER_FILE,"%d",&OutputUnits);
		}

		//==============
		// building info
		//==============
		else if( !strcmp(keyword,"geometry_file")){
			fscanf(HEADER_FILE,"%s", geometry_file);
		}
		else if( !strcmp(keyword,"material_file")){
			fscanf(HEADER_FILE,"%s", material_file);
		}
		else if( !strcmp(keyword,"scene_rotation_angle")){
			fscanf(HEADER_FILE,"%f", &rotation_angle);
			//printf("scene rotation angle %f\n",rotation_angle);
		}
        else if( !strcmp(keyword,"radiance_source_files")){
			fscanf(HEADER_FILE,"%d",&number_of_radiance_source_files);
			//printf("Number of RADIANCE Source Files = %d\n", number_of_radiance_source_files);
            if (number_of_radiance_source_files == 0)
				return;
            radiance_source_files=
				(char (*)[PATH_SIZE])malloc(sizeof(char[PATH_SIZE])*number_of_radiance_source_files);
		    if(radiance_source_files == NULL)
				{
					printf("radiance_source_files: out of memory \n");
					exit(1);
				}

  		    while( my_letter != comma && !feof(HEADER_FILE) )
				my_letter = fgetc(HEADER_FILE);

			my_letter = 0;

			for (i=0 ; i<(number_of_radiance_source_files) && !feof(HEADER_FILE) ; i++){
				getWord(radiance_source_files[i],HEADER_FILE, PATH_SIZE, ',');
				//printf("\nfile %d %s",i+1,radiance_source_files[i]);
			}
		}
		else if ( !strcmp(keyword, "input_unit_index")){
			fscanf(HEADER_FILE, "%d", &input_units);
		}
		else if (!strcmp(keyword, "output_unit_index")){
			fscanf(HEADER_FILE, "%d", &output_units);
			}
		else if(!strcmp(keyword, "display_unit_index")){
			fscanf(HEADER_FILE, "%d", &display_units);
		}
		else if(!strcmp(keyword, "calc_grid")){
			fscanf(HEADER_FILE, "%f", &grid_array[0]);
			for (i=0;i<grid_array[0];i++){
				fscanf(HEADER_FILE, "%s", &grid_layers[i]);
			}
			for (i=0;i<8;i++){
				fscanf(HEADER_FILE, "%f", &grid_array[i+1]);
			}
		}
		else if( !strcmp(keyword,"sensor_file")){
			getWord(sensor_file,HEADER_FILE, PATH_SIZE, '\n');

		}
		else if( !strcmp(keyword,"DDS_sensor_file")){
			getWord(DDS_sensor_file,HEADER_FILE, PATH_SIZE, '\n');
			sprintf(keyword,"%s%s",project_directory,DDS_sensor_file);
			strcpy(DDS_sensor_file,keyword);
		}
		else if( !strcmp(keyword,"DDS_file")){
			getWord(DDS_file,HEADER_FILE, PATH_SIZE, '\n');
			sprintf(keyword,"%s%s",project_directory,DDS_file);
			strcpy(DDS_file,keyword);
		}
		//================
		//	Luminaire data								Added by Craig Casey [PSU]
		//================
		else if( !strcmp(keyword, "luminaire_schedule")){
			fscanf(HEADER_FILE,"%d", &NumSched);
			for (i=0 ; i<NumSched ; i++){
				fscanf(HEADER_FILE, "%s", LumLabel[i]);
				fscanf(HEADER_FILE, "%s", IESFile[i]);
				fscanf(HEADER_FILE, "%s", LumBallastDriverLabel[i]);
				fscanf(HEADER_FILE, "%f", &LLF[i]);
				fscanf(HEADER_FILE, "%f", &LampLumens[i]);
			}
		}
		else if( !strcmp(keyword, "luminaire_layout")){
			fscanf(HEADER_FILE, "%d", &NumLayout);
			for (i=0; i<NumLayout; i++){
				fscanf(HEADER_FILE, "%s", LayoutLumLabel[i]);
				fscanf(HEADER_FILE, "%s", LayoutZoneLabel[i]);
				for (j=0; j<6;j++){
					fscanf(HEADER_FILE, "%f", &LayoutLocOrient[i][j]);
				}
			}
		}
		else if( !strcmp(keyword, "ballast_driver_schedule")){
			fscanf(HEADER_FILE, "%d", &NumBD);
			if (NumBD>20 || NumBD<0){
				printf("Error in ballast_driver_schedule:  \n The number of ballasts/drivers specified is incorrect.\nThe value needs to be between 0 and 20.");
			}
			for (i=0; i<NumBD; i++){
				fscanf(HEADER_FILE, "%s", BallastDriverLabel[i]);
				fscanf(HEADER_FILE, "%d", &BDtype[i]);
				switch (BDtype[i]){
					case 1:		// Type 1= BFMax BFMin WattMax WattMin
						for (j=0;j<4;j++){
							fscanf(HEADER_FILE, "%f", &BDTypeArguments[i][j]);
						}
						break;
					default:
						printf("Error ballast_driver_schedule: \nThe BDtype for the ballast/driver labeled %s is invalid.\n",BallastDriverLabel[i]);
				}
			}
		}
		//================
		//	Control data									Added by Craig Casey [PSU]
		//================
		else if( !strcmp(keyword, "photosensor_control")){
			fscanf(HEADER_FILE, "%d", &NumSensors);
			if (NumSensors>20 || NumSensors<0){
				printf("Error in photosensors_control:  \n The number of photosensors specified is incorrect.\nThe value needs to be between 0 and 20.");
			}
			for (i=0; i<NumSensors; i++){
				fscanf(HEADER_FILE, "%s", PSLabel[i]);
				fscanf(HEADER_FILE, "%d", &PhotosensorType[i]);
				fscanf(HEADER_FILE, "%s", PhotosensorFile[i]);
			}
		}
		else if( !strcmp(keyword,"shading")){
			fscanf(HEADER_FILE,"%d",&NumBlindSettings);
			if(NumBlindSettings <-20 || NumBlindSettings >10){ // for now the number of blind gorups is capped at 10 for the most advanced shade model
				fprintf(stderr,"read_in_header: variable \'shading\' is out of range (%d)\nMaximum number of shading device settings is 10\n",NumBlindSettings);
				exit(1);
			}
			//===================
			// Simple blind model
			//===================
			// Generates a second 'blinds down' illuminance file based on the basic (no blind) case.
			if(NumBlindSettings ==0){
				NumBlindSettings =1;
				fscanf(HEADER_FILE,"%s",shading_variant_name[1] );
				sprintf(shading_variant_name[2],"%s.blinds_down",shading_variant_name[1]);
				sprintf(BlindGroupName[0],"%s",shading_variant_name[1]);
				sprintf(BlindGroupName[1],"%s",shading_variant_name[2]);

				fscanf(HEADER_FILE,"%s", shading_dc_file[1]);
				sprintf(shading_dc_file[2],"%s", shading_dc_file[1]);
				sprintf(BlindGroupDiffDC_File[0][0],"%s",shading_dc_file[1]);

				fscanf(HEADER_FILE,"%s", shading_illuminance_file[1]);
				sprintf(BlindGroupIlluminanceProfiles[0][0],"%s",shading_illuminance_file[1]);
				fscanf(HEADER_FILE,"%s", shading_illuminance_file[2]);
				sprintf(BlindGroupIlluminanceProfiles[0][1],"%s",shading_illuminance_file[2]);

				simple_blinds_model=1;
				TotalNumberOfDCFiles=2;
				NumberOfSettingsInBlindgroup[0]=1;
				NumberOfSettingsInBlindgroup[1]=1;
				NumberOfBlindGroups=1;
				//by default the simple blind model is set to manual control
				BlindSystemType[1]=1; 
				
			//================
			// Static geometry
			//================
			}else if(NumBlindSettings==1){
				fscanf(HEADER_FILE,"%s", shading_variant_name[1]);
				sprintf(BlindGroupName[0],"%s",shading_variant_name[1]);

				fscanf(HEADER_FILE,"%s", shading_dc_file[1]);
				sprintf(BlindGroupDiffDC_File[0][0],"%s",shading_dc_file[1]);

				fscanf(HEADER_FILE,"%s", shading_illuminance_file[1]);
				sprintf(BlindGroupIlluminanceProfiles[0][0],"%s",shading_illuminance_file[1]);

				simple_blinds_model=0;
				TotalNumberOfDCFiles=NumBlindSettings;
				NumberOfSettingsInBlindgroup[0]=1;
				NumberOfBlindGroups=0;

			//=========================
			// Intermediate blind model
			//=========================
			} else if(NumBlindSettings >=2){
				for (i=1 ; i<=NumBlindSettings ; i++){
					fscanf(HEADER_FILE,"%s", shading_variant_name[i]);
					sprintf(BlindGroupName[i],"%s",shading_variant_name[i]);

					fscanf(HEADER_FILE,"%s", shading_rad_file[i]);
					fscanf(HEADER_FILE,"%s", shading_dc_file[i]);
					fscanf(HEADER_FILE,"%s", shading_illuminance_file[i]);
					if(i==0)
					{
						sprintf(BlindGroupGeometryInRadiance[0][0],"%s",shading_rad_file[i]);
						sprintf(BlindGroupDiffDC_File[0][0],"%s",shading_dc_file[i]);
						sprintf(BlindGroupIlluminanceProfiles[0][0],"%s",shading_illuminance_file[i]);
					}else{
						sprintf(BlindGroupGeometryInRadiance[i-1][1],"%s",shading_rad_file[i]);
						sprintf(BlindGroupDiffDC_File[i-1][1],"%s",shading_dc_file[i]);
						sprintf(BlindGroupIlluminanceProfiles[i-1][1],"%s",shading_illuminance_file[i]);
					}

				}
				TotalNumberOfDCFiles=NumBlindSettings;
				NumberOfSettingsInBlindgroup[0]=1;
				NumberOfSettingsInBlindgroup[1]=NumBlindSettings-1;
				NumberOfBlindGroups=1;


			//=====================
			// Advanced blind model
			//=====================
			}else if(NumBlindSettings <0 && NumBlindSettings>-10){
				AdvancedBlindModel=1;
				NumBlindSettings=(-1)*NumBlindSettings; // set number of blind groups
				NumberOfBlindGroups=NumBlindSettings;

				//===========================

				//set variable for base geometry
				sprintf(BlindGroupName[0],"base_geometry");
				sprintf(shading_variant_name[1],"%s",BlindGroupName[0]);
				NumberOfSettingsInBlindgroup[0]=1;


				fscanf(HEADER_FILE,"%s", BlindGroupDiffDC_File[0][0]);
				sprintf(shading_dc_file[TotalNumberOfDCFiles+1],"%s",BlindGroupDiffDC_File[0][0]);

				fscanf(HEADER_FILE,"%s", BlindGroupIlluminanceProfiles[0][0]);
				sprintf(shading_illuminance_file[TotalNumberOfDCFiles+1],"%s",BlindGroupIlluminanceProfiles[0][0]);
				TotalNumberOfDCFiles++;

				//====================================
				//read in results for each blind group
				//====================================
				for (i=1 ; i<=NumberOfBlindGroups ; i++)
				{
					fscanf(HEADER_FILE,"%s", BlindGroupName[i]);
					fscanf(HEADER_FILE,"%d", &NumberOfSettingsInBlindgroup[i]);
					
				

					// set blind control strategy
					//===========================
					fscanf(HEADER_FILE,"%s", blind_control_name[i]);
					if(!strcmp(blind_control_name[i],"SystemAlwaysUp"))
					{
						BlindSystemType[i]=0;
					}else if(!strcmp(blind_control_name[i],"ManualControl"))
					{
						BlindSystemType[i]=1;
					}else if(!strcmp(blind_control_name[i],"IdealizedAutomatedControl"))
					{
						BlindSystemType[i]=2;
					}else if(!strcmp(blind_control_name[i],"AutomatedThermalControl"))
					{
						BlindSystemType[i]=3;
					}else if(!strcmp(blind_control_name[i],"AutomatedThermalControlWithOccupancy"))
					{
						BlindSystemType[i]=4;
						fscanf(HEADER_FILE,"%d %d",&month,&day); //start month and day
						BlindGroupCoolingPeriodJulianDay[i][0]=julian_date( month, day);
						BlindGroupCoolingPeriodJulianDay[i][2]=month;
						BlindGroupCoolingPeriodJulianDay[i][3]=day;			
						fscanf(HEADER_FILE,"%d %d",&month,&day); //end month and day
						BlindGroupCoolingPeriodJulianDay[i][1]=julian_date( month, day);
						BlindGroupCoolingPeriodJulianDay[i][4]=month;
						BlindGroupCoolingPeriodJulianDay[i][5]=day;

					}else if(!strcmp(blind_control_name[i],"AutomatedGlareControl"))
					{
						BlindSystemType[i]=5;
						fscanf(HEADER_FILE,"%f",&BlindGroupThresholdForGlareSensor[i]);
						fscanf(HEADER_FILE,"%f",&BlindGroupAzimuthRange[i][0]); //lower azimuth
						fscanf(HEADER_FILE,"%f",&BlindGroupAzimuthRange[i][1]); //higher azimuth
						fscanf(HEADER_FILE,"%f",&BlindGroupAltitudeRange[i][0]); //lower altitude
						fscanf(HEADER_FILE,"%f",&BlindGroupAltitudeRange[i][1]); //higher altitude
					}else if(!strcmp(blind_control_name[i],"AutomatedGlareControlWithOccupancy"))
					{
						BlindSystemType[i]=6;
						fscanf(HEADER_FILE,"%f",&BlindGroupThresholdForGlareSensor[i]);
						fscanf(HEADER_FILE,"%f",&BlindGroupAzimuthRange[i][0]); //lower azimuth
						fscanf(HEADER_FILE,"%f",&BlindGroupAzimuthRange[i][1]); //higher azimuth
						fscanf(HEADER_FILE,"%f",&BlindGroupAltitudeRange[i][0]); //lower altitude
						fscanf(HEADER_FILE,"%f",&BlindGroupAltitudeRange[i][1]); //higher altitude
						fscanf(HEADER_FILE,"%d %d",&month,&day); //start month and day
						BlindGroupCoolingPeriodJulianDay[i][0]=julian_date( month, day);
						BlindGroupCoolingPeriodJulianDay[i][2]=month;
						BlindGroupCoolingPeriodJulianDay[i][3]=day;			
						fscanf(HEADER_FILE,"%d %d",&month,&day); //end month and day
						BlindGroupCoolingPeriodJulianDay[i][1]=julian_date( month, day);
						BlindGroupCoolingPeriodJulianDay[i][4]=month;
						BlindGroupCoolingPeriodJulianDay[i][5]=day;
					}else if (!strcmp(blind_control_name[i], "AnnualShadingSchedule"))
					{
						BlindSystemType[i] = 7;
						fscanf(HEADER_FILE, "%s", AnnualShadingProfile[i]);

					}else{
						fprintf(stderr,"read_in_header:\'blind_control\' out of range: \'%s\'\n",blind_control_name[i]);
						exit(1);
					}
					
					//read in geometry of shading system in base mode. This may be a 'null.rad' file
					fscanf(HEADER_FILE,"%s", BlindGroupGeometryInRadiance[0][i]);
					if(!check_if_file_exists(BlindGroupGeometryInRadiance[0][i]))
					{
						sprintf(keyword,"%s%s",project_directory,BlindGroupGeometryInRadiance[0][i]);
						strcpy(BlindGroupGeometryInRadiance[0][i],keyword);
					}
					//appends the base seeting of all shading groups to the blinds up scene
					if( strcmp(shading_rad_file[1],""))
						sprintf(shading_rad_file[1],"%s\" \"%s",shading_rad_file[1],BlindGroupGeometryInRadiance[0][i]);
					else
						sprintf(shading_rad_file[1],"%s",BlindGroupGeometryInRadiance[0][i]);
					
					for (j=1 ; j<=NumberOfSettingsInBlindgroup[i] ; j++)
					{
						if(BlindSystemType[i]==3 ||BlindSystemType[i]==4 ||BlindSystemType[i]==5||BlindSystemType[i]==6) //single up/down target illuminance for all shading device settings
						{
							fscanf(HEADER_FILE,"%f",&BlindGroupSetpoints[i][j][0]);
							fscanf(HEADER_FILE,"%f",&BlindGroupSetpoints[i][j-1][1]);
						}
						sprintf(shading_variant_name[TotalNumberOfDCFiles+1],"%s.%d",BlindGroupName[i],j);
						fscanf(HEADER_FILE,"%s", BlindGroupGeometryInRadiance[j][i]);
						sprintf(shading_rad_file[TotalNumberOfDCFiles+1],"%s",BlindGroupGeometryInRadiance[j][i]);
						fscanf(HEADER_FILE,"%s", BlindGroupDiffDC_File[j][i]);
						sprintf(shading_dc_file[TotalNumberOfDCFiles+1],"%s",BlindGroupDiffDC_File[j][i]);
						fscanf(HEADER_FILE,"%s", BlindGroupIlluminanceProfiles[j][i]);
						sprintf(shading_illuminance_file[TotalNumberOfDCFiles+1],"%s",BlindGroupIlluminanceProfiles[j][i]);
						TotalNumberOfDCFiles++;
					}
				}
			}
			//================================================
			// Most advanced blind model used for sDA analysis					Added by Craig Casey [PSU]
			//================================================
			else if (NumBlindSettings<-10){	
				NumBlindSettings=(-1)*NumBlindSettings; // set number of blind groups
				NumBlindSettings=NumBlindSettings-10;
				AdvancedBlindModel=1;
				NumberOfBlindGroups=NumBlindSettings;
				fscanf(HEADER_FILE, "%s", BlindGroupDiffDC_File[0][0]);
				fscanf(HEADER_FILE, "%s", BlindGroupIlluminanceProfiles[0][0]);
				for (i=1 ; i<=NumBlindSettings ; i++){				//	Looping through the window groups
					fscanf(HEADER_FILE,"%s",BlindGroupName[i]);
					fscanf(HEADER_FILE, "%d",&NumberOfSettingsInBlindgroup[i]);
					fscanf(HEADER_FILE, "%s", ShadeControlMethod[i]);
					if (NumberOfSettingsInBlindgroup[i]>0){
						if (!strcmp(ShadeControlMethod[i], "null")){
							
						}
						else if (!strcmp(ShadeControlMethod[i], "automated_signal")){
							fscanf(HEADER_FILE, "%s", ShadeControlPhotosensorLabel[i]);
							for (j=0;j<7;j++){
								fscanf(HEADER_FILE,"%f", &ShadePSLocDir[i-1][j]);
							}
							for (j=0;j<NumberOfSettingsInBlindgroup[i];j++){
								fscanf(HEADER_FILE, "%f", &ShadeControlSignalArguments[i][j]);
							}
						}
						else if (!strcmp(ShadeControlMethod[i], "automated_profile_angle")){
							fscanf(HEADER_FILE, "%f", &ShadeControlAzimuth[i]);
							for (j=0;j<NumberOfSettingsInBlindgroup[i];j++){
								fscanf(HEADER_FILE, "%f", &ShadeControlAngleArguments[i][j]);
							}
						}
						else if (!strcmp(ShadeControlMethod[i], "automated_profile_angle_signal")){
							fscanf(HEADER_FILE, "%f", &ShadeControlAzimuth[i]);
							fscanf(HEADER_FILE, "%s", ShadeControlPhotosensorLabel[i]);
							for (j=0;j<7;j++){
								fscanf(HEADER_FILE,"%f", &ShadePSLocDir[i-1][j]);
							}
							for (j=0;j<NumberOfSettingsInBlindgroup[i];j++){
								fscanf(HEADER_FILE, "%f", &ShadeControlAngleArguments[i][j]);
								fscanf(HEADER_FILE, "%f", &ShadeControlSignalArguments[i][j]);
							}
						}else{
							printf("Error shading: \nThe control algorithm for the shades in the window group named %s is invalid.\n",BlindGroupName[i]);
						}
					}						
					fscanf(HEADER_FILE, "%s", keyword);
					fscanf(HEADER_FILE, "%d", &MaterialCase[i]);
					if (MaterialCase[i]==1){
						fscanf(HEADER_FILE, "%s", BlindGroupGeometryInRadiance[0][i]);
						if( strcmp(shading_rad_file[1],""))											//Added this section to aid in Gen_DC
							sprintf(shading_rad_file[1],"%s\" \"%s",shading_rad_file[1],BlindGroupGeometryInRadiance[0][i]);
						else
							sprintf(shading_rad_file[1],"%s",BlindGroupGeometryInRadiance[0][i]);
						fscanf(HEADER_FILE, "%d", &NumGlazingMaterials[i]);
						for (j=1;j<=NumGlazingMaterials[i];j++){
							fscanf(HEADER_FILE, "%s", GlazingMaterials[i][j]);
						}
						fscanf(HEADER_FILE, "%d", &sDA_setting[i]);
						for (j=0;j<NumberOfSettingsInBlindgroup[i];j++){
							fscanf(HEADER_FILE, "%s", BlindGroupGeometryInRadiance[j+1][i]);			//reversed the j and i
							fscanf(HEADER_FILE, "%s", BlindGroupDiffDC_File[j][i]);						//reversed the j and i
							fscanf(HEADER_FILE, "%s", BlindGroupIlluminanceProfiles[j][i]);				//reversed the j and i
							sprintf(shading_variant_name[TotalNumberOfDCFiles+1],"%s.%d",BlindGroupName[i],j);				//Added this section to aid in Gen_DC
							sprintf(shading_rad_file[TotalNumberOfDCFiles+1],"%s",BlindGroupGeometryInRadiance[j][i]);
							sprintf(shading_dc_file[TotalNumberOfDCFiles+1],"%s",BlindGroupDiffDC_File[j][i]);
							sprintf(shading_illuminance_file[TotalNumberOfDCFiles+1],"%s",BlindGroupIlluminanceProfiles[j][i]);
							TotalNumberOfDCFiles++;
						}
					}else if (MaterialCase[i]==2){
						fscanf(HEADER_FILE, "%s", BlindGroupGeometryInRadiance[i][0]);
						fscanf(HEADER_FILE, "%d", &NumGlazingMaterials[i]);
						for (j=0;j<NumGlazingMaterials[i];j++){
							fscanf(HEADER_FILE, "%s", GlazingMaterials[i][j]);
						}
						fscanf(HEADER_FILE, "%d", &NumBSDFMaterials[i][0]);
						if (NumBSDFMaterials[i][0]>0){
							for (j=0;j<NumBSDFMaterials[i][0];j++){
								fscanf(HEADER_FILE, "%s", BSDFExchangeMaterials[i][0][j]);
								fscanf(HEADER_FILE, "%s", BSDFMaterials[i][0][j]);
							}
						}
						fscanf(HEADER_FILE, "%d", &sDA_setting[i]);
						for (j=0;j<NumberOfSettingsInBlindgroup[i];j++){
							fscanf(HEADER_FILE, "%s", BlindGroupGeometryInRadiance[j+1][i]);
							fscanf(HEADER_FILE, "%s", BlindGroupDiffDC_File[j][i]);
							fscanf(HEADER_FILE, "%s", BlindGroupIlluminanceProfiles[j][i]);
							fscanf(HEADER_FILE, "%d", &NumBSDFMaterials[i][j]);
							for (k=0;k<NumBSDFMaterials[i][j];k++){
								fscanf(HEADER_FILE, "%s", BSDFExchangeMaterials[i][j][k]);
								fscanf(HEADER_FILE, "%s", BSDFMaterials[i][j][k]);
							}
							sprintf(shading_variant_name[TotalNumberOfDCFiles+1],"%s.%d",BlindGroupName[i],j);				//Added this section to aid in Gen_DC
							sprintf(shading_rad_file[TotalNumberOfDCFiles+1],"%s",BlindGroupGeometryInRadiance[j][i]);
							sprintf(shading_dc_file[TotalNumberOfDCFiles+1],"%s",BlindGroupDiffDC_File[j][i]);
							sprintf(shading_illuminance_file[TotalNumberOfDCFiles+1],"%s",BlindGroupIlluminanceProfiles[j][i]);
							TotalNumberOfDCFiles++;
						}
					}else{
						printf("Error shading: \nThe matl_case for the window group named %s is invalid.\n",BlindGroupName[i]);
					}
				}
			}
			ShadingSpecified=1;
		}
		else if( !strcmp(keyword,"AdaptiveZoneApplies") ){
			fscanf(HEADER_FILE,"%d",&AdaptiveZoneApplies);
			// AdaptiveZoneApplies	= 1 The lowest DGP values for all view positions is used.
			//						= 0 The highest DGP values for all view positions is used.
		}
		//======================
		//blind control strategy
		//======================
		else if( !strcmp(keyword,"blind_control") ){
			fscanf(HEADER_FILE,"%d",&number_of_blind_controls);
			if(number_of_blind_controls>10 || number_of_blind_controls<0){fprintf(stderr,"FATAL ERROR - header file: number of blind systems out of range 0-10 (%d).\n",number_of_blind_controls);exit(1);}
			for (i=0 ; i<number_of_blind_controls ; i++){
				fscanf(HEADER_FILE,"%d ",&BlindSystemType[i]);
				fscanf(HEADER_FILE,"%s", blind_control_name[i]);
				if(BlindSystemType[i]>2 || BlindSystemType[i] <0 ){fprintf(stderr,"read_in_header:\'blind_control\' out of range.\n");exit(1);}
			}
		}


		//==========
		// DGP Input
		//==========
		else if( !strcmp(keyword,"viewpoint_file") ){
			getWord(viewpoint_file,HEADER_FILE, PATH_SIZE, '\n');
			 if( !strcmp(viewpoint_file,"no_DGP_view_file_provided") )
			 	sprintf(viewpoint_file,"%s","");
		}


		//==================
		// DAYSIM simulation
		//==================
		else if( !strcmp(keyword,"coupling_mode") )
			fscanf(HEADER_FILE,"%d",&dc_coupling_mode);
			// keyword for ds_illum to specify how direct sunlight is coupled with the
			// direct daylight coefficients
			// see Reinhart C F, Walkenhorst O, Dynamic RADIANCE-based daylight simulations
			//     for a full-scale test office with outer venetian blinds.
			//	   Energy & Buildings, 33:7 pp. 683-697, 2001.
			// synopsis coupling_mode <integer n>
			// n=0 interpolated [default]
			//n=2  shadow testing: all raytracing is done at the beginning
			//     the first point in sensor_file of type "1" is chosen as
			// 	   the reference sensor
			//n =3 direct sunlight is discarded


		/*
		 * rtrace parameter
		 */
		else if( rtraceParameterGroup( keyword, HEADER_FILE ) == 0 ) { /* simply parse */
		} else if( parse( keyword, HEADER_FILE, parse_params ) == 0 ) {
		} else if( parse( keyword, HEADER_FILE, rtrace_parse_params ) == 0 ) {
			copy_rtrace_options= 1;
		}
		/* rotate geometry and measuring points */		//Changed these from strcasecmp to strcmp
		else if( !strcmp( keyword, "rotate_x" ) ) {
			rotationAxis[rotationNumber]= XAxis;
			fscanf( HEADER_FILE, "%lf", &rotationAngle[rotationNumber++] );
		} else if( !strcmp( keyword, "rotate_y" ) ) {
			rotationAxis[rotationNumber]= YAxis;
			fscanf( HEADER_FILE, "%lf", &rotationAngle[rotationNumber++] );
		} else if( !strcmp( keyword, "rotate_z" ) ) {
			rotationAxis[rotationNumber]= ZAxis;
			fscanf( HEADER_FILE, "%lf", &rotationAngle[rotationNumber++] );
		}

		//=============
		// output files
		//=============
		else if( !strcmp(keyword,"daylight_autonomy")){
			fscanf(HEADER_FILE,"%s", da_file);
		}
		else if( !strcmp(keyword,"daylight_factor")){
			fscanf(HEADER_FILE,"%s", df_file);
		}
		else if( !strcmp(keyword,"daylight_availability_active_RGB")){
					fscanf(HEADER_FILE,"%s", da_availability_active_RGB_file);
				}
		else if( !strcmp(keyword,"daylight_availability_passive_RGB")){
							fscanf(HEADER_FILE,"%s", da_availability_passive_RGB_file);
						}
		else if( !strcmp(keyword,"daylight_autonomy_active_RGB")){
					fscanf(HEADER_FILE,"%s", da_active_RGB_file);
				}
		else if( !strcmp(keyword,"daylight_autonomy_passive_RGB")){
							fscanf(HEADER_FILE,"%s", da_passive_RGB_file);
						}
		else if( !strcmp(keyword,"daylight_factor_RGB")){
							fscanf(HEADER_FILE,"%s", df_RGB_file);
						}

		else if( !strcmp(keyword,"continuous_daylight_autonomy_active_RGB")){
					fscanf(HEADER_FILE,"%s", con_da_active_RGB_file);
				}
		else if( !strcmp(keyword,"continuous_daylight_autonomy_passive_RGB")){
							fscanf(HEADER_FILE,"%s", con_da_passive_RGB_file);
						}
		else if( !strcmp(keyword,"DA_max_active_RGB")){
							fscanf(HEADER_FILE,"%s", DA_max_active_RGB_file);
						}
		else if( !strcmp(keyword,"DA_max_passive_RGB")){
							fscanf(HEADER_FILE,"%s", DA_max_passive_RGB_file);
						}

		else if( !strcmp(keyword,"UDI_100_active_RGB")){
					fscanf(HEADER_FILE,"%s", UDI_100_active_RGB_file);
				}
		else if( !strcmp(keyword,"UDI_100_passive_RGB")){
							fscanf(HEADER_FILE,"%s", UDI_100_passive_RGB_file);
						}
		else if( !strcmp(keyword,"UDI_100_2000_active_RGB")){
					fscanf(HEADER_FILE,"%s", UDI_100_2000_active_RGB_file);
				}
		else if( !strcmp(keyword,"UDI_100_2000_passive_RGB")){
							fscanf(HEADER_FILE,"%s", UDI_100_2000_passive_RGB_file);
						}
		else if( !strcmp(keyword,"UDI_2000_active_RGB")){
					fscanf(HEADER_FILE,"%s", UDI_2000_active_RGB_file);
				}
		else if( !strcmp(keyword,"UDI_2000_passive_RGB")){
							fscanf(HEADER_FILE,"%s", UDI_2000_passive_RGB_file);
						}
		else if( !strcmp(keyword,"DSP_active_RGB")){
					fscanf(HEADER_FILE,"%s", DSP_active_RGB_file);
				}
		else if( !strcmp(keyword,"DSP_passive_RGB")){
							fscanf(HEADER_FILE,"%s", DSP_passive_RGB_file);
					}

		else if( !strcmp(keyword,"rshow_da")){
			fscanf(HEADER_FILE,"%s",rshow_da_file);
		}
		else if( !strcmp(keyword,"rshow_df")){
			fscanf(HEADER_FILE,"%s",rshow_df_file);
		}
		else if( !strcmp(keyword,"direct_sunlight_file")){
			fscanf(HEADER_FILE,"%s", direct_sunlight_file);
		}
		else if( !strcmp(keyword,"percentage_of_visible_sky_file")){
			fscanf(HEADER_FILE,"%s", percentage_of_visible_sky_file);
		}
		else if( !strcmp(keyword,"direct_glare_file")){
			fscanf(HEADER_FILE,"%s", direct_glare_file);
		}
		else if( !strcmp(keyword,"electric_lighting")){
			fscanf(HEADER_FILE,"%s", el_file);
		}
		else if( !strcmp(keyword,"occupancy_profile")  ){
			fscanf(HEADER_FILE,"%s", occ_output_file);
		}
		else if( !strcmp(keyword,"thermal_simulation") || !strcmp(keyword,"thermal_simulation_active")){
			fscanf(HEADER_FILE,"%s", thermal_sim_file_active);
		}
		else if( !strcmp(keyword,"thermal_simulation_passive")){
			fscanf(HEADER_FILE,"%s", thermal_sim_file_passive);
		}
		else if ( !strcmp(keyword, "sDA_settings")){							//Added by Craig Casey [PSU]
			fscanf(HEADER_FILE, "%f", &sDA_settings[0]);
			fscanf(HEADER_FILE, "%f", &sDA_settings[1]);
			fscanf(HEADER_FILE, "%f", &sDA_settings[2]);
			fscanf(HEADER_FILE, "%f", &sDA_settings[3]);
		}
		else if(!strcmp(keyword, "target_illuminance")){
			fscanf(HEADER_FILE, "%f", &target_ill);
		}

		//=================================
		// electric lighting control system
		//=================================
		else if( !strcmp(keyword,"zone_description")){ // string to describe the investigated building zone
			fscanf(HEADER_FILE,"%s[^\n]",zone_description);
		}
		else if( !strcmp(keyword,"electric_lighting_system")){
			fscanf(HEADER_FILE,"%d",&NumberOfLightingGroups);
			if(NumberOfLightingGroups>10 || NumberOfLightingGroups<0){fprintf(stderr,"FATAL ERROR - header file: number of lighting scenarios out of range 0-10 (%d).\n",NumberOfLightingGroups);exit(1);}
			for (i=1 ; i<=NumberOfLightingGroups ; i++){
				fscanf(HEADER_FILE,"%d ",&LightingSystemType[i]);
				switch( LightingSystemType[i] ) {
				case 1: //manual on/off switch by the door; no lighting controls
					fscanf( HEADER_FILE,"%s %f %f", lighting_scenario_name[i], &LightPower[i], &ZoneSize[i]);
					StandbyPower[i]=0;
					break;
				case 2: /*energy efficient occupancy sensor */
					fscanf( HEADER_FILE,"%s %f %f %f %d", lighting_scenario_name[i], &LightPower[i], &ZoneSize[i],
							&StandbyPower[i], &OccSenDelayTime[i] );
					break;
				case 3: /*on/off occupancy sensor */
					fscanf( HEADER_FILE,"%s %f %f %f %d", lighting_scenario_name[i], &LightPower[i], &ZoneSize[i],
							&StandbyPower[i], &OccSenDelayTime[i] );
					break;
				case 4: // direct photo-controlled dimmed lighting
					fscanf( HEADER_FILE,"%s %f %f %f %f %f", lighting_scenario_name[i],
							&LightPower[i], &ZoneSize[i], &StandbyPower[i], &MinDimLevel[i], &MinIllLevel[i] );
					break;
				case 5: // direct photo-controlled & eff_occ sensor
						//with a simulated photocell
					fscanf( HEADER_FILE,"%s %f %f %f %f %f %d", lighting_scenario_name[i],
							&LightPower[i], &ZoneSize[i], &StandbyPower[i], &MinDimLevel[i], &MinIllLevel[i],&OccSenDelayTime[i] );
					break;
				case 6: // direct photo-controlled & on/off sensor
					fscanf( HEADER_FILE, "%s %f %f %f %f %f %d", lighting_scenario_name[i],
							&LightPower[i], &ZoneSize[i], &StandbyPower[i], &MinDimLevel[i],&MinIllLevel[i], &OccSenDelayTime[i] );
					break;
				case 7: /* indirect dimmed lighting */
					fscanf( HEADER_FILE, "%s %f %f %f %f %f", lighting_scenario_name[i],
							&LightPower[i], &ZoneSize[i], &StandbyPower[i], &MinDimLevel[i], &MinIllLevel[i] );
					break;
				case 10: /* scheduled lighting with an on/off photocell control*/
					fscanf( HEADER_FILE, "%s %f %f %f %f %f", lighting_scenario_name[i],
							&LightPower[i], &ZoneSize[i],&MinIllLevel[i], &schedule_start[i], &schedule_end[i] );
					break;
				case 11: /* photocell controlled dimmed lighting with a time schedule */
					fscanf( HEADER_FILE,"%s %f %f %f %f %f %f %f",
							lighting_scenario_name[i], &LightPower[i], &ZoneSize[i], &StandbyPower[i],
							&MinDimLevel[i], &MinIllLevel[i], &schedule_start[i], &schedule_end[i]);
					break;
				case 20: /* advanced photo control using the Advanced Header File*/
					fscanf(HEADER_FILE, "%s", zone_label[i]);
					fscanf(HEADER_FILE, "%s", ControlMethod[i]);
					if (strcmp(ControlMethod[i],"dim_to_min") && strcmp(ControlMethod[i],"dim_to_off") && strcmp(ControlMethod[i],"on") && strcmp(ControlMethod[i],"off") && strcmp(ControlMethod[i],"switched")){
						printf("Error in electric_lighting_system:  \n The control method for the zone named %s is incorrect.\n",zone_label[i]);
					}
					fscanf(HEADER_FILE, "%s", ZonePSLabel[i]);
					//printf("I have made it into the electric lighting system case 20.  The string is %s.", ZonePSLabel[i]);
					if (!strcmp(ZonePSLabel[i], "null")){

					}
					else{
						for (j=0;j<7;j++){
							fscanf(HEADER_FILE, "%f", &PSLocDir[i][j]);
						}
					}
					if ((!strcmp(ControlMethod[i],"on") || !strcmp(ControlMethod[i],"off")) && !strcmp(ZonePSLabel[i], "null"))
					{
					}else{
						fscanf(HEADER_FILE, "%s", CriticalPointMethod[i]);
						if( !strcmp(CriticalPointMethod[i], "CP_analysis")){ // file name with specified number of critical points
							for (j=0;j<2; j++){
								fscanf(HEADER_FILE, "%s", CPArguments[i][j]);
							}
							fscanf(HEADER_FILE, "%d", &numCP[i]);
							fscanf(HEADER_FILE, "%f", &TPval[i]);
							if (TPval[i]>1 || TPval[i]<0){
								printf("Error in electric_lighting_system:  \n The target percentage in CP_analysis for the zone named %s is incorrect.\nThe value needs to be between 0 and 1.",zone_label[i]);
							}
							fscanf(HEADER_FILE, "%s", CPArguments[i][2]);
						}
						else if( !strcmp(CriticalPointMethod[i], "CP_user_specified")){ // file name with floating critical points
							fscanf(HEADER_FILE, "%s", CPArguments[i][0]);
							if (!strcmp(CPArguments[i][0], "null")){

							}
							else{
								fscanf(HEADER_FILE, "%s", CPArguments[i][1]);
								fscanf(HEADER_FILE, "%d", &numCP[i]);
								fscanf(HEADER_FILE, "%f", &TPval[i]);
								if (TPval[i]>1 || TPval[i]<0){
									printf("Error in electric_lighting_system:  \n The target percentage in CP_user_specified for the zone named %s is incorrect.\nThe value needs to be between 0 and 1.",zone_label[i]);
								}
								fscanf(HEADER_FILE, "%s", CPArguments[i][2]);
							}
						}else{
							printf("Error electric_lighting_system: \nThe Critical Point Method for the zone named %s is invalid.\n",zone_label[i]);
						}
					}
					if (!strcmp(ZonePSLabel[i], "null")){

					}
					else{
						fscanf(HEADER_FILE, "%s", PSAlgorithm[i]);
						if (!strcmp(PSAlgorithm[i], "null")){

						}
						else if ( !strcmp(PSAlgorithm[i], "open_dimming") || !strcmp(PSAlgorithm[i], "closed_proportional")){
							for (j=0;j<3;j++){
								fscanf(HEADER_FILE, "%f", &PSAlgorithmArguments[i][j]);
							}
						}
						else if( !strcmp(PSAlgorithm[i], "constant_setpoint")){
							for (j=0; j<2;j++){
								fscanf(HEADER_FILE, "%f", &PSAlgorithmArguments[i][j]);
							}
						}
						else if( !strcmp(PSAlgorithm[i], "open_switching") || !strcmp(PSAlgorithm[i], "closed_switching")){
							for (j=0; j<2; j++){
								fscanf(HEADER_FILE, "%f", &PSAlgorithmArguments[i][j]);
							}
						}else{
							printf("Error electric_lighting_system: \nThe control algorithm for the zone named %s is invalid.\n",zone_label[i]);
						}
					}
					break;
				}
			}
		}
		else if( !strcmp(keyword,"minimum_illuminance_level")){ // for daylight autonomy calculation
			fscanf(HEADER_FILE,"%f",&ill_min);
			if(ill_min<0){fprintf(stderr,"FATAL ERROR - header file: illuminance levels is negative (%.0f).\n",ill_min);exit(1);}
		}
		//=================
		// occupancy module
		//=================
		else if( !strcmp(keyword,"daylight_savings_time")){
			fscanf(HEADER_FILE,"%d",&daylight_savings_time);
			if(daylight_savings_time!=0 && daylight_savings_time!=1){
				fprintf(stderr,"read_in_header: variable \'daylight_savings_time\' out of bound (%d)\n",daylight_savings_time);
				exit(1);
			}
		}
		else if( !strcmp(keyword,"first_weekday")){
			fscanf(HEADER_FILE,"%d",&first_weekday);
			if(first_weekday<0 || first_weekday>7){
				fprintf(stderr,"read_in_header: variable \'first_weekday\' out of bound (%d)\n",first_weekday);
				exit(1);
			}
		}
		else if( !strcmp(keyword,"occupancy")){
			fscanf(HEADER_FILE,"%d",&occupancy_mode);
			if(occupancy_mode==0){	/*occpuancy from start work to end work */
				fscanf(HEADER_FILE,"%f",&start_work);
				fscanf(HEADER_FILE,"%f",&end_work);
			}
			if(occupancy_mode==1){	/*occpuancy from start work to end work */
				fscanf(HEADER_FILE,"%f",&start_work);
				fscanf(HEADER_FILE,"%f",&end_work);
			}
			if(occupancy_mode==2){  /*stochastic occupancy model based on routine file*/
				fscanf(HEADER_FILE,"%s", routine_file);
			}
			if(occupancy_mode==3){  /*stochastic occupancy model based on measured data*/
				fscanf(HEADER_FILE,"%s", measured_occ);
				fscanf(HEADER_FILE,"%s", routine_file);
				fscanf(HEADER_FILE,"%f ",&lunch_time );
				fscanf(HEADER_FILE,"%f ",&lunch_time_length );
			}
			if(occupancy_mode==4){  // occupancy profile for Lightswitch Classrooms
				fscanf(HEADER_FILE,"%s", static_occupancy_profile);
			}
			if(occupancy_mode==5){  //reads in an external occupancy profile
				fscanf(HEADER_FILE,"%s", measured_occ);
			}
			if(occupancy_mode>5){
				fprintf(stderr,"read_in_header: variable \'occupancy\' is not valid (%d)\n",occupancy_mode);
				exit(1);
			}
		}

		else if( !strcmp(keyword,"user_profile") ){
			fscanf(HEADER_FILE,"%d",&NumUserProfiles);
			if(NumUserProfiles>10 || NumUserProfiles<0){
				fprintf(stderr,"FATAL ERROR - header file: number of user behaviors out of range 0-10 (%d).\n",number_of_blind_controls);exit(1);
			}
			frequency_sum=0;
			for (i=0 ; i<NumUserProfiles ; i++){
				fscanf(HEADER_FILE,"%s", user_type_name[i]);
				fscanf(HEADER_FILE,"%f ",&user_type_frequency[i]);
				frequency_sum+=user_type_frequency[i];
				fscanf(HEADER_FILE,"%d ",&UserLightControl[i]);
				fscanf(HEADER_FILE,"%d ",&UserBlindControl[i]);
				if(UserBlindControl[i]==2)
				{ //shading device always in the same position
					fscanf(HEADER_FILE,"%d ",&static_shading_scenario[i]);
					// static_shading_scenario == position of the blinds throughout the year
				
				}
			}
			for (i=0 ; i<NumUserProfiles ; i++){ // normalize frequencies to unity
				user_type_frequency[i]*=100.0/frequency_sum;
			}
		}
		else if( !strcmp(keyword,"DirectIrradianceGlareThreshold") ){ //Direct threshold level in W/m2 for blind control
			fscanf(HEADER_FILE,"%f",&DirectIrradianceGlareThreshold);
			if(DirectIrradianceGlareThreshold<0){
				fprintf(stderr,"FATAL ERROR - header file - keyword DirectIrradianceGlareThreshold: The direct threshold level has to be positive (%f).\n",DirectIrradianceGlareThreshold);
				exit(1);
			}
		}
		else if( !strcmp(keyword,"wea_data_file")){
			fscanf(HEADER_FILE,"%[^\n]s", wea_data_file);
			trim(wea_data_file,keyword);
            strcpy(wea_data_file,keyword);
		}
		else if( !strcmp(keyword,"wea_data_file_units") || !strcmp(keyword,"input_format_genweather") || !strcmp(keyword,"input_type") || !strcmp(keyword,"input_weather_data_shortterm") || !strcmp(keyword,"input_irradiance_data")){
			fscanf(HEADER_FILE,"%d",&wea_data_file_units);
		}
		else if( !strcmp(keyword,"wea_data_short_file")){
			fscanf(HEADER_FILE,"%s", wea_data_short_file);
		}
		else if( !strcmp(keyword,"wea_data_short_file_units")  ){
			fscanf(HEADER_FILE,"%d",&wea_data_short_file_units);
		}
		else if( !strcmp(keyword,"direct_radiance_file") ){
			fscanf(HEADER_FILE,"%s", direct_radiance_file);
		}
		else if( !strcmp(keyword,"direct_radiance_file_2305") ){
			fscanf(HEADER_FILE,"%s", direct_radiance_file_2305);
		}
		else if( !strcmp(keyword,"PNGScheduleExists") ){
			fscanf(HEADER_FILE,"%d", &PNGScheduleExists);
		}
		else {
			fscanf(HEADER_FILE,"%*[^\n]");
			fscanf(HEADER_FILE,"%*[\n\r]");
		}
	} //end parsing header file for the first time

//set dgp file name
//	strcpy( Effective_DGP_file, shading_illuminance_file[1]);
//	c= Effective_DGP_file + strlen(shading_illuminance_file[1]) -4; //cut ".ill"
//	strcpy( c, "_effective.dgp" );
sprintf(Effective_DGP_file_html,"%s_effective.dgp",project_name);
sprintf(Effective_DGP_file,"%s\\%s_effective.dgp",tmp_directory,project_name);


	//set default user profile
	if(NumUserProfiles==0)
	{
		NumUserProfiles=1;
		sprintf(user_type_name[0],"ActiveBlindAndLightingControl");
		user_type_frequency[0]=100;
		UserLightControl[i]=1;
		UserBlindControl[i]=3;
	}


//===================================
// add absolute paths to output files
//===================================
	for (i=0 ; i<=TotalNumberOfDCFiles ; i++){
		//if( strcmp(shading_rad_file[i],"")){
		//		if(!check_if_file_exists(shading_rad_file[i])){
		//			sprintf(keyword,"%s%s",project_directory,shading_rad_file[i]);
		//			strcpy(shading_rad_file[i],keyword);
		//		}
		//}
		if( strcmp(shading_dc_file[i],"")){
				sprintf(keyword,"%s%s",project_directory,shading_dc_file[i]);
				strcpy(shading_dc_file[i],keyword);
		}
		if( strcmp(shading_illuminance_file[i],"")
			&& strcmp( shading_illuminance_file[i], "-" )) {
			sprintf(keyword,"%s%s",project_directory,shading_illuminance_file[i]);
			strcpy(shading_illuminance_file[i],keyword);
		}
	}





//Tito: might takeo out: test
//===========================

	if (AdvancedBlindModel==1){							//Inhereted from PennState Daysim, Craig Casey [PSU] added the if statement to avoid conflicts
		k=0;
		for (i=0 ; i<= NumBlindSettings; i++)
		{
			for (j=0 ; j< NumberOfSettingsInBlindgroup[i]; j++)
			{
				if( strcmp(BlindGroupDiffDC_File[j][i],"")){
					sprintf(keyword,"%s%s",project_directory,BlindGroupDiffDC_File[j][i]);
					strcpy(BlindGroupDiffDC_File[j][i],keyword);
				}
				if( strcmp(BlindGroupIlluminanceProfiles[j][i],"")){
					sprintf(keyword,"%s%s",project_directory,BlindGroupIlluminanceProfiles[j][i]);
					strcpy(BlindGroupIlluminanceProfiles[j][i],keyword);
					sprintf(shading_illuminance_file[k],"%s",BlindGroupIlluminanceProfiles[j][i]);

				}
				k++;
			}

		}
	}
//Tito: might takeo out: end
//===========================






	prepend_path_m( project_directory, geometry_file, PATH_SIZE );
	prepend_path_m( project_directory, material_file, PATH_SIZE );

	if( strcmp(sensor_file,"")){
		sprintf(keyword,"%s%s",project_directory,sensor_file);
		TEST_FILE = fopen(keyword, "r");
		if ( TEST_FILE != NULL){
			close_file(TEST_FILE);
			strcpy(sensor_file,keyword);
		}else {
	    	TEST_FILE = fopen(sensor_file, "r");
			if ( TEST_FILE != NULL){
				close_file(TEST_FILE);
			}else{
				printf("FATAL INPUT ERROR: Sensor file %s does not exist. Pro dir %s.",sensor_file,project_directory);
				exit(1);
			}
		}
	}
	    number_of_sensors=number_of_lines_in_file(sensor_file);
		if ((sensor_typ=(int*) malloc (sizeof(int)* number_of_sensors)) == NULL)
		{
			fprintf(stderr,"work plane sensor: Out of memory in function \'read_in_header\'\n");
			exit(1);
		}
		if ((sensor_unit=(int*) malloc (sizeof(int)* number_of_sensors)) == NULL)
		{
			fprintf(stderr,"work plane sensor: Out of memory in function \'read_in_header\'\n");
			exit(1);
		}
		for (i=0 ; i<( number_of_sensors) ; i++)
		{
			sensor_typ[i]=0; 	// array determines the sensor type
			if(OutputUnits==1)
			{
				sensor_unit[i]=2;	//set sensor units to irradiance
			}
			else{
				sensor_unit[i]=0;	//default sensors are illuminance
			}	
		}


		BlindGroup=(int**) malloc (sizeof(int*)* 10);
		for (i=0 ; i<10 ; i++){
			BlindGroup[i] =(int*) malloc (sizeof(int)* number_of_sensors);
		}
		for (i=0 ; i<10 ; i++){
			for (j=0 ; j< number_of_sensors ; j++){
				BlindGroup[i][j]=0;
			}
		}

		LightingGroup=(int**) malloc (sizeof(int*)* 11);
		for (i=0 ; i<=10 ; i++){
			LightingGroup[i] =(int*) malloc (sizeof(int)* number_of_sensors);
		}
		for (i=0 ; i<=10 ; i++){
			for (j=0 ; j< number_of_sensors ; j++){
				LightingGroup[i][j]=0;
			}
		}



		sensor_file_info=(char**) malloc (sizeof(char*)* number_of_sensors);
		for (i=0 ; i<number_of_sensors ; i++){
			sensor_file_info[i] =(char*) malloc (sizeof(float)* 100); // up to 100 characters in the string
			sensor_file_info[i]="0";
		}





	

	//===================
	// DGP second reading
	//===================

	
	if( strcmp(viewpoint_file,"")){
		sprintf(keyword,"%s%s",project_directory,viewpoint_file);
		TEST_FILE = fopen(keyword, "r");
		if ( TEST_FILE != NULL) {
			close_file(TEST_FILE);
			strcpy(viewpoint_file,keyword);
		} else {
	    	TEST_FILE = fopen(viewpoint_file, "r");
			if ( TEST_FILE != NULL){
				close_file(TEST_FILE);
			} else {
				printf("FATAL INPUT ERROR: Viewpoint file %s does not exist.",viewpoint_file);
				exit(1);
			}
		}
	    number_of_view_points=number_of_lines_in_file(viewpoint_file);
	    number_of_views=number_of_view_points;
	}


	for (i = 0; i < 10; i++)
	{
		if (strcmp(AnnualShadingProfile[i], "")){
			sprintf(keyword, "%s%s", project_directory, AnnualShadingProfile[i]);
			strcpy(AnnualShadingProfile[i], keyword);
		}
	}

	if( strcmp(da_file,"")){
		sprintf(keyword,"%s%s",project_directory,da_file);
		strcpy(da_file,keyword);
	}
	if( strcmp(da_availability_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,da_availability_active_RGB_file);
		strcpy(da_availability_active_RGB_file,keyword);
	}
	if( strcmp(da_availability_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,da_availability_passive_RGB_file);
		strcpy(da_availability_passive_RGB_file,keyword);
	}
	if( strcmp(da_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,da_active_RGB_file);
		strcpy(da_active_RGB_file,keyword);
	}
	if( strcmp(da_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,da_passive_RGB_file);
		strcpy(da_passive_RGB_file,keyword);
	}
	if( strcmp(df_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,df_RGB_file);
		strcpy(df_RGB_file,keyword);
	}
	if( strcmp(con_da_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,con_da_active_RGB_file);
		strcpy(con_da_active_RGB_file,keyword);
	}
	if( strcmp(con_da_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,con_da_passive_RGB_file);
		strcpy(con_da_passive_RGB_file,keyword);
	}
	if( strcmp(DA_max_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,DA_max_active_RGB_file);
		strcpy(DA_max_active_RGB_file,keyword);
	}
	if( strcmp(DA_max_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,DA_max_passive_RGB_file);
		strcpy(DA_max_passive_RGB_file,keyword);
	}
	if( strcmp(UDI_100_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_100_active_RGB_file);
		strcpy(UDI_100_active_RGB_file,keyword);
	}
	if( strcmp(UDI_100_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_100_passive_RGB_file);
		strcpy(UDI_100_passive_RGB_file,keyword);
	}
	if( strcmp(UDI_100_2000_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_100_2000_active_RGB_file);
		strcpy(UDI_100_2000_active_RGB_file,keyword);
	}
	if( strcmp(UDI_100_2000_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_100_2000_passive_RGB_file);
		strcpy(UDI_100_2000_passive_RGB_file,keyword);
	}
	if( strcmp(UDI_2000_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_2000_active_RGB_file);
		strcpy(UDI_2000_active_RGB_file,keyword);
	}
	if( strcmp(UDI_2000_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,UDI_2000_passive_RGB_file);
		strcpy(UDI_2000_passive_RGB_file,keyword);
	}
	if( strcmp(DSP_active_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,DSP_active_RGB_file);
		strcpy(DSP_active_RGB_file,keyword);
	}
	if( strcmp(DSP_passive_RGB_file,"")){
		sprintf(keyword,"%s%s",project_directory,DSP_passive_RGB_file);
		strcpy(DSP_passive_RGB_file,keyword);
	}
	if( strcmp(df_file,"")){
		sprintf(keyword,"%s%s",project_directory,df_file);
		strcpy(df_file,keyword);
	}
	if( strcmp(rshow_da_file,"")){
		sprintf(keyword,"%s%s",project_directory,rshow_da_file);
		strcpy(rshow_da_file,keyword);
	}
	if( strcmp(rshow_df_file,"")){
		sprintf(keyword,"%s%s",project_directory,rshow_df_file);
		strcpy(rshow_df_file,keyword);
	}

	if( strcmp(measured_occ,"")){
		sprintf(keyword,"%s%s",project_directory,measured_occ);
		strcpy(measured_occ,keyword);
	}
	
	if( strcmp(direct_sunlight_file,"")){
		sprintf(keyword,"%s%s",project_directory,direct_sunlight_file);
		strcpy(direct_sunlight_file,keyword);
	}
	if( strcmp(percentage_of_visible_sky_file,"")){
		sprintf(keyword,"%s%s",project_directory,percentage_of_visible_sky_file);
		strcpy(percentage_of_visible_sky_file,keyword);
	}
	if( strcmp(direct_glare_file,"")){
		sprintf(keyword,"%s%s",project_directory,direct_glare_file);
		strcpy(direct_glare_file,keyword);
	}
	if( strcmp(direct_radiance_file,"")){
		sprintf(keyword,"%s%s",project_directory,direct_radiance_file);
		strcpy(direct_radiance_file,keyword);
	}
	if( strcmp(el_file,"")){
		sprintf(keyword,"%s%s",project_directory,el_file);
		strcpy(el_file,keyword);
	}
	if( strcmp(el_file_short,"")){												//Inhereted from PennState Daysim
		sprintf(keyword,"%s%s",project_directory,el_file_short);
		strcpy(el_file_short,keyword);
	}
	if( strcmp(occ_output_file,"")){
		sprintf(keyword,"%s%s",project_directory,occ_output_file);
		strcpy(occ_output_file,keyword);
	}
	if( strcmp(thermal_sim_file_active,"")){
		sprintf(keyword,"%s%s",project_directory,thermal_sim_file_active);
		strcpy(thermal_sim_file_active,keyword);
	}
	if( strcmp(thermal_sim_file_passive,"")){
		sprintf(keyword,"%s%s",project_directory,thermal_sim_file_passive);
		strcpy(thermal_sim_file_passive,keyword);
	}

	if( strcmp(wea_data_short_file,"")){
		sprintf(keyword,"%s%s",project_directory,wea_data_short_file);
		strcpy(wea_data_short_file,keyword);
	}

	if( strcmp(dgp_check_file,"")){
		checkfile= 1;
		sprintf(keyword,"%s%s",tmp_directory,dgp_check_file);
		strcpy(dgp_check_file,keyword);
	}

	if( !strcmp(direct_radiance_file,"")){
		sprintf(direct_radiance_file,"%sdirect_radiance_file.tmp.rad",tmp_directory);
	}
	if( !strcmp(direct_radiance_file_2305,"")){
		sprintf(direct_radiance_file_2305,"%sdirect_radiance_file.tmp.2305suns.rad",tmp_directory);
	}


	if( copy_rtrace_options ) {
		memcpy( &directOptions, &rtrace_options, sizeof(RtraceOptions) );
		directOptions.illumination= DirectIllumination;

		memcpy( &diffuseOptions, &rtrace_options, sizeof(RtraceOptions) );
	}

	/*=======================*/
	/* second header reading */
	/*=======================*/

	HEADER_FILE=open_input(header_file);
	while( EOF != fscanf(HEADER_FILE,"%s", keyword)){
		// sensors can be of the following types:
		// 0	ignore for el. lighting calculation (DEFAULT)
		// 1	Reference sensors for lighting group 1 and blind group 1 (legacy option)
		//	A list of comma seperate strings, LG1,BG2,LG2 means: Reference sensor for lighting groups 1 a0nd 2 and blind group 1
		if( !strcmp(keyword,"sensor_file_info")){
			for (i=0 ; i<( number_of_sensors) ; i++){
				//fscanf(HEADER_FILE,"%d",&sensor_typ[i]);
				fscanf(HEADER_FILE,"%s",keyword_sensor);
				sensor_file_info[i]=keyword_sensor;

				//now we need to decompose the string into its comma separated elements
				if( strcmp(keyword_sensor,"0")){
					if( !strcmp(keyword_sensor,"1")){
						//legacy header files used to assign "0" for "BG1,LG1"
						BlindGroup[1][i]=1;
						BlindGroup[0][i]=1;
						LightingGroup[1][i]=1;
						LightingGroup[0][i]=1;
					}else{
						j=0;
						while(j<100) //there are up to 10 x "BG1,LG1," = 80 characters in the string
						if( sensor_file_info[i][j]=='B' || sensor_file_info[i][j]=='L' || sensor_file_info[i][j]=='C') //Blind or Lighting Group
						{

							if( sensor_file_info[i][j]=='B') //Blind Group
							{
								j=j+2;
								sprintf(keyword_sensor,"%c",sensor_file_info[i][j]);
								Blind_Group_index=atoi(keyword_sensor);
								if(Blind_Group_index==1 && sensor_file_info[i][j+1]=='0') //test if BG1 or BG10
								{
									Blind_Group_index=10;
									j++;
								}
									
								//test if the sensor is an internal blind sensor
								if(Blind_Group_index>=1 && Blind_Group_index<=NumberOfBlindGroups)
								{
									BlindGroup[Blind_Group_index][i]=1;
									BlindGroup[0][i]=1;
									//printf("Blind_Group[%d][%d] = %d\n",Blind_Group_index, i,BlindGroup[Blind_Group_index][i]);
								}else{
									fprintf(stdout,"FATAL ERROR reading Daysim header file - Blind Group Index (%d) out of range for sensor number %d. There are/is only %d blind group(s) specified under keuword 'shading'.\n",Blind_Group_index,i+1,NumberOfBlindGroups);
									exit(0);
								}
								//test if the sensor is an external blind sensor
								if(sensor_file_info[i][j+1]=='_' && sensor_file_info[i][j+2]=='E'&& sensor_file_info[i][j+3]=='x'&& sensor_file_info[i][j+4]=='t') 
								{
									BlindGroup[Blind_Group_index][i]=2;
									BlindGroup[0][i]=2;
									j=j+4;
								}
							}
							else if( sensor_file_info[i][j]=='C') //Critical Point
							{
								j=j+2;
								sprintf(keyword_sensor,"%c",sensor_file_info[i][j]);
								if(atoi(keyword_sensor)==1 && sensor_file_info[i][j+1]=='0') //test if CP1 or CP10
								{
									CriticalPoints[9][NumCPsensor[i]]=i;
									j++;
									NumCPsensor[9]++;
								}else{
									CriticalPoints[atoi(keyword_sensor)-1][NumCPsensor[atoi(keyword_sensor)-1]]=i;
									NumCPsensor[atoi(keyword_sensor)-1]++;
								}
							}
							else{ //Lighting Group
								j=j+2;
								sprintf(keyword_sensor,"%c",sensor_file_info[i][j]);
								Lighting_Group_index=atoi(keyword_sensor);
								if(Lighting_Group_index==1 && sensor_file_info[i][j+1]=='0') //test if BG1 or BG10
								{
									Lighting_Group_index=10;
									j++;
								}

								if(Lighting_Group_index>=1 && Lighting_Group_index<=NumberOfLightingGroups)
								{
									LightingGroup[Lighting_Group_index][i]=1;
									LightingGroup[0][i]=1;
								}else{
									fprintf(stdout,"FATAL ERROR reading Daysim header file - Lighting Group Index (%d) out of range for sensor number %d: \'%s\'\n",Lighting_Group_index,i,sensor_file_info[i]);
									exit(0);
								}
							}
							j++;
							if( sensor_file_info[i][j]!=',')
								j=100;
						}else{
							j++;
						}
					}
				}
			}
		}


	
		// sensors can have the following units:
		// 0	illuminance [lux] (DEFAULT)
		// 1	luminance [Cd/m2]
		// 2	irradiance [W/m2]
		// 3	radiance [W/m2 Ster]
		if( !strcmp(keyword,"sensor_file_unit")){
			for (i=0 ; i<( number_of_sensors) ; i++){
				fscanf(HEADER_FILE,"%d",&sensor_unit[i]);
			}
		}

		if( !strcmp(keyword,"DGP_Profiles") ){
				if(number_of_view_points>0)
				{
					//printf("%s %d\n",DaylightGlareProbability_ViewPoints_file,number_of_view_points);
					for (i=0 ; i<( NumBlindSettings) ; i++){
						fscanf(HEADER_FILE,"%s",DGP_Profiles_file[i]);
						//printf("DGP Profile[%d] %s\n",i,DGP_Profiles_file[i]);
						if(!check_if_file_exists(DGP_Profiles_file[i])){
							sprintf(keyword,"%s%s",project_directory,DGP_Profiles_file[i]);
							strcpy(DGP_Profiles_file[i],keyword);
							if(!check_if_file_exists(DGP_Profiles_file[i])){
								printf("FATAL INPUT ERROR - Read in header: DGP Profiles file %s does not exist.",DGP_Profiles_file[i]);
								exit(1);
							}
						}
					}
				}else{
					printf("FATAL INPUT ERROR - Read in header: No Daylight Glare Probability View Points provided of file is empty.");
					exit(1);
				}
		}

				else if(!strcmp( keyword, "ab")){							//Added by Craig Casey [PSU]
			fscanf(HEADER_FILE, "%f", &abPARAM);
		}
		else if(!strcmp( keyword, "ad")){
			fscanf(HEADER_FILE, "%f", &adPARAM);
		}
		else if(!strcmp( keyword, "as")){
			fscanf(HEADER_FILE, "%f", &asPARAM);
		}
		else if(!strcmp( keyword, "ar")){
			fscanf(HEADER_FILE, "%f", &arPARAM);
		}
		else if(!strcmp( keyword, "aa")){
			fscanf(HEADER_FILE, "%f", &aaPARAM);
		}
		else if(!strcmp( keyword, "lr")){
			fscanf(HEADER_FILE, "%f", &lrPARAM);
		}
		else if(!strcmp( keyword, "st")){
			fscanf(HEADER_FILE, "%f", &stPARAM);
		}
		else if(!strcmp( keyword, "sj")){
			fscanf(HEADER_FILE, "%f", &sjPARAM);
		}
		else if(!strcmp( keyword, "lw")){
			fscanf(HEADER_FILE, "%f", &lwPARAM);
		}
		else if(!strcmp( keyword, "dj")){
			fscanf(HEADER_FILE, "%f", &djPARAM);
		}
		else if(!strcmp( keyword, "ds")){
			fscanf(HEADER_FILE, "%f", &dsPARAM);
		}
		else if(!strcmp( keyword, "dr")){
			fscanf(HEADER_FILE, "%f", &drPARAM);
		}
		else if(!strcmp( keyword, "dp")){
			fscanf(HEADER_FILE, "%f", &dpPARAM);
		}

		
		
		
		fscanf(HEADER_FILE,"%*[^\n]");
		fscanf(HEADER_FILE,"%*[\n\r]");
	} //end second header file reading
	close_file(HEADER_FILE);


	weekday_occ_mea=first_weekday;

	/*=====================================================*/
	/* check whether a work plane illuminance sensor exists */
	/*=====================================================*/
	for (j=1 ; j<11 ; j++)
	{
		LightingGroupSensorSpecified[j]=0;
		BlindGroupSensorSpecified[j]=0;
		ExternalBlindGroupSensorSpecified[j]=0;
	}

	for (i=0 ; i<( number_of_sensors) ; i++)
	{
		for (j=1 ; j<=( NumberOfBlindGroups) ; j++)
		{
			if(BlindGroup[j][i]==1)
				BlindGroupSensorSpecified[j]=1;
			if(BlindGroup[j][i]==2)
				ExternalBlindGroupSensorSpecified[j]=1;
		}
		for (j=1 ; j<=( NumberOfLightingGroups) ; j++)
		{
			if(LightingGroup[j][i]==1)
				LightingGroupSensorSpecified[j]=1;
		}
		if(sensor_unit[i]==0)
			IllumianceSensorExists=1;
	}

	// if no Blind Group sensors, all illuminance sensor are set to this work place
	for (j=1 ; j<=( NumberOfBlindGroups) ; j++)
	{
		if(BlindGroupSensorSpecified[j]==0 && IllumianceSensorExists ==1)
		{
			for (i=0 ; i<(number_of_sensors) ; i++)
			{
				if(sensor_unit[i]==0)
					BlindGroup[j][i]=1;
			}
		}
	}
	// if no Lighting Group sensors, all illuminance sensor are set to this work place
	for (j=1 ; j<=( NumberOfLightingGroups) ; j++)
	{
		if(LightingGroupSensorSpecified[j]==0 && IllumianceSensorExists ==1)
		{
			for (i=0 ; i<(number_of_sensors) ; i++)
			{
				if(sensor_unit[i]==0)
					LightingGroup[j][i]=1;
			}
		}
	}
	
	for (j=1 ; j<=( NumberOfBlindGroups) ; j++)
	{
//		if(MaxNumberOfSettingsInBlindgroup<NumberOfSettingsInBlindgroup[j])
//			NumberOfSettingsInBlindgroup[j]=MaxNumberOfSettingsInBlindgroup;
	}



	
}
