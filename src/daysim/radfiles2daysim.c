/*  Copyright (c) 2003
 *  National Research Council Canada
 *  written by Christoph Reinhart
 */

// radfiles2daysim:
// DAYSIM GUI subprogram that imports RADIANCE sources files  and converts them into DAYSIM format
// the program requires the DAYSIM header material_types:
//		radiance_source_files (input)
// 		material_file (output)
//		geometry file (output)
// checks included:
// 		(1) plastic, glas, metal, dielectric, and trans are monochrome or turned to monochrome
// 		(2) light, glow, spotlight are turned into a black plastic (zero reflectance)
//		(3) illum is changed from a secondary light source to it affiliated material
//		(4) texfunc, texdata, brightdata, brighttext, mixdata,mixfunc, antimatter,
//			mixtext, and brightfunc are NOT modified
//		(5) colorfunc are converted into brightfunc
//		(6) source are ignored

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "version.h"
#include "rterror.h"
#include "paths.h"
#include  "fropen.h"
#include  "read_in_header.h"

char header_file[200]="";
char  *progname;
char material_line[1000]="";
char material_type[200]="";
char modifier_1[200]="";
char modifier_2[200]="";
char placeholder_1[200]="";
char placeholder_2[200]="";
double weight_r = 0.3; // photometric units from RADIANCE source code
double weight_g = 0.59;
double weight_b = 0.11;
int my_letter ;
int last_line_was_comment=0;
int FileExists=0;
int num_of_elements_following;
char new_line = '\n';
char new_tab = '\t';

char (*modifier1)[200];
int *modifier_status;
FILE* MAT_FILE;
FILE* GEO_FILE;
int write_out_material=0;
int write_out_geometry=0;
int UseMaterialDatabase=0;


//************************************************
// This function checks whether modifier exists in
// the material database.
//************************************************
int CheckIfMaterialExistsInDatabase(char *modifier)
{
	FILE* DatabaseFILE;
//	FILE* SketchUp2DaysimMaterialConversionFILE;
	char filename[1024]="";
	char line_string[1024]="";
//	char SketchUpModifier[1024]="";
	char  DaysimModifier[1024]="";
//	char SketchUp2DaysimMaterialConversionFilename[1024]="";
	char  ShortModifier[1024]="";


	// if the 3DS file was generated with SketchUp using the Daysim material library,
	// the material modifiers are shortened since SketchUp only export 8 digit material name.
	// in the following the shortened SketchUp material modifier are replace with the full ones
//	sprintf(SketchUp2DaysimMaterialConversionFilename,"%sSketchUp2Daysim.dat",MaterialDatabaseDirectory);
//	if(check_if_file_exists(SketchUp2DaysimMaterialConversionFilename))
//	{
//		SketchUp2DaysimMaterialConversionFILE=open_input(SketchUp2DaysimMaterialConversionFilename);
//		while( EOF != fscanf(SketchUp2DaysimMaterialConversionFILE,"%s %s", SketchUpModifier, DaysimModifier))
//		{
//			if( !strcmp(SketchUpModifier,modifier))
//			{
//				printf("#Material \'%s\' is defined as an \'alias\' for \'%s\'.\n",modifier,DaysimModifier);
//				sprintf(ShortModifier,"%s",modifier);
//				sprintf(modifier,"%s",DaysimModifier);
//			}
//		}
 //       close_file(SketchUp2DaysimMaterialConversionFILE);
//	}


	// check whether a file called "modifier.rad" exists
	// material database directory
	sprintf(filename,"%s%s.rad",MaterialDatabaseDirectory,modifier);
	if(check_if_file_exists(filename))
	{

		// write content of file into Daysim material file
		if(write_out_material)
		{
			fprintf(MAT_FILE,"\n#<----------------------------------------------\n");
			fprintf(MAT_FILE,"# The material description for %s is taken from the Daysim database (%s)\n",modifier,MaterialDatabaseDirectory);
			printf("#Material description for %s is taken from the Daysim database %s\n",modifier,MaterialDatabaseDirectory);

			DatabaseFILE=open_input(filename);
			while(0 != fgets(line_string,500,DatabaseFILE) )
            {
				//fgets(line_string,500,DatabaseFILE);
				fprintf(MAT_FILE,"%s",line_string);
			}
            close_file(DatabaseFILE);

			//alias SketchUp modifier to Daysim modifier
			if( strcmp(ShortModifier,""))
			{
				fprintf(MAT_FILE,"#Material \'%s\' is defined as an \'alias\' for \'%s\'.\n",modifier,DaysimModifier);
				fprintf(MAT_FILE,"void alias %s %s \n\n",ShortModifier, modifier);
			}

            fprintf(MAT_FILE,"#<----------------------------------------------\n\n");

		}
		return(1);
	}else{
		return(0);
	}
}



