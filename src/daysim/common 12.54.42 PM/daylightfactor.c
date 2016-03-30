/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

/* function to generate daylight factors from a daylight coefficient file
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "fropen.h"
#include "read_in_header.h"


float diffuse_dc[149], overcast[149],DZ[149];
float *daylight_factor;

double CIE_SKY(double Dz)
{	/* CIE overcast sky: values taken from gensky 6 21 12  */
	float A2=41.6; /*  zenith brightness */
	float A3=6.47;   /* ground plane brightness*/
	double luminance, intersky;
	intersky= A2 * (1 + 2*Dz)/3;
	luminance=(pow(Dz+1.01,10)*intersky+pow(Dz+1.01,-10)*A3)/(pow(Dz+1.01,10)+pow(Dz+1.01,-10));
	return(luminance);
}


//=====================================
// function calculates daylight factors
//=====================================
void getDaylightFactor( )
{

	int i,j,k;
	float ill_hor=101.5; /* outside horizontal irradiance under the CIE overcat sky at June 21st noon */
	char test_string[300]="";
	FILE *INPUT_FILE;

	//initialize arrays
	for (i=1;i<=148;i++)
	{
		diffuse_dc[i]=0;
		overcast[i]=0;
	}


	//allocate menory for  daylight factor array
	daylight_factor=(float*) malloc (sizeof(float)*number_of_sensors);
	for (i=0 ; i<number_of_sensors ; i++){
		daylight_factor[i]=0.0;
	}

	//initialize directiorns of diffuse daylight coefficients
	DZ[1]=0.104528;DZ[2]=0.104528;
	DZ[3]=0.104528;  DZ[4]=0.104528;  DZ[5]=0.104528;  DZ[6]=0.104528;  DZ[7]=0.104528;  DZ[8]=0.104528;  DZ[9]=0.104528;  DZ[10]=0.104528; DZ[11]=0.104528; DZ[12]=0.104528; DZ[13]=0.104528;
	DZ[14]=0.104528; DZ[15]=0.104528; DZ[16]=0.104528; DZ[17]=0.104528; DZ[18]=0.104528; DZ[19]=0.104528; DZ[20]=0.104528; DZ[21]=0.104528; DZ[22]=0.104528; DZ[23]=0.104528; DZ[24]=0.104528;
	DZ[25]=0.104528; DZ[26]=0.104528; DZ[27]=0.104528; DZ[28]=0.104528; DZ[29]=0.104528; DZ[30]=0.104528; DZ[31]=0.309017; DZ[32]=0.309017; DZ[33]=0.309017; DZ[34]=0.309017; DZ[35]=0.309017;
	DZ[36]=0.309017; DZ[37]=0.309017; DZ[38]=0.309017; DZ[39]=0.309017; DZ[40]=0.309017; DZ[41]=0.309017; DZ[42]=0.309017; DZ[43]=0.309017; DZ[44]=0.309017; DZ[45]=0.309017; DZ[46]=0.309017;
	DZ[47]=0.309017; DZ[48]=0.309017; DZ[49]=0.309017; DZ[50]=0.309017; DZ[51]=0.309017; DZ[52]=0.309017; DZ[53]=0.309017; DZ[54]=0.309017; DZ[55]=0.309017; DZ[56]=0.309017; DZ[57]=0.309017;
	DZ[58]=0.309017; DZ[59]=0.309017; DZ[60]=0.309017; DZ[61]=0.500000; DZ[62]=0.500000; DZ[63]=0.500000; DZ[64]=0.500000; DZ[65]=0.500000; DZ[66]=0.500000; DZ[67]=0.500000; DZ[68]=0.500000;
	DZ[69]=0.500000; DZ[70]=0.500000; DZ[71]=0.500000; DZ[72]=0.500000; DZ[73]=0.500000; DZ[74]=0.500000; DZ[75]=0.500000; DZ[76]=0.500000; DZ[77]=0.500000; DZ[78]=0.500000; DZ[79]=0.500000;
	DZ[80]=0.500000; DZ[81]=0.500000; DZ[82]=0.500000; DZ[83]=0.500000; DZ[84]=0.500000; DZ[85]=0.669131; DZ[86]=0.669131; DZ[87]=0.669131; DZ[88]=0.669131; DZ[89]=0.669131; DZ[90]=0.669131;
	DZ[91]=0.669131; DZ[92]=0.669131; DZ[93]=0.669131; DZ[94]=0.669131; DZ[95]=0.669131; DZ[96]=0.669131; DZ[97]=0.669131; DZ[98]=0.669131; DZ[99]=0.669131; DZ[100]=0.669131;DZ[101]=0.669131;DZ[102]=0.669131;
	DZ[103]=0.669131;DZ[104]=0.669131;DZ[105]=0.669131;DZ[106]=0.669131;DZ[107]=0.669131;DZ[108]=0.669131;DZ[109]=0.809017;DZ[110]=0.809017;DZ[111]=0.809017;DZ[112]=0.809017;DZ[113]=0.809017;
	DZ[114]=0.809017;DZ[115]=0.809017;DZ[116]=0.809017;DZ[117]=0.809017;DZ[118]=0.809017;DZ[119]=0.809017;DZ[120]=0.809017;DZ[121]=0.809017;DZ[122]=0.809017;DZ[123]=0.809017;DZ[124]=0.809017;DZ[125]=0.809017;DZ[126]=0.809017;DZ[127]=0.913545;DZ[128]=0.913545;DZ[129]=0.913545;DZ[130]=0.913545;DZ[131]=0.913545;DZ[132]=0.913545;DZ[133]=0.913545;DZ[134]=0.913545;
	DZ[135]=0.913545;DZ[136]=0.913545;DZ[137]=0.913545;DZ[138]=0.913545;DZ[139]=0.978148;DZ[140]=0.978148;DZ[141]=0.978148;DZ[142]=0.978148;DZ[143]=0.978148;DZ[144]=0.978148;DZ[145]=1.000000;
	DZ[146]=-0.087156;DZ[147]=-0.342 ;DZ[148]=-1;

	//calculate luminances of diffuse sky patches
	for (i=1;i<=148;i++){
		overcast[i]=CIE_SKY(DZ[i])/ill_hor;}

	//open DC input files
	if(!check_if_file_exists(shading_dc_file[1]))
	{
		printf("getDaylightFactor - WARNING: %s does not exist.\n",shading_dc_file[1]);
	}else{
		INPUT_FILE=open_input(shading_dc_file[1]);
		for (j=0 ; j<number_of_sensors ; j++){
				/* read in daylight coefficients */
				fscanf(INPUT_FILE,"%s ",test_string);
				while(test_string[0] == '#'){
					fscanf(INPUT_FILE,"%*[^\n]");
					fscanf(INPUT_FILE,"%*[\n\r]");
					fscanf(INPUT_FILE,"%s ",test_string);
				}
				diffuse_dc[1]=atof(test_string);

				for (k=2;k<=148;k++)
					fscanf(INPUT_FILE,"%f ",&diffuse_dc[k]);
				fscanf(INPUT_FILE,"%*[^\n]");
				fscanf(INPUT_FILE,"%*[\n\r]");

				/* calculate daylight factor */
	        	for (k=1;k<=148;k++)
	        	{
	        		daylight_factor[j]+=diffuse_dc[k]*overcast[k];
	        	}
				//printf("\n\n");
	        	daylight_factor[j]*=100.0;
		}

		// close DC file
		fclose(INPUT_FILE);
	}


}


