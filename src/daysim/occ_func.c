/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fropen.h"
#include "read_in_header.h"
#include "occ_func.h"
#include "sun.h"

//=======================================================================
//procedure create occupancy routine for Lightswitch Wizard single office
//=======================================================================
void gen_Lightswitch_routine()
{
	int j,i;
	float occ_routine[288][9];
	float length_of_working_day;

	length_of_working_day=end_work-start_work;
	lunch_time=0.5*(end_work+start_work)-0.25; //lunch at half time
	lunch_time_length=1.0;

	for (j=0 ; j<288 ; j++){
		occ_routine[j][0]=j*(1.0/12.0);
		occ_routine[j][1]=0;
		occ_routine[j][2]=0;
		occ_routine[j][3]=0;
		occ_routine[j][4]=0;
		occ_routine[j][5]=0;
		occ_routine[j][6]=0;
		occ_routine[j][7]=0;
		occ_routine[j][8]=0;
	}

	//  The user arrives and leaves for the day within a plus/minus 15 minutes time window
	//  with respect to the input arrival and departure times.
	if(length_of_working_day>0.25){
		for (j=0 ; j<6 ; j++){
			i=(int)(start_work*12.0);
			occ_routine[j-2+i][1]=0.166; 	//arrival
			i=(int)(end_work*12.0);
			occ_routine[j-2+i][2]=0.166;	//departure
		}
	}


	//  If the working day is longer than 6 hours, the user leaves for two 30 minute breaks
	//  and a 60 minute lunch break.
	if(length_of_working_day>6.00){
		for (j=0 ; j<6 ; j++){
			i=(int)(0.5*(0.5*(start_work+end_work)+start_work)*12.0);
			occ_routine[j-2+i][3]=0.166; 	//morning break
		}
		occ_routine[6][4]=1.0;				// length of morning break

		for (j=0 ; j<6 ; j++){
			i=(int)(0.5*(end_work+start_work)*12.0);
			occ_routine[j-2+i][5]=0.166; 	//lunch break

		}
		occ_routine[12][6]=1.0;				// length of lunch break

		for (j=0 ; j<6 ; j++){
			i=(int)(0.5*(0.5*(start_work+end_work+2.0)+end_work)*12.0);
			occ_routine[j-2+i][7]=0.166; 	//afternoon break
		}
		occ_routine[6][8]=1.0;				// length of afternoon break
	}else{ // end (length_of_working_day>6.00)
		// If the working day is between 3 and 6 hours leaves, the user leaves the work place
		// twice for 30 minute breaks.
		if(length_of_working_day>3.00){
			for (j=0 ; j<6 ; j++){
				i=(int)((start_work+ 0.3333*(end_work-start_work))*12.0);
				occ_routine[j-2+i][3]=0.166; 	//morning break
			}
			occ_routine[6][4]=1.0;				// length of morning break

			for (j=0 ; j<6 ; j++){
				i=(int)((start_work+ 0.6666*(end_work-start_work))*12.0);
				occ_routine[j-2+i][7]=0.166; 	//afternoon break
			}
			occ_routine[6][8]=1.0;				// length of afternoon break
		}else{
		// If the working day is less than 3 hours long, the user leaves the work place once
		// for a 30 minute break.
			for (j=0 ; j<6 ; j++){
				i=(int)((start_work+ 0.5*(end_work-start_work))*12.0);
				occ_routine[j-2+i][3]=0.166; 	//morning break
			}
			occ_routine[6][4]=1.0;				// length of morning break
		}
	}


	//assign cummulated daily occupancy profile
	//=========================================
	occ_cummulated[0][0]=occ_routine[0][0];occ_cummulated[0][1]=occ_routine[0][1];
	occ_cummulated[0][2]=occ_routine[0][2];occ_cummulated[0][3]=occ_routine[0][3];
	occ_cummulated[0][4]=occ_routine[0][4];occ_cummulated[0][5]=occ_routine[0][5];
	occ_cummulated[0][6]=occ_routine[0][6];occ_cummulated[0][7]=occ_routine[0][7];
	occ_cummulated[0][8]=occ_routine[0][8];
	for (j=1 ; j<288 ; j++){
		occ_cummulated[j][0]=occ_routine[j][0];
		occ_cummulated[j][1]=occ_routine[j][1]+occ_cummulated[j-1][1];/*arrival*/
		occ_cummulated[j][2]=occ_routine[j][2]+occ_cummulated[j-1][2];/*departure*/
		occ_cummulated[j][3]=occ_routine[j][3]+occ_cummulated[j-1][3];/*morning break */
		occ_cummulated[j][4]=occ_routine[j][4]+occ_cummulated[j-1][4];/*morning break length */
		occ_cummulated[j][5]=occ_routine[j][5]+occ_cummulated[j-1][5];/*lunch break */
		occ_cummulated[j][6]=occ_routine[j][6]+occ_cummulated[j-1][6];/*lunch break length */
		occ_cummulated[j][7]=occ_routine[j][7]+occ_cummulated[j-1][7];/*afternoon break */
		occ_cummulated[j][8]=occ_routine[j][8]+occ_cummulated[j-1][8];/*afternoon break length */
	}
	for (j=1 ; j<288 ; j++){
		occ_cummulated[j][2]/=occ_cummulated[287][2]*1.0;
		occ_cummulated[j][4]/=occ_cummulated[287][4]*1.0;
		occ_cummulated[j][6]/=occ_cummulated[287][6]*1.0;
		occ_cummulated[j][8]/=occ_cummulated[287][8]*1.0;
	}

}


