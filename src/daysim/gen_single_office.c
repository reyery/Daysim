/*	This program has been written by Christoph Reinhart at the
*	Institue for Research in Construction
*	last changes were added in 3/05/2003 by Melissa Morrison
*/
/* program generates a RADIANCE file of an open plan office space  */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
//#include <strings.h>

#include "version.h"
#include "rterror.h"
#include "paths.h"
#include "fropen.h"
#include "gen_single_office.h"

int main( int argc, char  *argv[])
{
	//int i=0, j=0,k=0;
	//int ill_outside=0;

	progname = fixargv0(argv[0]);

	if (argc == 1)
	{
		writeErrorMessage();
		exit(0);
	}

	/* get the arguments */
	checkPassedArguements(argc, argv);

	/* make vf file*/
	writeVFFile();

	/*make Radiance header file*/
	writeRadianceHeaderFile(argc, argv);
}//end main

//This method prints an error message to the screen if not enough parameters are entered by user
//when calling the gen_single_office program

void writeErrorMessage()
{
	fprintf(stdout, "\n%s: \n", progname);
	printf("Program that generates a RADIANCE file of an open plan office \n");
	printf("\n");

	printf("\n");
	printf("SUPPORTED OPTIONS ARE THE FOLLOWING: \n");
	printf("====================================\n");
	printf("\n");
	printf("Note that the -o option has to be specified!\n");
	printf("Note that the -i option has to be specified!\n");
	printf("\n");

	printf("-i\t make input point files, specify the pts file name \n");
	printf("-o\t name of radiance output file\n");
	printf("-x\t bin directory (default="" \n");
	printf("\n");

	printf("TARGET OFFICE GEOMETRY AND MATERIALS:\n");
	printf("--------------------------------------\n");
	printf("-a\t ballustrade 0=no ballustrade/1=ballustrade (default=1)\n");
	printf("-b\t facade orientation     [0-360 DEG] (default South=0,East corresponds to 90)\n");
	printf("-c\t add white background and darken side walls\n");
	printf("-d\t office depth: [ft] (default= 15ft)\n");
	printf("-w\t office width: [ft] (default=10ft) \n");
	printf("-h\t floor to ceiling height [ft] (default=9ft) \n");
	printf("-f\t window frame width     [\"] (default=4\")\n");
	printf("-s\t window sill            [\"] (default=1\")\n");
	printf("-t\t visual transmittance of glazing (default 80) \n");
	printf("-m\t ceiling reflectance / wall reflectance / floor reflectance (default 80 / 50 / 20) \n");
	printf("-p\t seat positions\n");

	printf("-z\t facade type (1, 2 or 3)          (default=1) \n");
	printf("-e\t window full float [0...1] 0=no window/ 1=full width (default=1)\n");


//tito
//	-obstrcution angle 0, 15, ... 75Deg
// extenral structure continuous
//building height: singe sotry of neighboring building

	printf("\n");
	printf("TARGET OFFICE BUILDING GEOMETRY AND MATERIALS:\n");
	printf("----------------------------------------------\n");
	printf("-k\t building depth: [ft] (default= 150ft)\n");
	printf("-n\t building width: [ft] (default=150ft) \n");
	printf("-y\t number of floors in office building (default = 10) \n");
	printf("\t NOTE: Ground floor = floor 1, First floor = floor 2, etc...  \n");
	printf("-l\t Floor of target office (floor 1, 2, etc) (default = 1) \n");
	printf("-j\t facade reflectance of target building [%%]  (default=30 %%)\n");

	printf("\n");
	printf("EXTERNAL SHADING OPTIONS:\n");
	printf("-------------------------\n");
	printf("-U\t obstruction present [0=no or 1=yes]  (default value = 0) \n");
	printf("-W\t obstructing building width [ft]  (default=150ft)\n");
	printf("-D\t obstructing building depth [ft]  (default=150ft)\n");
	printf("-H\t obstructing building height [ft]  (default=100ft)\n");
	printf("-M\t obstructing building reflectance [%%] (default=30%%)\n");
	printf("-G\t ground/street reflectance [%%] (default = 30%%)\n");
	printf("-B\t street width [ft] (default= 60 ft)\n");
	printf("-C\t continuous or discontinuous obstruction [0=continuous, 1=discontinuous] (default value = 0)\n");
}

