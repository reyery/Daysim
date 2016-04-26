#include <stdio.h>
#include <string.h>
#include "fropen.h"
#include "ds_shortterm.h"


void  create60minTempFile()
{
	// This function creates a temporary input file with a 60 min time step to be used
	// to create a shorter time step.


	int i,num_to_be_averaged;
	int month, day;
	float time;
	float irrad_beam_nor=0;
	float irrad_dif=0;
	float av_time=0;
	float av_irrad_beam_nor=0;
	float av_irrad_dif=0;

	num_to_be_averaged=60/test_input_time_step;
	sprintf(temp_file,"%s.60min.tmp",input_weather_data_shortterm);
	printf("create temporary file with a 60 min time step: %s... \n",temp_file);

  	SIXTYMIN_DATA = open_output(temp_file);
    //print file header
  	fprintf(SIXTYMIN_DATA,"%s", header_line_1);
  	fprintf(SIXTYMIN_DATA,"%s", header_line_2);
  	fprintf(SIXTYMIN_DATA,"%s", header_line_3);
  	fprintf(SIXTYMIN_DATA,"%s", header_line_4);
  	fprintf(SIXTYMIN_DATA,"%s", header_line_5);
  	fprintf(SIXTYMIN_DATA,"%s", header_line_6);

	i=0;
	while( EOF != fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_nor, &irrad_dif)){
		i++;
		av_time+=time;
		av_irrad_beam_nor+=irrad_beam_nor;
		av_irrad_dif+=irrad_dif;
		if(i==num_to_be_averaged){
			i=0;
			fprintf(SIXTYMIN_DATA,"%d %d  %.3f %.0f %.0f\n",month, day, av_time*1.0/num_to_be_averaged,av_irrad_beam_nor*1.0/num_to_be_averaged,av_irrad_dif*1.0/num_to_be_averaged);
			av_time=0;
			av_irrad_beam_nor=0;
			av_irrad_dif=0;
		}


	}
    close_file(HOURLY_DATA);
  	close_file(SIXTYMIN_DATA);
  	test_input_time_step=60;

	HOURLY_DATA = open_input(temp_file);
	if( !strcmp(keyword,"place") ){
		rewind(HOURLY_DATA);
		fgets(header_line_1,300,HOURLY_DATA);
		fgets(header_line_2,300,HOURLY_DATA);
		fgets(header_line_3,300,HOURLY_DATA);
		fgets(header_line_4,300,HOURLY_DATA);
		fgets(header_line_5,300,HOURLY_DATA);
		fgets(header_line_6,300,HOURLY_DATA);
	}else{
		rewind(HOURLY_DATA);
	}

}