//=======================================================
// procedure generates a routine file for a user based on
// a given time series of measured occupancy data
//=======================================================
void generate_routine_file(char *filename,int weekday,char *filename1)
{
	FILE *INPUT_FILE;
	FILE *OUTPUT_FILE;
	int occ_status[289];
	int day_old=0,i,j;
	int arr=0,dep=0;
	int histo_arr[288],histo_dep[288];
	int histo_lunch[288],histo_morning[288],histo_afternoon[288];
	int histo_lunch_l[288],histo_morning_l[288],histo_afternoon_l[288];
	int number_of_weekdays=0, number_of_occ_weekdays=0;
	int number_of_mornings=0;
	int number_of_lunchs=0;
	int month1,day1;
	int total_occupancy=0;
	float hour1;
	float norm_factor=1;
	int number_of_afternoons=0;

	for (i=0 ; i<288 ; i++){
		histo_arr[i]=0;
		histo_dep[i]=0;
		histo_lunch[i]=0;
		histo_morning[i]=0;
		histo_afternoon[i]=0;
		histo_lunch_l[i]=0;
		histo_morning_l[i]=0;
		histo_afternoon_l[i]=0;
	}
	i=1;
	INPUT_FILE=open_input(filename);
	fscanf(INPUT_FILE,"%d %d %f %d",&month1,&day_old, &hour1,&occ_status[0]);
	while( EOF != fscanf(INPUT_FILE,"%d %d %f %d",&month1,&day1, &hour1,&occ_status[i])){
		if(day_old==day1){
			i++;
		}else{
			day_old=day1;
			if(weekday<6){
				j=0;
				for (i=0 ; i<288 ; i++){j+=occ_status[i];}
				if(j>=0){
					number_of_weekdays++;
					total_occupancy+=j;
					for (i=0 ; i<288 ; i++){
						if(occ_status[i]==0 && occ_status[i+1]==1 && arr==0){arr=i+1;}
						if(occ_status[i]==1 && occ_status[i+1]==0 ){dep=i;}
					}
					if(arr>0){
						number_of_occ_weekdays++;
						histo_arr[arr]+=1;
						histo_dep[dep]+=1;
					}
					/* devide the days into morning, lunch and afternoon */
					if((dep-arr)>24  ){
					for (i=arr ; (i<280 && i<dep) ; i++){
						if(occ_status[i]==1 && occ_status[i+1]==0  ){
							j=1;
							while(occ_status[i+j]==0){j++;} /* get break length */
						if (j>6){
							if((i*1.0/12.0)<lunch_time){
								number_of_mornings+=1;
								histo_morning[i]+=1;
								histo_morning[i+1]+=1;
								histo_morning[i+2]+=1;
								histo_morning_l[j]+=1;
							}
							if((i*1.0/12.0)>=lunch_time && (i*1.0/12.0)<(lunch_time+lunch_time_length)){
								number_of_lunchs+=1;
								histo_lunch[i]+=1;
								histo_lunch[i+1]+=1;
								histo_lunch[i+2]+=1;
								histo_lunch_l[j]+=1;
							}
							if((i*1.0/12.0)>=(lunch_time+lunch_time_length)){
								number_of_afternoons+=1;
								histo_afternoon[i]+=1;
								histo_afternoon[i+1]+=1;
								histo_afternoon[i+2]+=1;
								histo_afternoon_l[j]+=1;
							}
						}
						i+=j;
						}
						j=0;
					}
					}
				}
			}
			occ_status[0]=occ_status[288];
			i=1;
			arr=0;
			dep=0;
			weekday++;if(weekday>7){weekday=1;}
		}
	}
	/*printf("%f %f\n",lunch_time,lunch_time_length);*/
	OUTPUT_FILE=open_output(filename1);
	fprintf(OUTPUT_FILE,"lunch_time %f lunch_time_length %f\n",lunch_time,lunch_time_length);
	norm_factor=1.0;
	for (i=0 ; i<288 ; i++){
		fprintf(OUTPUT_FILE,"%2.3f %1.8f %1.6f ",i*(1/12.0),(histo_arr[i]*norm_factor)/number_of_weekdays,(histo_dep[i]*1.0)/number_of_occ_weekdays);
		fprintf(OUTPUT_FILE,"%1.6f %1.6f ",(histo_morning[i]*0.333333)/number_of_mornings,histo_morning_l[i]*1.0/number_of_mornings);
		fprintf(OUTPUT_FILE,"%1.6f %1.6f ",(histo_lunch[i]*0.333333)/number_of_lunchs,histo_lunch_l[i]*1.0/number_of_lunchs);
		fprintf(OUTPUT_FILE,"%1.6f %1.6f ",(histo_afternoon[i]*0.333333)/number_of_afternoons,histo_afternoon_l[i]*1.0/number_of_afternoons);
		fprintf(OUTPUT_FILE,"\n");
	}
	printf("   statistics: measured occupancy data:\n\t%d weekdays have been measured of which were %d occupied.",number_of_weekdays,number_of_occ_weekdays);
	if(number_of_occ_weekdays>0)
	  printf("\tAverage occupancy: %.1f [hours per weekday]\n",(total_occupancy*1.0/12)/(number_of_occ_weekdays));
	close_file(OUTPUT_FILE);
	close_file(INPUT_FILE);
}


