/*  Copyright (c) 20013
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  Massachusetts Institute of Technology
 *  written by Christoph Reinhart
 */

/* ds_illum is a DAYSIM subprogram that calculates the luminance/irradiance
 * distribution of arbitrary skies based on time, location, direct and diffuse irradiances.
 */

#include <stdio.h>
#include <string.h>
#include <rtmath.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include  "paths.h"

#include  "ds_illum.h"
#include  "calculate_sky_patches.h"	/* calculates direct daylight coefficients for investigated site */
#include  "calculate_Perez.h"		/* calculates illumiances for daylight coefficients */
#include  "shadow_testing.h"		/* carries out a shadow testing for each direct daylight coefficient */
#include  "check_direct_sunlight.h"

#include  "sun.h"
#include  "fropen.h"
#include  "read_in_header.h"

#define BUF_SIZE	1024

/* information from input file */
/*site information */
int start_month, start_day;
int all_warnings=0; // optional switch for ds_illum. If activated, warnings for direct and
                    // diffuse input irradiances that lie within 15 minutes after sunrise or
                    // before sunset are written to standard error.
					// synopsis  all_warnings <integer n>
					// n =1  on
					// n=0   off [default]

// switches that decide whether luminance or solar radiation are calculated.
// by default only lumiances are calculated. The relevant input varialbe in the header file
// is: "sensor_unit"
int 		CalculateLuminance=1;
int 		CalculateSolarRadiation=1;
float start_hour, centrum_hour;
char	direct_file[1024]="";
char	ill_file[1024]="";
char	temp_octree[1024]="";
char	temp_rad[1024]="";
int number_dc_points=0;
float** dc;
int *shadow_testing_results;

int direct_view[10];

//#ifndef PROCESS_ROW
float*** dc_shading;
int*** dc_shading_next;
//#endif

float* SkyPatchLuminance;
float* SkyPatchSolarRadiation;
int* NextNonEmptySkyPatch;
float Dx_dif_patch[148],Dy_dif_patch[148],Dz_dif_patch[148]; /*direction of diffuse sky patches */
FILE *DIRECT_SUNLIGHT_FILE;
FILE *INPUT_DATAFILE;
FILE *SHADING_ILLUMINANCE_FILE[100]; // file pointer for the different blind settings defined in "shading"

/* introduce shading*/
int solar_pen=0; /*switch: if =1,
				   then the input direct and diffuse irradiances are changed into
				   dir=0 and dif -> reduction_dif*dif */
float lower_lat=0,upper_lat=0,lower_azi=0,upper_azi, reduction_diffuse=0;

/* i/o format */
//visible watt=0, solar watt=1, lumen=2
int	input_unities=1;	/*define the input for the calulation*/
/* Perez parameters: epsilon, delta =0*/
/* direct normal Irradiance [W/m^2] diffuse horizontal Irrad. [W/m^2] =1 */
/* direct horizontal Irradiance [W/m^2] diffuse horizontal Irrad. [W/m^2] =2 */
/* direct normal Illuminance [Lux] diffuse horizontal Ill. [Lux] =3*/
int	input_type=2; /* input data format */
/* month, day, hour, direct, diffuse =2*/
double lowest_altitude=2;		/* lowest solar altitude in degree */
char  *progname;

/*file names*/
char input_file[200]="", input_datafile[200]="", Perez_outputfile[200]="",direct_points_file[200]="";
char diffuse_points_file[200]="",  points_input_file[200]="";
FILE *PEREZ_OUTPUTFILE;
FILE *DC_FILE;
FILE *ILL_FILE;
/* diverse arrays */
float diffuse_pts[145][2]; /* position of sky patches of diffuse daylight coefficients */
float direct_pts[288][4];    /* position of sky patches of direct daylight coefficients if n.n.*/
float direct_calendar[13][25][4]; /* position of sky patches of direct daylight coefficients otherwise*/
float horizon[36];
float horizon_factor[145];
double  sundir[3];
float point_coefficients[6];

/* diverse functions */
int shadow_testing_new=0;
void process_dc_shading(int number_direct_coefficients);
void pre_process_dds_shadowtesting();
int number_of_blindGroup_combinations=0;


/* */
static struct dc_shading_coeff_s* dc_shading_coeff;
static struct dc_shading_coeff_s* init_dc_shading_coeff( int, int );


/*
 *
 *
 *
 */
