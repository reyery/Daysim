/*  Copyright (c) 2008
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

/* gen_dc is a DAYSIM subprogram used to calculate daylight coefficients */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "version.h"
#include "rterror.h"
#include "paths.h"
#include "fropen.h"
#include "read_in_header.h"
#include "ds_constants.h"

#include "calculate_sky_patches_gen_dc.h"
#include "write_dds_files.h"


enum Task {
	CalculateDirect=		1,
	CalculateDiffuse=		2,
	MergeFiles=				4,
	SubstractDirectDirect=	8,
	AddDirectDirect=		16
};
enum Options {
	OptionsRtrace=			1,
	OptionsRtracePmap,
	OptionsMkpmap
};

int number_direct_coefficients=0;
int USE_RTRACE_DC_2305=0;
float diffuse_pts[145][3]; /* position of sky patches of diffuse daylight coefficients */
float direct_pts[288][4];    /* position of sky patches of direct daylight coefficients if n.n.*/
float direct_calendar[13][25][4]; /* position of sky patches of direct daylight coefficients otherwise*/
char Radiance_Parameters[99999];

/* file name where the sensor types are stored for read by rtrace */
static char sensorOptionFile[]= ".sensorXXXXXX";

int direct_direct_resolution=2305;

/*
 * Rotate measuring points around rotation axis(Z).
 * Modifies 'sensorFile'.
 */
static int rotateMeasuringPoints( char* sensorFile, int numberOfRotations,
								  enum RotationAxis *rotationAxis, double *angleInDegree )
{
	char   buffer[1024];
	double matrix[3][3];
	double data[6], rot[6];
	double angle;
	FILE   *infile, *outfile;
	int    i, j, offset=0, index=0;

	if( (infile= fopen( sensorFile, "r" )) == NULL ) {
		sprintf(errmsg, "cannot open file '%s' for reading.\n", sensorFile);
		error(SYSTEM, errmsg);
		return 0;
	}

	/* append ".rotate" to original sensor file name */
	strcat( sensorFile, ".rotated" );
	if( (outfile= fopen( sensorFile, "w" )) == NULL ) {
		sprintf(errmsg, "cannot open file '%s' for writing.\n", sensorFile);
		error(SYSTEM, errmsg);
		fclose( infile );
		return 0;
	}

	while( fgets( buffer, 1024, infile ) != NULL ) {
		if( buffer[0] != '#' ) {
			sscanf( buffer, "%lf %lf %lf %lf %lf %lf",
					&data[0], &data[1], &data[2], &data[3], &data[4], &data[5] );

			for( index= 0; index < numberOfRotations; index++ ) {
				switch( rotationAxis[index] ) {
				case XAxis: offset= 0; break;
				case YAxis: offset= 2; break;
				case ZAxis: offset= 1; break;
				}

				/* rotation matrix(around x-axis) */
				angle= radians(angleInDegree[index]);
				matrix[0][0]= 1.0; matrix[0][1]= 0.0;        matrix[0][2]= 0.0;
				matrix[1][0]= 0.0; matrix[1][1]= cos(angle); matrix[1][2]= -sin(angle);
				matrix[2][0]= 0.0; matrix[2][1]= sin(angle); matrix[2][2]= cos(angle);

				for( i= 0; i < 6; i++ )
					rot[i]= 0.0;

				for( i= 0; i < 3; i++ ) {
					for( j= 0; j < 3; j++ ) {
						rot[i]+=     matrix[(i + offset)%3][(j + offset)%3]*data[j];
						rot[i + 3]+= matrix[(i + offset)%3][(j + offset)%3]*data[j + 3];
					}
				}

				for( i= 0; i < 6; i++ )
					data[i]= rot[i];
			}
			fprintf( outfile, "%12.7g %12.7g %12.7g  %12.7g %12.7g %12.7g\n",
					 rot[0], rot[1], rot[2], rot[3], rot[4], rot[5] );
		}
	}

	fclose( outfile );
	fclose( infile );
	return 1;
}


/*
 * Merge the diffuse and direct daylight coefficient files
 */
