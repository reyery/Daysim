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
#include <rterror.h>

#include "fropen.h"
#include "read_in_header.h"

#define NUM_COEFFICIENTS	148


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
	float diffuse_dc[NUM_COEFFICIENTS], overcast[NUM_COEFFICIENTS], DZ[NUM_COEFFICIENTS];
	int i,j,k;
	float ill_hor=101.5; /* outside horizontal irradiance under the CIE overcat sky at June 21st noon */
	char test_string[300]="";
	FILE *INPUT_FILE;

	//initialize arrays
	for (i = 0; i < NUM_COEFFICIENTS; i++)
	{
		diffuse_dc[i]=0;
		overcast[i]=0;
	}


	//allocate menory for  daylight factor array
	daylight_factor=(float*) malloc (sizeof(float)*number_of_sensors);
	if (daylight_factor == NULL) error(SYSTEM, "out of memory in getDaylightFactor");
	for (i=0 ; i<number_of_sensors ; i++){
		daylight_factor[i]=0.0;
	}

	//initialize directiorns of diffuse daylight coefficients
	DZ[0] = DZ[1] = DZ[2] = DZ[3] = DZ[4] = DZ[5] = DZ[6] = DZ[7] = DZ[8] = DZ[9] = DZ[10] = DZ[11] = DZ[12] = DZ[13] = DZ[14] = DZ[15] = DZ[16] = DZ[17] = DZ[18] = DZ[19] = DZ[20] = DZ[21] = DZ[22] = DZ[23] = DZ[24] = DZ[25] = DZ[26] = DZ[27] = DZ[28] = DZ[29] = 0.104528f;
	DZ[30] = DZ[31] = DZ[32] = DZ[33] = DZ[34] = DZ[35] = DZ[36] = DZ[37] = DZ[38] = DZ[39] = DZ[40] = DZ[41] = DZ[42] = DZ[43] = DZ[44] = DZ[45] = DZ[46] = DZ[47] = DZ[48] = DZ[49] = DZ[50] = DZ[51] = DZ[52] = DZ[53] = DZ[54] = DZ[55] = DZ[56] = DZ[57] = DZ[58] = DZ[59] = 0.309017f;
	DZ[60] = DZ[61] = DZ[62] = DZ[63] = DZ[64] = DZ[65] = DZ[66] = DZ[67] = DZ[68] = DZ[69] = DZ[70] = DZ[71] = DZ[72] = DZ[73] = DZ[74] = DZ[75] = DZ[76] = DZ[77] = DZ[78] = DZ[79] = DZ[80] = DZ[81] = DZ[82] = DZ[83] = 0.500000f;
	DZ[84] = DZ[85] = DZ[86] = DZ[87] = DZ[88] = DZ[89] = DZ[90] = DZ[91] = DZ[92] = DZ[93] = DZ[94] = DZ[95] = DZ[96] = DZ[97] = DZ[98] = DZ[99] = DZ[100] = DZ[101] = DZ[102] = DZ[103] = DZ[104] = DZ[105] = DZ[106] = DZ[107] = 0.669131f;
	DZ[108] = DZ[109] = DZ[110] = DZ[111] = DZ[112] = DZ[113] = DZ[114] = DZ[115] = DZ[116] = DZ[117] = DZ[118] = DZ[119] = DZ[120] = DZ[121] = DZ[122] = DZ[123] = DZ[124] = DZ[125] = 0.809017f;
	DZ[126] = DZ[127] = DZ[128] = DZ[129] = DZ[130] = DZ[131] = DZ[132] = DZ[133] = DZ[134] = DZ[135] = DZ[136] = DZ[137] = 0.913545f;
	DZ[138] = DZ[139] = DZ[140] = DZ[141] = DZ[142] = DZ[143] = 0.978148f;
	DZ[144] = 1.0f;
	DZ[145] = -0.087156f;
	DZ[146] = -0.342f;
	DZ[147] = -1.0f;

	//calculate luminances of diffuse sky patches
	for (i = 0; i < NUM_COEFFICIENTS; i++) {
		overcast[i]=CIE_SKY(DZ[i])/ill_hor;
	}

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
				diffuse_dc[0]=atof(test_string);

				for (k = 1; k < NUM_COEFFICIENTS; k++)
					fscanf(INPUT_FILE,"%f ",&diffuse_dc[k]);
				fscanf(INPUT_FILE,"%*[^\n]");
				fscanf(INPUT_FILE,"%*[\n\r]");

				/* calculate daylight factor */
				for (k = 0; k < NUM_COEFFICIENTS; k++)
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