int main( int argc, char **argv )
{
	int  i,j;
	int shadow_testing=0, dc_direct_resolution=0,dir_rad=0,dif_pts=0,dir_pts=0,number_direct_coefficients=0;
	char keyword[200]="";
	char sensor_file_old[1024]="";
	char horizon_data_file[200];
	FILE *INPUT_FILE;
	FILE *COMMAND;
	FILE *HORIZON_DATA_FILE;

	for (i=0;i<36;i++){horizon[i]=0;}
	for (i=0;i<145;i++){horizon_factor[i]=1;}
	progname = argv[0];

	if (argc == 1) {
		fprintf(stdout,"ds_illum: fatal error -  header file missing\n");
		fprintf(stdout,"start program with:  ds_illum  <header file>\n ");
		fprintf(stderr,"\t-dds use generalized daylight coefficient file format (dds)\n ");
		fprintf(stderr,"\t-s do a shadow test at eahc time step (only in combination with -dds option)\n ");
		exit(1);
	} else {
		//set header file
		strcpy(input_file, argv[1]);
		if( argc > 2 ) {
			//test if dds methd is invoked
			for( i= 2; i < argc; i++ ) {
				if( !strcmp(argv[i],"-dds") )
					dds_file_format = 1; // use dds file format for DCs
			}
			// test whether direct testing is invoked as well (works only together with '-dds' option
			for( i= 2; i < argc; i++ ) {
				if( !strcmp(argv[i],"-s") )	{
					if(dds_file_format)	{
						dds_file_format=2;
					} else {
						fprintf(stderr,"\t-s option can only be invoked in combination with -dds\n ");
						exit(1);
					}
				}
			}
			//test WHETEHR '-EXT' OPTION IS USED
			for( i= 2; i < argc; i++ ) {
				if( !strcmp(argv[i],"-ext") )
					// add extension to file names for *.pts, *dc, *.ill
					strcpy(file_name_extension,argv[++i]);
			}
		}
	}


	/* read in input file */
	read_in_header(input_file);

	//========================================
	// Modifiy file names for gen_dgp_profile
	// This option is invoked if ds_illum or gen_dc
	// are started with '-ext  ,extension string>'
	//========================================
	if(strcmp(file_name_extension,""))
	{
		sprintf(sensor_file_old,"%s",sensor_file);
		sprintf(sensor_file,"%s%s",sensor_file,file_name_extension);

		//get the  number of sensors of sensor file with extension
        number_of_sensors=number_of_lines_in_file(sensor_file);
		//change *.dc and *.ill file names so that the originals do not get overwirtten
		for (i=1 ; i<=TotalNumberOfDCFiles ; i++){
			sprintf(shading_dc_file[i],"%s%s",shading_dc_file[i],file_name_extension);
			sprintf(shading_illuminance_file[i],"%s%s",shading_illuminance_file[i],file_name_extension);
		}

	}


{
	for (i=0 ; i<TotalNumberOfDCFiles ; i++)
	{
		sprintf(shading_dc_file[i],"%s",shading_dc_file[i+1]);
		//sprintf(shading_illuminance_file[i],"%s",shading_illuminance_file[i+1]);
		sprintf(shading_variant_name[1],"%s",shading_variant_name[i+1]);

	}
}


	if(dds_file_format) {
		if(!strcmp(DDS_sensor_file,"")){
			printf("FATAL ERROR - ds_illum: variable DDS_sensor_file not provided in header file.\n");
			exit(0);
		}
		if(!strcmp(DDS_file,"")){
			printf("FATAL ERROR - ds_illum: variable DDS_file not provided in header file.\n");
			exit(0);
		}
	}


	if(ShadingSpecified!=1){
		fprintf(stderr,"Fatal Error ds_illum: shading varialbe not in header file/n");
		exit(0);
	}



	strcpy(input_datafile,wea_data_short_file);
	if(dc_coupling_mode == 2)
		shadow_testing=1;
	input_unities=wea_data_short_file_units;



	INPUT_FILE=open_input(input_file);
	while( EOF != fscanf(INPUT_FILE,"%s", keyword)){
		if( !strcmp(keyword,"all_warnings") ){
			fscanf(INPUT_FILE,"%d",&all_warnings);
		}
		if( !strcmp(keyword,"horizon_data_out") ){
			fscanf(INPUT_FILE,"%s",horizon_data_file);/* explicit horizon  data */
			HORIZON_DATA_FILE=open_input(horizon_data_file);
			for (i=0;i<36;i++){
				fscanf(HORIZON_DATA_FILE,"%f ",&horizon[i]);
				if(horizon[i]>24){
					printf("WARNING ds_illum: horizon > 24 Deg. Horizon is set to 24 Deg!/n");
					horizon[i]=24;
				}
				if(horizon[i]<0){
					printf("WARNING ds_illum: negative horizon. Horizon is set to 0 Deg!/n");
					horizon[i]=0;
				}
			}
			i=close_file(HORIZON_DATA_FILE);
			get_horizon_factors();
		}
		if( !strcmp(keyword,"direct_points") ){
			fscanf(INPUT_FILE,"%s",direct_points_file);dir_pts=1;
		}
		if( !strcmp(keyword,"solar_pen") ){
			fscanf(INPUT_FILE,"%f %f %f %f %f",&lower_lat,&upper_lat,&lower_azi,&upper_azi, &reduction_diffuse);
			solar_pen=1;
			lower_lat*=0.01745329;
			upper_lat*=0.01745329;
			lower_azi*=0.01745329;
			upper_azi*=0.01745329;
		}
		if( !strcmp(keyword,"diffuse_points") ){
			fscanf(INPUT_FILE,"%s", diffuse_points_file); dif_pts=1;
		}
		if( !strcmp(keyword,"dc_direct_resolution") ){dc_direct_resolution=1;}
		if( !strcmp(keyword,"perez_output") ){
			fscanf(INPUT_FILE,"%s", Perez_outputfile);
			fscanf(INPUT_FILE,"%d",&OutputUnits);
			fscanf(INPUT_FILE,"%d",&dc_coupling_mode);
			fscanf(INPUT_FILE,"%f",&dir_threshold);
			fscanf(INPUT_FILE,"%f",&dif_threshold);
		}
	}

	

	/* consistency checks */
	if (fabs(s_meridian - s_longitude) > 30 * DTR){
		fprintf(stdout,
				"ds_illum: warning -  %.1f hours btwn. standard meridian and longitude\n",
				(s_longitude-s_meridian)*12/PI);
		exit(1);
	}
	i=0;

	/* calculate sky patches*/
	calculate_sky_patches(&dc_direct_resolution, &dir_rad,&dif_pts,&dir_pts,&number_direct_coefficients);

	if(dds_file_format) // new dds file format chosen
		number_direct_coefficients= 145+1+145+2305-146;

	if(dds_file_format==2)  //direct shadow testing for each sensor point
		pre_process_dds_shadowtesting();

	/* alloc memory if dc file is given */
	process_dc_shading(number_direct_coefficients);

	// generate building scene and name files
	if(dc_coupling_mode==2 ) {
		sprintf(temp_octree,"%s%s.shadow_testing.oct",tmp_directory,project_name);
		sprintf(temp_rad,"%s.shadow_testing_temp_sensors.pts",sensor_file);
	}

	/* calculate diffuse sky patch directions Dx_dif_patch... */
	for (j=0 ; j<145 ; j++){
		Dx_dif_patch[j]=cos((0.017453292)*diffuse_pts[j][1])*cos((0.017453292)*(90-diffuse_pts[j][0]));
		Dy_dif_patch[j]=sin((0.017453292)*diffuse_pts[j][1])*cos((0.017453292)*(90-diffuse_pts[j][0]));
		Dz_dif_patch[j]=sin((0.017453292)*(90-diffuse_pts[j][0]));
	}
	if(dds_file_format) // new dds file format chosen
		{
			Dx_dif_patch[145]=0.9397;
			Dy_dif_patch[145]=0.0;
			Dz_dif_patch[145]=-0.342;
		}else{
		Dx_dif_patch[145]=0.9961946;
		Dy_dif_patch[145]=0.0;
		Dz_dif_patch[145]=-0.087156;
		Dx_dif_patch[146]=0.9397;
		Dy_dif_patch[146]=0.0;
		Dz_dif_patch[146]=-0.342;
		Dx_dif_patch[147]=0.0;
		Dy_dif_patch[147]=0.0;
		Dz_dif_patch[147]=-1.0;
	}

	/* calculate Perez if chosen */
	calculate_perez(&shadow_testing, &dir_rad,&number_direct_coefficients);

	if(dc_coupling_mode==2  ) //delete temporary oct and rad files
		{
			COMMAND = fopen(temp_octree, "r");
			if ( COMMAND != NULL){
				close_file(COMMAND);
				remove(temp_octree);
			}else{close_file(COMMAND);}
			COMMAND = fopen(temp_rad, "r");
			if ( COMMAND != NULL){
				close_file(COMMAND);
				remove(temp_rad);
			}else{close_file(COMMAND);}
		}
	return 0;
}