/* procedure reads in an user routine generated by "genrate_routine_file" */
void read_occupancy_profile(char *filename)
{
	FILE *INPUT_FILE;
	int j;
	char word_1[200]="";
	char word_2[200]="";
	float occ_routine[288][9];

	INPUT_FILE=open_input(filename);
	fscanf(INPUT_FILE,"%s %f %s %f",word_1,&lunch_time,word_2,&lunch_time_length);
	for (j=0 ; j<288 ; j++){
		fscanf(INPUT_FILE,"%f %f %f %f %f ",&occ_routine[j][0],&occ_routine[j][1],&occ_routine[j][2],&occ_routine[j][3],&occ_routine[j][4]);
		fscanf(INPUT_FILE,"%f %f %f %f \n",&occ_routine[j][5],&occ_routine[j][6],&occ_routine[j][7],&occ_routine[j][8]);
	}
	close_file(INPUT_FILE);

	for (j=0 ; j<288 ; j++){
		/*printf("s %f %f \n",occ_routine[j][1],occ_routine[j][2]);*/
		occ_cummulated[j][0]=0;occ_cummulated[j][1]=0;occ_cummulated[j][2]=0;occ_cummulated[j][3]=0;occ_cummulated[j][4]=0;
		occ_cummulated[j][5]=0;occ_cummulated[j][6]=0;occ_cummulated[j][7]=0;occ_cummulated[j][8]=0;
	}
	occ_cummulated[0][0]=occ_routine[0][0];occ_cummulated[0][1]=occ_routine[0][1];
	occ_cummulated[0][2]=occ_routine[0][2];occ_cummulated[0][3]=occ_routine[0][3];
	occ_cummulated[0][4]=occ_routine[0][4];occ_cummulated[0][5]=occ_routine[0][5];
	occ_cummulated[0][6]=occ_routine[0][6];occ_cummulated[0][7]=occ_routine[0][7];
	occ_cummulated[0][8]=occ_routine[0][8];
	for (j=1 ; j<288 ; j++){
		occ_cummulated[j][0]=occ_routine[j][0];
		occ_cummulated[j][1]=occ_routine[j][1]+occ_cummulated[j-1][1];/*arrival*/
		occ_cummulated[j][2]=occ_routine[j][2]+occ_cummulated[j-1][2];/*departure*/
		occ_cummulated[j][3]=occ_routine[j][3]+occ_cummulated[j-1][3];/*morning break */
		occ_cummulated[j][4]=occ_routine[j][4]+occ_cummulated[j-1][4];/*morning break length */
		occ_cummulated[j][5]=occ_routine[j][5]+occ_cummulated[j-1][5];/*lunch break */
		occ_cummulated[j][6]=occ_routine[j][6]+occ_cummulated[j-1][6];/*lunch break length */
		occ_cummulated[j][7]=occ_routine[j][7]+occ_cummulated[j-1][7];/*afternoon break */
		occ_cummulated[j][8]=occ_routine[j][8]+occ_cummulated[j-1][8];/*afternoon break length */
	}

	for (j=1 ; j<288 ; j++){
		occ_cummulated[j][2]/=occ_cummulated[287][2]*1.0;
		occ_cummulated[j][4]/=occ_cummulated[287][4]*1.0;
		occ_cummulated[j][6]/=occ_cummulated[287][6]*1.0;
		occ_cummulated[j][8]/=occ_cummulated[287][8]*1.0;
	}
}

