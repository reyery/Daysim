/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Eneregy Systems
 *  written by Christoph Reinhart
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../lib/fropen.h"
#include "../lib/read_in_header.h"
#include "../lib/sun.h"



// Write out DDS sensor point file
void write_dds_sensor_file(char *DDS_sensor_file)
{
	int    i,k;
	float **SensorPoints;
	char keyword[1000];
	FILE *SENSOR_FILE, *DDS_SENSOR_FILE;

	DDS_SENSOR_FILE = fopen(DDS_sensor_file, "r");
	if ( DDS_SENSOR_FILE != NULL){ //file exists
		close_file(DDS_SENSOR_FILE);
		printf("Overwrite existing DDS sensor file: %s\n",DDS_sensor_file);
	}
	//generate SEN files
	// read in sensor point coordinates from RADIANCE PTS file*/
	SensorPoints=(float**) malloc (sizeof(float*)*number_of_sensors);
	if( SensorPoints == NULL) {
		fprintf( stderr, "SensorPoints: Out of memory in function \'write_dds_sensor_file\'\n");
		exit(1);
	}
	for (i=0 ; i<(number_of_sensors) ; i++){
		SensorPoints[i]=(float*)malloc (sizeof(float)*6);
		if( SensorPoints[i] == NULL) {
			fprintf( stderr, "SensorPoints[%d]: Out of memory in function \'write_dds_sensor_file\'\n", i );
			exit(1);
		}
	}
	for (k=0 ; k<(number_of_sensors) ; k++){
		for (i=0 ; i<6 ; i++){
			SensorPoints[k][i]=0.0;
		}
	}
	SENSOR_FILE=open_input(sensor_file);
	for (k=0 ; k<(number_of_sensors) ; k++){
		fscanf(SENSOR_FILE,"%f %f %f %f %f %f",&SensorPoints[k][0],&SensorPoints[k][1],&SensorPoints[k][2],&SensorPoints[k][3], &SensorPoints[k][4],&SensorPoints[k][5]);
	}
	close_file(SENSOR_FILE);


	DDS_SENSOR_FILE = open_output(DDS_sensor_file);
	for(i=0;i<number_of_sensors;i++){
		//sensor type: illumiance, lumiance, irradiance, radiance
		if(sensor_unit[i]==0)
			sprintf(keyword,"ill");
		else if(sensor_unit[i]==1)
			sprintf(keyword,"lum");
		else if(sensor_unit[i]==2)
			sprintf(keyword,"irr");
		else if(sensor_unit[i]==3)
			sprintf(keyword,"rad");
		fprintf(DDS_SENSOR_FILE,"%s ",keyword);

		// sensor thermal zone
		fprintf(DDS_SENSOR_FILE,"%s ","void");

		// sensor lighting zone
		fprintf(DDS_SENSOR_FILE,"%s ","void");

		// sensor component
		fprintf(DDS_SENSOR_FILE,"%s ","void");

		// sensor position and orientation
		fprintf(DDS_SENSOR_FILE,"%f %f %f %f %f %f",SensorPoints[i][0],SensorPoints[i][1],SensorPoints[i][2],SensorPoints[i][3],SensorPoints[i][4],SensorPoints[i][5]);

		fprintf(DDS_SENSOR_FILE,"\n");
	}
	close_file(DDS_SENSOR_FILE);


}

void write_dds_file(char *DDS_file,int direct_direct_resolution, char Radiance_Parameters[99999])
{
	FILE *DDS_FILE;

	DDS_FILE= open_output( DDS_file );
	//header and basic scene description
	fprintf(DDS_FILE,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(DDS_FILE,"<scene>\n");
	fprintf(DDS_FILE,"\t<description>");
	fprintf(DDS_FILE,"%s","DDS file generate by DAYSIM sub-program \'gen_dc\'");
	fprintf(DDS_FILE,"</description>\n");
	fprintf(DDS_FILE,"\t<radsettings>%s",	Radiance_Parameters);
	fprintf(DDS_FILE,"</radsettings>\n");
	fprintf(DDS_FILE,"\t<radscene>\n");
	if(strcmp(material_file,"")){
		fprintf(DDS_FILE,"\t\t<textfile>");
		fprintf(DDS_FILE,"%s", material_file);
		fprintf(DDS_FILE,"</textfile>\n");
	}
	if(strcmp(geometry_file,"")){
		fprintf(DDS_FILE,"\t\t<textfile>");
		fprintf(DDS_FILE,"%s", geometry_file);
		fprintf(DDS_FILE,"</textfile>\n");
	}
	fprintf(DDS_FILE,"\t</radscene>\n");
	//daylight coefficient file
	fprintf(DDS_FILE,"\t<daylightcoefficients>\n");
	fprintf(DDS_FILE,"\t\t<textfile>");
	fprintf(DDS_FILE,"%s", shading_dc_file[0]);
	fprintf(DDS_FILE,"</textfile>\n");
	fprintf(DDS_FILE,"\t\t<direct_direct_resolution>");
	fprintf(DDS_FILE,"%i", direct_direct_resolution);
	fprintf(DDS_FILE,"</direct_direct_resolution>\n");
	fprintf(DDS_FILE,"\t</daylightcoefficients>\n\n");

	fprintf(DDS_FILE,"</scene>\n\n");

	//sensor file
	fprintf(DDS_FILE,"<sensors>\n");
	fprintf(DDS_FILE,"\t<textfile>");
	fprintf(DDS_FILE,"%s", DDS_sensor_file);
	fprintf(DDS_FILE,"</textfile>\n");
	fprintf(DDS_FILE,"</sensors>\n\n");

}