void process_dc_shading(int number_direct_coefficients)
{
	int i,j,number_patches,k;
	int c;
	int number_of_elements_in_DC_file=0;
	int number_of_dc_lines=0;
	int number_of_DC_lines;
	int last_element_was_seperator;
	int number_of_diffuse_and_ground_DC=148;
	char line_string[2000000]="";
	char CurrentDC_FileName[2000]="";
//#ifndef PROCESS_ROW
	int i_last;
//#endif

	// This function reads in the daylight cofficients from a DC file.
	if(dds_file_format)
		number_of_diffuse_and_ground_DC=146;

	// At first the required arrays are dynamically allocated.

	number_patches=number_direct_coefficients+number_of_diffuse_and_ground_DC;

	/* malloc coefficients[number_patches][number_points]*/
//#ifndef PROCESS_ROW
	dc_shading=(float***) malloc (sizeof(float**)*TotalNumberOfDCFiles);
	for (i=0 ; i<(TotalNumberOfDCFiles) ; i++){
		dc_shading[i]=(float**)malloc (sizeof(float*)*number_of_sensors);
		if( dc_shading == NULL) {
			fprintf( stderr, "ds_shading: Out of memory in function \'process_dc_shading\'\n");
			exit(1);
		}

		for ( k= 0 ; k < number_of_sensors; k++ ) {
			dc_shading[i][k]=(float*)malloc (sizeof(float)*number_patches);
			if( dc_shading[i][k] == NULL) {
				fprintf( stderr, "ds_shading[%d][%d]: Out of memory in function \'process_dc_shading\'\n", i, k);
				exit(1);
			}
		}
	}
	/* malloc coefficients[number_patches][number_points]*/
	dc_shading_next=(int***) malloc (sizeof(int**)*TotalNumberOfDCFiles);
	if( dc_shading_next == NULL) {
		fprintf( stderr, "ds_shading_next: Out of memory in function \'process_dc_shading\'\n");
		exit(1);
	}
	for (i=0 ; i < TotalNumberOfDCFiles; i++){
		dc_shading_next[i]=(int**)malloc (sizeof(int*)*number_of_sensors);
		if( dc_shading_next[i] == NULL) {
			fprintf( stderr, "ds_shading_next[%d]: Out of memory in function \'process_dc_shading\'\n", i );
			exit(1);
		}
		for (k=0 ; k<(number_of_sensors) ; k++){
			dc_shading_next[i][k]=(int*)malloc (sizeof(int)*number_patches);
			if( dc_shading_next[i][k] == NULL) {
				fprintf( stderr, "ds_shading_next[%d][%d]: Out of memory in function \'process_dc_shading\'\n", i, k );
				exit(1);
			}
		}
	}

	/* set to zero */
	for (i=0 ; i< number_patches; i++){
		for (j=0 ; j<number_of_sensors ; j++){
			for (k=0 ; k < TotalNumberOfDCFiles; k++){
				dc_shading[k][j][i]=0;
				dc_shading_next[k][j][i]=0;
			}
		}
	}
//#endif
	
	SkyPatchLuminance=(float*) malloc (sizeof(float*)*number_patches);
	for (i=0 ; i< number_patches; i++)
		SkyPatchLuminance[i]=0;	
	
	SkyPatchSolarRadiation=(float*) malloc (sizeof(float*)*number_patches);
	for (i=0 ; i< number_patches; i++)
		SkyPatchSolarRadiation[i]=0;	
		

	// This array contains the next non-empty direct sky patch. The array is used in
	// calculate_Perez.c to steed up the linear superposition of the sky patches with
	// the dayliht coefficients.
	NextNonEmptySkyPatch=(int*) malloc (sizeof(int*)*number_patches);
	for (i=0 ; i< number_of_diffuse_and_ground_DC; i++)
		NextNonEmptySkyPatch[i]=i+1;

	for (i=number_of_diffuse_and_ground_DC ; i< number_patches; i++)
		NextNonEmptySkyPatch[i]=0;

	/* */
	dc_shading_coeff= init_dc_shading_coeff( TotalNumberOfDCFiles, number_patches );
	if( dc_shading_coeff == NULL )
		exit(1);

	//==============================
	// read in daylight coefficients
	//==============================
	for (k=0 ; k < TotalNumberOfDCFiles; k++) {
		{
			sprintf(CurrentDC_FileName,"%s",shading_dc_file[k]);
			if( ( DC_FILE= open_input(shading_dc_file[k]) ) == 0 )
			{
				fprintf(stderr,"ds_illum: fatal error: Cannot open DC file %s.\n",shading_dc_file[k]);
				exit(1);
			}

		}


		// Now we test whether the number of daylight coefficients per
		// sensor point corresponds to the number of daylight coefficients
		// expected for the latitude of the weather data site.

		//Test if DC file has any uncommented lines.
		number_of_DC_lines=0;
		while( (c= fgetc( DC_FILE )) != EOF ) {
			ungetc( c, DC_FILE );
			if( c != '#' && !iscntrl(c) )
				number_of_DC_lines++;

			fscanf( DC_FILE, "%*[^\n]\n" );
		}

		if(number_of_DC_lines==0) {
			fprintf(stderr,"ds_illum: fatal error: File %s does not contain any uncommented lines.\n\nIN CASE YOU ARE USING DAYSIM UNDER WINDOWS, THE \"RAPYPATH\" VARIALBE MIGHT NOT HAVE BEEN SET CORRECTLY. TO TEST THIS, OPEN A DOS PROMPT AND TYPE \'set\'. AMONG THE VARIALBLES DISPLAYED SHOULD BE \'Raypath c:\\Daysim\\lib\'.\nPLEASE REFER TO THE DAYSIM TUTORIAL FOR MORE INFORMATION...", CurrentDC_FileName );
			exit( 1 );
		}
		rewind(DC_FILE);

		// Now we read in the first uncommented line in the DC file
		while( (c= fgetc( DC_FILE )) != EOF ) {
			ungetc( c, DC_FILE );
			if( c != '#' && !iscntrl(c) ) {
				fgets(line_string,200000,DC_FILE); // TODO: does not work for lines longer then 200000
				break;
			} else {
				fscanf( DC_FILE, "%*[^\n]\n" );
			}
		}

		rewind(DC_FILE);


		// Now we count the number of DC for the last sensor.
		number_of_elements_in_DC_file=1; /*get number of daylight coefficients */
		last_element_was_seperator=1;
		for (i=0;i<200000;i++) {
			//make sure you don't exceed the bounds of the array
			if (line_string[i] == '\n')
				break; //exits the loop without completing the current iteration
			if( isblank(line_string[i]) )	{
				if(last_element_was_seperator ==0) {
					number_of_elements_in_DC_file++;
					last_element_was_seperator=1;
				}
			}else{
				last_element_was_seperator=0;
			}
		}
		if(last_element_was_seperator ==1)
			number_of_elements_in_DC_file--;

		//Now we test whether the latitude for DC and weather file match.
		if( number_of_elements_in_DC_file != number_patches ) {
			fprintf(stderr,"ds_illum: fatal error: The number of daylight coefficients in file %s is %d and should be %d according to the latitude given in the header file.\n",shading_dc_file[k], number_of_elements_in_DC_file , number_patches);
			exit( 1 );
		}
		//		rewind(DC_FILE);

		//read in DC coefficients
		j= 0;
		number_of_dc_lines=0;
		while( (c= fgetc( DC_FILE )) != EOF ) {
			//if(!iscntrl(c))
			ungetc( c, DC_FILE );
			if( c != '#' && !iscntrl(c)  ) {
				for ( i= 0 ; i < number_patches ; i++ ) {
//#ifndef PROCESS_ROW
					fscanf(DC_FILE,"%f",&dc_shading[k][j][i]);
//#else
//					float x;
//					fscanf(DC_FILE,"%f",&x);
//
//#endif
				}
//#ifndef PROCESS_ROW
				i_last=0;
				dc_shading_next[k][j][0]=number_patches;

				for (i=1 ; i<number_patches ; i++) {
					if( dc_shading[k][j][i] > 0.0 ) {
						dc_shading_next[k][j][i_last]=i;
						dc_shading_next[k][j][i]=number_patches;
						i_last=i;
					}
				}
//#endif
				j++;
			} else {
				fscanf( DC_FILE, "%*[^\n]\n" );
			}
		}

		close_file(DC_FILE);

		number_of_dc_lines= j;

		if(number_of_dc_lines!=number_of_sensors) {
			fprintf(stderr,"ds_illum: fatal error: The number of daylight coefficient sets in file %s is %d and does not correspond to the number of sensors in the sensor point file (%s) which is %d.\n",shading_dc_file[k], number_of_dc_lines , sensor_file,number_of_sensors);
			exit( 1 );
		}
	}

}