static int mergeFiles( char* shadingFile, char directFilename[1024], char diffuseFilename[1024] )
{
	FILE *directFile, *diffuseFile, *mergedFile;
	char *line;
	float DiffuseDC=0, GroundContribution=0;
	int  read,i;


	/* open input files */

	directFile = open_input( directFilename );
	diffuseFile= open_input( diffuseFilename );


	/* open output files */
	mergedFile= open_output( shadingFile );

	if( (line= (char*)calloc( 1, 100240 )) == NULL )
		return 0;

	//dc header for non DDS file
	if(!dds_file_format){
		fprintf( mergedFile, "# merged daylight coefficients from %s and %s.\n",
				 diffuseFilename, directFilename );
		fprintf( mergedFile, "# latitude: %.2f\n", degrees(s_latitude));
	}

	if(dds_file_format){
		while( (read= fscanf( directFile, "%[^\n]\n", line )) != EOF ) {
			if( line[0] == '#' ) {
				//program assumes that the same lines are commented in both DC files
				fscanf( diffuseFile, "%[^\n]\n", line );
			} else {
				for (i=0;i<148; i++){
					fscanf( diffuseFile, "%e", &DiffuseDC );
					if(i<145)
						fprintf( mergedFile, "%e\t", DiffuseDC );
					if(i==145)
						GroundContribution= DiffuseDC ;
					if(i==146)
						GroundContribution+= DiffuseDC ;
					if(i==147){
						GroundContribution+= DiffuseDC ;
						fprintf( mergedFile, "%e\t", GroundContribution );
					}
				}
				fprintf( mergedFile, "%s\n", line );
			}
		}

	}else{
		while( (read= fscanf( directFile, "%[^\n]\n", line )) != EOF ) {
			if( line[0] == '#' ) {
				fprintf( mergedFile, "# direct:\t%s\n", &line[1] );
				//program assumes that the same lines are commented in both DC files
				fscanf( diffuseFile, "%[^\n]\n", line );
				fprintf( mergedFile, "# diffuse:\t%s\n", &line[1] );
			} else{
				for (i=0;i<148; i++){
					fscanf( diffuseFile, "%e", &DiffuseDC );
					fprintf( mergedFile, "%e\t", DiffuseDC );
				}
				fprintf( mergedFile, "%s\n", line );
			}
		}
	}

	close_file( mergedFile );
	close_file( diffuseFile );
	close_file( directFile );

	free( line );


	return 1;
}



/*
 * Read in DC file and substract ab0 direct-indirect DCs
 */
static int Substract_ab0( char* shadingFile, char* direct_ab0_Filename )
{
	FILE *direct_ab0_File, *mergedFile, *new_mergedFile;
	char new_merged_Filename[1024];
	float DiffuseDC=0, IndirectDirect_DC_withAB0=0,IndirectDirect_DC_AB0=0,resultingDC=0;
	int  i;



	direct_ab0_File= open_input( direct_ab0_Filename );
	//scan first two header lines
	fscanf( direct_ab0_File, "%*[^\n]\n" );
	fscanf( direct_ab0_File, "%*[^\n]\n" );

	if (!does_file_exist("merged file", shadingFile)) {
		sprintf(errmsg, "file %s does not exist", shadingFile);
		error(USER, errmsg);
	}
	mergedFile= open_input( shadingFile );

	/* open output file */
	sprintf(new_merged_Filename , "%s_tmp", direct_ab0_Filename );
	new_mergedFile= open_output( new_merged_Filename );


	while( fscanf(mergedFile, "%e", &DiffuseDC) != EOF ) {
		//printf diffuse and ground DCs
		fprintf( new_mergedFile, "%e\t", DiffuseDC );
		for (i=0;i<145; i++){
			fscanf( mergedFile, "%e", &DiffuseDC );
			fprintf( new_mergedFile, "%e\t", DiffuseDC );
		}
		for (i=0;i<145; i++){
			fscanf( mergedFile, "%e", &IndirectDirect_DC_withAB0 );
			fscanf( direct_ab0_File, "%e", &IndirectDirect_DC_AB0 );
			resultingDC= IndirectDirect_DC_withAB0-IndirectDirect_DC_AB0;
			fprintf( new_mergedFile, "%e\t", resultingDC);
		}
		fprintf( new_mergedFile, "\n");
	}
	close_file( mergedFile );
	close_file( new_mergedFile);
	close_file( direct_ab0_File );

	//write content of new merged file into old merged file
	mergedFile= open_output( shadingFile );
	new_mergedFile= open_input( new_merged_Filename );
	while( fscanf(new_mergedFile, "%e", &DiffuseDC) != EOF ) {
		//printf diffuse and ground DCs
		fprintf( mergedFile, "%e\t", DiffuseDC );
		for (i=0;i<290; i++){
			fscanf( new_mergedFile, "%e", &DiffuseDC );
			fprintf( mergedFile, "%e\t", DiffuseDC );
		}
		fprintf( mergedFile, "\n");
	}
	close_file( mergedFile );
	close_file( new_mergedFile);

	//empty temporary file
	new_mergedFile= open_output( new_merged_Filename );
	fprintf( new_mergedFile, "temporary file\t");
	close_file( new_mergedFile);

	return 1;
}


