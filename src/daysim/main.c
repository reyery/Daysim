/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

// radfiles2daysim:
// DAYSIM GUI subprogram that imports RADIANCE sources files  and converts them into DAYSIM format
// the program requires the DAYSIM header keywords:
//		radiance_source_files (input)
// 		material_file (output)
//		geometry file (output)
// checks included:
// 		(1) plastic, glas, metal are grey or converted to grey
// 		(2) light and glow are converted into plastic
//		(3) texfunc, clolrfunc, brightfunc are converted into plastic


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include  "fropen.c"
#include  "read_in_header.c"

int KnownModifier(char *keyword_now);
void GetNumMaterials();
void UpdateModifier();
void AssignMaterials();
int NumMaterials=0;
int NumCurrentMaterials=1;
char header_file[200]="";
char  *progname;
char material_line[1000]="";
char keyword[200]="";
char keyword_2[200]="";
char keyword_3[200]="";
char keyword_4[200]="";
char (*modifier1)[200];
int NumMaterialsSupported=1000;
char modifier[200][1000];


int main(int argc, char *argv[])
{
 	int i,j,k;
	int write_out_material=0;
	int write_out_geometry=0;
	int value_int=0;
	int num_of_elements_following=0;
	float value_float_r=0;
	float value_float_g=0;
	float value_float_b=0;
	float val=0;
	FILE* MAT_FILE;
	FILE* GEO_FILE;
	FILE* SOURCE_RAD_FILE;

	srand(1);
	progname = argv[0];

	if (argc < 2){
		fprintf(stderr,"radfiles2daysim: no input file specified\n");
		fprintf(stderr,"start program with\n");
		fprintf(stderr,"radfiles2daysim <file>.hea\n\n");
		fprintf(stderr,"further options are:\n");
		fprintf(stderr,"-g only geometry file is updated\n");
		fprintf(stderr,"-m only material file is updated\n\n");
		exit(1);
	}
	strcpy(header_file,argv[1]);

	for (i = 2; i < argc; i++)
		if (argv[i][0] == '-' )
			switch (argv[i][1])
			{

			case 'm':
					write_out_material=1;
					break;
			case 'g':
					write_out_geometry=1;
					break;


			}//end switch (argv[i][1])
 
 /* read in header file */
	read_in_header(header_file);

	if(write_out_material){
		MAT_FILE=open_output(material_file);
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE,"# MATERIAL FILE           \n");
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE,"# This file has been created with rif2hea           \n");
		fprintf(MAT_FILE,"# Copyright (c) Christoph Reinhart           	      \n");
		fprintf(MAT_FILE,"# National Research Council Canada           	      \n");
		fprintf(MAT_FILE,"# IRC, Lighting Group                               \n");
		fprintf(MAT_FILE,"# Ont K1A 0R6, Canada                               \n");
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE," \n");
		fprintf(MAT_FILE,"# radfiles2hea is a sub program used by the DAYSIM GUI to\n");
		fprintf(MAT_FILE,"# adapt regular RADIANCE scene and material to the DAYSIM\n");
		fprintf(MAT_FILE,"# format, i.e. only materials supported by rtrace_dc are \n");
		fprintf(MAT_FILE,"# admitted. Those admitted are turned gray.\n");
		fprintf(MAT_FILE," \n");
		fprintf(MAT_FILE," \n");
	}
	if(write_out_geometry){
		GEO_FILE=open_output(geometry_file);
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE,"# GEOMETRY FILE           \n");
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE,"# This file has been created with rif2hea           \n");
		fprintf(GEO_FILE,"# Copyright (c) Christoph Reinhart           	      \n");
		fprintf(GEO_FILE,"# National Research Council Canada           	      \n");
		fprintf(GEO_FILE,"# IRC, Lighting Group                               \n");
		fprintf(GEO_FILE,"# Ont K1A 0R6, Canada                               \n");
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE," \n");
		fprintf(GEO_FILE,"# radfiles2hea is a sub program used by the DAYSIM GUI to\n");
		fprintf(GEO_FILE,"# adapt regular RADIANCE scene and material to the DAYSIM\n");
		fprintf(GEO_FILE,"# format, i.e. only materials supported by rtrace_dc are \n");
		fprintf(GEO_FILE,"# admitted. Those admitted are turned gray.\n");
		fprintf(GEO_FILE," \n");
		fprintf(GEO_FILE," \n");
	}

	if(number_of_radiance_source_files>0){
		// get number of materials
		GetNumMaterials();
	  	modifier1=(char(*)[200])malloc(sizeof(char)*200*NumMaterials); 
		if(NumMaterials>0)
			strcpy(modifier1[0],"void");

		printf("++++++++++++++++++++++++\n");
		printf("+ radiance_source_files+\n");
		printf("++++++++++++++++++++++++\n");

		for (i=0 ; i<(number_of_radiance_source_files) ; i++){
			printf("Importing %s ...\n",radiance_source_files[i]);
			if(!does_file_exist("file", radiance_source_files[i])){
				fprintf(stderr,"FATAL ERROR: radfiles2hea \n");
				exit(1);
			}
			SOURCE_RAD_FILE=open_input(radiance_source_files[i]);
			if(write_out_geometry)
				fprintf(GEO_FILE,"\n# SOURCE FILE: %s \n\n",radiance_source_files[i]);
			if(write_out_material)
				fprintf(MAT_FILE,"\n# SOURCE FILE: %s \n\n",radiance_source_files[i]);

			//import file...
			while(EOF != fscanf(SOURCE_RAD_FILE,"%s",&keyword_2))
			{
				if( KnownModifier(keyword_2)){
					fscanf(SOURCE_RAD_FILE,"%s",&keyword);
					if( !strcmp(keyword,"plastic")||!strcmp(keyword,"metal")||!strcmp(keyword,"dielectric")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier1[NumCurrentMaterials]);
						if(write_out_material)
							fprintf(MAT_FILE,"\tvoid %s %s 0 0 5 ",keyword,modifier1[NumCurrentMaterials]);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_b);
						if(write_out_material)
							fprintf(MAT_FILE,"%.4f %.4f %.4f ",.33333*(value_float_r+value_float_g+value_float_b),.33333*(value_float_r+value_float_g+value_float_b),.33333*(value_float_r+value_float_g+value_float_b));
						if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							printf("\t# %s %s has been turned grey\n",keyword,modifier1[NumCurrentMaterials]);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_g);
						if(write_out_material)
							fprintf(MAT_FILE,"%.4f %.4f \n\n",value_float_r,value_float_g);
						UpdateModifier();
					}//end plastic
					else if( !strcmp(keyword,"glass")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier1[NumCurrentMaterials]);
						if(write_out_material)
							fprintf(MAT_FILE,"\tvoid glass %s 0 0 3 ",modifier1[NumCurrentMaterials]);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%f", &value_float_b);
						if(write_out_material)
							fprintf(MAT_FILE,"%.4f %.4f %.4f \n\n",.33333*(value_float_r+value_float_g+value_float_b),.33333*(value_float_r+value_float_g+value_float_b),.33333*(value_float_r+value_float_g+value_float_b));
						if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							printf("\t# %s %s has been turned grey\n",keyword,modifier1[NumCurrentMaterials]);
						UpdateModifier();
					}//end glass
					else if( !strcmp(keyword,"alias")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier1[NumCurrentMaterials]);
						if(write_out_material)
							fprintf(MAT_FILE,"v\toid alias %s ",modifier1[NumCurrentMaterials]);
						fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						if(write_out_material)
							fprintf(MAT_FILE,"%s\n\n",keyword);
						UpdateModifier();
					}//end alias
					else if( !strcmp(keyword,"texfunc") || !strcmp(keyword,"brightfunc") || !strcmp(keyword,"colorfunc")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier1[NumCurrentMaterials]);
						if(write_out_material)
						{
							fprintf(MAT_FILE,"\t# ++++++++++++++++++++++++\n");
							fprintf(MAT_FILE,"\t# changed \'%s\' from %s to plastic\n",modifier1[NumCurrentMaterials],keyword);
							fprintf(MAT_FILE,"\tvoid plastic %s 0 0 ",modifier1[NumCurrentMaterials]);
							fprintf(MAT_FILE,"5 1 1 1 0 0\n\n");
						}
						printf("\t# changed \'%s\' from %s to plastic\n",modifier1[NumCurrentMaterials],keyword);

						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++){
							fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						}
						fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++){
							fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						}
						UpdateModifier();
					} //texfunc/end brightfunc/colorfunc
					else if(  !strcmp(keyword,"light")||!strcmp(keyword,"glow")||!strcmp(keyword,"spotlight")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier1[NumCurrentMaterials]);
						if(write_out_material)
						{
							fprintf(MAT_FILE,"\t# ++++++++++++++++++++++++\n");
							fprintf(MAT_FILE,"\t# changed \'%s\' from %s to plastic\n",modifier1[NumCurrentMaterials],keyword);
							fprintf(MAT_FILE,"\tvoid plastic %s 0 0 ",modifier1[NumCurrentMaterials]);
							fprintf(MAT_FILE,"5 1 1 1 0 0\n\n");
						}
						printf("\t# changed \'%s\' from %s to plastic\n",modifier1[NumCurrentMaterials],keyword);
						fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++){
								fscanf(SOURCE_RAD_FILE,"%s",&keyword);
						}
						UpdateModifier();
					} //light/glow/spotlight
					else if(  !strcmp(keyword,"source")){
						fscanf(SOURCE_RAD_FILE,"%s",&keyword_3);
						if(write_out_geometry)
						{
							fprintf(GEO_FILE,"\t# ++++++++++++++++++++++++\n");
							fprintf(GEO_FILE,"\t# source \'%s\' ignored\n",keyword_3);
							fprintf(GEO_FILE,"\t# %s source %s 0 0 4 ",keyword_2,keyword_3);
						}
						printf("\t# source \'%s\' ignored\n",keyword_3);
						fscanf(SOURCE_RAD_FILE,"%s %s %s",&keyword,&keyword_2,&keyword_3);
						fscanf(SOURCE_RAD_FILE,"%s %s %s %s",&keyword,&keyword_2,&keyword_3,&keyword_4);
						if(write_out_geometry)
						{
							fprintf(GEO_FILE,"%s %s %s %s\n\n",keyword,keyword_2,keyword_3,keyword_4);
						}
					} //source
					else{ //the material does not correspond to any of material identifiers
						fgets(material_line,1000,SOURCE_RAD_FILE);
						if(write_out_geometry)
							fprintf(GEO_FILE,"\t%s %s %s",keyword_2,keyword, material_line);
					}
				}else{
					fgets(material_line,1000,SOURCE_RAD_FILE);
					if(write_out_geometry)
						fprintf(GEO_FILE,"\t%s %s",keyword_2, material_line);
				}

			}

			close_file(SOURCE_RAD_FILE);

		}
	}else{
		fprintf(stderr,"keyword: \"radiance_source_files\" missing in DAYSIM header \n");
	}

	if(write_out_material)
		close_file(MAT_FILE);

	if(write_out_geometry)
		close_file(GEO_FILE);


	exit(0);
}