/*
 *
 */
static struct dc_shading_coeff_s* init_dc_shading_coeff( int blinds, int coefficients ) {
	int i;
	struct dc_shading_coeff_s* dcsc=
		(struct dc_shading_coeff_s*)malloc( sizeof(struct dc_shading_coeff_s)
											*TotalNumberOfDCFiles );
	if( dcsc == NULL ) {
		perror( "failed to allocate memory for the shading coefficients" );
		return NULL;
	}

	for( i= 0; i < blinds; i++ ) {
		int j;

		{
			if( (dcsc[i].fp= open_input( shading_dc_file[i] ) ) == NULL ) {
				printf("failed to open shading dc file [%d]: '%s'",i,shading_dc_file[i]);
				perror( "failed to open shading dc file ");
				return NULL;
			}

			if( (dcsc[i].data.dc= (float*)malloc( sizeof(float)*coefficients )) == NULL ) {
				perror( "failed to allocate memory for daysim coefficients" );
				return NULL;
			}
			if( (dcsc[i].data.next= (int*)malloc( sizeof(int)*coefficients )) == NULL ) {
				perror( "failed to allocate memory for daysim coefficients" );
				return NULL;
			}

			for( j= 0; j < coefficients; j++ ) {
				dcsc[i].data.dc[j]= 0.0;
				dcsc[i].data.next[j]= 0;
			}

			dcsc->data.coefficients= coefficients;
		}
	}

