/* function file of the Lightswitch-2002 manual lighting control algorithm
 *  Copyright (c) 2003
 *  written by Christoph Reinhart
 *  National Research Council Canada
 *  Institute for Research in Construction
 *
 *	Update by Nathaniel Jones at MIT, March 2016
 */

// The Lightswitch Algorithm is described in
// Reinhart,

#include "numerical.h"
#include "ds_el_lighting.h"

//======================================
// Hunt's Switch On Probability function
//======================================
	double	a_hunt=-0.0175;
	double	b_hunt=-4.0835;
	double	c_hunt=1.0361;
	double	m_hunt=1.8223;
	double switch_prob(float ill, float a,float b, float c, float m)
	{
		double prob=0;
		/* Hunt's function for arbitrary user profiles */
		if (c!=0)
		{
			prob=a+c/(1+exp(-b*(log10(ill)-m)));
		}else{
			prob=a;
		}
		if(prob<0){prob=0;}
		if(prob>1){prob=1;}
		return(prob);
	}

//============================================
// Intermediate Switch On Probability function
//============================================
	double switch_on_prob_intermediate_prob(float ill)
	{
		double prob=0;
		double a,b,c,m;
		/* Hunt's function with Lamparter Data *
		 * Reinhart C.F., Voss K.,Monitoring Manual Control of Electric
	       Lighting and Blinds, Lighting Research & Technology, 35:3 pp. 243-260, 2003.
	       .
		*/
		a=0.0027;
		b=-64.19;
		c=0.017;
		m=2.41;
		prob=a+c/(1+exp(-b*(log10(ill)-m)));
		if(prob<0){prob=0;}
		if(prob>1){prob=1;}
		if(ill==0){prob=1;}
		return(prob);
	}