//This method checks the arguements passed for gen_single_office lie within acceptable
//ranges, and if so sets the variables to user's preferences.
void checkPassedArguements(int argc, char  *argv[])
{
	int i=0;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-') {

			if (!strcmp(argv[i], "-version")) {
				puts(VersionID);
				exit(0);
			}

			switch (argv[i][1])
			{

			case 'b'://orientation
				orientation = atof(argv[++i]);
				if (!(orientation == 0 || orientation == 90 || orientation == 180 || orientation == 270 || orientation == 360))
				{
					error(USER, "Orientation must be 0, 90, 180, 270 or 360!");
				}
				break;

			case 'a'://switch for ballustrade
				ballustrade = atoi(argv[++i]);
				break;

			case 'c'://add white background and darken sidewalls
				addWiteGroundPlate = 1;
				darkerwall *= 0.6;
				break;

			case 'd'://office depth
				office_depth_ft = atof(argv[++i]);
				office_depth = feet2meters(office_depth_ft);
				break;

			case 'e'://office depth
				window_width = atof(argv[++i]);
				if (window_width < 0 || window_width>1)
				{
					error(USER, "Widnow Width (option \'e\' must be betwee 0 and 1)");
				}
				break;


			case 'f':
				frame_width = inch2meters(atof(argv[++i]));
				break;

			case 'h'://ceiling height
				ceiling_height = feet2meters(atof(argv[++i]));
				floor_height = ceiling_height + feet2meters(1);
				break;

			case 'i'://input file for points
				strcpy(pts_file, argv[++i]);
				break;

			case 'm':
				ceiling_refl = atof(argv[++i]);
				wall_refl = atof(argv[++i]);
				floor_refl = atof(argv[++i]);
				ceiling_refl *= 0.01;
				wall_refl *= 0.01;
				floor_refl *= 0.01;
				break;

			case 'o':
				strcpy(rad_file, argv[++i]);
				break;

			case 'p':

				seating_position = atoi(argv[++i]);
				if (seating_position == 1 || seating_position == 2 || seating_position == 3)
				{
					seating_position *= 0.3048; //TODO this sets them all to zero!!!
					break;
				}
				else
				{
					error(USER, "seating position must be 1, 2 or 3!");
				}
				break;
			case 's':
				sill = inch2meters(atof(argv[++i]));
				break;
			case 't':
				visual_transmittance = 0.01*atof(argv[++i]);
				break;
			case 'x':
				strcpy(bin_dir, argv[++i]);
				break;

			case 'w':
				office_width_ft = atof(argv[++i]);
				office_width = feet2meters(office_width_ft);
				break;

			case 'z':
				facade_type = atoi(argv[++i]);
				if (!(facade_type == 1 || facade_type == 2 || facade_type == 3))
				{
					error(USER, "facade type must be 1, 2 or 3!");
				}
				break;

			case 'l':
				office_level = atoi(argv[++i]);

				if (office_level >= 0 || office_level < num_building_floors) //TODO is zero allowed?
				{
					points_height = (office_level - 1)*floor_height + 0.85;
				}
				else
				{
					error(USER, "The office cannot be located on a floor higher than the office building and cannot be 0 or negative");
				}

				break;

			case 'y':
				num_building_floors = atoi(argv[++i]);

				if (num_building_floors >= 0)
				{
					if (office_level > num_building_floors)
					{
						error(USER, "The office building cannot be lower than the floor that the office is located on");
					}
					else
					{
						building_height = num_building_floors*(floor_height);
					}
				}
				else
				{
					error(USER, "The building cannot have a negative height!");
				}

				break;
			case 'n':
				building_width_ft = atof(argv[++i]);
				if (building_width_ft >= office_width_ft)
				{
					building_width = feet2meters(building_width_ft);
				}
				else
				{
					error(USER, "The building not less wide than the office itself!");
				}
				break;
			case 'k':
				building_depth_ft = atof(argv[++i]);
				if (building_depth_ft >= office_depth_ft)
				{
					building_depth = feet2meters(building_depth_ft);
				}
				else
				{
					error(USER, "The building cannot be less deep than the office itself!");
				}
				break;

			case 'j':
				facade_refl = atof(argv[++i]);
				if (facade_refl >= 0 || facade_refl <= 100)
				{
					facade_refl *= 0.01;
				}
				else
				{
					error(USER, "The office buiding can only have a reflectance between 0 and 100%% (inclusive)!");
				}
				break;

			case 'U':
				obstruction = atoi(argv[++i]);
				if (!(obstruction == 0 || obstruction == 1))
				{
					error(USER, "Obstruction must exist (1) or not exist (0)...only 0 or 1 can be passed as parameters!");
				}
				break;
			case 'M':
				building_refl = atof(argv[++i]);
				if (building_refl >= 0 || building_refl <= 100)
				{
					building_refl *= 0.01;
				}
				else
				{
					error(USER, "Obstructing buildings must have a reflectance between 0 and 100 %% (inclusive)!");
				}
				break;

			case 'W':
				obstruction_building_width_ft = atof(argv[++i]);
				if (obstruction_building_width_ft != building_width_ft)
				{
					error(USER, "Obstructing building and Office building must abide by same building standards...widths must be equal!");
				}
				else if (obstruction_building_width_ft > 0)
				{
					obstruction_building_width = feet2meters(obstruction_building_width_ft);
				}
				else
				{
					error(USER, "Obstructing building must have a positive width!");
				}
				break;

			case 'D':
				obstruction_building_depth_ft = atof(argv[++i]);
				if (obstruction_building_depth_ft != building_depth_ft)
				{
					error(USER, "Obstructing building and Office building must abide by same building standards...depths must be equal!");
				}
				else if (obstruction_building_depth_ft > 0)
				{
					obstruction_building_depth = feet2meters(obstruction_building_depth_ft);
				}
				else
				{
					error(USER, "Obstructing building must have a positive depth!");
				}
				break;

			case 'H':
				obstruction_building_height_ft = atof(argv[++i]);
				if (obstruction_building_height_ft > 0) /***/
				{
					obstruction_building_height = feet2meters(obstruction_building_height_ft);
				}
				else
				{
					error(USER, "Obstructing building can only have a height greater than 0!");
				}
				break;

			case 'A':
				street_width_ft = atof(argv[++i]);
				if (street_width_ft > 0)
				{
					street_width = feet2meters(street_width_ft);
				}
				else
				{
					error(USER, "Street width must be greater than 0!");
				}
				break;

			case 'G':
				street_refl = atof(argv[++i]);
				if (street_refl >= 0 || street_refl <= 100)
				{
					street_refl *= 0.01;
				}
				else
				{
					error(USER, "Street reflectance must be between 0 and 100 %% (inclusive)!");
				}
				break;

			case 'C':
				obstruction_type = atoi(argv[++i]);
				if (!(obstruction_type == 0 || obstruction_type == 1))
				{
					error(USER, "Obstruction can only be continuous (0) or discontinuous (1)");
				}

				break;



			}//end switch (argv[i][1])
		}
		else
		{
			sprintf(errmsg, "bad option for input arguments: %s", argv[i]);
			error(USER, errmsg);
		}//end else
	}
}