	return dcsc;
}


/*
 *
 */
int dc_shading_coeff_rewind( int blind ) {
	if( blind >= TotalNumberOfDCFiles || blind < 0 ) {
		fprintf( stderr, "rewind: invalid blind index\n" );
		return 1;
	}
	rewind( dc_shading_coeff[blind].fp );
	return 0;
}

/*
 * read in one line of coefficients.
 */
struct dc_shading_coeff_data_s* dc_shading_coeff_read( int blind ) {
	struct dc_shading_coeff_data_s* ret;
	char buf[BUF_SIZE];
	struct dc_shading_coeff_s* dcsc;

	if( blind >= TotalNumberOfDCFiles || blind < 0 ) {
		fprintf( stderr, "read in: invalid blind index\n" );
		return NULL;
	}

	dcsc= &dc_shading_coeff[blind];

	for( ;; ) {
		buf[0]= fgetc( dcsc->fp );
		if( buf[0] == '#' ) { /* skip line */
			while( fgets( buf, BUF_SIZE, dcsc->fp ) && buf[strlen(buf) - 1] != '\n' );
		} else {
			int i;
			int i_last;

			ungetc( buf[0], dcsc->fp );

			ret= &dcsc->data;
			for( i= 0; i < dcsc->data.coefficients; i++ ) {
				if( fscanf( dcsc->fp, "%f", &dcsc->data.dc[i] ) != 1 ) {
					fprintf( stderr, "failed to read daylight coefficient\n");
					ret= NULL;
					break;
				}
			}

			i_last=0;
			dcsc->data.next[0]= dcsc->data.coefficients;
			for( i= 1; i < dcsc->data.coefficients; i++ ) {
				if( dcsc->data.dc[i] > 0.0 ) {
					dcsc->data.next[i_last]= i;
					dcsc->data.next[i]= dcsc->data.coefficients;
					i_last= i;
				} else {
					dcsc->data.next[i]= 0;
				}
			}
			break;
		}
	}
	return ret;
}