/*
 * Add direct-direct dc to DC file
 */
static int Add_ab0( char* shadingFile, char* direct_ab0_Filename )
{
	FILE *direct_ab0_File, *mergedFile, *new_mergedFile;
	char new_merged_Filename[1024];
	float DiffuseDC=0, DirectDirect_DC_AB0=0;
	int  i;


	direct_ab0_File= open_input( direct_ab0_Filename );
	//scan first two header lines and ignore them
	fscanf( direct_ab0_File, "%*[^\n]\n" );
	fscanf( direct_ab0_File, "%*[^\n]\n" );

	mergedFile= open_input( shadingFile );

	/* open output files */
	sprintf(new_merged_Filename , "%s_tmp", direct_ab0_Filename );
	new_mergedFile= open_output( new_merged_Filename );



	while( fscanf(mergedFile, "%e", &DiffuseDC) != EOF ) {
		//printf diffuse, ground and indirect-direct DCs
		fprintf( new_mergedFile, "%e\t", DiffuseDC );
		for (i=0;i<290; i++){
			fscanf( mergedFile, "%e", &DiffuseDC );
			fprintf( new_mergedFile, "%e\t", DiffuseDC );
		}
		//printf direct-direct DCs
		for (i=0;i<2305; i++){
			fscanf( direct_ab0_File, "%e", &DirectDirect_DC_AB0 );
			fprintf( new_mergedFile, "%e\t", DirectDirect_DC_AB0);
		}
		fprintf( new_mergedFile, "\n");
	}

	close_file( mergedFile );
	close_file( new_mergedFile);
	close_file( direct_ab0_File );

	//write content of new merged file into old merged file
	mergedFile= open_output( shadingFile );
	new_mergedFile= open_input( new_merged_Filename );
	while( fscanf(new_mergedFile, "%e", &DiffuseDC) != EOF ) {
		//printf diffuse and ground DCs
		fprintf( mergedFile, "%e\t", DiffuseDC );
		for (i=0;i<(290+2305); i++){
			fscanf( new_mergedFile, "%e", &DiffuseDC );
			fprintf( mergedFile, "%e\t", DiffuseDC );
		}
		fprintf( mergedFile, "\n");
	}
	close_file( mergedFile );
	close_file( new_mergedFile);

	//empty temporary file
	new_mergedFile= open_output( new_merged_Filename );
	fprintf( new_mergedFile, "temporary file\t");
	close_file( new_mergedFile);


	return 1;
}


// This function invokes the Radiance program "oconv" to create an octree from
// the material, geometry, shading, and sky RADIANCE input files.
// In case the keywords "rotate_x", "rotate_y", and/or "rotate_z" are given in
// Daysim header file, the Radiance scene is rotated accordingly using the
// RADIANCE program "xform".
void callOconv( const int ExtendedOutput, const char* binDir, const char* sky,
				const char* material, const char* geometry,	const char* shading,
				char* sensors, const char* octree )
{
	FILE *fp;
	char cmd[99999];
	char buf[10024];
	char radFiles[10024];
	int i;

	if( strlen( shading ) == 0 )
		sprintf( radFiles, "\"%s\" \"%s\"", material, geometry );
	else
		sprintf( radFiles, "\"%s\" \"%s\" \"%s\"", material, shading, geometry );

	if( rotationNumber > 0 ) {
		char xform[2024]= "xform ";

		for( i= 0; i < rotationNumber; i++ ) {
			switch( rotationAxis[i] ) {
			case XAxis:
				sprintf( xform, "\"%sxform\" -rx %f", binDir, rotationAngle[i] );
				break;
			case YAxis:
				sprintf( xform, "\"%sxform\" -ry %f", binDir, rotationAngle[i] );
				break;
			case ZAxis:
				sprintf( xform, "\"%sxform\" -rz %f", binDir, rotationAngle[i] );
				break;
			}
		}

		sprintf( xform, "%s %s > %s%srotated-scene.tmp.rad", xform, radFiles,tmp_directory,project_name );

		fp= popen( xform, "r" );
		while( fscanf( fp, "%s", buf ) != EOF ) {
			printf("%s \n",buf);
		}
		fclose( fp );

		rotateMeasuringPoints( sensors, rotationNumber, rotationAxis, rotationAngle );

		strcpy( radFiles, tmp_directory );
		strcat( radFiles, project_name );
		strcat( radFiles, "rotated-scene.tmp.rad" );

	}


	sprintf( cmd, "\"%soconv\" -f \"%s\" %s > \"%s\"", binDir, sky, radFiles, octree );
	//printf("%s\n",cmd);;

	if( ExtendedOutput )
		printf("%s\n", cmd );

	fp= popen( cmd, "r" );
	while( fscanf( fp, "%s", buf ) != EOF ) {
		printf("%s \n",buf);
	}
	pclose( fp );

}


