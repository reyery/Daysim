/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */
#ifndef OCC_FUNC_H
#define OCC_FUNC_H



#define M1 259200
#define IA1 7141
#define IC1 54773
#define RM1 (1.0/M1)
#define M2 134456
#define IA2 8121
#define IC2 28411
#define RM2 (1.0/M2)
#define M3 243000
#define IA3 4561
#define IC3 51349


void generate_routine_file(char *filename,int weekday,char *filename1);
void read_occupancy_profile(char *filename);
int  get_occupancy(int weekday, int time_step, float hour);
void occupancy_profile( int occupancy_mode, int daylight_savings_time,
						int start_work, int end_work, int time_step,
						int first_weekday);
void read_occupancy_profile(char *filename);
void gen_Lightswitch_routine();
void nrerror ( char *error_text );
float ran1 ( long *idum );


#endif
