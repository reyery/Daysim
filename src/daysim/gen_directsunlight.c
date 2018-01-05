/*  Copyright (c) 2002
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
*/

/* gen_directsunlight is a subprogram of the Lightswitch Lighting anaylsis software */
/* gen_directsunlight is usually executed during the pre-calculation and -like the
   daylight coefficient file- is stored for later analysis*/

/* Specifically, the program predicts for the various blind settings when direct
   sunlight above 50Wm-2 is incident on any of the sensors in the sensor point file
   that are characterized as a "work plane sensor". The keyword in the header file to
   specfy a workplane sensor is 'sensor_info x 1 x ...'

*/

#include  <stdio.h>
#include  <string.h>
#include  <math.h>
#include  <stdlib.h>

#include  "version.h"
#include  "rterror.h"
#include  "sun.h"
#include  "paths.h"
#include  "fropen.h"
#include  "read_in_header.h"


float ***BlindGroupSensor;
char dir_tmp_filename[500][1024];
int NumberOfSensorsInBlindGroup[10];
int NumberOfBlindGroupCombinations=0;
int BlindGroupIndex=0;
int BlindGroupNumberForThisCombination[500];
char octree[200]="";
char long_sensor_file[11][1024];
char direct_sunlight_file_tmp[1024]="";
char sun_rad[200]="";
char BlindRadianceFiles[1024]="";
char BlindRadianceFiles_2ndLoop[1024]="";
char BlindRadianceFiles_3rdLoop[1024]="";
char BlindRadianceFiles_Combined[1024]="";
char buf[1024];
char befehl[1024]="";
		    
/* defines current time */
double hour;
int day, month;
double alt, azi;
float dir, dif;


void run_oconv_and_rtrace()
{
	FILE *POINTS;
	
	//generate octree for this case
	//=============================
	sprintf(befehl,"%soconv -f %s %s %s \"%s\" > %s\n","",material_file, BlindRadianceFiles_Combined, geometry_file, sun_rad, octree);
	//printf("%s ",befehl);
	POINTS=popen(befehl,"r");
	while( fscanf( POINTS, "%s", buf ) != EOF )
		printf("%s \n",buf);
	pclose(POINTS);
	// run rtrace and calcualte the shading status for this case
	//==========================================================
	sprintf(befehl,"rtrace_dc -ab 0 -h -lr 6 -dt 0 \"%s\" < %s > %s\n",octree,long_sensor_file[BlindGroupIndex],dir_tmp_filename[NumberOfBlindGroupCombinations]);
	//printf("%s ",befehl);
	POINTS=popen(befehl,"r");
	while( fscanf( POINTS, "%s", buf ) != EOF )
		printf("%s \n",buf);
	pclose(POINTS);
	//delete files
	remove(octree);
	BlindGroupNumberForThisCombination[NumberOfBlindGroupCombinations]=BlindGroupIndex;
	NumberOfBlindGroupCombinations++;
}
	
void  make_annual_point_file(  char long_sensor_file[1024],int BlindGroupIndex)
{
	// function creates an rtrace input file that creates for all time steps
	// of the year when direct sunlight is above the DirectIrradianceGlareThreshold which
	// by default is 50Wm-2 a ray that is emitted from one of the active sensor points and
	// pointed towards the momentary sun position.
	int i=0;
	FILE *WEA;
	FILE *POINTS;
	int jd=0;
	double sd=0;
	double solar_time = 0;
	char befehl[1024]="";


	WEA=open_input(wea_data_short_file);
	fscanf(WEA,"%s",befehl);
	  	if( !strcmp(befehl,"place") ) // check weather the climate file has a header
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

	POINTS=open_output(long_sensor_file);
  	while(fscanf(WEA,"%d %d %lf %f %f",&month,&day,&hour,&dir,&dif) != EOF)
  	{

		/* get sun position */
       	jd= jdate(month, day);
		sd=sdec(jd);
		solar_time = hour + stadj(jd);
		alt = salt( sd,solar_time);
		azi = sazi(sd,solar_time);
  		if(alt > 0 && dir > DirectIrradianceGlareThreshold){
			for (i=0 ; i<NumberOfSensorsInBlindGroup[BlindGroupIndex]; i++)
				fprintf(POINTS,"%f %f %f %f %f %f\n",BlindGroupSensor[BlindGroupIndex][0][i],BlindGroupSensor[BlindGroupIndex][1][i],BlindGroupSensor[BlindGroupIndex][2][i],-cos(alt)*sin(azi),-cos(alt)*cos(azi),sin(alt) );
        }
  	}

	close_file(POINTS);
	close_file(WEA);

}