int main( int argc, char  *argv[])
{
	int i,l,k;
	int value_int=0;
	double value_float_r=0;
	double value_float_g=0;
	double value_float_b=0;
	float spec=0;
	float rough=0;
	float trans=0;
	float tspec=0;
	double mean_reflection=0;
	FILE* SOURCE_RAD_FILE;

	srand(1);
	progname = fixargv0(argv[0]);

	if (argc < 2){
		fprintf(stderr, "%s: no input file specified\n", progname);
		fprintf(stderr,"start program with\n");
		fprintf(stderr, "%s <file>.hea\n\n", progname);
		fprintf(stderr,"further options are:\n");
		fprintf(stderr,"-g only geometry file is updated\n");
		fprintf(stderr,"-m only material file is updated\n");
		fprintf(stderr,"-d use material database\n");
		exit(1);
	}

	if (!strcmp(argv[1], "-version")) {
		puts(VersionID);
		exit(0);
	}

	strcpy(header_file, argv[1]);

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
			case 'd':
					UseMaterialDatabase=1;
					break;
			}//end switch (argv[i][1])

//===================================
// read in DAYSIM project header file
//===================================
	read_in_header(header_file);


//=================================================
// write out header for material and geometry files
//=================================================
	if(write_out_material){
		MAT_FILE=open_output(material_file);
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE,"# Daysim Material File           \n");
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE,"# This file has been created with radfiles2daysim     \n");
		fprintf(MAT_FILE,"# Copyright (c) Christoph Reinhart           	      \n");
		fprintf(MAT_FILE,"# National Research Council Canada           	      \n");
		fprintf(MAT_FILE,"# IRC, Lighting Group                               \n");
		fprintf(MAT_FILE,"# Ont K1A 0R6, Canada                               \n");
		fprintf(MAT_FILE,"#####################################################\n");
		fprintf(MAT_FILE," \n");
		fprintf(MAT_FILE,"# radfiles2daysim is a converter used by the DAYSIM GUI to\n");
		fprintf(MAT_FILE,"# import regular RADIANCE scene and material into DAYSIM\n");
		fprintf(MAT_FILE,"# format, i.e. only materials supported by Daysim are \n");
		fprintf(MAT_FILE,"# admitted. Those admitted are turned monochrome.\n");
		fprintf(MAT_FILE," \n");
		fprintf(MAT_FILE,"# This file contains the definitions of all materials in\n");
		fprintf(MAT_FILE,"# the RADIANCE scene.\n");
		if(write_out_geometry)
		  fprintf(MAT_FILE,"# The scene geometry is stored in \'%s\'.\n",geometry_file);
		fprintf(MAT_FILE," \n");
	}
	if(write_out_geometry){
		GEO_FILE=open_output(geometry_file);
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE,"# Daysim Geometry File           \n");
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE,"# This file has been created with radfiles2daysim     \n");
		fprintf(GEO_FILE,"# Copyright (c) Christoph Reinhart           	      \n");
		fprintf(GEO_FILE,"# National Research Council Canada           	      \n");
		fprintf(GEO_FILE,"# IRC, Lighting Group                               \n");
		fprintf(GEO_FILE,"# Ont K1A 0R6, Canada                               \n");
		fprintf(GEO_FILE,"#####################################################\n");
		fprintf(GEO_FILE," \n");
		fprintf(GEO_FILE,"# radfiles2daysim is a converter used by the DAYSIM GUI to\n");
		fprintf(GEO_FILE,"# import a regular RADIANCE scene and material file into DAYSIM\n");
		fprintf(GEO_FILE,"# format, i.e. only materials supported by Daysim are \n");
		fprintf(GEO_FILE,"# admitted. Those admitted are turned monochrome.\n");
		fprintf(GEO_FILE," \n");
		fprintf(GEO_FILE,"# This file contains the geometry description of \n");
		fprintf(GEO_FILE,"# the RADIANCE scene.\n");
		if(write_out_material)
		  fprintf(GEO_FILE,"# All materials are defined in \'%s\'.\n",material_file);
		fprintf(GEO_FILE," \n");
		fprintf(GEO_FILE," \n");
	}

