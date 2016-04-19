/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */
#ifndef OCC_FUNC_H
#define OCC_FUNC_H



void generate_routine_file(char *filename,int weekday,char *filename1);
void read_occupancy_profile(char *filename);
int  get_occupancy(int weekday, int time_step, double hour);
void occupancy_profile( int occupancy_mode, int daylight_savings_time,
	float start_work, float end_work, int time_step, int first_weekday);
void read_occupancy_profile(char *filename);
void gen_Lightswitch_routine();


#endif