int main(int argc, char **argv)
{
	int  i,j,k,m,shading;
	int current_blind_group=0;
	int jd=0;
	double sd=0;
	float x_cor,y_cor,z_cor,x_dir,y_dir,z_dir;
	double solar_time = 0;
	float r,g,b;
	char temp_points[1024]="";
	char  *progname;
    FILE *DIR;
    FILE *WEA;
    FILE *POINTS;
	//FILE *DIR_TMP[500];
    FILE** DIR_TMP = malloc(sizeof(FILE*) * (500));
	if (DIR_TMP == NULL) goto memerr;

 
	progname = fixargv0(argv[0]);
	if (argc != 2){
		fprintf(stderr, "WARNING %s: input file missing\n", progname);
		fprintf(stderr, "start program with:  %s  <header file>\n ", progname);
		exit(1);
	}


	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

	read_in_header( argv[1]);

	if(!strcmp(direct_sunlight_file,""))
	{
		error(USER, "keyword \"direct_sunlight_file\" missing");
	}

	if(NumberOfBlindGroups==0)
	{
		error(USER, "no blind groups specified");
	}

	/* malloc BlindGroupSensor[NumberOfBlindGroups][6][number_of_sensors]*/
	BlindGroupSensor=(float***) malloc (sizeof(float**)*(NumberOfBlindGroups+1));
	if (BlindGroupSensor == NULL) goto memerr;
	for (i=0 ; i<=NumberOfBlindGroups ; i++){
		BlindGroupSensor[i] =(float**) malloc (sizeof(float*)* 6);
		if (BlindGroupSensor[i] == NULL) goto memerr;
		for (j=0 ; j<6 ; j++){
			BlindGroupSensor[i][j]=(float*) malloc (sizeof(float)* number_of_sensors);
			if (BlindGroupSensor[i][j] == NULL) goto memerr;
			for (k=0 ; k< number_of_sensors ; k++)
			{
				BlindGroupSensor[i][j][k] = 0;
			}
		}
	}

	for (i=0 ; i<10 ; i++)
		NumberOfSensorsInBlindGroup[i]=0;

	// read in coordiantes of active sensors
	// all active sensors are of sensor type "1"
	// please note that if no work plane sensors have been assigned the read_in_header functions assumes
	// that all illuminance sensors are work plane sensors.
	POINTS=open_input(sensor_file);
    for (i=0 ; i<number_of_sensors ; i++){
		fscanf(POINTS,"%f %f %f %f %f %f",&x_cor,&y_cor,&z_cor,&x_dir,&y_dir,&z_dir);
		for (j=1 ; j<= NumberOfBlindGroups ; j++){
			if(BlindGroup[j][i]>0)
			{
				BlindGroupSensor[j][0][NumberOfSensorsInBlindGroup[j]]=x_cor;
				BlindGroupSensor[j][1][NumberOfSensorsInBlindGroup[j]]=y_cor;
				BlindGroupSensor[j][2][NumberOfSensorsInBlindGroup[j]]=z_cor;
				BlindGroupSensor[j][3][NumberOfSensorsInBlindGroup[j]]=x_dir;
				BlindGroupSensor[j][4][NumberOfSensorsInBlindGroup[j]]=y_dir;
				BlindGroupSensor[j][5][NumberOfSensorsInBlindGroup[j]]=z_dir;
				NumberOfSensorsInBlindGroup[j]++;
			}
		}
	}
 	close_file(POINTS);



	/* make outside_rad, i.e. a glowing celestial hemisphere */
	sprintf(sun_rad,"%s%s_direct_sun.tmp.rad",tmp_directory,project_name);
	POINTS=open_output(sun_rad);
	fprintf(POINTS,"void glow outside_sphere\n 0\n 0\n 4 1000	1000	1000 0\n\n ");
	fprintf(POINTS,"outside_sphere source himmel\n 0\n 0\n 4 0	0	1 360\n ");
	close_file(POINTS);

	if( strcmp(geometry_file,"")){
			sprintf(befehl,"\"%s\"",geometry_file);
			strcpy(geometry_file,befehl);
	}
	if( strcmp(material_file,"")){
			sprintf(befehl,"\"%s\"",material_file);
			strcpy(material_file,befehl);
	}

	/* make input point file for the year for all octrees*/
	for (j=1 ; j<= NumberOfBlindGroups ; j++){
		if( strcmp(sensor_file,"")){
			sprintf(long_sensor_file[j],"%s.BlindGroup%d.long",sensor_file,j);
		}
		make_annual_point_file(long_sensor_file[j],j);
	}

	for (i=0 ; i<TotalNumberOfDCFiles ; i++)
	{
		if( strcmp(shading_rad_file[i],""))
		{
				sprintf(befehl,"\"%s\"",shading_rad_file[i]);
				strcpy(shading_rad_file[i],befehl);
		}

		if(check_if_file_exists(shading_rad_file[i]))
		{
			sprintf(errmsg, "file %s does not exist", shading_rad_file[i]);
			error(USER, errmsg);
		}
	}


//for (BlindGroupIndex=1 ; BlindGroupIndex<=NumberOfBlindGroups ; BlindGroupIndex++)
//	for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
//		printf("Radiance BG[%d] Setting [%d]=%s\n",BlindGroupIndex,j,BlindGroupGeometryInRadiance[j][BlindGroupIndex]);
	
	NumberOfBlindGroupCombinations=0;
	for (BlindGroupIndex=1 ; BlindGroupIndex<=NumberOfBlindGroups ; BlindGroupIndex++)
	{
		current_blind_group=i;

		// Base Case: Combine blind group setting with no blinds
		for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
		{
			sprintf(BlindRadianceFiles_Combined,"%s ",BlindGroupGeometryInRadiance[j][BlindGroupIndex]);
			sprintf(octree,"%stmp.%s_group%d_setting%d.gen_directsunlight.oct",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j);
			sprintf(dir_tmp_filename[NumberOfBlindGroupCombinations],"%s%s_%d_%d.gen_directsunlight.rtrace.tmp",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j);
			//printf("Radiance Files[%d][%d]=%s\n",BlindGroupIndex,j,BlindRadianceFiles_Combined);
			run_oconv_and_rtrace();
		}
		
		// Combination for two blind groups: Combined settings for BG1 with settings for BG2
		if(BlindGroupIndex==2 )
		{
			for (i=1 ; i<=NumberOfSettingsInBlindgroup[1] ; i++)
			{
				sprintf(BlindRadianceFiles,"%s ",BlindGroupGeometryInRadiance[i][1]);
				for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
				{
					sprintf(BlindRadianceFiles_Combined,"%s %s ",BlindRadianceFiles,BlindGroupGeometryInRadiance[j][BlindGroupIndex]);
					sprintf(octree,"%stmp.%s_group%d_setting%d.CombinationWithBG1.gen_directsunlight.oct",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j);
					sprintf(dir_tmp_filename[NumberOfBlindGroupCombinations],"%s%s_%d_%d.CombinationWithBG1.%d.gen_directsunlight.rtrace.tmp",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j,i);
					//printf("Radiance Files[%d][%d]=%s\n",BlindGroupIndex,j,BlindRadianceFiles_Combined);
					run_oconv_and_rtrace();
				}
			}
		}

		// Combination for three blind groups: Combined settings for BG1 and BG2 with settings for BG3
		if(BlindGroupIndex==3 )
		{
			for (k=1 ; k<=NumberOfSettingsInBlindgroup[1] ; k++)
			{
				sprintf(BlindRadianceFiles,"%s ",BlindGroupGeometryInRadiance[k][1]);
				for (i=1 ; i<=NumberOfSettingsInBlindgroup[2] ; i++)
				{
					sprintf(BlindRadianceFiles_2ndLoop,"%s %s ",BlindRadianceFiles, BlindGroupGeometryInRadiance[i][2]);
					for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
					{
						sprintf(BlindRadianceFiles_Combined,"%s %s ",BlindRadianceFiles_2ndLoop,BlindGroupGeometryInRadiance[j][BlindGroupIndex]);
						sprintf(octree,"%stmp.%s_group%d_setting%d.gen_directsunlight.oct",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j);
						sprintf(dir_tmp_filename[NumberOfBlindGroupCombinations],"%s%s_%d_%d.CombinationWithBG1.AndBG2.%d.%d.gen_directsunlight.rtrace.tmp",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j,i,k);
						//printf("Radiance Files[%d][%d]=%s\n",BlindGroupIndex,j,BlindRadianceFiles_Combined);
						run_oconv_and_rtrace();
					}
				}
			}
		}

		// Combination for four blind groups: Combined settings for BG1, BG2 and BG3 with settings for BG4
		if(BlindGroupIndex==4 )
		{
		for (k=1 ; k<=NumberOfSettingsInBlindgroup[1] ; k++)
			{
				sprintf(BlindRadianceFiles,"%s ",BlindGroupGeometryInRadiance[k][1]);
				for (i=1 ; i<=NumberOfSettingsInBlindgroup[2] ; i++)
				{
					sprintf(BlindRadianceFiles_2ndLoop,"%s %s ",BlindRadianceFiles, BlindGroupGeometryInRadiance[i][2]);
					for (m=1 ; m<=NumberOfSettingsInBlindgroup[3] ; m++)
					{
						sprintf(BlindRadianceFiles_3rdLoop,"%s %s ",BlindRadianceFiles_2ndLoop, BlindGroupGeometryInRadiance[m][3]);
						for (j=0 ; j<=NumberOfSettingsInBlindgroup[BlindGroupIndex] ; j++)
						{
							sprintf(BlindRadianceFiles_Combined,"%s %s ",BlindRadianceFiles_3rdLoop,BlindGroupGeometryInRadiance[j][BlindGroupIndex]);
							sprintf(octree,"%stmp.%s_group%d_setting%d.gen_directsunlight.oct",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j);
							sprintf(dir_tmp_filename[NumberOfBlindGroupCombinations],"%s%s_%d_%d.CombinationWithBG1ToBG3.%d.%d.%d.gen_directsunlight.rtrace.tmp",tmp_directory, BlindGroupName[BlindGroupIndex],BlindGroupIndex,j,k,i,m);
							//printf("Radiance Files[%d][%d]=%s\n",BlindGroupIndex,j,BlindRadianceFiles_Combined);
							run_oconv_and_rtrace();
						}
					}	
				}
			}
		}
		
			
			
		
	}

//==============
//build dir file
//==============
WEA=open_input(wea_data_short_file);
fscanf(WEA,"%s",befehl);
if( !strcmp(befehl,"place") ) // check weather the climate file has a header
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

//open rtrace results files
for (j=0 ; j<NumberOfBlindGroupCombinations ; j++)
	DIR_TMP[j]=open_input(dir_tmp_filename[j]);

DIR=open_output(direct_sunlight_file);
while(fscanf(WEA,"%d %d %lf %f %f",&month,&day,&hour,&dir,&dif) != EOF)
{
	fprintf(DIR,"%d %d %.3f %.0f %.0f ",month,day,hour,dir,dif );
	/* get sun position */
   	jd= jdate(month, day);
	sd=sdec(jd);
	solar_time = hour + stadj(jd);
	alt = salt( sd,solar_time);
	azi = sazi(sd,solar_time);
	if(alt > 0 && dir > DirectIrradianceGlareThreshold)
	{
		for (i=0 ; i<NumberOfBlindGroupCombinations ; i++)
		{
			shading=0;
			for (j=0 ; j<NumberOfSensorsInBlindGroup[BlindGroupNumberForThisCombination[i]] ; j++)
			{
				fscanf(DIR_TMP[i],"%f %f %f",&r,&g,&b);
				if ( (r/1000.0*dir) >DirectIrradianceGlareThreshold)
					shading=1;
			}
			fprintf(DIR,"%d ", shading );
		}
    }else{
		for (i=0 ; i<NumberOfBlindGroupCombinations ; i++)
			fprintf(DIR,"%d ", 0);
    }
 	fprintf(DIR,"\n");
}

//======================
//close and delete files
//======================
close_file(DIR);
close_file(WEA);

for (i=0 ; i<NumberOfBlindGroupCombinations ; i++)
{
	close_file(DIR_TMP[i]);
	remove(dir_tmp_filename[i]);
}

sprintf(temp_points,"%s.tmp", sensor_file);
DIR = fopen(temp_points, "r");
if ( DIR != NULL){
	close_file(DIR);
	remove(temp_points);
}else{close_file(DIR);}

sprintf(temp_points,"%s.long", sensor_file);
DIR = fopen(temp_points, "r");
if ( DIR != NULL){
	close_file(DIR);
	if(remove(temp_points))
		printf("Remove %s failed.\n",temp_points);
}else{close_file(DIR);}

sprintf(temp_points,"%s.tmp", direct_sunlight_file);
DIR = fopen(temp_points, "r");
if ( DIR != NULL){
	close_file(DIR);
	remove(temp_points);
}else{close_file(DIR);}

	return 0;
memerr:
	error(SYSTEM, "out of memory in main");
}