//==============================
// read in RADIANCE source files
//==============================

		for (i=0 ; i<(number_of_radiance_source_files) ; i++){

			//check if files exist
			if (!check_if_file_exists(radiance_source_files[i])){
				sprintf(errmsg, "cannot find file %s", radiance_source_files[i]);
				error(USER, errmsg);
			}

			SOURCE_RAD_FILE=open_input(radiance_source_files[i]);
			if(write_out_geometry)
				fprintf(GEO_FILE,"\n# SOURCE FILE: %s \n\n",radiance_source_files[i]);
			if(write_out_material)
				fprintf(MAT_FILE,"\n# SOURCE FILE: %s \n\n",radiance_source_files[i]);

			//import file...
		    while(!feof(SOURCE_RAD_FILE) )
            {
                fscanf(SOURCE_RAD_FILE,"%s",modifier_1);

                l=0;
                if(modifier_1[0] == '#' || modifier_1[1] == '#' ) //comment line
				{
					if(!last_line_was_comment && write_out_geometry)
						fprintf(GEO_FILE,"\n");

					if(write_out_geometry)
						fprintf(GEO_FILE,"#%s", modifier_1);
					my_letter = ' ';
					while ( my_letter != new_line && my_letter != '\n' && !feof(SOURCE_RAD_FILE))
					{
                    	my_letter = getc(SOURCE_RAD_FILE);
                    	if(write_out_geometry)
							fprintf(GEO_FILE,"%c",my_letter);
					}
					last_line_was_comment=1;
				}else{
                //no comment line: read to the end of word or line and copy into modifier_1
                	if(last_line_was_comment && write_out_geometry)
                		fprintf(GEO_FILE,"\n");

                	last_line_was_comment=0;
                	fscanf(SOURCE_RAD_FILE,"%s",material_type);
					FileExists=0;
					//*************************
					// plastic/metal/dielectric
					//*************************
					if( !strcmp(material_type,"plastic")||!strcmp(material_type,"metal")||!strcmp(material_type,"dielectric")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
                      	if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s %s %s 0 0 5 ",modifier_1,material_type,modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_b);
						//weigh material relfectances by rgb weights
						mean_reflection=weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b;
						if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"%.4f %.4f %.4f ",mean_reflection,mean_reflection,mean_reflection);
							//if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							//	printf("\t#%s : %s %s has been turned monochrome\n",radiance_source_files[i], material_type,modifier_2);
						}
						fscanf(SOURCE_RAD_FILE,"%f", &spec);
						fscanf(SOURCE_RAD_FILE,"%f", &rough);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%.4f %.4f \n\n",spec,rough);
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					}//end plastic
					//********
					// skyfunc
					//********
					else if( !strcmp(modifier_1,"skyfunc")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &num_of_elements_following);
						for (k=0 ; k<(num_of_elements_following) ; k++)
							fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						printf("\t#%s : \'skyfunc\' material \'%s\' has been ignored\n",radiance_source_files[i], modifier_2);
					    if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# \'%s\' has been ignored\n\n",modifier_1);
				        }
					}//end skyfunc

					//*******************
					// !gensky !gendaylit
					//*******************
					else if( !strcmp(modifier_1,"!gensky")|| !strcmp(modifier_1,"!gendaylit") ){

					if(write_out_geometry)
						fprintf(GEO_FILE,"#%s", modifier_1);
					my_letter = ' ';
					while ( my_letter != new_line && my_letter != '\n' && !feof(SOURCE_RAD_FILE))
					{
                    	my_letter = getc(SOURCE_RAD_FILE);
                    	if(write_out_geometry)
							fprintf(GEO_FILE,"%c",my_letter);
					}

					    printf("\t#%s : \'%s\' has been commented out\n",radiance_source_files[i], modifier_1);
					}//end skyfunc

					//******
					// trans
					//******
					else if( !strcmp(material_type,"trans")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s %s %s 0 0 7 ",modifier_1,material_type,modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_b);
						fscanf(SOURCE_RAD_FILE,"%f", &spec);
						fscanf(SOURCE_RAD_FILE,"%f", &rough);
						fscanf(SOURCE_RAD_FILE,"%f", &trans);
						fscanf(SOURCE_RAD_FILE,"%f", &tspec);
						if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"%.4f %.4f %.4f ",weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b);
							fprintf(MAT_FILE,"%.4f %.4f %.4f %.4f \n\n",spec,rough,trans,tspec);
							//if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							//	printf("\t#%s: %s %s has been turned monochrome\n",radiance_source_files[i],material_type,modifier_2);
						}
                    	if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					}//end trans
					//******
					// glass
					//******
					else if( !strcmp(material_type,"glass")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &num_of_elements_following);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s glass %s 0 0 %d ",modifier_1,modifier_2,num_of_elements_following);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_b);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%.4f %.4f %.4f ",weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b);
						for (k=3 ; k<num_of_elements_following ; k++)
						{
						   	fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
							if(write_out_material && !FileExists)
								fprintf(MAT_FILE,"%s ",placeholder_1);
						}
						if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"\n\n");
							//if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							//	printf("\t#%s: %s %s has been turned monochrome\n",radiance_source_files[i],material_type,modifier_2);
						}
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					}//end glass
					//*******
					// mirror
					//*******
					else if( !strcmp(material_type,"mirror")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%s", placeholder_1);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE," %s mirror %s \n\t1 %s\n\t0\n",modifier_1,modifier_2,placeholder_1);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_g);
						fscanf(SOURCE_RAD_FILE,"%lf", &value_float_b);
						if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE," 3 %.4f %.4f %.4f \n\n",weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b,weight_r*value_float_r+weight_g*value_float_g+weight_b*value_float_b);
							//if(value_float_r!=value_float_g || value_float_r!=value_float_b)
							//	printf("\t#%s: %s %s has been turned monochrome\n",radiance_source_files[i],material_type,modifier_2);
						}
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					}//end mirror
					//******
					// alias
					//******
					else if( !strcmp(material_type,"alias")){
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s alias %s ",modifier_1,modifier_2);
						fscanf(SOURCE_RAD_FILE,"%s",material_type);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE," %s\n\n",material_type);
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					}//end alias
					//*********************
					// light/glow/spotlight
					//*********************
					else if(  !strcmp(material_type,"light")||!strcmp(material_type,"glow")||!strcmp(material_type,"spotlight")){
					    fscanf(SOURCE_RAD_FILE, "%s",modifier_2);
                        if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"# ++++++++++++++++++++++++++++++++++++++\n");
							fprintf(MAT_FILE,"# changed \'%s\' from %s to a black plastic (zero reflectance)\n",modifier_2,material_type);
							fprintf(MAT_FILE,"%s plastic %s 0 0 ",modifier_1,modifier_2);
							fprintf(MAT_FILE,"5 0 0 0 0 0\n\n");
							printf("# %s: changed \'%s\' from %s to plastic\n",radiance_source_files[i],modifier_2,material_type);
						}fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++){
						      fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						}
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					} //light/glow/spotlight
					//******
					// illum
					//******
					else if(  !strcmp(material_type,"illum")){
					    fscanf(SOURCE_RAD_FILE, "%s",modifier_2);
                        fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%s", placeholder_1);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"# ++++++++++++++++++++++++++++++++++++++\n");
							fprintf(MAT_FILE,"# changed \'%s\' from %s to %s\n",modifier_2,material_type, placeholder_1);
							fprintf(MAT_FILE," %s alias %s %s ",modifier_1,modifier_2,placeholder_1);
							fprintf(MAT_FILE,"\n\n");
							printf("# %s: changed \'%s\' from %s to alias %s\n",radiance_source_files[i],modifier_2,material_type,placeholder_1);
						}
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++){
						      fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
						}
                        if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					} //illum
					//*******
					// source
					//*******
					else if(  !strcmp(material_type,"source")){
					    fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
						if(write_out_geometry)
						{
							fprintf(GEO_FILE,"\n# +++++++++++++++++++++++++++++++\n");
							fprintf(GEO_FILE,"# source \'%s\' ignored (sources are neglected during a DAYSIM simulation)\n",placeholder_1);
							fprintf(GEO_FILE,"# %s source %s 0 0 4 ",modifier_1,placeholder_1);
							printf("#%s:  source \'%s\' ignored\n",radiance_source_files[i],placeholder_1);
						}
						fscanf(SOURCE_RAD_FILE,"%s %s %s",material_type,modifier_1,placeholder_1);
						fscanf(SOURCE_RAD_FILE,"%s %s %s %s",material_type,modifier_1,placeholder_1,placeholder_2);
						if(write_out_geometry)
						{
							fprintf(GEO_FILE,"%s %s %s %s\n\n",material_type,modifier_1,placeholder_1,placeholder_2);
						}
					} //end source

					//***********
					// brightfunc
					//***********
					else if(  !strcmp(material_type,"brightfunc") ||
							  !strcmp(material_type,"texfunc") ||
							  !strcmp(material_type,"texdata") ||
							  !strcmp(material_type,"texfunc") ||
							  !strcmp(material_type,"texdata") ||
							  !strcmp(material_type,"brightdata") ||
							  !strcmp(material_type,"brighttext") ||
							  !strcmp(material_type,"mixdata") ||
							  !strcmp(material_type,"mixfunc") ||
							  !strcmp(material_type,"mixtext") ||
							  !strcmp(material_type,"plastic2") ||
							  !strcmp(material_type,"metal2") ||
							  !strcmp(material_type,"trans2") ||
							  !strcmp(material_type,"interface") ||
							  !strcmp(material_type,"plasfunc") ||
							  !strcmp(material_type,"metfunc") ||
							  !strcmp(material_type,"transfunc") ||
							  !strcmp(material_type,"BRTDfunc") ||
							  !strcmp(material_type,"plasdata") ||
							  !strcmp(material_type,"metdata") ||
							  !strcmp(material_type,"transdata") ||
							  !strcmp(material_type,"colortext") ||
							  !strcmp(material_type,"mixpict") ||
							  !strcmp(material_type,"prism1") ||
							  !strcmp(material_type,"prism2") ||
							  !strcmp(material_type,"antimatter")){
						fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s %s %s\n ",modifier_1,material_type, modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						if(write_out_material  && !FileExists)
							fprintf(MAT_FILE,"%d ",num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++)
						{
                        	fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
							if(write_out_material && !FileExists)
								fprintf(MAT_FILE," %s ",placeholder_1);
						}
     	                fscanf(SOURCE_RAD_FILE," %d",&num_of_elements_following);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"\n%d ",num_of_elements_following);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"\n%d ",num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++)
						{
                        	fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
							if(write_out_material && !FileExists)
								fprintf(MAT_FILE,"%s ",placeholder_1);
						}
						if(write_out_material)
							fprintf(MAT_FILE,"\n\n");
						if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }
					} //end texfunc, texdata, brightdata, brighttext, mixdata,
					  // mixfunc, antimatter, mixtext, texfunc, texdata, and brightfunc

					//*************************
					// colorfunc
					//*************************
					else if(  !strcmp(material_type,"colorfunc") )
					{
					    fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						if(UseMaterialDatabase)
							FileExists=CheckIfMaterialExistsInDatabase(modifier_2);
                        if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"%s brightfunc %s\n ",modifier_1,modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE," %d ",num_of_elements_following-2);
						for (k=0 ; k<num_of_elements_following ; k++)
						{
                        	fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
							if(write_out_material && num_of_elements_following!=1 && num_of_elements_following!=2 )
								fprintf(MAT_FILE,"%s ",placeholder_1);
						}
     	                fscanf(SOURCE_RAD_FILE," %d",&num_of_elements_following);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"\n%d ",num_of_elements_following);
						fscanf(SOURCE_RAD_FILE,"%d",&num_of_elements_following);
						if(write_out_material && !FileExists)
							fprintf(MAT_FILE,"\n%d ",num_of_elements_following);
						for (k=0 ; k<num_of_elements_following ; k++)
						{
                        	fscanf(SOURCE_RAD_FILE, "%s",placeholder_1);
							if(write_out_material && !FileExists)
								fprintf(MAT_FILE,"%s ",placeholder_1);
						}
						if(write_out_material && !FileExists)
						{
							fprintf(MAT_FILE,"\n\n");
							printf("#%s: %s %s has been turned into brightfunc\n",radiance_source_files[i],material_type,modifier_2);
					}
						if(write_out_geometry)
 		                {
							fprintf(GEO_FILE,"\n# <---------------------------\n");
							fprintf(GEO_FILE,"# %s \'%s\' is defined in the material file\n\n",material_type,modifier_2);
				        }

					}//end colorfunc
					//********
					// polygon  WARNING: the polygon procedure leads to rounding errors!
					//********
					else if( !strcmp(material_type,"polygon")){
					   fscanf(SOURCE_RAD_FILE,"%s",modifier_2);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &value_int);
						fscanf(SOURCE_RAD_FILE,"%d", &num_of_elements_following);
						if(write_out_geometry)
							fprintf(GEO_FILE,"\n%s polygon %s\n0 0 %d ",modifier_1,modifier_2,num_of_elements_following);
						for (k=0 ; k<(int)(num_of_elements_following/3) ; k++)
						{
							fscanf(SOURCE_RAD_FILE,"%lf", &value_float_r);
							fscanf(SOURCE_RAD_FILE,"%lf", &value_float_g);
							fscanf(SOURCE_RAD_FILE,"%lf", &value_float_b);
							if(write_out_geometry)
								fprintf(GEO_FILE,"\t%f %f %f\n",value_float_r,value_float_g,value_float_b);
						}
						if(write_out_geometry)
							fprintf(GEO_FILE,"\n");
					}//end polygon


					//*****************************************************
					// the following materials are not supported as of yet:
					//*****************************************************
					// mist
					else if( !strcmp(material_type,"mist"))
					{
						sprintf(errmsg, "material %s is currently not supported: please exchange the material with another material in the radiance source file %s", material_type, radiance_source_files[i]);
						error(USER, errmsg);
					}
					else{ //the material does not correspond to any material identifier
						    fgets(material_line,1000,SOURCE_RAD_FILE);
						    if(write_out_geometry)
						    {
                        		fprintf(GEO_FILE,"%s %s %s",modifier_1,material_type, material_line);
							}
					}

				}//end no comment line
				strcpy(material_line,"");
				strcpy(modifier_1,"");
				modifier_1[0]='\0';
				modifier_1[1]='\0';
				strcpy(modifier_2,"");
				strcpy(material_type,"");
				strcpy(placeholder_1,"");
			}
			close_file(SOURCE_RAD_FILE);
		}


	if(write_out_material)
		close_file(MAT_FILE);

	if(write_out_geometry)
		close_file(GEO_FILE);

	return 0;
}

