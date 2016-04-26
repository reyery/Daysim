/*  Copyright (c) 2009
 *  written by Jan Wienold
 *  Fraunhofer ISE
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
//#include <strings.h>
#include <errno.h>

#include  "version.h"
#include  "paths.h"
#include  "fropen.h"
#include  "read_in_header.h"
#include  "ds_constants.h"


#define CMD_GEN_DC		"gen_dc"
#define CMD_DS_ILLUM	"ds_illum"
#define CMD_OCONV		"oconv"
#define CMD_RPICT		"rpict"
#define CMD_GENDAYLIT	"gendaylit"
#define CMD_EVALGLARE	"evalglare"

/*
 * structure for storing viewpoints
 */
typedef struct view_point {
	double x, y, z;
	double xdir, ydir, zdir;
} view_point;

/*
 * structure storing the current shading variant to process - its like an iterator
 */
typedef struct shading_variant {
	int variant;		/* current shading variant */
	int groupVariant;	/* current group variant */
} shading_variant;

/*
 * call gen_dc and ds_illum
 */
void preprocessing( char* header ) {
	char cmd[1024];
	FILE* fp1;
	FILE* fp2;

	sprintf( cmd, "%s %s -dif -ext .tmp", CMD_GEN_DC, header );
	fp1= popen( cmd, "r" );
	if( fp1 == NULL ) {
		fprintf( stderr, "'%s' failed\n", cmd );
		exit(1);
	}

	sprintf( cmd, "%s %s -dir -ext .tmp", CMD_GEN_DC, header );
	fp2= popen( cmd, "r" );
	if( fp2 == NULL ) {
		pclose( fp1 );
		fprintf( stderr, "'%s' failed\n", cmd );
		exit(1);
	}

	pclose( fp1 );
	pclose( fp2 );

	sprintf( cmd, "%s %s -paste -ext .tmp ", CMD_GEN_DC, header );
	fp1= popen( cmd, "r" );
	if( fp1 == NULL ) {
		fprintf( stderr, "'%s' failed\n", cmd );
		exit(1);
	}
	pclose( fp1 );

	
	sprintf( cmd, "%s %s -ext .tmp ", CMD_DS_ILLUM, header );
	fp1= popen( cmd, "r" );
	if( fp1 == NULL ) {
		fprintf( stderr, "'%s' failed\n", cmd );
		exit(1);
	}
	pclose( fp1 );
	
}

/*
 *
 */
void write_outside_rad( char* radfile ) {
	FILE* RAD= open_output( radfile );
	fprintf(RAD,"skyfunc glow sky_glow\n 0\n 0\n 4 1 1 1 0\n\n ");
	fprintf(RAD,"sky_glow source sky\n 0\n 0\n 4 0 0 1 180\n\n ");
	fprintf(RAD,"skyfunc glow ground_glow\n 0\n 0\n 4 1 1 1 0\n\n ");
	fprintf(RAD,"ground_glow source ground\n 0\n 0\n 4 0 0 -1 180\n\n ");
	close_file(RAD);
}


/*
 * Extract the viewpoint information from the viewpoint file. Store the results in
 * a struct and write the sensor file.
 */
void write_sensor_file( char* viewpoint_file, struct view_point** vp,
						char* sensor_file ) {
	FILE* FP;
	int i;

	FP= open_input(viewpoint_file);
	if( FP == NULL ) {
		printf("gen_dgp_profile: ERROR failed to open viewpoint file '%s'\n",
			   viewpoint_file );
		exit(1);
	}

	*vp=(view_point*)calloc ( number_of_views, sizeof(view_point) );
	if( *vp == NULL ) {
		printf("gen_dgp_profile: ERROR viewpoints ran out of memory!");
		exit(1);
	}

	char buf [4096];
	
	for ( i=0 ; i < number_of_views ; i++ ) {
		if (!(fgets( buf, 4096, FP ))) {
			fprintf(stderr, "error reading view\n");
			break;
		};
		sscanf( strstr( buf, "-vp" ), "-vp %lf %lf %lf", &(*vp)[i].x, &(*vp)[i].y, &(*vp)[i].z );
		sscanf( strstr( buf, "-vd" ), "-vd %lf %lf %lf", &(*vp)[i].xdir, &(*vp)[i].ydir, &(*vp)[i].zdir );
	}
	close_file(FP);

	FP= open_output( sensor_file );
	if( FP == NULL ) {
		printf("gen_dgp_profile: ERROR failed to open sensor file '%s' for writing\n",
			   sensor_file );
		exit(1);
	}

	for( i= 0; i < number_of_views; i++ ) {
		fprintf( FP, "%f %f %f  %f %f %f\n",
				 (*vp)[i].x, (*vp)[i].y, (*vp)[i].z,
				 (*vp)[i].xdir, (*vp)[i].ydir, (*vp)[i].zdir );
	}
	close_file( FP );
		
}