//================================
// Switch Off Probability function
//================================
	double switch_off(float time_of_absence, int EffLightSystem)
	{
		float switch_off_prob[6][8];
		int i;
		/* data from:
	   (1) Pigg S, Eilers M, and Reed J.  Behavioral Aspects of
	       Lighting and Occupancy Sensors in Privates Offices:
	       A case study of a University Office Building: Proceedings
	       of the 1996 ACEEE Summer Study on Energy Efficiency in
	       Buildings 1996;8: 8.161-8.171.
	   (2) Reinhart C.F., Voss K.,Monitoring Manual Control of Electric
	       Lighting and Blinds, Lighting Research & Technology, 35:3 pp. 243-260, 2003.
		*/

		/* time intervals */
		switch_off_prob[0][0]=0.0;
		switch_off_prob[0][1]=0.5;
		switch_off_prob[0][2]=1.0;
		switch_off_prob[0][3]=2.0;
		switch_off_prob[0][4]=4.0;
		switch_off_prob[0][5]=12.0;
		switch_off_prob[0][6]=24.0;
		switch_off_prob[0][7]=8760.0;

		/*  Manual on/off switch near the door (no lighting control) */
		switch_off_prob[1][0]=0.0;
		switch_off_prob[1][1]=0.086;
		switch_off_prob[1][2]=0.314;
		switch_off_prob[1][3]=0.386;
		switch_off_prob[1][4]=0.6;
		switch_off_prob[1][5]=0.96;
		switch_off_prob[1][6]=0.999;
		switch_off_prob[1][7]=0.999;

		/*  Switch off occupancy sensor */
		switch_off_prob[2][0]=0.0;
		switch_off_prob[2][1]=0.092;
		switch_off_prob[2][2]=0.128;
		switch_off_prob[2][3]=0.1857;
		switch_off_prob[2][4]=0.3;
		switch_off_prob[2][5]=0.5142;
		switch_off_prob[2][6]=0.572;
		switch_off_prob[2][7]=0.593;

		/*  Photosensor controlled indirect dimming system (presently not in use) */
		switch_off_prob[4][0]=0.0;
		switch_off_prob[4][1]=0.065;
		switch_off_prob[4][2]=0.099;
		switch_off_prob[4][3]=0.165;
		switch_off_prob[4][4]=0.497;
		switch_off_prob[4][5]=0.676;
		switch_off_prob[4][6]=0.724;
		switch_off_prob[4][7]=0.724;

	    for(i=1 ; i<=7 ; i++ ) // assign value
		{
			if(time_of_absence>=switch_off_prob[0][i-1] && time_of_absence < switch_off_prob[0][i])
		    return(switch_off_prob[EffLightSystem][i]);
		}
		return 0;
}

	int Pigg_switch_off(int current_time, int LightSystem, int UserLight,int dep)
	{ 	int k=0;
		int EffLightSystem=0;
		double random;
		// function determines whether the lighting is manually switched off
		// during departure depeding on:
		//      - user type,
		//      - length of absence, and
		//      - type of lighting system.

	// get length of absence
	while(occ_profile[current_time+k]==0 && (current_time+k) < 8760*(int)(60/time_step))
		k++;

	//=============================================================================
	//depending on the type of lighting system a switch off probability is assigned
	//=============================================================================
	if( LightSystem ==1 ) // Manual on/off switch near the door (no lighting control)
		EffLightSystem=1;
	if( LightSystem ==2 ) // Switch off occupancy sensor
		EffLightSystem=2;
	if( LightSystem ==3 ) // On/Off occupancy sensor
		;					//This system has no manual control
	if( LightSystem ==4 || LightSystem ==40) // Photosensor controlled dimming system
		EffLightSystem=1; // is treated like direct lighting system
	if( LightSystem ==5 ||  LightSystem ==50) // Combination switch-off occupancy & dimming system
		EffLightSystem=2; // is treated like direct lighting system
	if( LightSystem ==6 || LightSystem ==60) // Combination on/off occupancy & dimming system
		;//This system has no manual control

	//if( (LightSystem ==4 ) && min_work_plane_ill[shading_profile[i+j]][i+j] <=(IllThres*0.0));
	//if( (LightSystem ==5 ) && min_work_plane_ill[shading_profile[i+j]][i+j] <=(IllThres*0.75));
	random=ran1(&idum);
	if(UserLight ==1) // active lighting control (in dependance of ambient daylight)
	{
		if( random <= switch_off(k*(time_step/60.0), EffLightSystem ) ){
			return(0);
		}else{
			return(1);
		}
	}
	if(UserLight ==2) //passive lighting control (always on during working day)
	{
		if((current_time-1) == dep )
		{
			if( random <= switch_off(k*(time_step/60.0), EffLightSystem ) ){
				return(0);
			}else{
				return(1);
			}
		}else{
			return(1);
		}
	}
	return 0;
}