//This method writes the vf file

void writeVFFile ()
{
	int i = 0, j= 0;
	strcpy(vf_file1,pts_file);
	VF_FILE1=open_output(vf_file1);

	for(i=1;i<((int)(office_depth_ft));i++)
	{
		//for each row of grid, we have to create a point where the illuminance is measured at
		double y = feet2meters(i);

		for(j=1;j<((int)(office_width_ft));j++)
		{

			double x = feet2meters(j);
			/*float points_height=((office_level-1)*floor_height)+0.85;*/


			double x_new = (cos(orientation*(-3.14159 / 180.0))*x + sin(orientation*(-3.14159 / 180.0))*y);
			double y_new = (-1.0)*sin(orientation*(-3.14159 / 180.0))*x + cos(orientation*(-3.14159 / 180.0))*y;
			fprintf(VF_FILE1,"%.4f %.4f %.4f 0 0 1\n",x_new,y_new,points_height);

    	}//end for

	}//end for

	close_file(VF_FILE1);
}

//This method writes the Radiance header file
void writeRadianceHeaderFile(int argc, char  *argv[])
{
	int i=0;
	RAD_FILE=open_output(rad_file);
	/* file header */
	fprintf(RAD_FILE,"# ");

	for (i = 0; i < argc; i++)
	{
		fprintf(RAD_FILE,"%s ",argv[i]);
	}//end for

	writeCopyrightHeader();
	writeGeneralOfficeParameters();
	writeMaterialDescription();
	writeSkyDescription();
	writeWhiteGround();
	writeOfficeGeometry();

	if (obstruction == 1)
	{
		writeSurroundings();
	}

	close_file(RAD_FILE);
	setOfficeOrientation();
}

//This method writes the copyright information about the Radiance file created.
void writeCopyrightHeader()
{
	//writes Top portion of radiance header file
	fprintf(RAD_FILE,"\n\n");
	fprintf(RAD_FILE,"#####################################################\n");
	fprintf(RAD_FILE,"# This file has been created with gen_single office #\n");
	fprintf(RAD_FILE,"# Copyright (c) Christoph Reinhart           	      #\n");
	fprintf(RAD_FILE,"# National Research Council Canada           	      #\n");
	fprintf(RAD_FILE,"# IRC, Lighting Group                               #\n");
	fprintf(RAD_FILE,"# Ont K1A 0R6, Canada                               #\n");
	fprintf(RAD_FILE,"#####################################################\n");

}