void read_view_file( char* viewpoint_file, struct view_point** vp,
						char* sensor_file ) {
	FILE* FP;
	int i;

	FP= open_input(viewpoint_file);
	if( FP == NULL ) {
		printf("gen_dgp_profile: ERROR failed to open viewpoint file '%s'\n",
			   viewpoint_file );
		exit(1);
	}

	*vp=(view_point*)calloc ( number_of_views, sizeof(view_point) );
	if( *vp == NULL ) {
		printf("gen_dgp_profile: ERROR viewpoints ran out of memory!");
		exit(1);
	}

	char buf [4096];
	for ( i=0 ; i < number_of_views ; i++ ) {
		if (!(fgets( buf, 4096, FP ))) {
			fprintf(stderr, "error reading view\n");
			break;
		};
		sscanf( strstr( buf, "-vp" ), "-vp %lf %lf %lf", &(*vp)[i].x, &(*vp)[i].y, &(*vp)[i].z );
		sscanf( strstr( buf, "-vd" ), "-vd %lf %lf %lf", &(*vp)[i].xdir, &(*vp)[i].ydir, &(*vp)[i].zdir );
	}
	close_file(FP);

}

/*
 * open weather data file.
 */
void open_wea_file( char* wea_filename,
					float* dir_f, float* dif_f,
					int* month_f, int* day_f, float* hour_f)
{
	FILE* WEA;
	char cmd[1024];
	int i;
	int month, day;
	float hour;
	float dir, dif;

	WEA= open_input( wea_filename );
	fscanf( WEA, "%s", cmd );
	if( !strcmp(cmd,"place") ) { /* check weather the climate file as a header */
		/* skip header */
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
		fscanf(WEA,"%*[^\n]");fscanf(WEA,"%*[\n\r]");
	} else {
		rewind(WEA);
	}

	for( i= 0; fscanf(WEA,"%i %i %f %f %f",&month,&day,&hour,&dir,&dif) != EOF; i++ ) {
		dir_f[i]=dir;
		dif_f[i]=dif;
		month_f[i]=month;
		day_f[i]=day;
		hour_f[i]=hour;
	}
                
	close_file(WEA);
}


/*
 * open the illuminance data.
 */
void open_illuminance_file( char* filename, float** ill_f ) {
	FILE* ILLU= open_input(filename);
	int ts;
	char* line= calloc(4096, sizeof(char) );
	int month;
	int day;
	float hour;
	float ill;

	for( ts= 0; fgets(line, 4096, ILLU ) ; ts++ ) {
		int j, n;

		if( line[0] != '#' ) { 
			sscanf( line, "%d %d %f%n", &month, &day, &hour, &n );
			for( j= 0; j < number_of_views; j++ ) {
				int b;
				sscanf( line + n, "%f%n", &ill, &b );
				if( b != EOF ) {
					n+= b;
					ill_f[j][ts]= ill;
				}
			}
		}
	}
	close_file(ILLU);

	free( line );
}


/*
 *
 */