void test_date()
{
	int month=0,day=0;

	if(month==1 && day>31)
		month++;day-=31;
	if(month==2 && day>28)
		month++;day-=28;
	if(month==3 && day>31)
		month++;day-=31;
	if(month==4 && day>30)
		month++;day-=30;
	if(month==5 && day>31)
		month++;day-=31;
	if(month==6 && day>30)
		month++;day-=30;
	if(month==7 && day>31)
		month++;day-=31;
	if(month==8 && day>31)
		month++;day-=31;
	if(month==9 && day>30)
		month++;day-=30;
	if(month==10 && day>31)
		month++;day-=31;
	if(month==11 && day>30)
		month++;day-=30;
	if(month==12 && day>31)
		month=1;day-=31;
}

void get_arrival()
{
	float v1;
	int j,i;
	int month=0,day=0;
	double hour=0;
	float ran1 ( long *idum );

	v1=ran1(&idum);
	j=0;
	while(v1>occ_cummulated[j][1] && j<288){j++;}
	if(j<288){arrival=occ_cummulated[j][0];}else{arrival=-1;}

	if(arrival==-1)
	{
		departure=-1;
	}else{
		departure=0;
		i=0;
		while(departure<arrival)
		{
			j=0;i++;
			v1=ran1(&idum);
			while(!( (v1>=occ_cummulated[j][2]) && (v1<occ_cummulated[j+1][2])) || j==300)
			{
				if(j<287)
				{
					j++;
				}else{
					j=300;
				}
			}
			if(j<300)
			{
				departure=occ_cummulated[j][0];
			}else{
				fprintf(stderr,"WARNING: departure probability not 1\n");
				exit(1);
			}
		}
		if(departure<arrival && i>100)
		{
			fprintf(stderr,"WARNING: departure (%f) smaller arrival (%f) at %d %d %f !\n",departure,arrival,month,day,hour);
		}
	}
}//end of get arrival


// This function determines in which calendar week a given day is.
// function input are: month, day and the first weekday of the year.
int SetCalendarWeek(month, day,first_weekday)
{
	div_t e;

	e=div((jdate( month, day)+first_weekday-2),7);
	return(e.quot+1);

}