void pre_process_dds_shadowtesting()
{
	//This function generates an array that is used for direct shadow testing for
	// direct sun positions. It first reads in the weather file  (*.wea) and then
	// adds a light source at the position of the sun for each considered time step
	// in a Radiance file (*.rad).

	// In step two it runs an ambient aobunce =1 shadow test for each usn position.
	// Since the cutom Daysim version of rtrace called rtrace_dc_2305 can only consider
	// up to 2355 direct sun positions, an rtrace_dc_2305 run is started every 2350 sun positions.


	FILE *WEA= NULL, *POINTS= NULL, *RADFILE_OUTSIDE= NULL;
	char befehl[1024]="";
	char octree[200]="";
	char direct_sunlight_file_tmp[1024]="";
	char RadFile_Outside[1024]="";
	char buf[1024];
	int jd,i=0,j=0,k=0;
	int month, day;
	int LightSourceCounter=0,NumberofLightSourcesInWeaFile=0;
	int NumberofEntriesInWeaFile=0;
	int WeatherFileCounter=0;
	int StartNewRadianceSunPositionFile=1;
	int LastLightSourceCounter=0;
	float hour;
	float solar_time=0,sd;
	float sunrise,sunset;


	//======
	//Step 1
	//======
	// Go through weatehr file once and identify the number of direct elegible sun psoitons in weather file.
	// To be elegible a sun positon

	WEA=open_input(wea_data_short_file);
	fscanf(WEA,"%s",befehl);
	  	if( !strcmp(befehl,"place") ) // check weather the climate file as a header
	  	{   // skip header
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		}else{
			rewind(WEA);
		}
  	while(fscanf(WEA,"%d %d %f %f %f",&month,&day,&hour,&dir,&dif) != EOF)
  	{
		NumberofEntriesInWeaFile++;

		//calculate center of time interval
		sunrise=12+12-stadj(jdate(month, day))-solar_sunset(month,day);
		sunset=solar_sunset(month,day)-stadj(jdate(month, day));
		if( ( hour-(0.5*time_step/60.0)<=sunrise ) && ( hour +(0.5*time_step/60.0)> sunrise )){
			hour=0.5*(hour +(0.5*time_step/60.0) )+0.5*sunrise;
		}else{
			if( ( hour-(0.5*time_step/60.0)<=sunset ) && ( hour +(0.5*time_step/60.0)> sunset )){
					hour=0.5*(hour -(0.5*time_step/60.0) )+0.5*sunset;
			}
		}


		/* get sun position */
       	jd= jdate(month, day);
		sd=sdec(jd);
		solar_time=hour+stadj(jdate(month, day));
		alt = salt( sd,solar_time);
		azi = sazi(sd,solar_time);
  		if(alt >= 0 && dir > dir_threshold && dif >=dif_threshold){
			NumberofLightSourcesInWeaFile++;
        }
  	}
	close_file(WEA);

	//allocate memory for the direct-direct shading array
	//dc_ab0[sensor][skycondition][BlindGroup]
	dc_ab0=(float***) malloc (sizeof(float**)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++)
		dc_ab0[i]=(float**) malloc (sizeof(float*)*NumberofLightSourcesInWeaFile);
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<NumberofLightSourcesInWeaFile ; j++)
			dc_ab0[i][j]=(float*) malloc (sizeof(float)*TotalNumberOfDCFiles);

	//initialize array
	for (i=0 ; i<number_of_sensors ; i++)
		for (j=0 ; j<LightSourceCounter ; j++)
			for (k=0 ; k<TotalNumberOfDCFiles ; k++)
				dc_ab0[i][j][k]=0;




	// go through weather file and generate:
	// radiance file with suns at all required positions



	WEA=open_input(wea_data_short_file);
	fscanf(WEA,"%s",befehl);
	if( !strcmp(befehl,"place") ) // check weather the climate file as a header
	  	{   // skip header
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
			fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		}else{
		rewind(WEA);
	}



	for (WeatherFileCounter=0 ; WeatherFileCounter<NumberofEntriesInWeaFile ; WeatherFileCounter++)
	{
	  if(NumberofLightSourcesInWeaFile>0)
	    {
		if(LightSourceCounter<2300 && LightSourceCounter < NumberofLightSourcesInWeaFile)
		{
			if(StartNewRadianceSunPositionFile==1 ) //add header to Radiance sun position file
			{
				sprintf(RadFile_Outside,"%s%s_DDS_ShadowTestingRadianceFile_outside.tmp.rad",tmp_directory,project_name);
				RADFILE_OUTSIDE=open_output(RadFile_Outside);
				fprintf(RADFILE_OUTSIDE,"# ********************** Direct Shadowtesting File ************************** \n");
				fprintf(RADFILE_OUTSIDE,"# Radiance input file generate by DAYSIM subbprogram ds_ill for the \n");
				fprintf(RADFILE_OUTSIDE,"# direct shadowtesting simulation\n\n");
				StartNewRadianceSunPositionFile=0;
			}

			//read in header file
			fscanf(WEA,"%d %d %f %f %f",&month,&day,&hour,&dir,&dif);

 			//calculate center of time interval
			sunrise=12+12-stadj(jdate(month, day))-solar_sunset(month,day);
			sunset=solar_sunset(month,day)-stadj(jdate(month, day));
			if( ( hour-(0.5*time_step/60.0)<=sunrise ) && ( hour +(0.5*time_step/60.0)> sunrise )){
				hour=0.5*(hour +(0.5*time_step/60.0) )+0.5*sunrise;
			}else{
				if( ( hour-(0.5*time_step/60.0)<=sunset ) && ( hour +(0.5*time_step/60.0)> sunset )){
					hour=0.5*(hour -(0.5*time_step/60.0) )+0.5*sunset;
				}
			}


 			/* get sun position */
 	      	jd= jdate(month, day);
			sd=sdec(jd);
			solar_time=hour+stadj(jdate(month, day));
			alt = salt( sd,solar_time);
			azi = sazi(sd,solar_time);

 	 		if(alt >= 0 && dir > dir_threshold && dif >=dif_threshold){
				// radiance file with suns at all required positions
				LightSourceCounter++;
				fprintf(RADFILE_OUTSIDE,"#date: %d %d time %.3f\n",month,day,hour);
				fprintf(RADFILE_OUTSIDE,"void light solar%d\n0 \n0 \n3 1000 1000 1000\n\n",LightSourceCounter);
				fprintf(RADFILE_OUTSIDE,"solar%d source sun\n0\n0\n",LightSourceCounter);
				fprintf(RADFILE_OUTSIDE,"4 %f %f %f 0.533\n\n",-cos(alt)*sin(azi),-cos(alt)*cos(azi),sin(alt));

			}
		}

		if(LightSourceCounter ==2300 || LightSourceCounter == NumberofLightSourcesInWeaFile)
		{ // make oconv and read results into array
			close_file(RADFILE_OUTSIDE);
			StartNewRadianceSunPositionFile=1;

			// make octrees for all dc files and
			// run rtrace for shadow testing
			if(AdvancedBlindModel) // advanced blind model
			{
				for (i=0 ; i<TotalNumberOfDCFiles ; i++)
				{
					for (j=0 ; j<NumberOfSettingsInBlindgroup[i] ; j++)
					{
						//add'' to file names for Windows
						if( strcmp(BlindGroupGeometryInRadiance[j][i],"")){
							sprintf(befehl,"\"%s\"",BlindGroupGeometryInRadiance[j][i]);
							strcpy(BlindGroupGeometryInRadiance[j][i],befehl);;
						}

						// make octrees for all blind combinations
						// calculate number of blind combinations
						sprintf(octree,"%s%s.%d.ds_illum.DDS_ShadowTesting.file.oct",tmp_directory, BlindGroupName[i],j);
			  			sprintf(befehl,"%soconv -f %s %s %s \"%s\" > %s\n","",material_file, BlindGroupGeometryInRadiance[j][i], geometry_file, RadFile_Outside, octree);
						//printf("befehl:%s\n",befehl);
						POINTS=popen(befehl,"r");
						while( fscanf( POINTS, "%s", buf ) != EOF ) {
							printf("%s \n",buf);
						}
						pclose(POINTS);

						// run rtrace for shadow testing
						sprintf(direct_sunlight_file_tmp,"%s%s.%d.ds_illum.DDS_ShadowTesting.tmp",tmp_directory, BlindGroupName[i],j);
						calculate_shading_status_ab0(octree,direct_sunlight_file_tmp,LightSourceCounter);

						RTRACE_RESULTS[number_of_blindGroup_combinations]=open_input(direct_sunlight_file_tmp);

				    	//rename *ill and *dc files
				    	sprintf(shading_dc_file[number_of_blindGroup_combinations],"%s",BlindGroupDiffDC_File[j][i]);
		  				sprintf(shading_illuminance_file[number_of_blindGroup_combinations],"%s",BlindGroupIlluminanceProfiles[j][i]);

						number_of_blindGroup_combinations++;

						//deletete octrees
						remove(octree);

					}
				}
				//rename *ill and *dc files
				NumBlindSettings=number_of_blindGroup_combinations;

			}else{ // old blind model
				for (i=0 ; i<NumBlindSettings ; i++){

					//add'' to file names for Windows
					if( strcmp(shading_rad_file[i],"")){
						sprintf(befehl,"\"%s\"",shading_rad_file[i]);
						strcpy(shading_rad_file[i],befehl);
					}
					// make octrees for all blind combinations
					sprintf(octree,"\"%stmp.%s.ds_illum.DDS_ShadowTesting.file.oct\"",tmp_directory, shading_variant_name[i]);
			  		sprintf(befehl,"%soconv -f %s %s %s \"%s\" > %s\n","",material_file, shading_rad_file[i], geometry_file, RadFile_Outside, octree);
					POINTS=popen(befehl,"r");
					while( fscanf( POINTS, "%s", buf ) != EOF ) {
						printf("%s \n",buf);
					}
					pclose(POINTS);

					// calculate the status for all points at each time step using rtrace
					sprintf(direct_sunlight_file_tmp,"%s%s.ds_illum.DDS_ShadowTesting.tmp",tmp_directory, shading_variant_name[i]);
					calculate_shading_status_ab0(octree,direct_sunlight_file_tmp,LightSourceCounter);


					//open rtrace results files
					RTRACE_RESULTS[i]=open_input(direct_sunlight_file_tmp);

					//deletete octrees
					POINTS = fopen(octree, "r");
					if ( POINTS != NULL){
						close_file(POINTS);
						if(remove(octree))
						printf("ds_illum: Remove %s failed.\n",octree);
					}else{
						close_file(POINTS);
					}
			    }
			}  // old blind model

			//delete outside radiance file
			if(remove(RadFile_Outside))
				printf("ds_illum: Remove %s failed.\n",RadFile_Outside);

			//read direct-direct DC into a table
			//**********************************
			for (k=0 ; k<NumBlindSettings ; k++)
			{
				for (j=0 ; j<number_of_sensors ; j++)
				{
					for (i=LastLightSourceCounter ; i<LightSourceCounter+LastLightSourceCounter ; i++)
					{
						fscanf(RTRACE_RESULTS[k],"%f",&dc_ab0[j][i][k]);
					}
				}
			}

			for (k=0 ; k<NumBlindSettings ; k++)
				close_file(RTRACE_RESULTS[k]);

			LastLightSourceCounter+=LightSourceCounter;
			LightSourceCounter=0;


		}//end make oconv and read results into array

  }// if(NumberofLightSourcesInWeaFile>0)

	}// end of wheather file loop

	//printf("weather file closed (# of source in wea:%d) \n",NumberofLightSourcesInWeaFile);

	close_file(WEA);



}