void gen_dgp( char* header_filename, view_point* viewpoints ) {
	int ts;						/* time step */
	int view;					/* current view */
	int numberoftimesteps;
	int lengthskyfile;
	//int j;
	//	int firsttime= 1;
	int* month_f;
	int DCFileCounter;
	int* day_f;
	int OconvIsDone=0;
	float* dir_f;
	float* dif_f;
	float* dgp_f;
	float** ill_f;
	float* hour_f;
	double low_light_corr;
	char cmd[1024]="";
	char ill_filename[1024]= "";
	char variant_rad_filename[1024]= "";
	char rpictoptions[1024]= ""; 
	char dgp_out_file[1024]= "";
	char dgp_check_option[1024]= "";
	char octree[1024]= "";
	char buf[1024];
	char* c;
	float dgp_cut_value;
	
    char skyradfile[1024]="";
    char outside_rad[1024]="outside_daysim.rad";
    FILE* DGP;
    FILE* RAD;
	FILE* WEA;
	dgp_cut_value=14310.0;
        low_light_corr=1.0;

        if( dgp_profile_cut == 1 ) {
	          dgp_cut_value=4940.0;
	       		 }


	strcpy( ill_filename, shading_illuminance_file[0] );
       
	if( strcmp(geometry_file,"") ) {
		sprintf(cmd,"\"%s\"",geometry_file);
		strcpy(geometry_file,cmd);
	}
	if( strcmp(material_file,"") ) {
		sprintf(cmd,"\"%s\"",material_file);
		strcpy(material_file,cmd);
	}
	if( strcmp(viewpoint_file,"") ) {
		sprintf(cmd,"\"%s\"",viewpoint_file);
		strcpy(viewpoint_file,cmd);
	}

	numberoftimesteps= number_of_lines_in_file(wea_data_short_file);
	// check with weather file has a six lines header and substract lines from the number of lines
	WEA= fopen( wea_data_short_file, "r" );
	if( WEA == NULL ) {
		fprintf( stderr, "'%s' failed\n", wea_data_short_file );
		exit(1);
	}
	fscanf(WEA,"%s", cmd);
 	if( !strcmp(cmd,"place") )
		numberoftimesteps-=6;
	pclose(WEA);
				
				
	
	dir_f= (float*) calloc ( numberoftimesteps, sizeof(float) );
	dif_f= (float*) calloc ( numberoftimesteps, sizeof(float) );

	month_f= (int*) calloc ( numberoftimesteps, sizeof(int) );
	day_f= (int*) calloc ( numberoftimesteps, sizeof(int) );
	hour_f= (float*) calloc ( numberoftimesteps, sizeof(float) );

	dgp_f= (float*) calloc ( number_of_views, sizeof(float) );

	if( dir_f == NULL  ||  dif_f == NULL  ||  dgp_f == NULL
		||  month_f == NULL  || day_f == NULL  ||  hour_f == NULL ) {
		fprintf( stderr, "failed to allocate memory\n" );
		exit(1);
	}

	ill_f= (float**) calloc ( number_of_views, sizeof(float*) );
	if( ill_f == NULL ) {
		fprintf( stderr, "failed to allocate memory for illuminance data\n" );
		exit(1);
	}
	for( view= 0; view < number_of_views; view++ ) {
		ill_f[view]= (float*)calloc( numberoftimesteps, sizeof(float) );
		if( ill_f[view] == NULL ) {
			fprintf( stderr, "failed to allocate memory for illuminance data\n" );
			exit(1);
		}
	}

	open_wea_file( wea_data_short_file, dir_f, dif_f, month_f, day_f, hour_f );
	
/*	sprintf( rpictoptions, "-x 800 -y 800  -aa 0.2 -ab 0 -ad 4096 -ar 128 -as 128 -dj 0 -dr 2 -ds 0  -lr 5 -lw 0.000001  -st 0.15 -ps 0 -vta -vv 180 -vh 180 -vu 0 0 1 -vs 0 -vl 0 " ); */
/*	sprintf( rpictoptions, "-x %i -y %i  -aa 0.2 -ab 0 -ad 4096 -ar 128 -as 128 -dj 0 -dr 2 -ds 0  -lr 5 -lw 0.000001  -st 0.15 -ps 0 -pj 0 -vta -vv 180 -vh 180 -vu 0 0 1 -vs 0 -vl 0 ",dgp_image_x_size,dgp_image_y_size); */
	sprintf( rpictoptions, "-x %i -y %i  -ab 0 -dj 0 -dr 2 -lr 4 -lw 0.000001  -st 0.15 -ps 0 -pj 0 -vta -vv 180 -vh 180 -vu 0 0 1 -vs 0 -vl 0 ",dgp_image_x_size,dgp_image_y_size); 

	sprintf( octree, "\"%sgen_dgp_sky.oct\"", tmp_directory );
   /*      printf ("%s \n",octree);*/
	write_outside_rad( outside_rad );

	/* iterate over all shading variants */
	for (DCFileCounter=1 ; DCFileCounter<=TotalNumberOfDCFiles ; DCFileCounter++)
	{								 
	sprintf(ill_filename,"%s",shading_illuminance_file[DCFileCounter]);	
	sprintf(variant_rad_filename,"%s",shading_rad_file[DCFileCounter]);	
	
		/* */
		open_illuminance_file( ill_filename, ill_f );
 

		/* generatre dgp-filename */
		strcpy( dgp_out_file, ill_filename );
		//if( (c= strrchr( dgp_out_file, '.' )) == NULL )
			c= dgp_out_file + strlen(dgp_out_file) -8; //cut ".ill.tmp
		strcpy( c, ".dgp" );
		//for (j = 0; j < (strlen(ill_filename)-8); j++)
		 //   {
		 //       dgp_out_file[j] = ill_filename[j];
    	//}
		//sprintf(dgp_out_file,"%s.dgp",dgp_out_file);
		//fprintf( stderr, "dgp: %s ill: %s\n", dgp_out_file, ill_filename );
		
		
		DGP= open_output(dgp_out_file);
		// loop viewpoints
		for( ts= 0; ts < numberoftimesteps; ts++ ) {
			fprintf( DGP, "%d %d %.3f ", month_f[ts], day_f[ts], hour_f[ts] );

			if ( dir_f[ts] > 25.0  ) 
			{
				sprintf( skyradfile, "%sgen_dgp_sky.rad", tmp_directory);
				sprintf( cmd, "%s %d %d %.3f  -a %.3f -o %.3f -m %.3f  -W  %.3f  %.3f > %s\n",
						 CMD_GENDAYLIT,
						 month_f[ts], day_f[ts], hour_f[ts],
						 degrees(s_latitude), degrees(s_longitude), degrees(s_meridian),
						 dir_f[ts], dif_f[ts], skyradfile );

				RAD= popen(cmd,"r");
				pclose(RAD);
				lengthskyfile=number_of_lines_in_file(skyradfile);
				//printf ("%i \n",lengthskyfile);

				if ( lengthskyfile > 4 ) {
				
					
					/* generate octree */
					for( view= 0; view < number_of_views; view++ ) 

					if(OconvIsDone==0)
					{
						/*oconv is done only the first time, no frozen octree */
						sprintf( cmd, "%s -n 32 %s %s %s %s %s > %s",
								CMD_OCONV, material_file, geometry_file, skyradfile, outside_rad, variant_rad_filename, octree );
						RAD= popen( cmd, "r" );
						while ( fgets( buf, sizeof buf, RAD) )
								sprintf( buf, "%s", buf );
						pclose( RAD );
						OconvIsDone=1;
					}
						
					for( view= 0; view < number_of_views; view++ ) 
					{
						/* Change: calculation is skipped, when Ev>14310, DGP>1 */
						/* if dgp_cut_value is set to one, then Ev>4940 , DGP>0.45 */
						// March 2012 [C Reinhart]: In order to save time the contrast calculation is skipped when
						// the contrast calculaiton in evalglare is ksipped then the glare due to the illumiance 
						// alreay leads ot intolerable glare (dgp>0.4): 4276.0 = (0.4-0.184)/0.0000622
						if(ill_f[view][ts]<dgp_cut_value )
						{
							/* uncomment to pipe oconv result to rpict*/
							/*						sprintf( cmd,
								 "%s -f %s %s %s %s"
								 " | "					
								 "%s %s -vp %f %f %f  -vd %f %f %f"
								 " | "
								 "%s -b 40000 -i %.2f -D -y -1"
								 ,
								 CMD_OCONV, material_file, geometry_file, skyradfile, outside_rad,
								 CMD_RPICT, rpictoptions,
								 viewpoints->x, viewpoints->y, viewpoints->z,
								 viewpoints->xdir, viewpoints->ydir, viewpoints->zdir,

														 CMD_EVALGLARE, ill_f[view][ts] );*/
							if (checkfile==1){
								sprintf(dgp_check_option, " -c %s.%i.pic ",dgp_check_file,ts);
							}

							sprintf( cmd,
									"%s %s -vp %f %f %f  -vd %f %f %f  %s"	/* rpict */
									" | "
									"%s -b 8000 -i %.2f -D -y -1 %s"		/* evalglare */
									,
									CMD_RPICT, rpictoptions,
									viewpoints->x, viewpoints->y, viewpoints->z,
									viewpoints->xdir, viewpoints->ydir, viewpoints->zdir,
									octree,
									CMD_EVALGLARE, ill_f[view][ts],dgp_check_option );

							RAD= popen( cmd, "r" );  
							while ( fgets( buf, sizeof buf, RAD) )
								sprintf( buf, "%s", buf );
							pclose(RAD);
							dgp_f[view] = (float)atof(buf);
						}else{
							//illumiance at the viewpoint itself leads to DGP>=1
							dgp_f[view]= 1.0;
       						if( dgp_profile_cut == 1 ) {
								dgp_f[view]=99.9f;
	  						}
						}
					}
				} else { //dir_f[ts] <= 25.0
					for( view= 0; view < number_of_views; view++ ) {

						if ( ill_f[view][ts]< 1000) {
                            low_light_corr=exp(0.024*ill_f[view][ts]-4)/(1+exp(0.024*ill_f[view][ts]-4));
						} else {low_light_corr=1.0 ;}
						dgp_f[view] = (float)(low_light_corr*(6.22e-5*ill_f[view][ts] + 0.184));
						
						
						if (dgp_f[view]>1)
							dgp_f[view]=1 ;
					}
					
					fprintf(stderr,"%s call failed at time: %d %d %.3f\n",
							CMD_GENDAYLIT, month_f[ts],day_f[ts],hour_f[ts]);
				}
			} else {
				for( view= 0; view < number_of_views; view++ ) 
				{
					if ( ill_f[view][ts]< 1000) {
                        low_light_corr=exp(0.024*ill_f[view][ts]-4)/(1+exp(0.024*ill_f[view][ts]-4));
					} else {low_light_corr=1.0 ;}
					dgp_f[view] = (float)(low_light_corr*(6.22e-5*ill_f[view][ts] + 0.184));
					if (dgp_f[view]>1)
						dgp_f[view]=1 ;
				}
			}

			for( view= 0; view < number_of_views; view++ ) 
				fprintf( DGP, " %.4f", dgp_f[view] );
			fprintf( DGP, "\n" );
		}
		close_file(DGP);
	}
}