void get_day_profile()
{

	float v1;
	int j,i;
	int morning_break=0;
	int lunch_break=0;
	int AfternoonBreak=0;
	float ran1 ( long *idum );

	
	v1=ran1(&idum);
	j=0;
	while(v1>occ_cummulated[j][1] && j<288){j++;}
	if(j<288){arrival=occ_cummulated[j][0];}else{arrival=-1;}
	/*if(j==288){printf("date %d %d \n",month,day);}*/

	if(arrival==-1){departure=-1;}else{
		departure=0;
		i=0;
		while(departure<arrival){
			i++;j=0;v1=ran1(&idum);
			while(!( (v1>=occ_cummulated[j][2]) && (v1<occ_cummulated[j+1][2])) || j==300){if(j<288){j++;}else{j=300;}}
				if(j<300){departure=occ_cummulated[j][0];}else{fprintf(stderr,"WARNING: departure probability not 1\n");exit(1);}
				if(departure<arrival && i>100){
					fprintf(stderr,"WARNING: departure (%f) smaller arrival (%f)  !\n",departure,arrival);
				}
			}
	}
	morning_break=0;morning_length=0;morning_start=0;
	lunch_break=0;lunch_length=0;lunch_start=0;
	afternoon_length=0;afternoon_start=0;
/* get morning break */
	if((departure-arrival)>2 && (arrival<lunch_time)){
		v1=ran1(&idum);j=0;
		while(!( (v1>=occ_cummulated[j][3]) && (v1<occ_cummulated[j+1][3])) && j<300){
			if(j<287){j++;}else{j=300;}
		}
		if(j<300){
			morning_start=occ_cummulated[j+1][0];
			morning_break=1;
			v1=ran1(&idum);j=0;
			while(!( (v1>=occ_cummulated[j][4]) && (v1<occ_cummulated[j+1][4])) && j<300){
				if(j<287){j++;}else{j=300;}
			}
			if(j<300){morning_length=occ_cummulated[j+1][0];}else{morning_length=0;}
		}else{morning_break=0;morning_length=0;morning_start=0;}
		if(morning_start >departure){morning_break=0;morning_length=0;morning_start=0;}
		if(morning_start+morning_length>departure){morning_length=departure-morning_start;}
	}
/* get lunch break */
	if((departure-arrival)>2 && (departure>=lunch_time)){
		v1=ran1(&idum);j=0;
		while(!( (v1>=occ_cummulated[j][5]) && (v1<occ_cummulated[j+1][5])) && j<300){
			if(j<287){j++;}else{j=300;}
		}
		if(j<300){
			lunch_start=occ_cummulated[j+1][0];
			lunch_break=1;
			v1=ran1(&idum);j=0;
			while(!( (v1>=occ_cummulated[j][6]) && (v1<occ_cummulated[j+1][6])) && j<300){
				if(j<287){j++;}else{j=300;}
			}
			if(j<300){lunch_length=occ_cummulated[j+1][0];}else{lunch_length=0;}
		}else{lunch_break=0;lunch_start=0;lunch_length=0;}
		if(morning_start+morning_length>lunch_start){lunch_start=morning_start+morning_length;}
		if(lunch_start>departure){lunch_break=0;lunch_start=0;lunch_length=0;}
		if(lunch_start+lunch_length>departure){lunch_length=departure-lunch_start;}
	}
/* get afternoon break */
	if((departure-arrival)>2 && (departure>=(lunch_time+lunch_time_length))){
		v1=ran1(&idum);j=0;
		while(!( (v1>=occ_cummulated[j][7]) && (v1<occ_cummulated[j+1][7])) && j<300){
			if(j<287){j++;}else{j=300;}
		}
		if(j<300){
			afternoon_start=occ_cummulated[j+1][0];
			AfternoonBreak=1;
			v1=ran1(&idum);j=0;
			while(!( (v1>=occ_cummulated[j][8]) && (v1<occ_cummulated[j+1][8])) && j<300){
				if(j<287){j++;}else{j=300;}
			}
			if(j<300){afternoon_length=occ_cummulated[j+1][0];}else{afternoon_length=0;}
		}else{AfternoonBreak=0;afternoon_start=0;afternoon_length=0;}
		if(lunch_start+lunch_length>afternoon_start){afternoon_start=lunch_start+lunch_length;}
		if(afternoon_start>departure){AfternoonBreak=0;afternoon_start=0;afternoon_length=0;}
		if(afternoon_start+afternoon_length>departure){afternoon_length=departure-afternoon_start;}
	}



}


// This function is called by "occupancy_profile" and sets the occupancy for a given time step depending on
// predetermined arrival, departure, morning, lunch, and afternoon break
int get_occupancy(int weekday, int time_step, float hour)
{
int occupancy;
	if((hour+(time_step/120.0))>=arrival && hour-(time_step/120.0)<=departure && weekday<6 ){
		occupancy=1;
		if(hour-(time_step/120.0)<arrival){
			if(arrival-(hour-(time_step/120.0))> (time_step/120.0))
		           occupancy=0;
		}
		if( hour+(time_step/120.0)> departure){
			if( (hour+(time_step/120.0)- departure) > (time_step/120.0) ) {
		           	occupancy=0;
		        }
		}

		if( hour>=morning_start && hour-(time_step/120.0)<morning_start){
			occupancy=0;
		}
		if( hour>=lunch_start && hour-(time_step/120.0)<lunch_start){
			occupancy=0;
		}
		if( hour>=afternoon_start && hour-(time_step/120.0)<afternoon_start){
			occupancy=0;
		}


		if( hour > morning_start && hour < morning_start+morning_length){
			occupancy=0;
		}
		if( hour > lunch_start && hour < lunch_start+lunch_length){
			occupancy=0;
		}
		if( hour > afternoon_start && hour < afternoon_start+afternoon_length){
			occupancy=0;
		}

	}else{
		occupancy=0;
	}
	return(occupancy);
}