int KnownModifier(char *keyword_now)
{
	int i;
	for (i=0 ; i<NumCurrentMaterials ; i++){
		if(!strcmp(modifier1[i],keyword_now))
			return(1);
	}
	return(0);
}

void UpdateModifier()
{
	int i;
	int test=1;
	//test if the modifier1 is already in the list
	for (i=0 ; i<NumCurrentMaterials ; i++){
		if(!strcmp(modifier1[i],modifier1[NumCurrentMaterials]))
			test=0;
	}
	if(test)
	{
		NumCurrentMaterials++;
		//printf("\t# added %s to modifier1 list (element no. %d)\n",modifier1[NumCurrentMaterials],NumCurrentMaterials);
	}
}



void GetNumMaterials()
{
	// function reads all the RADIANCE source files and counts the number of modifiers
	int i;
	FILE* SOURCE_RAD_FILE;

	for (i=0 ; i<(number_of_radiance_source_files) ; i++){
		if(!does_file_exist("file", radiance_source_files[i])){
			fprintf(stderr,"FATAL ERROR: radfiles2hea file %s not found\n",radiance_source_files[i]);
			exit(1);
		}
		SOURCE_RAD_FILE=open_input(radiance_source_files[i]);
		//import file...
		while(EOF != fscanf(SOURCE_RAD_FILE,"%s",&keyword_2))
		{
			fscanf(SOURCE_RAD_FILE,"%s",&keyword);
			fscanf(SOURCE_RAD_FILE,"%*[^\n]");
			fscanf(SOURCE_RAD_FILE,"%*[\n\r]");
			if( !strcmp(keyword,"plastic"))
				NumMaterials++;
			if( !strcmp(keyword,"glass"))
				NumMaterials++;
			if( !strcmp(keyword,"alias"))
							NumMaterials++;
			if(!strcmp(keyword,"texfunc") || !strcmp(keyword,"brightfunc") || !strcmp(keyword,"colorfunc"))
							NumMaterials++;
			if( !strcmp(keyword,"light")||!strcmp(keyword,"glow"))
							NumMaterials++;
		}
		close_file(SOURCE_RAD_FILE);

	}
	printf("Number of materials found %d.\n",NumMaterials);
	if(NumMaterials>NumMaterialsSupported)
	{
		printf("Maximum number of supported materials is %d!",NumMaterialsSupported);
		exit(1);
	}
}