/*===========================================================================*/
/*================ Lightswitch 2002 algorithm ===============================*/
/*===========================================================================*/
/* This function determines the status of the blinds and eletric lighting for
   each 5 minute time sep of a year. Funtion inputs are:
	(1) UserLight 		1  user considers daylight (active use of lighting)
						2  user does not consider daylight (passive use of lighting)
	(2)	UserBlind 		1  active user (avoid direct sunlight)
						2  user leaves blinds in fixed position (passive use of blinds)
						3  active user (avoid discomfort glare)
	(3)	LightSystem = 	1  manual on/off switch by the door; no lighting controls
						2  manual on/off switch by the door; energy effiicient occ. sensor
						3  manual on/off switch by the door; on/off occ. sensor
						4  manual on/off; ideal photo-control dimmed lighting(direct)
						40 manual on/off; photo-control dimmed lighting(direct); photosensor specified, muti-zone
						5  manual on/off ; ideal photo-control dimmed lighting(direct);eff_occ sensor
						50 manual on/off ; phot-ocontrol dimmed lighting(direct);eff_occ sensor; photosensor specified, muti-zone
						6  manual on/off ; ideal photo-control dimmed lighting(direct);on/off sensor
						60 manual on/off ; photo-control dimmed lighting(direct);on/off sensor; photosensor specified, muti-zone
	(4)	DelayTime		   occupancy sensor [min]
	(5)	BlindSystem		0  no or static blinds
						1  blinds are manually operated
						2  blinds are automaticall operated
						3  automated blinds, constant setpoints for each shading device setting for up and down
						4  automated blinds, individual setpoints for each shading device setting for up and down
	(6)	StaticBlind		   blinds a fixed in this postion postion in case of static manual blind usage
	(7)	IllThres		   minimum illuminance threshold
    (8) l_system		   index of lighting system investigated

   Function outputs are:
	(1)	shading_profile[i]: 0,1...n status of the shading device
	(2)	electric_lighting_profile[i]: 0/1			lighting status on/off
*/
void lightswitch_function( int UserLight)
{
int i,j,k,intermediate_counter=0;
int LightingGroupIndex;
int arr=0,dep=0,current_weekday=0;
int blind_closed=0;
double random;

current_weekday=first_weekday-1;

for (LightingGroupIndex=1 ; LightingGroupIndex<= NumberOfLightingGroups ; LightingGroupIndex++)
{
	
	//===============
	// loop over year
	//===============
	for (i=1 ; i<8760*(int)(60/time_step) ; i+=24*(int)(60/time_step))
	{

		/*============================================*/
		/*get arrival and departure times for the day */
		/*============================================*/
		arr=0;
		dep=0;
		for (j=0 ; j<24*(int)(60/time_step) ; j++)
		{
			if(occ_profile[i+j]==1 && arr==0) 
				arr=i+j;
			if(occ_profile[i+j-1]==1 && occ_profile[i+j]==0 && arr> 0 ) 
				dep=i+j-1;
		}
		current_weekday++;
		if(current_weekday==8)
			current_weekday=1;

		/*=========================*/
		/* process data day by day */
		/*=========================*/
		for (j=0 ; j<24*(int)(60/time_step) ; j++)
		{ /* day loop */

		if(electric_lighting_profile[LightingGroupIndex][i+j-1]==1)
			electric_lighting_profile[LightingGroupIndex][i+j]=1;
		else
			electric_lighting_profile[LightingGroupIndex][i+j]=0;
			
			
		/*==================*/
		/* Lighting Control */
		/*==================*/
		switch ( LightingSystemType[LightingGroupIndex]) {
			case 1: case 2: case 4: case 40: case 5: case 50: { // manual on/off wall switch
			if(occ_profile[i+j-1]==1)
			{ /* Is the work place already occupied? YES */
				if(electric_lighting_profile[LightingGroupIndex][i+j-1]==1){ /* Is the light already switched on? YES */
					if(occ_profile[i+j]==0)
					{ /* Does the occupant leave? YES */
						if(UserLight==1 || UserLight==2){
							//Pigg's switch off probability
							electric_lighting_profile[LightingGroupIndex][i+j]=Pigg_switch_off(i+j, LightingSystemType[LightingGroupIndex],UserLight,dep);
						}
		   	      		if(LightingSystemType[LightingGroupIndex] ==2 || LightingSystemType[LightingGroupIndex] ==5 || LightingSystemType[LightingGroupIndex] ==50 )
						{ // energy-efficient occupancy sensor
		   			    	k=0;
		     			    while(occ_profile[i+j-k]==0 && (i+j-k) >=0)
		     			    	k++;
							//printf ("LG %d k test %d time %d\n",LightingGroupIndex,k,j);
							//keep the electric ighting on for the duration of the occupancy sensor delay time
		   			    	if(k*time_step > OccSenDelayTime[LightingGroupIndex])
		   						electric_lighting_profile[LightingGroupIndex][i+j]=OccSenDelayTime[LightingGroupIndex]/60.0;
		   				}
					}else{ /* Does the occupant leave? NO */;}
				}else{ // Is the light already switched on? NO
					if(occ_profile[i+j]==0){ /* Does the occupant leave? YES */;
					}else{ /* Does the occupant leave? NO */
						if(UserLight==1){
							if(blind_closed){
								// Hunt's switch on probability after a closing of the blinds
								random=ran1(&idum);
								if(random <= switch_prob(minimum_work_plane_ill[LightingGroupIndex][i+j], a_hunt,b_hunt, c_hunt, m_hunt))
									electric_lighting_profile[LightingGroupIndex][i+j]=1;
							}else{
								// intermediate switch on
								if(time_step==60) // if the time step of the simulation is 60 minutes, the intermediate switch on is activated 12 times
								{
									for(intermediate_counter=0 ; intermediate_counter<12 ; intermediate_counter++ )
									{
										if(ran1(&idum) <= switch_on_prob_intermediate_prob(minimum_work_plane_ill[LightingGroupIndex][i+j]) )
										{
											electric_lighting_profile[LightingGroupIndex][i+j]=(12-intermediate_counter)/12.0;
											intermediate_counter=100;
										}	
									}
								}else{
									if(ran1(&idum) <= switch_on_prob_intermediate_prob(minimum_work_plane_ill[LightingGroupIndex][i+j]) )
										electric_lighting_profile[LightingGroupIndex][i+j]=1;
								}
							}
						}// end UserLight==1
					}
				} //end Is the light already switched on? NO
		   }else{ /* Is the work place already occupied? NO */
		   	if(electric_lighting_profile[LightingGroupIndex][i+j]==1){ /* Is the light already switched on? YES */
		   	      if(occ_profile[i+j]==1){ /* Does the occupant arrive? YES */;
		          }else{ /* Does the occupant arrive? NO */
		   	      		if(LightingSystemType[LightingGroupIndex] ==2 || LightingSystemType[LightingGroupIndex] ==5 || LightingSystemType[LightingGroupIndex] ==50 )
						{ // energy-efficient occupancy sensor
		   			    	k=0;
		     			    while(occ_profile[i+j-k]==0 && (i+j-k) >=0)
		     			    	k++;
		   			    	if(k*time_step > OccSenDelayTime[LightingGroupIndex])
		   						electric_lighting_profile[LightingGroupIndex][i+j]=0;
		   				}
		          }
		   	}else{ /* Is the light already switched on? NO */
		   		if(occ_profile[i+j]==1){ /* Does the occupant arrive? YES */
		   	    	if(UserLight==1){
		   	        	/* Hunt's switch on probability */
		   	            random=ran1(&idum);
						if(random <= switch_prob(minimum_work_plane_ill[LightingGroupIndex][i+j], a_hunt,b_hunt, c_hunt, m_hunt))
		   	            	electric_lighting_profile[LightingGroupIndex][i+j]=1;
		   		    }
		           	if(UserLight==2){
		           	   	electric_lighting_profile[LightingGroupIndex][i+j]=1;
		   	      	}
		        }else{ /* Does the occupant arrive? NO */;
		        }
		   	}
		   } // end Is the work place already occupied? NO
	       break;
		}
		case 3: case 6: case 60: { // on/off occupancy sensor
			if(occ_profile[i+j]==1){ //lighting is always on during occupancy
			    electric_lighting_profile[LightingGroupIndex][i+j]=1;
			}else{ //lighting is switched off at delay time
				if(electric_lighting_profile[LightingGroupIndex][i+j-1]==1)
				{
					k=0;
					while(occ_profile[i+j-k]==0 && (i+j-k) >=0)
						k++;
					if(k*time_step > OccSenDelayTime[LightingGroupIndex])
						electric_lighting_profile[LightingGroupIndex][i+j]=OccSenDelayTime[LightingGroupIndex]/60.0;
				}
			}
			break;
		}
		case 10: case 11: { // time clock
			if(current_weekday<6 && ((j+1)*(time_step/60.0) >=schedule_start[LightingGroupIndex]) && (j+1)*(time_step/60.0)<schedule_end[LightingGroupIndex]){
				electric_lighting_profile[LightingGroupIndex][i+j]=1;
			}else{
				electric_lighting_profile[LightingGroupIndex][i+j]=0;
			}
			break;
		}
		} //end of switch ( LightSystem)
			blind_closed=0;
			
		} //end day loop
	} // end loop over year
} //end loop LightingGroupIndex

}