int main(int argc, char **argv) {
	char* progname;
	char* header_filename= NULL;
	int i,only_preprocessing;						
	int no_preprocessing;					
	view_point* viewpoints;

    only_preprocessing=0;
    no_preprocessing=0;
	progname = fixargv0(argv[0]);

	if (argc == 1){
		fprintf(stderr, "WARNING %s: input file missing\n", progname);
		fprintf(stderr,"start program with:  %s <header file>\n", progname );
		fprintf(stderr,"\t-i calculate only vertical illuminance, no timestep dgp\n ");
		fprintf(stderr,"\t-d calculate only timestep dgp, no vertical illuminance\n ");
		exit(1);
	}

	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

    header_filename= argv[1];
	if( argc > 2 ) {
		for( i= 2; i < argc; i++ ) {
			if( !strcmp(argv[i],"-i") ) {
				only_preprocessing=1;
			} else if( !strcmp(argv[i],"-d") ) {
				no_preprocessing=1;
			} 
		}
	}

	read_in_header( header_filename );
	//change point file name so that the original does not get overwritten
	sprintf(sensor_file,"%s.tmp",sensor_file);

	//change *.dc and *.ill file names so that the originals do not get overwirtten
	for (i=1 ; i<=TotalNumberOfDCFiles ; i++){
		sprintf(shading_dc_file[i],"%s.tmp",shading_dc_file[i]);
		sprintf(shading_illuminance_file[i],"%s.tmp",shading_illuminance_file[i]);
	}

	/* read in coordiantes of views */
	write_sensor_file( viewpoint_file, &viewpoints, sensor_file );

	/* call gen_dc and ds_illum */
        if ( no_preprocessing==0){
	    preprocessing( header_filename );  }

        if ( only_preprocessing==0){
	      gen_dgp( header_filename, viewpoints );}


	unlink (sensor_file);
	for (i=1 ; i<=TotalNumberOfDCFiles ; i++){
		unlink (shading_dc_file[i]);
		unlink (shading_illuminance_file[i]);
	}

	return 0;
}