//This method writes general building parameters for the Radiance header file
void writeGeneralOfficeParameters()
{
	fprintf(RAD_FILE, "# floor-ceiling height     =%4.4f (ft)\n", meters2meet(ceiling_height));
	fprintf(RAD_FILE, "# office width             =%4.4f (ft)\n", meters2meet(office_width));
	fprintf(RAD_FILE, "# office depth	     \t=%4.4f (ft)\n", meters2meet(office_depth));
	fprintf(RAD_FILE,"# ceiling reflection       =%4.4f (%%)\n", ceiling_refl*100);
	fprintf(RAD_FILE,"# floor reflection         =%4.4f (%%)\n", floor_refl*100);
	fprintf(RAD_FILE,"# window transmission      =%4.4f (%%)\n", visual_transmittance*100);
	fprintf(RAD_FILE," \n\n");
}

//This method writes the materials in the Radiance header file
void writeMaterialDescription()
{
	/* make materials*/
	fprintf(RAD_FILE,"########################\n");
	fprintf(RAD_FILE,"# material description #\n");
	fprintf(RAD_FILE,"########################\n\n");
	fprintf(RAD_FILE,"void plastic floor         0 0 5 %f %f %f 0 0\n", floor_refl,floor_refl,floor_refl);
	fprintf(RAD_FILE,"void plastic ceiling       0 0 5 %f %f %f 0 0\n", ceiling_refl,ceiling_refl,ceiling_refl);
	fprintf(RAD_FILE,"void plastic wall     0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void plastic side_wall     0 0 5 %f %f %f 0 0\n", wall_refl*darkerwall,wall_refl*darkerwall,wall_refl*darkerwall);
	fprintf(RAD_FILE,"void plastic ballustrade   0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void plastic window_frame  0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void plastic table         0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void plastic tisch_bein    0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void plastic head    	     0 0 5 %f %f %f 0 0\n", wall_refl,wall_refl,wall_refl);
	fprintf(RAD_FILE,"void glass glazing  	     0 0 3 %f %f %f\n", transmissivity(visual_transmittance),transmissivity(visual_transmittance),transmissivity(visual_transmittance));
	fprintf(RAD_FILE,"void plastic pavement		 0 0 5 %f %f %f 0 0\n",street_refl,street_refl,street_refl);
	fprintf(RAD_FILE,"void plastic facade		 0 0 5 %f %f %f 0 0\n",facade_refl,facade_refl,facade_refl);
	fprintf(RAD_FILE,"void plastic obstruct_building		 0 0 5 %f %f %f 0 0\n",building_refl,building_refl,building_refl);

	fprintf(RAD_FILE," \n\n");
}

//This method writes the sky conditions/description to the Radiance header file
void writeSkyDescription()
{
	if(add_sky)
	{ /*puts in a sky and more detailed furniture*/
		/* surrounding sky mapping */
		fprintf(RAD_FILE,"###################\n");
		fprintf(RAD_FILE,"# surrounding sky #\n");
		fprintf(RAD_FILE,"###################\n\n");
		fprintf(RAD_FILE,"!gensky %d %d %f \n",month,day,hour);
		fprintf(RAD_FILE,"skyfunc glow sky_glow\n0\n0\n4 1 1 1 0\n\n");
		fprintf(RAD_FILE,"skyfunc colorpict mysky 7 clip_r clip_g clip_b\n");
		fprintf(RAD_FILE,"../instances/skyline_south.pic ../instances/proj_pict.cal pic_u pic_v\n");
		fprintf(RAD_FILE,"0\n5  0 0 120 1.49 1\n\n");
		fprintf(RAD_FILE,"mysky glow sky_glow\n0\n0\n4  1 1 1  0\n\n");
		fprintf(RAD_FILE,"sky_glow source sky\n0\n0\n4  0 0 1 180\n\n");
		fprintf(RAD_FILE,"mysky glow ground_glow\n0\n0\n4 1 1 1 0\n\n");
		fprintf(RAD_FILE,"ground_glow source sky\n0\n0\n4 0 0 -1 180\n\n");
	}//end if(add_sky)
}

//This method writes the office geometry in the Radiance header file
void writeOfficeGeometry()
{
	int i = 0;
	first_line=(int)(-1.0*number_of_neighbors*1.0/2);
	for(i=first_line;i<(first_line+number_of_neighbors);i++)
	{
		/* make floor and ceiling*/

		fprintf(RAD_FILE,"###################\n");
		fprintf(RAD_FILE,"# office geometry #\n");
		fprintf(RAD_FILE,"###################\n\n");
		writeCeilingGeometry();
		writeFloorGeometry();
		writeOfficeFacadeGeometry(i);
		writeOfficeWallGeometry();
		if (obstruction==1)
		{
			writeBuildingGeometry(i);
			writeBuildingFacadeGeometry(i);
		}

	}//end for
}


//This method writes the white background floor for the Radiance header file