/**
 *
 */
char* getOptionString( int type, const RtraceOptions* opts, char* options )
{
	int i=0;
	int DifferentSensorUnitTest=0;
	options[0]= '\0';

	switch( type ) {
	case OptionsRtracePmap:
		if( opts->pmap.globalCollect != 0 )
			sprintf( options, "%s -apg %s %d",
					 options, opts->pmap.globalPmap, opts->pmap.globalCollect );
		if( opts->pmap.causticCollect != 0 )
			sprintf( options, "%s -apc %s %d",
					 options, opts->pmap.causticPmap, opts->pmap.causticCollect );

	case OptionsRtrace:

/*         --------- for the old rtrace_dc version for windows the -sj option has to be supplied
                     the current rtrace_dc version uses -ss  instead (as rtrace does)
		     modified by J. Wienold 16.6.2011
		     */

#ifdef __MINGW32__
		sprintf( options, "%s -h %s%s -oc -aa %f -ab %d -ad %d -ar %d -as %d -dj %f -dr %d -ds %f  -lr %d -lw %f  -sj %f -st %f  %s  -L %f ",
				 options,
				 strlen(opts->rad.irradianceSwitch) ? "-" : "", opts->rad.irradianceSwitch,
				 opts->rad.aa, opts->rad.ab, opts->rad.ad, opts->rad.ar, opts->rad.as,
				 opts->rad.dj, opts->rad.dr, opts->rad.ds, opts->rad.lr, opts->rad.lw,
				 opts->rad.sj, opts->rad.st,
				 opts->rad.additional,
				 luminanceOfSkySegments );
#else
		sprintf( options, "%s -h %s%s -oc -aa %f -ab %d -ad %d -ar %d -as %d -dj %f -dr %d -ds %f  -lr %d -lw %f  -ss %f -st %f  %s  -L %f ",
				 options,
				 strlen(opts->rad.irradianceSwitch) ? "-" : "", opts->rad.irradianceSwitch,
				 opts->rad.aa, opts->rad.ab, opts->rad.ad, opts->rad.ar, opts->rad.as,
				 opts->rad.dj, opts->rad.dr, opts->rad.ds, opts->rad.lr, opts->rad.lw,
				 opts->rad.sj, opts->rad.st,
				 opts->rad.additional,
				 luminanceOfSkySegments );
#endif


		// append sensor types unless all sensors are illuminance sensors
		DifferentSensorUnitTest=0;
		for (i = 0; i < number_of_sensors; i++)	{
			if(sensor_unit[i]==1 || sensor_unit[i]==3) { /* lumiance or radiance sensor */
				DifferentSensorUnitTest=1;
				break;
			}
		}

		// tito will change this into -sen option instead
		if( DifferentSensorUnitTest ) {
			int sf_exists= 0;
#if defined(_WIN32) || defined(_WIN64)
			if (mktemp(sensorOptionFile) == NULL
				&& !(sf_exists= (access( sensorOptionFile, F_OK ) == 0)) ) {
#elif defined __MINGW32__
			if( mkstemps( sensorOptionFile, 0 ) == -1
				&& !(sf_exists= (access( sensorOptionFile, F_OK ) == 0)) ) {
#else
			if( mkstemp( sensorOptionFile ) == -1
				&& !(sf_exists= (access( sensorOptionFile, F_OK ) == 0)) ) {
#endif
				fprintf( stderr, "exists: %d\n", sf_exists );
				perror( "failed to create temporary filename" );
				return NULL;
			}

			sprintf( options, "%s @%s ",options, sensorOptionFile );

			if( !sf_exists ) { /* file does not exist from previous pass */
				FILE* fp= fopen( sensorOptionFile, "w" );
				if( fp == NULL )
					return NULL;

				fprintf( fp, "-U %d ", number_of_sensors );
				for (i = 0; i < number_of_sensors; i++)	{
					if(sensor_unit[i]==0 || sensor_unit[i]==2 ) // illuminance or irradiance
						fprintf( fp, "1 " );
					if(sensor_unit[i]==1 || sensor_unit[i]==3 ) // luminance or radiance
						fprintf( fp, "0 " );
				}
				fclose( fp );
			}
		}

		if( opts->illumination == DirectIllumination ){
			if(dds_file_format)
				sprintf( options, "%s -N %d -Dm -dt %f",
						 options, 145, opts->rad.dt );
			else
				sprintf( options, "%s -N %d -Dm -dt %f",
						 options, number_direct_coefficients, opts->rad.dt );
		} else if( opts->illumination == DiffuseIllumination ){
			sprintf( options, "%s -N %d -Dd", options, 148 );
		}
		break;
	case OptionsMkpmap:
		if( opts->pmap.report != 0 )                 /* time in s between reports */
			sprintf( options, "%s -t %d", options, opts->pmap.report );
		if( opts->pmap.globalPhotons != 0 )          /* global pmap */
			sprintf( options, "%s -apg %s %ld",
					 options, opts->pmap.globalPmap, opts->pmap.globalPhotons );
		if( opts->pmap.causticPhotons != 0 )         /* caustic pmap */
			sprintf( options, "%s -apc %s %ld",
					 options, opts->pmap.causticPmap, opts->pmap.causticPhotons );
		if( opts->pmap.predistribution != HUGE_VAL ) /* predistribution rate */
			sprintf( options, "%s -apD %f", options, opts->pmap.predistribution );
		if( strcmp( opts->pmap.photonPort, "" ) != 0 )
			sprintf( options, "%s -apo %s", options, opts->pmap.photonPort );

		if( opts->rad.dp != 0 )                      /* PDF samples/sr */
			sprintf( options, "%s -dp %f", options, opts->rad.dp );

		if( opts->rad.ds != 0.0 )
			sprintf( options, "%s -ds %f", options, opts->rad.ds );

		break;
	}

	return options;
}


/**
 *
 */
void createSkyFile( char *filename )
{
	FILE *skyFile;

	skyFile= open_output( filename );

	fprintf( skyFile, "void glow sky\n0\n0\n4  1000 1000 1000  0\n\n" );
	fprintf( skyFile, "sky source himmel\n0\n0\n4  0 0 1  360\n\n" );

	close_file( skyFile );
}


/*
 *
 */
void callMkpmap( const int ExtendedOutput, const char* binDir, const char* octree,
				 const char* dc, const RtraceOptions* opts )
{
	FILE *fp;
	char cmd[99999];
	char buf[1024];

	/* remove existing photon maps */
	remove( opts->pmap.globalPmap);
	remove(opts->pmap.causticPmap);

	getOptionString( OptionsMkpmap, opts, Radiance_Parameters );


	if( opts->illumination == DiffuseIllumination )
		sprintf( cmd, "\"%smkpmap\" %s -N %d -Dd %s	", binDir,
				 Radiance_Parameters, numberOfSkySegments, octree );
	else if( opts->illumination == DirectIllumination )
		sprintf( cmd, "\"%smkpmap\" %s -N %d -Dm %s	", binDir,
				 Radiance_Parameters, number_direct_coefficients, octree );

	if( ExtendedOutput )
		printf( "\t%s\n", cmd );

	// additional comment line confuses merge operation and ds_illum
	//	fp= fopen( dc, "w" );
	//	fprintf( fp, "# %s\n", cmd );
	//	fclose( fp );

	fp= popen( cmd, "r" );
	while( fscanf( fp, "%s", buf ) != EOF ) {
		printf("%s \n",buf);
	}
	pclose( fp );
}


/**
 *
 */
void callRtraceDC( const int ExtendedOutput, const char* binDir, char *AdditionalRaidanceParameters, const char* octree,
				   const char* sensorFile, const char* dc, const RtraceOptions* opts )
{
	FILE *fp;
	char cmd[99999];
	char buf[1024];


	if( opts->calculationMode == RtracePhotonMap ) { /* create photon map */
		callMkpmap( ExtendedOutput, binDir, octree, dc, opts );

		getOptionString( OptionsRtracePmap, opts, Radiance_Parameters );
	} else {
		getOptionString( OptionsRtrace, opts, Radiance_Parameters );
	}

	/* record radiance version */
	sprintf( cmd, "%srtrace_dc -version", binDir );

	fp= popen( cmd, "r" );
	fgets( buf, 1024, fp );
	pclose( fp );
	if(!USE_RTRACE_DC_2305){
		sprintf( cmd, "\"%srtrace_dc\" %s %s \"%s\" < \"%s\" >> \"%s\"",
				 binDir, Radiance_Parameters,AdditionalRaidanceParameters, octree, sensorFile, dc );
	} else {
		sprintf( cmd, "\"%srtrace_dc_2305\" %s %s \"%s\" < \"%s\" >> \"%s\"",
				 binDir, Radiance_Parameters,AdditionalRaidanceParameters, octree, sensorFile, dc );
	}

	if( ExtendedOutput )
		printf( "gen_dc: %s\n", cmd );

	/* create also new file when pmap, wienold, July 2012*/
	if( opts->calculationMode == RtracePhotonMap )
		fp= fopen( dc, "w" );
	else /* create new file */
		fp= fopen( dc, "w" );

	fprintf( fp, "# %s# %s\n", buf, cmd );
	fclose( fp );

	fp= popen( cmd, "r" );
	while( fscanf( fp, "%s", buf ) != EOF ) {
		printf("%s \n",buf);
	}
	pclose( fp );
}



/*
 *
 */
void usage( char* command ) {
	fprintf(stderr, "WARNING %s: input file missing\n", command);
	fprintf(stderr, "start program with:  %s  <header file>\n ", command);
	fprintf(stderr,"\t-dds use generalized daylight coefficient file format (dds)\n ");
	fprintf(stderr,"\t-dir calculate direct daylight coefficients only\n ");
	fprintf(stderr,"\t-dif calculate diffuse daylight coefficients only\n ");
	fprintf(stderr,"\t-substract substract direct-direct daylight coefficients only\n ");
	fprintf(stderr,"\t-add add direct-direct daylight coefficients only\n ");
	fprintf(stderr,"\t-paste direct and diffuse daylight coefficient files only\n ");
	fprintf(stderr,"\t-h reduced output\n ");
	//fprintf(stderr,"\t-res direct_direct_resolution (the default value is 2305)\n");
}


/**
 *
 */
int main( int argc, char **argv )
{
	int  i;
	int  ExtendedOutput=0;
	int no_individual_tasks=1;
	char skyPatch[1024];
   	enum Task task= 0;
	enum Illumination illumination;
	char octree[1024];
	char dcFile[1024];
	char dcFile_dir[1024];
	char dcFile_dif[1024];
	char tmpSensorFile[1024];
	char sensor_file_old[1000];


	if (argc == 1){
		usage(fixargv0(argv[0]));
		exit(1);
	}

	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

	read_in_header( argv[1] );
	if (argc == 2){  // gen_dc file.hea
		task= CalculateDirect | CalculateDiffuse | MergeFiles;
		illumination= DirectIllumination | DiffuseIllumination;
	}
	if( argc > 2 ) {
		for( i= 2; i < argc; i++ ) {
			if( !strcmp(argv[i],"-dds") ) {
				dds_file_format = 1; // use dds file format for DCs
				if(!strcmp(DDS_sensor_file,"")){
					error(USER, "variable DDS_sensor_file not provided in header file");
				}
				if(!strcmp(DDS_file,"")){
					error(USER, "variable DDS_file not provided in header file");
				}
			}
		}
		for( i= 2; i < argc; i++ ) {
			if( !strcmp(argv[i],"-dir") ) {
				task|= CalculateDirect;
				illumination= DirectIllumination;
				no_individual_tasks=0;
			} else if( !strcmp(argv[i],"-dif") ) {
				task|= CalculateDiffuse;
				illumination= DiffuseIllumination;
				no_individual_tasks=0;
			} else if( !strcmp(argv[i],"-paste") ) {
				task|= MergeFiles;
				no_individual_tasks=0;
			} else if( !strcmp(argv[i],"-substract" ) && dds_file_format) {
				task|= SubstractDirectDirect;
				no_individual_tasks=0;
			} else if( !strcmp(argv[i],"-add" )&& dds_file_format ) {
				task|= AddDirectDirect;
				no_individual_tasks=0;
			} else if( !strcmp(argv[i],"-h") ) {
				ExtendedOutput= 1;
			} else if( !strcmp(argv[i],"-f") ) {
				strcpy(bin_dir,argv[++i]);
			} else if( !strcmp(argv[i],"-ext") ){
				// add extension to file names for *.pts, *dc, *.ill
				strcpy(file_name_extension,argv[++i]);
			}
		}
		if( no_individual_tasks)
		{
			if(dds_file_format)
			{
				task= CalculateDirect | CalculateDiffuse | MergeFiles | SubstractDirectDirect | AddDirectDirect;
				illumination= DirectIllumination | DiffuseIllumination  ;
			}else{
				task= CalculateDirect | CalculateDiffuse | MergeFiles ;
				illumination= DirectIllumination | DiffuseIllumination  ;
			}
		}


	}

	//========================================
	// Modifiy file names for gen_dgp_profile
	// This option is invoked if ds_illum or gen_dc
	// are started with '-ext  ,extension string>'
	//========================================
	if(strcmp(file_name_extension,""))
	{
		//printf("EXTENSION name: %s\n",file_name_extension);
		//change point file name so that the original does not get overwritten
		sprintf(sensor_file_old,"%s",sensor_file);
		sprintf(sensor_file,"%s%s",sensor_file,file_name_extension);
	// No Copy needed here for gen_dgp_profile - the sensor file is already generated in gen_dgp_profile
	// Change by J. Wienold, 2011-06-16

	/*	copy_file(sensor_file_old,sensor_file);*/

		//change *.dc and *.ill file names so that the originals do not get overwirtten
		for (i=1 ; i<=TotalNumberOfDCFiles ; i++){
			sprintf(shading_dc_file[i],"%s%s",shading_dc_file[i],file_name_extension);
			sprintf(shading_illuminance_file[i],"%s%s",shading_illuminance_file[i],file_name_extension);
		}
	}

	if( simple_blinds_model == 1 ) // if the simple blind model is used the daylight
		TotalNumberOfDCFiles=1;   // coefficient files for blinds up and down are identical.



	if(dds_file_format){
		printf("*******************************************************\n");
		printf("*             DDS file format                         *\n");
		printf("*******************************************************\n");
	}

	if( task & CalculateDiffuse ) {
		printf("=======================================================\n");
		printf("= gen_dc: calculate diffuse daylight coefficients     =\n");
		printf("= (ignore WARNING: no light sources found)            =\n");
		printf("= (this simulation may take several minutes to hours) =\n");
		printf("=======================================================\n");
		sprintf( skyPatch, "%s/skyPatches.rad", tmp_directory );
		createSkyFile( skyPatch );//uniformly glowing sphere for diffuse calculation
	}

	if( (task & CalculateDirect ) || (task &  SubstractDirectDirect ) ) {
		printf("=======================================================\n");
		printf("= gen_dc: calculate direct daylight coefficients      =\n");
		printf("= (this simulation may take several minutes to hours) =\n");
		printf("=======================================================\n");
		if(dds_file_format){
			calculate_sky_patchesDDS(145,direct_radiance_file); // 145 sun positions for dir DC
		}else{
			calculate_sky_patches(direct_radiance_file); // ~65 representative sun positions for dir DC
		}
	}



	{
		for( i= 1 ; i <= TotalNumberOfDCFiles ; i++ ) {
			if(ExtendedOutput)
				fprintf(stdout,"daylight coefficients for: %s\n",shading_variant_name[i]);

			if( task & CalculateDiffuse ) {  /* Calculate Diffuse Daylight Coefficients */
				printf( "calculate diffuse daylight coefficients for variant %s...\n",
						shading_variant_name[i] );

				sprintf( octree, "%s/%s.dif.oct", tmp_directory, shading_variant_name[i] );
				sprintf( dcFile, "%s/%s.dif.dc", tmp_directory, shading_variant_name[i] );
				strcpy( tmpSensorFile, sensor_file );
				callOconv( ExtendedOutput, bin_dir, skyPatch, material_file, geometry_file,
						   shading_rad_file[i], tmpSensorFile, octree );

				callRtraceDC( ExtendedOutput, bin_dir, "", octree, tmpSensorFile, dcFile,
							  &diffuseOptions );
				remove(octree);
			}

			if( task & CalculateDirect ) {  /* Calculate Direct Daylight Coefficients */
				printf( "calculate %d direct daylight coefficients for variant %s...\n",
						number_direct_coefficients, shading_variant_name[i] );

				sprintf( octree, "%s/%s.dir.oct", tmp_directory, shading_variant_name[i] );
				sprintf( dcFile, "%s/%s.dir.dc", tmp_directory, shading_variant_name[i] );

				strcpy( tmpSensorFile, sensor_file );
				callOconv( ExtendedOutput, bin_dir, direct_radiance_file, material_file,
						   geometry_file, shading_rad_file[i], tmpSensorFile, octree );

				callRtraceDC( ExtendedOutput, bin_dir, "", octree, tmpSensorFile, dcFile, &directOptions );

				/* remove the octree file */
				if( access( octree, F_OK ) == 0 ) {
					if (remove(octree) == -1) {
						sprintf(errmsg, "failed to remove temporary file '%s'", octree);
						error(WARNING, errmsg);
					}
				}

			}



			if( task & MergeFiles ) {       /* merge lines of files */
				sprintf( dcFile_dif, "%s/%s.dif.dc", tmp_directory, shading_variant_name[i] );
				sprintf( dcFile_dir, "%s/%s.dir.dc", tmp_directory, shading_variant_name[i] );
				mergeFiles( shading_dc_file[i], dcFile_dir, dcFile_dif );
			}


			if( (task & SubstractDirectDirect) && dds_file_format) {  /* Substract Direct Daylight Coefficients */
				printf( "substract direct contribution from direct-indirect daylight coefficients for variant %s...\n",
						shading_variant_name[i] );

				sprintf( octree, "%s/%s.dir.oct", tmp_directory, shading_variant_name[i] );
				sprintf( dcFile, "%s/%s.dir.ab0.dc", tmp_directory, shading_variant_name[i] );

				strcpy( tmpSensorFile, sensor_file );
				callOconv( ExtendedOutput, bin_dir, direct_radiance_file, material_file,
						   geometry_file, shading_rad_file[i], tmpSensorFile, octree );

				callRtraceDC( ExtendedOutput, bin_dir, "-ab 0 ", octree, tmpSensorFile, dcFile, &directOptions );
				//substract ab0
				Substract_ab0(shading_dc_file[i], dcFile );
			}

			if( (task & AddDirectDirect) && dds_file_format) {  /* Add DirectDirect component */
				printf( "add direct-direct daylight coefficients for variant %s...\n",
						shading_variant_name[i] );

				calculate_sky_patchesDDS(2305,direct_radiance_file_2305); // 2305 sun positions for dir DC
				sprintf( octree, "%s/%s.dir.oct", tmp_directory, shading_variant_name[i] );
				sprintf( dcFile, "%s/%s.dir-dir.ab0.dc", tmp_directory, shading_variant_name[i] );

				strcpy( tmpSensorFile, sensor_file );
				callOconv( ExtendedOutput, bin_dir, direct_radiance_file_2305, material_file,
						   geometry_file, shading_rad_file[i], tmpSensorFile, octree );

				USE_RTRACE_DC_2305=1;
				callRtraceDC( ExtendedOutput, bin_dir, "-ab 0 -N 2305 ", octree, tmpSensorFile, dcFile, &directOptions );
				//add 2305 direct-direct coefficients
				Add_ab0(shading_dc_file[i], dcFile );
			}

		}
	}

	//###############
	//write DDS files
	//###############
	if(dds_file_format)	{
		write_dds_file( DDS_file,direct_direct_resolution,Radiance_Parameters);

		//generate DDS sensor file if file does not exist
		write_dds_sensor_file(DDS_sensor_file);
	}


	/* remove the sensor file? may be it's better to keep the file for evaluation */
	if( access( sensorOptionFile, F_OK ) == 0 ) {
		if (remove(sensorOptionFile) == -1) {
			sprintf(errmsg, "failed to remove temporary file '%s'", sensorOptionFile);
			error(WARNING, errmsg);
		}
	}

	return 0;
}