//*********************************************************************
// main function called to set the annual occupancy profile for a space
//*********************************************************************
// Input:
// occupancy_mode		;
// 		0: Lightswitch single offices
// 		1: static occupancy on weekdays from "start work" to "end work"
// 		2: read in an already existing occupancy routine file
// 		3: occupancy routine file from measured occupancy profile
//		4: ocupancy profiles for classrooms
//		5: read in an annaul ccupancy file generated by another program

// daylight_savings_time: switch for DST
// start_work			: beginning of work day
// end_work				: end of work day
// time_step			: time step of weahter file in minutes
// first_weekday		: weekday of the first day of the annual weather file
void occupancy_profile(int occupancy_mode, int daylight_savings_time, int start_work, int end_work, int time_step, int first_weekday)
{
	int k,i,InSession;
	int month=1, day=1;
	char header_line[1000]="";
	int OccCounter=0;
	float month_from_measured_occupancy=1;
	float day_from_measured_occupancy=1;
	int value_from_measured_occupancy=0;
	int weekday;
	int time_step_occ_file= 60;
	float hour=0;
	float hour_from_measured_occupancy=0;
	float DaylightSavingsTimeShift=0;
	float EndOfSchoolDay;
	float v1,v2,v3;
	int OccPeriod1,OccPeriod2,OccPeriod3;
	float ran1 ( long *idum );
	FILE *MEASURED_OCCUPANCY_FILE= NULL;
	int test_header=1;
	char  character='\0';
	int num_header_lines=0;
	char  line_string[1000000]="";


	// create occupancy routine file
	if(occupancy_mode==0)
    {
			gen_Lightswitch_routine();
	}
	// read in an already existing occupancy routine file
	if(occupancy_mode==2 )
	{
		read_occupancy_profile(routine_file);
	}
	// create occupancy routine file from measured occupancy profile
	if(occupancy_mode==3)
	{
       	   generate_routine_file(measured_occ,weekday_occ_mea,routine_file);
       	   read_occupancy_profile(routine_file);
	}

	if(occupancy_mode==5)
	{
       	MEASURED_OCCUPANCY_FILE=open_input(measured_occ);
		test_header=1;
		while(test_header){
			//determine length on header
			fgets(line_string,1000000,MEASURED_OCCUPANCY_FILE);
			sscanf(line_string,"%c ",&character);
			if( character == '#' ) {
				num_header_lines++;
				//check if the first keyword is 'time_step'
				sscanf(line_string,"%c %s",&character, header_line);
				if( !strcmp(header_line,"time_step")){
					sscanf(line_string,"%c %s %d",&character, header_line,&time_step_occ_file);
					if(time_step_occ_file!=60 && time_step_occ_file!=30 && time_step_occ_file!=20 && time_step_occ_file!=15 && time_step_occ_file!=12 && time_step_occ_file!=10 && time_step_occ_file!=6 && time_step_occ_file!=5 && time_step_occ_file!=4 && time_step_occ_file!=3 && time_step_occ_file!=2 && time_step_occ_file!=1){
						fprintf(stderr,"FATAL ERROR: Occupancy file (%s): variable \'time_step\' out of bound (%d)\n",measured_occ,time_step_occ_file);
						exit(1);
				    }
				}
			}else{
				test_header=0;
			}
		}
		close_file(MEASURED_OCCUPANCY_FILE);
       	MEASURED_OCCUPANCY_FILE=open_input(measured_occ);
		for (i=0;i<num_header_lines;i++)
		{
			fscanf( MEASURED_OCCUPANCY_FILE, "%*[^\n]\n" );
		}
	}

	weekday=first_weekday;
	month=1;
	day=1;
	hour=time_step*1.0/120;
	OccCounter=0;
	while( !(month==12 && day==31 && hour>=24.0)){

			// test whether it is daylight savings time or not
			if(daylight_savings_time && (month>=4 || month<=10)){
				DaylightSavingsTimeShift=-1.0;
			} else{
				DaylightSavingsTimeShift=0.0;
			}

			//static occupancy on weekdays from "start work" to "end work"
			//============================================================
			if(occupancy_mode==1){
				if(hour>(start_work+DaylightSavingsTimeShift) && hour<=(end_work+DaylightSavingsTimeShift))
				{
		 			if(weekday<6)
		 				  occ_profile[OccCounter]=1;
				}
				OccCounter++;
				hour+=time_step*1.0/60;
			}

			// stochastic occupancy for a weekday with up to three breaks
			//===========================================================
		    if(occupancy_mode==0 || occupancy_mode==2 || occupancy_mode==3)
		    {
		        get_day_profile(); // get arrival, departure and break times for a weekday
		        //account for daylight savings time
		           	arrival+=DaylightSavingsTimeShift;
		        	departure+=DaylightSavingsTimeShift;
		        	morning_start+=DaylightSavingsTimeShift;
		        	lunch_start+=DaylightSavingsTimeShift;
		        	afternoon_start+=DaylightSavingsTimeShift;

		        //assign daily occupancy
		        for (k=0 ; k<24*(int)(60.0/time_step) ; k++)
		        {
			  		occ_profile[OccCounter]=get_occupancy(weekday,time_step,hour );
					hour+=time_step*1.0/60;
					OccCounter++;
				}
			}

			//====================================================
		    //static occupancy profiles for Lightswitch classrooms
			//====================================================
		    if(occupancy_mode==4)
		    {
				//use month and day to determin whether classes are "in session" or not
				// session last from: 	week  2 to week 13
				//                   	week 15 to week 23
				//						week 36 to week 51
				InSession=0;
				k=SetCalendarWeek(month, day,first_weekday);
				if((k>=2 && k<=13) ||(k>=15 && k<=23)||(k>=36 && k<=51))
					InSession=1;

				//prepare stocastic processes for the day
				OccPeriod1=0;v1=ran1(&idum);if(v1>0.5)OccPeriod1=1;
				OccPeriod2=0;v2=ran1(&idum);if(v2>0.5)OccPeriod2=1;
				OccPeriod3=0;v3=ran1(&idum);if(v3>0.5)OccPeriod3=1;

				//printf("%s month %d day %d CW %d weekday %d Insession %d: %d %d %d\n",static_occupancy_profile,month, day, SetCalendarWeek(month, day,first_weekday),weekday, InSession,OccPeriod1,OccPeriod2,OccPeriod3 );

				// Four static occupancy schedules for classrooms from the U.S. EPA progrma EFAST
				//Primary School
				//==============
				if( !strcmp(static_occupancy_profile,"classroom_elementary") || !strcmp(static_occupancy_profile,"ClassroomPrimaryNoAfterSchoolActivities") || !strcmp(static_occupancy_profile,"ClassroomPrimaryWithAfterSchoolActivities"))
				{
					EndOfSchoolDay=17.0;
					if( !strcmp(static_occupancy_profile,"ClassroomPrimaryWithAfterSchoolActivities"))
						EndOfSchoolDay=20.0;
					for (k=0 ; k<24*(int)(60.0/time_step) ; k++)
				  	{
						occ_profile[OccCounter]=0;
						if(weekday<6)
						{
							if(InSession && weekday<6)
							{
								if(hour>=7.0  && hour<8.0 )occ_profile[OccCounter]=OccPeriod1;
								if(hour>=8.0  && hour<11.0)occ_profile[OccCounter]=1;
								if(hour>=11.0 && hour<14.0)occ_profile[OccCounter]=OccPeriod2;
								if(hour>=14.0 && hour<15.0)occ_profile[OccCounter]=1;
								if(hour>=15.0 && hour<EndOfSchoolDay)occ_profile[OccCounter]=OccPeriod3;
							}else{
								if( weekday<6 && !strcmp(static_occupancy_profile,"ClassroomPrimaryWithAfterSchoolActivities"))
								{
									if(hour>=8.0  && hour<11.0)occ_profile[OccCounter]=OccPeriod1;
									if(hour>=13.0 && hour<16.0)occ_profile[OccCounter]=OccPeriod2;
								}
							}
						}
						hour+=time_step*1.0/60;
						OccCounter++;
					}
				}

				//Secondary School
				//================
				if( !strcmp(static_occupancy_profile,"classroom_university") || !strcmp(static_occupancy_profile,"classroom_highschool") ||!strcmp(static_occupancy_profile,"ClassroomSecondaryNoAfterSchoolActivities") || !strcmp(static_occupancy_profile,"ClassroomSecondaryWithAfterSchoolActivities"))
				{
					EndOfSchoolDay=17.0;
					if( !strcmp(static_occupancy_profile,"ClassroomSecondaryWithAfterSchoolActivities"))
						EndOfSchoolDay=20.0;
					for (k=0 ; k<24*(int)(60.0/time_step) ; k++)
				  	{
						occ_profile[OccCounter]=0;
						if(weekday<6)
						{
							if(InSession && weekday<6)
							{
								if(hour>=6.0  && hour<7.0 )occ_profile[OccCounter]=OccPeriod1;
								if(hour>=7.0  && hour<10.0)occ_profile[OccCounter]=1;
								if(hour>=10.0 && hour<13.0)occ_profile[OccCounter]=OccPeriod2;
								if(hour>=13.0 && hour<14.0)occ_profile[OccCounter]=1;
								if(hour>=14.0 && hour<EndOfSchoolDay)occ_profile[OccCounter]=OccPeriod3;
							}else{
								if( weekday<6 && !strcmp(static_occupancy_profile,"ClassroomSecondaryWithAfterSchoolActivities"))
								{
									if(hour>=8.9  && hour<11.0)occ_profile[OccCounter]=OccPeriod1;
									if(hour>=13.9 && hour<16.0)occ_profile[OccCounter]=OccPeriod2;
								}
							}
						}
						hour+=time_step*1.0/60;
						OccCounter++;
					}
				}
			}//end occupancy_mode ==4



			//=======================================================
		    //read in externally generated occupancy profile (hourly)
			//=======================================================
		    if(occupancy_mode==5)
		    {
	      	   	fscanf(MEASURED_OCCUPANCY_FILE,"%f,%f,%f,%d",&month_from_measured_occupancy,&day_from_measured_occupancy, &hour_from_measured_occupancy,&value_from_measured_occupancy);

				//time_step_occ_file
				if(time_step<=60 && time_step_occ_file ==60)
				{
					for (k=0 ; k<(int)(60.0/time_step) ; k++)
					{
						occ_profile[OccCounter]=value_from_measured_occupancy;
						//printf("%f,%f,%f,%d\n",month_from_measured_occupancy,day_from_measured_occupancy, hour_from_measured_occupancy,occ_profile[OccCounter]);
						hour+=time_step*1.0/60;
						OccCounter++;


					}
				} else if (time_step!= time_step_occ_file && time_step_occ_file != 60)
				{
						fprintf(stderr,"FATAL ERROR: Occupancy file (%s): variable \'time_step\' should be 60 minutes an not %d minutes.\n",measured_occ,time_step_occ_file);
						exit(1);
				}

			}//end occupancy_mode ==5


		if(hour>=24.0 && (month!=12 || day!=31)){
			hour= time_step*1.0/120;
			day++;
			if(weekday==7){weekday=1;}else{weekday++;}
			if(month==1 && day>31){month++;day=1;}
			if(month==2 && day>28){month++;day=1;}
			if(month==3 && day>31){month++;day=1;}
			if(month==4 && day>30){month++;day=1;}
			if(month==5 && day>31){month++;day=1;}
			if(month==6 && day>30){month++;day=1;}
			if(month==7 && day>31){month++;day=1;}
			if(month==8 && day>31){month++;day=1;}
			if(month==9 && day>30){month++;day=1;}
			if(month==10 && day>31){month++;day=1;}
			if(month==11 && day>30){month++;day=1;}
			if(month==12 && day>31){month=1;day=1;}
			/*printf(" %d %d %f\n", month,day,hour);*/
		}
	}

	if(occupancy_mode==5)
		close_file(MEASURED_OCCUPANCY_FILE);
}



void nrerror ( char *error_text )
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

float ran1 ( long *idum )
{
	static long ix1,ix2,ix3;
	static float r[98];
	float temp;
	static int iff=0;
	int j;

	if (*idum < 0 || iff == 0) {
		iff=1;
		ix1=(IC1-(*idum)) % M1;
		ix1=(IA1*ix1+IC1) % M1;
		ix2=ix1 % M2;
		ix1=(IA1*ix1+IC1) % M1;
		ix3=ix1 % M3;
		for (j=1;j<=97;j++) {
			ix1=(IA1*ix1+IC1) % M1;
			ix2=(IA2*ix2+IC2) % M2;
			r[j]=(ix1+ix2*RM2)*RM1;
		}
		*idum=1;
	}
	ix1=(IA1*ix1+IC1) % M1;
	ix2=(IA2*ix2+IC2) % M2;
	ix3=(IA3*ix3+IC3) % M3;
	j=1 + ((97*ix3)/M3);
	if (j > 97 || j < 1) nrerror("RAN1: This cannot happen.");
	temp=r[j];
	r[j]=(ix1+ix2*RM2)*RM1;
	return temp;
}