void writeWhiteGround()
{
	if (addWiteGroundPlate)
	{
		fprintf(RAD_FILE,"void glow white_ground_mat \n0\n0\n4\n1 1 1 0\n");
		fprintf(RAD_FILE,"white_ground_mat source white_ground\n0\n0\n4\n0 0 -1 180\n");
	}
}

//This method writes the floor information for the Radiance header file

void writeFloorGeometry()
{
	fprintf(RAD_FILE,"floor polygon floor\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t0\t%.4f \n",0.0, (office_level-1)*floor_height);
	fprintf(RAD_FILE,"\t%.4f\t0\t%.4f \n",office_width, (office_level-1)*floor_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",office_width,office_depth,(office_level-1)*floor_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n\n",0.0,office_depth,(office_level-1)*floor_height);
}

//This method writes the ceiling information for the Radiance header file

void writeCeilingGeometry()
{
	fprintf(RAD_FILE,"ceiling polygon ceiling\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n",0.0,-1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n",office_width,-1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n",office_width,office_depth,((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n\n",0.0,office_depth,((office_level-1)*(floor_height)+ceiling_height));
}

//This method writes the facade information for the Radiance header file
void writeOfficeFacadeGeometry(int i)
{

	if (facade_type==1)
	{
		/* make facade*/
		if(ballustrade)
		{
			fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f 0.75",bin_dir,i,office_width,frame_width+0.2);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2),(office_level-1)*(floor_height));
		}else{
			//side ballustrade pieces near the window
			if(window_width<1)
			{
				fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f 0.75",bin_dir,i,office_width*0.5*(1.0-window_width),frame_width+0.2);
				fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2),(office_level-1)*(floor_height));
				fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f 0.75",bin_dir,i,office_width*0.5*(1.0-window_width),frame_width+0.2);
				fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*(1.0-0.5*(1.0-window_width)),-1*(frame_width+0.2),(office_level-1)*(floor_height));
			}

			/*bottom window frame */
			fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,1.0*window_width*office_width,frame_width,frame_width);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2), ((office_level-1)*(floor_height)));

			/*top window frame*/
			fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,1.0*window_width*office_width,frame_width,frame_width);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2), ((office_level-1)*(floor_height))+0.75-frame_width);

			/*side window frames*/
			fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,0.75-2*frame_width);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2),((office_level-1)*floor_height)+frame_width);

			fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,0.75-2*frame_width);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,i*office_width+office_width-frame_width-office_width*0.5*(1.0-window_width),-1*(frame_width+0.2),((office_level-1)*floor_height)+frame_width);

			//fprintf(RAD_FILE,"glazing polygon glazing_%d\n0\n0\n12\n",i);
			fprintf(RAD_FILE,"glazing polygon glazing_%d\n0\n0\n12\n",i);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",frame_width+office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),(office_level-1)*floor_height+frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width-office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),(office_level-1)*floor_height+frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width-office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),((office_level-1)*(floor_height)+0.75)-frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",frame_width+office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),((office_level-1)*(floor_height)+0.75)-frame_width);

		}

		//sill
		fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f %.4f",bin_dir,i,office_width,frame_width+0.2,sill);
		fprintf(RAD_FILE,"| %sxform -t %.4f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2),(office_level-1)*(floor_height)+ceiling_height-sill);

		//side facade pieces near the main window
		if(window_width<1)
		{
			fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f %.4f",bin_dir,i,office_width*0.5*(1.0-window_width),frame_width+0.2,ceiling_height-0.75-sill);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2),(office_level-1)*(floor_height)+0.75);
			fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f %.4f",bin_dir,i,office_width*0.5*(1.0-window_width),frame_width+0.2,ceiling_height-0.75-sill);
			fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*(1.0-0.5*(1.0-window_width)),-1*(frame_width+0.2),(office_level-1)*(floor_height)+0.75);
		}

		/* make window frame */
		/*bottom window frame */
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,1.0*window_width*office_width,frame_width,frame_width);
		fprintf(RAD_FILE,"| %sxform -t %.4f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2), ((office_level-1)*(floor_height)+0.75));

		/*top window frame*/
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,1.0*window_width*office_width,frame_width,frame_width);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2), ((office_level-1)*(floor_height)+ceiling_height)-sill-frame_width);

		/*side window frames*/
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,ceiling_height-0.75-2*frame_width-sill);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,i*office_width+office_width*0.5*(1.0-window_width),-1*(frame_width+0.2),((office_level-1)*floor_height)+0.75+frame_width);

		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,ceiling_height-0.75-2*frame_width-sill);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,i*office_width+office_width-frame_width-office_width*0.5*(1.0-window_width),-1*(frame_width+0.2),((office_level-1)*floor_height)+0.75+frame_width);

		/*make window glazing */
		if(!addWiteGroundPlate)
		{
			fprintf(RAD_FILE,"glazing polygon glazing_%d\n0\n0\n12\n",i);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",frame_width+office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),(office_level-1)*floor_height+0.75+frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width-office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),(office_level-1)*floor_height+0.75+frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width-office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height)-sill-frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",frame_width+office_width*0.5*(1.0-window_width),-1.0*(0.5*frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height)-sill-frame_width);
		}
	}//if (facade_type==1)

	if (facade_type==2)
	{
		/* make facade */
		fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f 0.75",bin_dir,i,office_width,frame_width+0.2);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2), (office_level-1)*floor_height);

		fprintf(RAD_FILE,"!%sgenbox ballustrade ballustrade_%d %.4f %.4f %.4f",bin_dir,i,office_width,frame_width+0.2,0.25*ceiling_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2), ((office_level-1)*(floor_height)+0.75*ceiling_height-sill));

		/* make window frame */

		/*bottom window frame */
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,office_width,frame_width,frame_width);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2), ((office_level-1)*floor_height)+0.75);

		/*top window frame*/
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,office_width,frame_width,frame_width);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2), ((office_level-1)*floor_height)+0.75*ceiling_height-sill-frame_width);

		/*side window frames*/
		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,0.75*ceiling_height-0.75-2*frame_width-sill);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,i*office_width,-1*(frame_width+0.2),((office_level-1)*floor_height)+0.75+frame_width);

		fprintf(RAD_FILE,"!%sgenbox window_frame window_frame_%d %.4f %.4f %.4f",bin_dir,i,frame_width,frame_width,0.75*ceiling_height-0.75-2*frame_width-sill);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %f \n\n",bin_dir,office_width-frame_width,-1*(frame_width+0.2),((office_level-1)*floor_height)+0.75+frame_width);

		/*make window glazing */
		if(!addWiteGroundPlate)
		{
			fprintf(RAD_FILE,"glazing polygon glazing_%d\n0\n0\n12\n",i);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,-1.0*(0.5*frame_width+0.2),(office_level-1)*(floor_height));
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width,-1.0*(0.5*frame_width+0.2),(office_level-1)*(floor_height));
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width-frame_width,-1.0*(0.5*frame_width+0.2),((office_level-1)*floor_height)+0.75*ceiling_height-sill-frame_width);
			fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,-1.0*(0.5*frame_width+0.2),((office_level-1)*floor_height)+0.75*ceiling_height-sill-frame_width);
		}
	}//end if (facade_type==2)

}

//This method specifies the right, left and back wall geometries in the Radiance header file
void writeOfficeWallGeometry()
{
	/*make back wall*/
	fprintf(RAD_FILE,"wall polygon back_wall\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", 0.0,office_depth,(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", office_width,office_depth,(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", office_width,office_depth,((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", 0.0,office_depth,((office_level-1)*(floor_height)+ceiling_height));

	/* make left walls*/
	fprintf(RAD_FILE,"side_wall polygon wall_left\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n",office_width,-1*(frame_width+0.2),(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f \n",office_width,office_depth,(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",office_width,office_depth,((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n\n",office_width,-1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height));

	/* make right walls*/
	fprintf(RAD_FILE,"side_wall polygon wall_right\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,-1*(frame_width+0.2),(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,office_depth,(office_level-1)*(floor_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,office_depth,((office_level-1)*(floor_height)+ceiling_height));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",0.0,-1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height));
}

void setOfficeOrientation()
{
	if(orientation!=0)
	{
		sprintf(befehl,"%sxform -rz %.0f %s > %s.tmp\n",bin_dir,orientation,rad_file,rad_file);
		/*printf("%s",befehl);*/
		ORIENT=popen(befehl,"r");
		pclose(ORIENT);
		sprintf(befehl,"mv %s.tmp %s\n",rad_file,rad_file);
		/*printf("%s",befehl);*/
		ORIENT=popen(befehl,"r");
		pclose(ORIENT);
	}//end  if(orientation!=0)

}


//This method sets the ground paramters in the radiance file
void writeBuildingGeometry(int i)
{
	/*building floor*/
	fprintf(RAD_FILE,"facade polygon building_floor\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",-1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",((building_width/2)+(office_width/2)),-1*(frame_width+0.2));//i*office_width+office_width);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0\n",((building_width/2)+(office_width/2)),(building_depth-(frame_width+0.2)));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0\n\n",-1*((building_width/2)-(office_width/2)),building_depth + -1*(frame_width+0.2));

	/*building left wall*/

	fprintf(RAD_FILE,"facade polygon building_left_wall\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",((building_width/2)+(office_width/2)),-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",((building_width/2)+(office_width/2)),(building_depth-(frame_width+0.2)));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",((building_width/2)+(office_width/2)),(building_depth-(frame_width+0.2)), building_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n\n",((building_width/2)+(office_width/2)),-1*(frame_width+0.2), building_height);

	/*building right wall*/

	fprintf(RAD_FILE,"facade polygon building_right_wall\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n",-1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n",-1*((building_width/2)-(office_width/2)),(building_depth-(frame_width+0.2)));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",-1*((building_width/2)-(office_width/2)),(building_depth-(frame_width+0.2)), building_height);
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",-1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2),building_height);

	/*building back wall*/
	fprintf(RAD_FILE,"facade polygon building_back_wall\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n", -1*((building_width/2)-(office_width/2)),(building_depth-(frame_width+0.2)));
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n", ((building_width/2)+(office_width/2)), (building_depth-(frame_width+0.2)));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",((building_width/2)+(office_width/2)), (building_depth-(frame_width+0.2)), building_height);
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", -1*((building_width/2)-(office_width/2)),(building_depth-(frame_width+0.2)), building_height);

	/*building roof wall*/
	fprintf(RAD_FILE,"facade polygon building_roof\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",-1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2),building_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",((building_width/2)+(office_width/2)),-1*(frame_width+0.2),building_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",((building_width/2)+(office_width/2)),(building_depth-(frame_width+0.2)),building_height);
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t%.4f\n",-1*((building_width/2)-(office_width/2)),building_depth + -1*(frame_width+0.2),building_height);


}

/* This method writes the surrounding ground information*/
void writePavementGeometry()
{
	fprintf(RAD_FILE,"pavement polygon pavement\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",(-1*((building_width/2)-(office_width/2))-(2*street_width)-building_width),(-1*(frame_width+0.2)-(2*street_width)-building_depth));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0 \n",((building_width/2)+(office_width/2)+(2*street_width)+building_width),(-1*(frame_width+0.2)-(2*street_width)-building_depth));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0\n",((building_width/2)+(office_width/2)+(2*street_width)+building_width),((building_depth-(frame_width+0.2))+(2*street_width)+building_depth));
	fprintf(RAD_FILE,"\t%.4f\t%.4f\t0\n\n",(-1*((building_width/2)-(office_width/2))-(2*street_width)-building_width),(building_depth-(frame_width+0.2)+(2*street_width)+building_depth));

}

//this function writes the building facade geometry for the Radiance header file
void writeBuildingFacadeGeometry(int i)
{
	/*left hand block of facade*/
	fprintf(RAD_FILE,"facade polygon left_block_of_facade\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n", -1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t0\t%f\t0 \n", -1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t0\t%f\t%f \n",-1*(frame_width+0.2), building_height);
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", -1*((building_width/2)-(office_width/2)),-1*(frame_width+0.2), building_height);

	if (facade_type==1)
	{
		/*top block of facade*/
		fprintf(RAD_FILE,"facade polygon top_block_of_facade\n0\n0\n12\n");
		fprintf(RAD_FILE,"\t0\t%f\t%f \n",-1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height-sill));
		fprintf(RAD_FILE,"\t%f\t%f\t%f \n", office_width, -1*(frame_width+0.2),((office_level-1)*(floor_height)+ceiling_height-sill));
		fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width, -1*(frame_width+0.2), building_height);
		fprintf(RAD_FILE,"\t0\t%f\t%f \n", -1*(frame_width+0.2), building_height);
	}
	if (facade_type == 2)
	{
		/*top block of facade*/
		fprintf(RAD_FILE,"facade polygon top_block_of_facade\n0\n0\n12\n");
		fprintf(RAD_FILE,"\t0\t%f\t%f \n",-1*(frame_width+0.2),((office_level-1)*(floor_height)+0.75*ceiling_height-sill));
		fprintf(RAD_FILE,"\t%f\t%f\t%f \n", office_width, -1*(frame_width+0.2),((office_level-1)*(floor_height)+0.75*ceiling_height-sill));
		fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width, -1*(frame_width+0.2), building_height);
		fprintf(RAD_FILE,"\t0\t%f\t%f \n", -1*(frame_width+0.2), building_height);
	}

	/*bottom block of facade*/
	fprintf(RAD_FILE,"facade polygon bottom_block_of_facade\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t0\t%f\t0 \n",-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n", office_width, -1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",office_width, -1*(frame_width+0.2), ((office_level-1)*floor_height)+0.75);
	fprintf(RAD_FILE,"\t0\t%f\t%f \n", -1*(frame_width+0.2), ((office_level-1)*floor_height)+0.75);


	/*right hand block of facade*/
	fprintf(RAD_FILE,"facade polygon right_block_of_facade\n0\n0\n12\n");
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n",office_width,-1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%f\t%f\t0 \n", ((building_width/2)+(office_width/2)), -1*(frame_width+0.2));
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n",((building_width/2)+(office_width/2)),-1*(frame_width+0.2), building_height);
	fprintf(RAD_FILE,"\t%f\t%f\t%f \n", office_width,-1*(frame_width+0.2), building_height);
}

/* This function writes the external shading surroundings*/
void writeSurroundings()
{
	writePavementGeometry();
	if (obstruction_type == 0)
	{
		writeContinuousObstBuildings();
	}
	else if (obstruction_type == 1)
	{
    	writeDiscontinuousObstBuildings();
	}
}


/*This function writes the continuous obstructing buildings*/
void writeContinuousObstBuildings()
{
		/*obstructing building on west side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade W_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2),0.0);

		/*obstructing building on east side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade E_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2),0.0);

		/*obstructing building on north-west side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade NW_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		/*obstructing building on north side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade N_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		/*obstructing building on north-east side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade NE_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		/*obstructing building on south-west side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade SW_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		/*obstructing building on south side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade S_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		/*obstructing building on south-east side of target office building*/
		fprintf(RAD_FILE,"!%sgenbox facade SE_obstr_building %.4f %.4f %.4f",bin_dir,obstruction_building_width,obstruction_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);
}

/*This function writes the discontinuous obstructing buildings*/

void writeDiscontinuousObstBuildings()
{
		/*******************block of obstructing buildings on west side of target office building**********************/
		fprintf(RAD_FILE,"!%sgenbox facade W1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade W2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade W3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2),0.0);


		fprintf(RAD_FILE,"!%sgenbox facade W1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+discont_building_depth+spacing_btw_discon_buildings,0.0);

		/*fprintf(RAD_FILE,"!%sgenbox facade W2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings),0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade W3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings),0.0);


		fprintf(RAD_FILE,"!%sgenbox facade W1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade W2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade W3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		/***********************block of obstructing buildings on east side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade E1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade E2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade E3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2),0.0);


		fprintf(RAD_FILE,"!%sgenbox facade E1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+discont_building_depth+spacing_btw_discon_buildings,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade E2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings),0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade E3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings),0.0);


		fprintf(RAD_FILE,"!%sgenbox facade E1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade E2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		fprintf(RAD_FILE,"!%sgenbox facade E3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings),0.0);

		/***********************block of obstructing buildings on north east side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade NE1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NE1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+discont_building_depth+spacing_btw_discon_buildings+obstruction_building_depth+street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade NE2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade NE3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NE1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		/***********************block of obstructing buildings on north side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade N1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade N2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade N3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade N1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade N2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade N3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade N1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade N2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade N3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		/***********************block of obstructing buildings on south east side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade NE1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NE1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade NE2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade NE3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NE1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NE3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,((obstruction_building_width/2)+(office_width/2)+street_width)+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);


		/***********************block of obstructing buildings on south side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade S1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade S2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade S3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade S1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade S2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade S3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade S1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2)),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade S2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade S3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);


		/***********************block of obstructing buildings on south-west side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade SW1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade SW2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade SW3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade SW1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade SW2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade SW3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade SW1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade SW2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade SW3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)-obstruction_building_depth-street_width,0.0);

		/***********************block of obstructing buildings on north-west side of target office building***********************/
		fprintf(RAD_FILE,"!%sgenbox facade NW1-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NW2-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NW3-1_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NW1-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);
		/*
		fprintf(RAD_FILE,"!%sgenbox facade NW2-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);
		*/
		fprintf(RAD_FILE,"!%sgenbox facade NW3-2_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);


		fprintf(RAD_FILE,"!%sgenbox facade NW1-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width,-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NW2-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

		fprintf(RAD_FILE,"!%sgenbox facade NW3-3_disc_obstr_building %.4f %.4f %.4f",bin_dir,discont_building_width,discont_building_depth,obstruction_building_height);
		fprintf(RAD_FILE,"| %sxform -t %f %.4f %.4f \n\n",bin_dir,-1*((obstruction_building_width/2)-(office_width/2))-obstruction_building_width-street_width+2*(discont_building_width+spacing_btw_discon_buildings),-1*(frame_width+0.2)+2*(discont_building_depth+spacing_btw_discon_buildings)+obstruction_building_depth+street_width,0.0);

}

double transmissivity(double visual_transmittance)
{
	double trans = sqrt(0.8402528435 + 0.0072522239*visual_transmittance*visual_transmittance);
	trans=(trans-0.9166530661)/0.0036261119/visual_transmittance;
	return(trans);
}
